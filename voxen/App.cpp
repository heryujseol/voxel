#include "App.h"
#include "Graphics.h"
#include "DXUtils.h"

#include <iostream>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>


App* g_app = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return g_app->EventHandler(hwnd, uMsg, wParam, lParam);
}

App::App()
	: m_hwnd(), m_constantBuffer(nullptr), m_constantData(), m_camera(), m_skybox(), m_cloud(),
	  m_light(), m_postEffect(), m_dateTime(0), m_mouseNdcX(0.0f), m_mouseNdcY(0.0f),
	  m_keyPressed{
		  false,
	  },
	  m_keyToggle{
		  false,
	  }
{
	g_app = this;
}

App::~App()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	DestroyWindow(m_hwnd);
}

LRESULT App::EventHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
		return true;

	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_KEYDOWN:
		if (UINT(wParam) == VK_ESCAPE) {
			DestroyWindow(hwnd);
			return 0;
		}
		m_keyPressed[UINT(wParam)] = true;
		m_keyToggle[UINT(wParam)] = !m_keyToggle[UINT(wParam)];
		break;

	case WM_KEYUP:
		m_keyPressed[UINT(wParam)] = false;
		break;

	case WM_MOUSEMOVE:
		m_mouseNdcX = (float)LOWORD(lParam) / (float)WIDTH * 2 - 1;
		m_mouseNdcY = -((float)HIWORD(lParam) / (float)HEIGHT * 2 - 1);

		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool App::Initialize()
{
	if (!InitWindow())
		return false;

	if (!InitDirectX())
		return false;

	if (!InitGUI())
		return false;

	if (!InitScene())
		return false;

	return true;
}

void App::Run()
{
	MSG msg = { 0 };
	while (WM_QUIT != msg.message) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			ImGui_ImplDX11_NewFrame(); // GUI 프레임 시작
			ImGui_ImplWin32_NewFrame();

			ImGui::NewFrame(); // 어떤 것들을 렌더링 할지 기록 시작
			ImGui::Begin("Scene Control");
			ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
				ImGui::GetIO().Framerate);

			ImGui::Text("x : %.2f y : %.2f z : %.2f", m_camera.GetPosition().x,
				m_camera.GetPosition().y, m_camera.GetPosition().z);

			ImGui::End();
			ImGui::Render(); // 렌더링할 것들 기록 끝

			Update(ImGui::GetIO().DeltaTime);
			Render(); // 우리가 구현한 렌더링

			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // GUI 렌더링

			Graphics::swapChain->Present(1, 0);
		}
	}
}

void App::Update(float dt)
{
	static float acc = 0.0f;

	if (!m_keyToggle['T']) {
		m_camera.Update(dt, m_keyPressed, m_mouseNdcX, m_mouseNdcY);
	}

	m_postEffect.Update(dt, m_camera.IsUnderWater());
	ChunkManager::GetInstance()->Update(dt, m_camera);

	if (m_keyToggle['F']) {
		acc += DAY_CYCLE_TIME_SPEED * dt;
		m_dateTime = (uint32_t)acc % DAY_CYCLE_AMOUNT;

		m_skybox.Update(m_dateTime);
		m_light.Update(m_dateTime);
		m_cloud.Update(dt, m_camera.GetPosition());
	}
	else {
		m_skybox.Update(m_dateTime);
		m_light.Update(m_dateTime);
		m_cloud.Update(0.0f, m_camera.GetPosition());
	}
}

void App::Render()
{
	DXUtils::UpdateViewport(Graphics::basicViewport, 0, 0, WIDTH, HEIGHT);
	Graphics::context->RSSetViewports(1, &Graphics::basicViewport);

	std::vector<ID3D11Buffer*> ppConstantBuffers;
	ppConstantBuffers.push_back(m_camera.m_constantBuffer.Get());
	ppConstantBuffers.push_back(m_skybox.m_constantBuffer.Get());
	ppConstantBuffers.push_back(m_light.m_lightConstantBuffer.Get());
	ppConstantBuffers.push_back(m_constantBuffer.Get());

	Graphics::context->VSSetConstantBuffers(
		7, (UINT)ppConstantBuffers.size(), ppConstantBuffers.data());
	Graphics::context->PSSetConstantBuffers(
		7, (UINT)ppConstantBuffers.size(), ppConstantBuffers.data());
	
	// 1. Deferred Render Pass
	{
		FillGBuffer();
		MaskMSAAEdge();
		RenderSSAO();
		ShadingBasic();
	}

	// 2. No-MSAA to MSAA Texture
	{
		ConvertToMSAA();
	}

	// 3. Forward Render Pass MSAA
	{
		if (m_camera.IsUnderWater()) {
			RenderFogFilter();
			RenderSkybox();
			RenderCloud();
			RenderWaterPlane();
		}
		else {
			RenderMirrorWorld();
			RenderWaterPlane();
			RenderFogFilter();
			RenderSkybox();
			RenderCloud();
		}
	}

	// 4. Post Effect
	{
		Graphics::context->ResolveSubresource(Graphics::basicBuffer.Get(), 0,
			Graphics::basicMSBuffer.Get(), 0, DXGI_FORMAT_R16G16B16A16_FLOAT);

		if (m_camera.IsUnderWater()) {
			RenderWaterFilter();
		}
		else {
			Graphics::context->CopyResource(
				Graphics::bloomBuffer[0].Get(), Graphics::basicBuffer.Get());
		}

		m_postEffect.Bloom();
	}
}

bool App::InitWindow()
{
	const wchar_t CLASS_NAME[] = L"Voxen Class";
	HINSTANCE hInstance = GetModuleHandle(0);

	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WindowProc, 0L, 0L, GetModuleHandle(NULL),
		NULL, NULL, NULL, NULL,
		CLASS_NAME, // lpszClassName, L-string
		NULL };

	if (!RegisterClassEx(&wc))
		return false;


	RECT wr = { 0, 0, (LONG)WIDTH, (LONG)HEIGHT };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, false);

	m_hwnd = CreateWindow(wc.lpszClassName, L"Voxen", WS_OVERLAPPEDWINDOW, 50, 50,
		wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, hInstance, NULL);

	if (m_hwnd == NULL)
		return false;

	ShowWindow(m_hwnd, SW_SHOWDEFAULT);
	UpdateWindow(m_hwnd);

	return true;
}

bool App::InitDirectX()
{
	DXGI_FORMAT pixelFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	if (!Graphics::InitGraphicsCore(pixelFormat, m_hwnd)) {
		return false;
	}

	if (!Graphics::InitGraphicsBuffer()) {
		return false;
	}

	if (!Graphics::InitGraphicsState()) {
		return false;
	}

	Graphics::InitGraphicsPSO();

	return true;
}

bool App::InitGUI()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.DisplaySize = ImVec2(float(WIDTH), float(HEIGHT));
	ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	if (!ImGui_ImplDX11_Init(Graphics::device.Get(), Graphics::context.Get())) {
		return false;
	}

	if (!ImGui_ImplWin32_Init(m_hwnd)) {
		return false;
	}

	return true;
}

bool App::InitScene()
{
	if (!m_camera.Initialize(Vector3(0.0f, 108.0f, 0.0f)))
		return false;

	if (!ChunkManager::GetInstance()->Initialize(m_camera.GetChunkPosition()))
		return false;

	if (!m_skybox.Initialize(550.0f))
		return false;

	if (!m_cloud.Initialize(m_camera.GetPosition()))
		return false;

	if (!m_light.Initialize())
		return false;

	if (!m_postEffect.Initialize())
		return false;

	m_constantData.appWidth = WIDTH;
	m_constantData.appHeight = HEIGHT;
	m_constantData.mirrorWidth = MIRROR_WIDTH;
	m_constantData.mirrorHeight = MIRROR_HEIGHT;
	if (!DXUtils::CreateConstantBuffer(m_constantBuffer, m_constantData)) {
		std::cout << "failed create constant buffer in app" << std::endl;
		return false;
	}

	return true;
}

void App::FillGBuffer()
{
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, -1.0f };
	Graphics::context->ClearRenderTargetView(Graphics::normalEdgeRTV.Get(), clearColor);
	Graphics::context->ClearRenderTargetView(Graphics::positionRTV.Get(), clearColor);
	Graphics::context->ClearRenderTargetView(Graphics::albedoRTV.Get(), clearColor);
	Graphics::context->ClearRenderTargetView(Graphics::coverageRTV.Get(), clearColor);
	Graphics::context->ClearDepthStencilView(
		Graphics::basicDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	std::vector<ID3D11RenderTargetView*> ppRTVs;
	ppRTVs.push_back(Graphics::normalEdgeRTV.Get());
	ppRTVs.push_back(Graphics::positionRTV.Get());
	ppRTVs.push_back(Graphics::albedoRTV.Get());
	ppRTVs.push_back(Graphics::coverageRTV.Get());
	Graphics::context->OMSetRenderTargets(
		(UINT)ppRTVs.size(), ppRTVs.data(), Graphics::basicDSV.Get());

	std::vector<ID3D11ShaderResourceView*> ppSRVs;
	ppSRVs.push_back(Graphics::atlasMapSRV.Get());
	ppSRVs.push_back(Graphics::grassColorMapSRV.Get());
	Graphics::context->PSSetShaderResources(0, (UINT)ppSRVs.size(), ppSRVs.data());

	ChunkManager::GetInstance()->RenderBasic(m_camera.GetPosition());
}

void App::MaskMSAAEdge()
{
	Graphics::context->ClearDepthStencilView(
		Graphics::deferredDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	Graphics::context->OMSetRenderTargets(
		1, Graphics::basicRTV.GetAddressOf(), Graphics::deferredDSV.Get());

	std::vector<ID3D11ShaderResourceView*> ppSRVs;
	ppSRVs.push_back(Graphics::albedoSRV.Get());
	ppSRVs.push_back(Graphics::positionSRV.Get());
	Graphics::context->PSSetShaderResources(0, (UINT)ppSRVs.size(), ppSRVs.data());

	Graphics::SetPipelineStates(Graphics::edgeMaskingPSO);
	m_postEffect.Render();
}

void App::RenderSSAO()
{
	// SSAO
	{
		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		Graphics::context->ClearRenderTargetView(Graphics::ssaoRTV.Get(), clearColor);

		Graphics::context->OMSetRenderTargets(
			1, Graphics::ssaoRTV.GetAddressOf(), Graphics::deferredDSV.Get());

		std::vector<ID3D11Buffer*> ppConstants;
		ppConstants.push_back(m_postEffect.m_ssaoConstantBuffer.Get());
		ppConstants.push_back(m_postEffect.m_ssaoNoiseConstantBuffer.Get());
		Graphics::context->PSSetConstantBuffers(0, (UINT)ppConstants.size(), ppConstants.data());

		std::vector<ID3D11ShaderResourceView*> ppSRVs;
		ppSRVs.push_back(Graphics::normalEdgeSRV.Get());
		ppSRVs.push_back(Graphics::positionSRV.Get());
		ppSRVs.push_back(Graphics::coverageSRV.Get());
		Graphics::context->PSSetShaderResources(0, (UINT)ppSRVs.size(), ppSRVs.data());

		Graphics::SetPipelineStates(Graphics::ssaoPSO);
		m_postEffect.Render();

		Graphics::SetPipelineStates(Graphics::ssaoEdgePSO);
		m_postEffect.Render();
	}

	// blur
	{
		Graphics::SetPipelineStates(Graphics::samplingPSO);
		m_postEffect.Blur(3, Graphics::ssaoSRV, Graphics::ssaoRTV, Graphics::ssaoBlurSRV,
			Graphics::ssaoBlurRTV, Graphics::blurSsaoPS);
	}
}

void App::ShadingBasic()
{
	Graphics::context->OMSetRenderTargets(
		1, Graphics::basicRTV.GetAddressOf(), Graphics::deferredDSV.Get());

	std::vector<ID3D11ShaderResourceView*> ppSRVs;
	ppSRVs.push_back(Graphics::normalEdgeSRV.Get());
	ppSRVs.push_back(Graphics::positionSRV.Get());
	ppSRVs.push_back(Graphics::albedoSRV.Get());
	ppSRVs.push_back(Graphics::ssaoSRV.Get());
	Graphics::context->PSSetShaderResources(0, (UINT)ppSRVs.size(), ppSRVs.data());

	Graphics::SetPipelineStates(Graphics::shadingBasicPSO);
	m_postEffect.Render();

	Graphics::SetPipelineStates(Graphics::shadingBasicEdgePSO);
	m_postEffect.Render();
}

void App::ConvertToMSAA()
{
	Graphics::context->OMSetRenderTargets(1, Graphics::basicMSRTV.GetAddressOf(), nullptr);

	Graphics::context->PSSetShaderResources(0, 1, Graphics::basicSRV.GetAddressOf());

	Graphics::SetPipelineStates(Graphics::samplingPSO);
	m_postEffect.Render();
}

void App::RenderSkybox()
{
	Graphics::context->OMSetRenderTargets(
		1, Graphics::basicMSRTV.GetAddressOf(), Graphics::basicDSV.Get());

	Graphics::SetPipelineStates(Graphics::skyboxPSO);
	m_skybox.Render();
}

void App::RenderCloud()
{
	Graphics::context->OMSetRenderTargets(
		1, Graphics::basicMSRTV.GetAddressOf(), Graphics::basicDSV.Get());

	Graphics::SetPipelineStates(Graphics::cloudPSO);
	m_cloud.Render();
}

void App::RenderFogFilter()
{
	Graphics::context->OMSetRenderTargets(1, Graphics::basicMSRTV.GetAddressOf(), nullptr);

	Graphics::context->CopyResource(
		Graphics::copyForwardRenderBuffer.Get(), Graphics::basicMSBuffer.Get());

	std::vector<ID3D11ShaderResourceView*> ppSRVs;
	ppSRVs.push_back(Graphics::copyForwardSRV.Get());
	ppSRVs.push_back(Graphics::basicDepthSRV.Get());
	Graphics::context->PSSetShaderResources(0, (UINT)ppSRVs.size(), ppSRVs.data());

	Graphics::context->PSSetConstantBuffers(
		0, 1, m_postEffect.m_fogFilterConstantBuffer.GetAddressOf());

	Graphics::SetPipelineStates(Graphics::fogFilterPSO);
	m_postEffect.Render();
}

void App::RenderWaterFilter()
{
	Graphics::context->OMSetRenderTargets(1, Graphics::bloomRTV[0].GetAddressOf(), nullptr);

	Graphics::context->PSSetShaderResources(0, 1, Graphics::basicSRV.GetAddressOf());

	Graphics::context->PSSetConstantBuffers(
		0, 1, m_postEffect.m_waterFilterConstantBuffer.GetAddressOf());

	Graphics::SetPipelineStates(Graphics::waterFilterPSO);
	m_postEffect.Render();
}

void App::RenderMirrorWorld()
{
	DXUtils::UpdateViewport(Graphics::mirrorWorldViewPort, 0, 0, MIRROR_WIDTH, MIRROR_HEIGHT);
	Graphics::context->RSSetViewports(1, &Graphics::mirrorWorldViewPort);

	const FLOAT clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	Graphics::context->ClearRenderTargetView(Graphics::mirrorDepthRTV.Get(), clearColor);
	Graphics::context->ClearRenderTargetView(Graphics::mirrorWorldRTV.Get(), clearColor);
	Graphics::context->ClearDepthStencilView(
		Graphics::mirrorWorldDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// stencil
	{
		Graphics::context->OMSetRenderTargets(
			1, Graphics::mirrorDepthRTV.GetAddressOf(), Graphics::mirrorWorldDSV.Get());
		Graphics::context->PSSetShaderResources(0, 1, Graphics::positionSRV.GetAddressOf());
		Graphics::SetPipelineStates(Graphics::mirrorMaskingPSO);
		ChunkManager::GetInstance()->RenderTransparency();
	}

	// mirror sky shader
	{
		Graphics::context->OMSetRenderTargets(
			1, Graphics::mirrorWorldRTV.GetAddressOf(), Graphics::mirrorWorldDSV.Get());
		Graphics::SetPipelineStates(Graphics::skyboxMirrorPSO);
		m_skybox.Render();
	}

	// mirror cloud
	{
		// mirror constant buffer: mirror plane view matrix
		Graphics::context->VSSetConstantBuffers(
			7, 1, m_camera.m_mirrorConstantBuffer.GetAddressOf());
		Graphics::SetPipelineStates(Graphics::cloudMirrorPSO);
		m_cloud.Render();
	}

	// mirror low lod world
	{
		std::vector<ID3D11ShaderResourceView*> ppSRVs;
		ppSRVs.push_back(Graphics::atlasMapSRV.Get());
		ppSRVs.push_back(Graphics::grassColorMapSRV.Get());
		ppSRVs.push_back(Graphics::mirrorDepthSRV.Get());
		Graphics::context->PSSetShaderResources(0, (UINT)ppSRVs.size(), ppSRVs.data());
		ChunkManager::GetInstance()->RenderMirrorWorld();
	}

	// blur mirror world
	{
		Graphics::SetPipelineStates(Graphics::samplingPSO);
		m_postEffect.Blur(3, Graphics::mirrorWorldSRV, Graphics::mirrorWorldRTV,
			Graphics::mirrorBlurSRV, Graphics::mirrorBlurRTV, Graphics::blurMirrorPS);
	}

	// 원래의 글로벌로 두기
	Graphics::context->VSSetConstantBuffers(7, 1, m_camera.m_constantBuffer.GetAddressOf());
	DXUtils::UpdateViewport(Graphics::basicViewport, 0, 0, WIDTH, HEIGHT);
	Graphics::context->RSSetViewports(1, &Graphics::basicViewport);
}

void App::RenderWaterPlane()
{
	Graphics::context->OMSetRenderTargets(
		1, Graphics::basicMSRTV.GetAddressOf(), Graphics::basicDSV.Get());

	Graphics::context->CopyResource(
		Graphics::copyForwardRenderBuffer.Get(), Graphics::basicMSBuffer.Get());

	std::vector<ID3D11ShaderResourceView*> ppSRVs;
	ppSRVs.push_back(Graphics::atlasMapSRV.Get());
	ppSRVs.push_back(Graphics::copyForwardSRV.Get());
	ppSRVs.push_back(Graphics::mirrorWorldSRV.Get());
	ppSRVs.push_back(Graphics::positionSRV.Get());
	Graphics::context->PSSetShaderResources(0, (UINT)ppSRVs.size(), ppSRVs.data());

	Graphics::SetPipelineStates(Graphics::waterPlanePSO);
	ChunkManager::GetInstance()->RenderTransparency();
}