#include "Graphics.h"
#include "DXUtils.h"
#include "App.h"

#include <iostream>

namespace Graphics {
	// Graphics Core
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	ComPtr<IDXGISwapChain> swapChain;


	// Input Layout
	ComPtr<ID3D11InputLayout> basicIL;
	ComPtr<ID3D11InputLayout> skyboxIL;
	ComPtr<ID3D11InputLayout> cloudIL;
	ComPtr<ID3D11InputLayout> samplingIL;
	ComPtr<ID3D11InputLayout> instanceIL;


	// Vertex Shader
	ComPtr<ID3D11VertexShader> basicVS;
	ComPtr<ID3D11VertexShader> skyboxVS;
	ComPtr<ID3D11VertexShader> cloudVS;
	ComPtr<ID3D11VertexShader> samplingVS;
	ComPtr<ID3D11VertexShader> instanceVS;
	ComPtr<ID3D11VertexShader> basicShadowVS;


	// Geometry Shader
	ComPtr<ID3D11GeometryShader> basicShadowGS;


	///  Pixel Shader
	ComPtr<ID3D11PixelShader> basicPS;
	ComPtr<ID3D11PixelShader> basicAlphaClipPS;
	ComPtr<ID3D11PixelShader> basicDepthClipPS;
	ComPtr<ID3D11PixelShader> basicAlphaDepthClipPS;
	ComPtr<ID3D11PixelShader> skyboxPS;
	ComPtr<ID3D11PixelShader> cloudPS;
	ComPtr<ID3D11PixelShader> samplingPS;
	ComPtr<ID3D11PixelShader> fogFilterPS;
	ComPtr<ID3D11PixelShader> mirrorMaskingPS;
	ComPtr<ID3D11PixelShader> waterPlanePS;
	ComPtr<ID3D11PixelShader> waterFilterPS;
	ComPtr<ID3D11PixelShader> blurMirrorPS[2];
	ComPtr<ID3D11PixelShader> blurSsaoPS[2];
	ComPtr<ID3D11PixelShader> ssaoNormalPS;
	ComPtr<ID3D11PixelShader> ssaoEdgePS;
	ComPtr<ID3D11PixelShader> edgeMaskingPS;


	// Rasterizer State
	ComPtr<ID3D11RasterizerState> solidRS;
	ComPtr<ID3D11RasterizerState> wireRS;
	ComPtr<ID3D11RasterizerState> noneCullRS;
	ComPtr<ID3D11RasterizerState> mirrorRS;


	// Sampler State
	ComPtr<ID3D11SamplerState> pointWrapSS;
	ComPtr<ID3D11SamplerState> linearWrapSS;
	ComPtr<ID3D11SamplerState> linearClampSS;
	ComPtr<ID3D11SamplerState> shadowPointSS;
	ComPtr<ID3D11SamplerState> shadowCompareSS;
	ComPtr<ID3D11SamplerState> pointClampSS;


	// Depth Stencil State
	ComPtr<ID3D11DepthStencilState> basicDSS;
	ComPtr<ID3D11DepthStencilState> edgeMaskingDSS;
	ComPtr<ID3D11DepthStencilState> stencilEqualDrawDSS;
	ComPtr<ID3D11DepthStencilState> mirrorMaskingDSS;
	ComPtr<ID3D11DepthStencilState> mirrorDrawMaskedDSS;


	// Blend State
	ComPtr<ID3D11BlendState> alphaBS;


	// Render Target Buffer
	ComPtr<ID3D11Texture2D> backBuffer;
	ComPtr<ID3D11RenderTargetView> backBufferRTV;

	ComPtr<ID3D11Texture2D> basicRenderBuffer;
	ComPtr<ID3D11RenderTargetView> basicRTV;
	ComPtr<ID3D11ShaderResourceView> basicSRV;

	ComPtr<ID3D11Texture2D> normalBuffer;
	ComPtr<ID3D11RenderTargetView> normalRTV;
	ComPtr<ID3D11ShaderResourceView> normalSRV;

	ComPtr<ID3D11Texture2D> positionBuffer;
	ComPtr<ID3D11RenderTargetView> positionRTV;
	ComPtr<ID3D11ShaderResourceView> positionSRV;

	ComPtr<ID3D11Texture2D> albedoEdgeBuffer;
	ComPtr<ID3D11RenderTargetView> albedoEdgeRTV;
	ComPtr<ID3D11ShaderResourceView> albedoEdgeSRV;

	ComPtr<ID3D11Texture2D> coverageBuffer;
	ComPtr<ID3D11RenderTargetView> coverageRTV;
	ComPtr<ID3D11ShaderResourceView> coverageSRV;

	ComPtr<ID3D11Texture2D> resolvedEdgeBuffer;
	ComPtr<ID3D11ShaderResourceView> resolvedEdgeSRV;

	ComPtr<ID3D11Texture2D> ssaoBuffer;
	ComPtr<ID3D11RenderTargetView> ssaoRTV;
	ComPtr<ID3D11ShaderResourceView> ssaoSRV;

	ComPtr<ID3D11Texture2D> ssaoBlurBuffer[2];
	ComPtr<ID3D11RenderTargetView> ssaoBlurRTV[2];
	ComPtr<ID3D11ShaderResourceView> ssaoBlurSRV[2];


	// Depth Stencil Buffer
	ComPtr<ID3D11Texture2D> basicDepthBuffer;
	ComPtr<ID3D11DepthStencilView> basicDSV;

	ComPtr<ID3D11Texture2D> deferredDepthBuffer;
	ComPtr<ID3D11DepthStencilView> deferredDSV;


	// Shadow Resource Buffer
	ComPtr<ID3D11Texture2D> atlasMapBuffer;
	ComPtr<ID3D11ShaderResourceView> atlasMapSRV;

	ComPtr<ID3D11Texture2D> grassColorMapBuffer;
	ComPtr<ID3D11ShaderResourceView> grassColorMapSRV;

	ComPtr<ID3D11Texture2D> sunBuffer;
	ComPtr<ID3D11ShaderResourceView> sunSRV;
	ComPtr<ID3D11Texture2D> moonBuffer;
	ComPtr<ID3D11ShaderResourceView> moonSRV;

	// Viewport
	D3D11_VIEWPORT basicViewport;
	D3D11_VIEWPORT mirrorWorldViewPort;


	// device, context, swapChain
	bool InitGraphicsCore(DXGI_FORMAT pixelFormat, HWND& hwnd);


	// RTV, DSV, SRV (+ UAV ...)
	bool InitGraphicsBuffer();
	bool InitRenderTargetBuffers();
	bool InitDepthStencilBuffers();
	bool InitShaderResourceBuffers();


	// VS, IL, PS, RS, SS, DSS (+ HS, DS, GS, BS ...)
	bool InitGraphicsState();
	bool InitVertexShaderAndInputLayouts();
	bool InitGeometryShaders();
	bool InitPixelShaders();
	bool InitRasterizerStates();
	bool InitSamplerStates();
	bool InitDepthStencilStates();
	bool InitBlendStates();


	// PSO
	void InitGraphicsPSO();
	void SetPipelineStates(GraphicsPSO& pso);
	GraphicsPSO basicPSO;
	GraphicsPSO basicMirrorPSO;
	GraphicsPSO semiAlphaPSO;
	GraphicsPSO skyboxPSO;
	GraphicsPSO cloudPSO;
	GraphicsPSO cloudMirrorPSO;
	GraphicsPSO fogFilterPSO;
	GraphicsPSO instancePSO;
	GraphicsPSO instanceMirrorPSO;
	GraphicsPSO mirrorMaskingPSO;
	GraphicsPSO blurPSO;
	GraphicsPSO waterPlanePSO;
	GraphicsPSO waterFilterPSO;
	GraphicsPSO basicDepthPSO;
	GraphicsPSO instanceDepthPSO;
	GraphicsPSO basicShadowPSO;
	GraphicsPSO ssaoNormalPSO;
	GraphicsPSO ssaoEdgePSO;
	GraphicsPSO edgeMaskingPSO;
}


// Function
bool Graphics::InitGraphicsCore(DXGI_FORMAT pixelFormat, HWND& hwnd)
{
	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;

	UINT deviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL levels[] = {
		// D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_9_3,
	};
	D3D_FEATURE_LEVEL featureLevel;

	HRESULT ret = D3D11CreateDevice(0, driverType, 0, deviceFlags, levels, ARRAYSIZE(levels),
		D3D11_SDK_VERSION, device.GetAddressOf(), &featureLevel, context.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create device and context" << std::endl;
		return false;
	}

	DXGI_SWAP_CHAIN_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.BufferDesc.Width = App::WIDTH;
	desc.BufferDesc.Height = App::HEIGHT;
	desc.BufferDesc.RefreshRate.Numerator = 60;
	desc.BufferDesc.RefreshRate.Denominator = 1;
	desc.BufferDesc.Format = pixelFormat;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = 2;
	desc.OutputWindow = hwnd;
	desc.Windowed = true;
	desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ret = D3D11CreateDeviceAndSwapChain(NULL, driverType, 0, deviceFlags, levels, ARRAYSIZE(levels),
		D3D11_SDK_VERSION, &desc, swapChain.GetAddressOf(), device.GetAddressOf(), &featureLevel,
		context.GetAddressOf());

	if (FAILED(ret)) {
		std::cout << "failed create swapchain" << std::endl;
		return false;
	}

	return true;
}

bool Graphics::InitGraphicsBuffer()
{
	if (!InitRenderTargetBuffers())
		return false;

	if (!InitDepthStencilBuffers())
		return false;

	if (!InitShaderResourceBuffers())
		return false;

	return true;
}

bool Graphics::InitRenderTargetBuffers()
{
	// backBuffer
	swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
	HRESULT ret =
		device->CreateRenderTargetView(backBuffer.Get(), nullptr, backBufferRTV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create back buffer rtv" << std::endl;
		return false;
	}

	// basic render
	DXGI_FORMAT format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	UINT bindFlag = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	if (!DXUtils::CreateTextureBuffer(
			basicRenderBuffer, App::WIDTH, App::HEIGHT, false, format, bindFlag)) {
		std::cout << "failed create basic render buffer" << std::endl;
		return false;
	}
	ret = device->CreateRenderTargetView(basicRenderBuffer.Get(), nullptr, basicRTV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create basic rtv" << std::endl;
		return false;
	}
	ret =
		device->CreateShaderResourceView(basicRenderBuffer.Get(), nullptr, basicSRV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create basic srv" << std::endl;
		return false;
	}

	// normal
	format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	bindFlag = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	if (!DXUtils::CreateTextureBuffer(
			normalBuffer, App::WIDTH, App::HEIGHT, true, format, bindFlag)) {
		std::cout << "failed create normal buffer" << std::endl;
		return false;
	}
	ret = device->CreateRenderTargetView(normalBuffer.Get(), nullptr, normalRTV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create normal rtv" << std::endl;
		return false;
	}
	ret = device->CreateShaderResourceView(
		normalBuffer.Get(), nullptr, normalSRV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create normal srv" << std::endl;
		return false;
	}

	// position
	format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	bindFlag = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	if (!DXUtils::CreateTextureBuffer(
			positionBuffer, App::WIDTH, App::HEIGHT, true, format, bindFlag)) {
		std::cout << "failed create position buffer" << std::endl;
		return false;
	}
	ret = device->CreateRenderTargetView(
		positionBuffer.Get(), nullptr, positionRTV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create position rtv" << std::endl;
		return false;
	}
	ret = device->CreateShaderResourceView(
		positionBuffer.Get(), nullptr, positionSRV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create position srv" << std::endl;
		return false;
	}

	// albedo edge
	format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	bindFlag = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	if (!DXUtils::CreateTextureBuffer(
			albedoEdgeBuffer, App::WIDTH, App::HEIGHT, true, format, bindFlag)) {
		std::cout << "failed create albedo edge buffer" << std::endl;
		return false;
	}
	ret = device->CreateRenderTargetView(
		albedoEdgeBuffer.Get(), nullptr, albedoEdgeRTV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create albedo edge rtv" << std::endl;
		return false;
	}
	ret = device->CreateShaderResourceView(
		albedoEdgeBuffer.Get(), nullptr, albedoEdgeSRV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create albedo edge srv" << std::endl;
		return false;
	}

	// coverage
	format = DXGI_FORMAT_R32_UINT;
	bindFlag = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	if (!DXUtils::CreateTextureBuffer(
			coverageBuffer, App::WIDTH, App::HEIGHT, true, format, bindFlag)) {
		std::cout << "failed create coverage buffer" << std::endl;
		return false;
	}
	ret = device->CreateRenderTargetView(coverageBuffer.Get(), nullptr, coverageRTV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create coverage rtv" << std::endl;
		return false;
	}
	ret =
		device->CreateShaderResourceView(coverageBuffer.Get(), nullptr, coverageSRV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create coverage srv" << std::endl;
		return false;
	}

	// resolved edge
	format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	bindFlag = D3D11_BIND_SHADER_RESOURCE;
	if (!DXUtils::CreateTextureBuffer(
			resolvedEdgeBuffer, App::WIDTH, App::HEIGHT, false, format, bindFlag)) {
		std::cout << "failed create resolved edge buffer" << std::endl;
		return false;
	}
	ret = device->CreateShaderResourceView(
		resolvedEdgeBuffer.Get(), nullptr, resolvedEdgeSRV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create resolved edge srv" << std::endl;
		return false;
	}

	// ssao
	format = DXGI_FORMAT_R32_FLOAT;
	bindFlag = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	if (!DXUtils::CreateTextureBuffer(
			ssaoBuffer, App::WIDTH, App::HEIGHT, false, format, bindFlag)) {
		std::cout << "failed create ssao buffer" << std::endl;
		return false;
	}
	ret = device->CreateRenderTargetView(ssaoBuffer.Get(), nullptr, ssaoRTV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create ssao rtv" << std::endl;
		return false;
	}
	ret = device->CreateShaderResourceView(ssaoBuffer.Get(), nullptr, ssaoSRV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create ssao srv" << std::endl;
		return false;
	}

	// blur
	format = DXGI_FORMAT_R32_FLOAT;
	bindFlag = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	for (int i = 0; i < 2; ++i) {
		if (!DXUtils::CreateTextureBuffer(
				ssaoBlurBuffer[i], App::WIDTH, App::HEIGHT, false, format, bindFlag)) {
			std::cout << "failed create ssao blur buffer" << std::endl;
			return false;
		}
		ret = device->CreateRenderTargetView(
			ssaoBlurBuffer[i].Get(), nullptr, ssaoBlurRTV[i].GetAddressOf());
		if (FAILED(ret)) {
			std::cout << "failed create ssao blur rtv" << std::endl;
			return false;
		}
		ret = device->CreateShaderResourceView(
			ssaoBlurBuffer[i].Get(), nullptr, ssaoBlurSRV[i].GetAddressOf());
		if (FAILED(ret)) {
			std::cout << "failed create ssao blur srv" << std::endl;
			return false;
		}
	}
	


	return true;
}

bool Graphics::InitDepthStencilBuffers()
{
	// basic DSV
	DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	UINT bindFlag = D3D11_BIND_DEPTH_STENCIL;
	if (!DXUtils::CreateTextureBuffer(
			basicDepthBuffer, App::WIDTH, App::HEIGHT, true, format, bindFlag)) {
		std::cout << "failed create basic depth stencil buffer" << std::endl;
		return false;
	}

	HRESULT ret = Graphics::device->CreateDepthStencilView(
		basicDepthBuffer.Get(), nullptr, basicDSV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create basic dsv" << std::endl;
		return false;
	}

	// deferred DSV
	format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	bindFlag = D3D11_BIND_DEPTH_STENCIL;
	if (!DXUtils::CreateTextureBuffer(
			deferredDepthBuffer, App::WIDTH, App::HEIGHT, false, format, bindFlag)) {
		std::cout << "failed create deferred depth stencil buffer" << std::endl;
		return false;
	}
	ret = Graphics::device->CreateDepthStencilView(
		deferredDepthBuffer.Get(), nullptr, deferredDSV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create deferred dsv" << std::endl;
		return false;
	}

	return true;
}

bool Graphics::InitShaderResourceBuffers()
{
	// Asset Files
	if (!DXUtils::CreateTextureArrayFromAtlasFile(
			atlasMapBuffer, atlasMapSRV, "../assets/blockatlas1.png")) {
		std::cout << "failed create texture from atlas file" << std::endl;
		return false;
	}

	/*if (!DXUtils::CreateTextureFromFile(
			grassColorMapBuffer, grassColorMapSRV, "../assets/grass_color_map.png")) {
		std::cout << "failed create texture from grass color map file" << std::endl;
		return false;
	}*/

	if (!DXUtils::CreateTexture2DFromFile(sunBuffer, sunSRV, "../assets/sun.png")) {
		std::cout << "failed create texture from sun file" << std::endl;
		return false;
	}

	if (!DXUtils::CreateTexture2DFromFile(moonBuffer, moonSRV, "../assets/moon.png")) {
		std::cout << "failed create texture from moon file" << std::endl;
		return false;
	}

	return true;
}

bool Graphics::InitGraphicsState()
{
	if (!InitVertexShaderAndInputLayouts())
		return false;

	if (!InitGeometryShaders())
		return false;

	if (!InitPixelShaders())
		return false;

	if (!InitRasterizerStates())
		return false;

	if (!InitSamplerStates())
		return false;

	if (!InitDepthStencilStates())
		return false;

	if (!InitBlendStates())
		return false;

	return true;
}

bool Graphics::InitVertexShaderAndInputLayouts()
{
	// BasicVS & BasicIL
	std::vector<D3D11_INPUT_ELEMENT_DESC> elementDesc = {
		{ "DATA", 0, DXGI_FORMAT_R32_UINT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	if (!DXUtils::CreateVertexShaderAndInputLayout(
			L"BasicVS.hlsl", basicVS, basicIL, elementDesc)) {
		std::cout << "failed create basic vs" << std::endl;
		return false;
	}

	if (!DXUtils::CreateVertexShaderAndInputLayout(
			L"BasicShadowVS.hlsl", basicShadowVS, basicIL, elementDesc)) {
		std::cout << "failed create basic vs" << std::endl;
		return false;
	}

	// SkyBox
	std::vector<D3D11_INPUT_ELEMENT_DESC> elementDesc2 = { { "POSITION", 0,
		DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 } };
	if (!DXUtils::CreateVertexShaderAndInputLayout(
			L"SkyboxVS.hlsl", skyboxVS, skyboxIL, elementDesc2)) {
		std::cout << "failed create skybox vs" << std::endl;
		return false;
	}

	// Cloud
	std::vector<D3D11_INPUT_ELEMENT_DESC> elementDesc3 = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "FACE", 0, DXGI_FORMAT_R8_UINT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	if (!DXUtils::CreateVertexShaderAndInputLayout(
			L"CloudVS.hlsl", cloudVS, cloudIL, elementDesc3)) {
		std::cout << "failed create cloud vs" << std::endl;
		return false;
	}

	// Sampling
	std::vector<D3D11_INPUT_ELEMENT_DESC> elementDesc4 = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	if (!DXUtils::CreateVertexShaderAndInputLayout(
			L"SamplingVS.hlsl", samplingVS, samplingIL, elementDesc4)) {
		std::cout << "failed create sampling vs" << std::endl;
		return false;
	}

	// Instance
	std::vector<D3D11_INPUT_ELEMENT_DESC> elementDesc6 = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TYPE", 0, DXGI_FORMAT_R32_UINT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};
	if (!DXUtils::CreateVertexShaderAndInputLayout(
			L"InstanceVS.hlsl", instanceVS, instanceIL, elementDesc6)) {
		std::cout << "failed create instance vs" << std::endl;
		return false;
	}

	return true;
}

bool Graphics::InitGeometryShaders() { return true; }

bool Graphics::InitPixelShaders()
{
	// BasicPS
	if (!DXUtils::CreatePixelShader(L"BasicPS.hlsl", basicPS)) {
		std::cout << "failed create basic ps" << std::endl;
		return false;
	}

	// BasicAlphaClipPS
	std::vector<D3D_SHADER_MACRO> macros;
	macros.push_back({ "USE_ALPHA_CLIP", "1" });
	macros.push_back({ NULL, NULL });
	if (!DXUtils::CreatePixelShader(L"BasicPS.hlsl", basicAlphaClipPS, macros.data())) {
		std::cout << "failed create basic alpha clip ps" << std::endl;
		return false;
	}

	// BasicDepthClipPS
	macros.clear();
	macros.push_back({ "USE_DEPTH_CLIP", "1" });
	macros.push_back({ NULL, NULL });
	if (!DXUtils::CreatePixelShader(L"BasicPS.hlsl", basicDepthClipPS, macros.data())) {
		std::cout << "failed create basic depth clip ps" << std::endl;
		return false;
	}

	// BasicDepthClipPS
	macros.clear();
	macros.push_back({ "USE_ALPHA_CLIP", "1" });
	macros.push_back({ "USE_DEPTH_CLIP", "1" });
	macros.push_back({ NULL, NULL });
	if (!DXUtils::CreatePixelShader(L"BasicPS.hlsl", basicAlphaDepthClipPS, macros.data())) {
		std::cout << "failed create basic alpha depth clip ps" << std::endl;
		return false;
	}

	// SkyboxPS
	if (!DXUtils::CreatePixelShader(L"SkyboxPS.hlsl", skyboxPS)) {
		std::cout << "failed create skybox ps" << std::endl;
		return false;
	}

	// CloudPS
	if (!DXUtils::CreatePixelShader(L"CloudPS.hlsl", cloudPS)) {
		std::cout << "failed create cloud ps" << std::endl;
		return false;
	}

	// SamplingPS
	if (!DXUtils::CreatePixelShader(L"SamplingPS.hlsl", samplingPS)) {
		std::cout << "failed create sampling ps" << std::endl;
		return false;
	}

	// fogFilterPS
	if (!DXUtils::CreatePixelShader(L"FogFilterPS.hlsl", fogFilterPS)) {
		std::cout << "failed create fog filter ps" << std::endl;
		return false;
	}

	// MirrorMaskingPS
	if (!DXUtils::CreatePixelShader(L"MirrorMaskingPS.hlsl", mirrorMaskingPS)) {
		std::cout << "failed create mirrorMasking ps" << std::endl;
		return false;
	}

	// WaterPlanePS
	if (!DXUtils::CreatePixelShader(L"WaterPlanePS.hlsl", waterPlanePS)) {
		std::cout << "failed create water plane ps" << std::endl;
		return false;
	}

	// WaterFilterPS
	if (!DXUtils::CreatePixelShader(L"WaterFilterPS.hlsl", waterFilterPS)) {
		std::cout << "failed create water filter ps" << std::endl;
		return false;
	}

	// BlurPS
	macros.clear();
	macros.push_back({ "USE_ALPHA_BLUR", "1" });
	macros.push_back({ "BLUR_X", "1" });
	macros.push_back({ NULL, NULL });
	if (!DXUtils::CreatePixelShader(L"BlurPS.hlsl", blurMirrorPS[0], macros.data())) {
		std::cout << "failed create blur mirror x ps" << std::endl;
		return false;
	}
	macros.clear();
	macros.push_back({ "USE_ALPHA_BLUR", "1" });
	macros.push_back({ "BLUR_Y", "1" });
	macros.push_back({ NULL, NULL });
	if (!DXUtils::CreatePixelShader(L"BlurPS.hlsl", blurMirrorPS[1], macros.data())) {
		std::cout << "failed create blur mirror y ps" << std::endl;
		return false;
	}
	macros.clear();
	macros.push_back({ "BLUR_X", "1" });
	macros.push_back({ NULL, NULL });
	if (!DXUtils::CreatePixelShader(L"BlurPS.hlsl", blurSsaoPS[0], macros.data())) {
		std::cout << "failed create blur ssao x ps" << std::endl;
		return false;
	}
	macros.clear();
	macros.push_back({ "BLUR_Y", "1" });
	macros.push_back({ NULL, NULL });
	if (!DXUtils::CreatePixelShader(L"BlurPS.hlsl", blurSsaoPS[1], macros.data())) {
		std::cout << "failed create blur ssao y ps" << std::endl;
		return false;
	}

	// SsaoPS
	if (!DXUtils::CreatePixelShader(L"SsaoPS.hlsl", ssaoNormalPS)) {
		std::cout << "failed create ssao normal ps" << std::endl;
		return false;
	}
	if (!DXUtils::CreatePixelShader(L"SsaoPS.hlsl", ssaoEdgePS, nullptr, "mainMSAA")) {
		std::cout << "failed create ssao edge ps" << std::endl;
		return false;
	}

	// EdgeMaskingPS
	if (!DXUtils::CreatePixelShader(L"EdgeMaskingPS.hlsl", edgeMaskingPS)) {
		std::cout << "failed create edge masking ps" << std::endl;
		return false;
	}

	return true;
}

bool Graphics::InitRasterizerStates()
{
	D3D11_RASTERIZER_DESC rastDesc;
	ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
	rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.DepthClipEnable = true;
	rastDesc.MultisampleEnable = true;

	// solidRS
	rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	HRESULT ret = Graphics::device->CreateRasterizerState(&rastDesc, solidRS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create solid RS" << std::endl;
		return false;
	}

	// wireRS
	rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
	ret = Graphics::device->CreateRasterizerState(&rastDesc, wireRS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create wire RS" << std::endl;
		return false;
	}

	// noneCullRS
	rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
	ret = Graphics::device->CreateRasterizerState(&rastDesc, noneCullRS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create noneCull RS" << std::endl;
		return false;
	}

	// mirrorRS
	rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
	rastDesc.FrontCounterClockwise = true;
	ret = Graphics::device->CreateRasterizerState(&rastDesc, mirrorRS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create mirror RS" << std::endl;
		return false;
	}

	return true;
}

bool Graphics::InitSamplerStates()
{
	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	desc.MinLOD = 0.0f;
	desc.MaxLOD = D3D11_FLOAT32_MAX;

	// point wrap
	desc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	HRESULT ret = Graphics::device->CreateSamplerState(&desc, pointWrapSS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create point wrap SS" << std::endl;
		return false;
	}

	// linear wrap
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	ret = Graphics::device->CreateSamplerState(&desc, linearWrapSS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create linear Wrap SS" << std::endl;
		return false;
	}

	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ret = Graphics::device->CreateSamplerState(&desc, linearClampSS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create linear Clamp SS" << std::endl;
		return false;
	}

	// shadowPointSS
	desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.BorderColor[0] = 1.0f; // ??Zê°?
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	device->CreateSamplerState(&desc, shadowPointSS.GetAddressOf());

	// shadowCompareSS
	desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.BorderColor[0] = 100.0f;
	desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	ret = Graphics::device->CreateSamplerState(&desc, shadowCompareSS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create shadowCompareSS" << std::endl;
		return false;
	}

	// point clamp
	desc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ret = Graphics::device->CreateSamplerState(&desc, pointClampSS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create point clamp SS" << std::endl;
		return false;
	}

	return true;
}

bool Graphics::InitDepthStencilStates()
{
	D3D11_DEPTH_STENCIL_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.DepthEnable = true;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
	desc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

	// basic DSS
	HRESULT ret = Graphics::device->CreateDepthStencilState(&desc, basicDSS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create basic DSS" << std::endl;
		return false;
	}
	
	// Edge Masking DSS
	ZeroMemory(&desc, sizeof(desc));
	desc.DepthEnable = true;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;
	desc.StencilEnable = true;
	desc.StencilReadMask = 0xFF;
	desc.StencilWriteMask = 0xFF;
	desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;	   // stencil X
	desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP; // stencil O depth X
	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;   // stencil O depth O
	desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP; 
	desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	ret = Graphics::device->CreateDepthStencilState(&desc, edgeMaskingDSS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create edge masking DSS" << std::endl;
		return false;
	}

	// Edge Pixel Draw DSS
	ZeroMemory(&desc, sizeof(desc));
	desc.DepthEnable = true;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;
	desc.StencilEnable = true;
	desc.StencilReadMask = 0xFF;
	desc.StencilWriteMask = 0xFF;
	desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;	   // stencil X
	desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP; // stencil O depth X
	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;   // stencil O depth O
	desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	ret = Graphics::device->CreateDepthStencilState(&desc, stencilEqualDrawDSS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create stencil equal draw DSS" << std::endl;
		return false;
	}


	// Mirror Masking DSS
	ZeroMemory(&desc, sizeof(desc));
	desc.DepthEnable = true;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
	desc.StencilEnable = true;
	desc.StencilReadMask = 0xFF;
	desc.StencilWriteMask = 0xFF;
	desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;	   // stencil X
	desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP; // stencil O depth X
	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;   // stencil O depth O
	desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	ret = Graphics::device->CreateDepthStencilState(&desc, mirrorMaskingDSS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create mirror masking DSS" << std::endl;
		return false;
	}

	// Mirror Draw Masked DSS
	ZeroMemory(&desc, sizeof(desc));
	desc.DepthEnable = true;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
	desc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
	desc.StencilEnable = true;
	desc.StencilReadMask = 0xFF;
	desc.StencilWriteMask = 0xFF;
	desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	ret = Graphics::device->CreateDepthStencilState(&desc, mirrorDrawMaskedDSS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create mirror draw masked DSS" << std::endl;
		return false;
	}

	return true;
}

bool Graphics::InitBlendStates()
{
	// alpha BS
	D3D11_BLEND_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.AlphaToCoverageEnable = false;
	desc.IndependentBlendEnable = false;
	desc.RenderTarget[0].BlendEnable = true;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HRESULT ret = Graphics::device->CreateBlendState(&desc, alphaBS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create alpha BS" << std::endl;
		return false;
	}

	return true;
}

void Graphics::InitGraphicsPSO()
{
	// basicPSO
	basicPSO.inputLayout = basicIL;
	basicPSO.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	basicPSO.vertexShader = basicVS;
	basicPSO.geometryShader = nullptr;
	basicPSO.rasterizerState = solidRS;
	basicPSO.pixelShader = basicPS;
	basicPSO.samplerStates.push_back(pointWrapSS.Get());
	basicPSO.samplerStates.push_back(linearWrapSS.Get());
	basicPSO.samplerStates.push_back(linearClampSS.Get());
	basicPSO.samplerStates.push_back(shadowPointSS.Get());
	basicPSO.samplerStates.push_back(shadowCompareSS.Get());
	basicPSO.samplerStates.push_back(pointClampSS.Get());
	basicPSO.depthStencilState = basicDSS;
	basicPSO.stencilRef = 0;
	basicPSO.blendState = nullptr;

	// basic mirrorPSO
	basicMirrorPSO = basicPSO;
	basicMirrorPSO.rasterizerState = mirrorRS;
	basicMirrorPSO.pixelShader = basicDepthClipPS;
	basicMirrorPSO.depthStencilState = mirrorDrawMaskedDSS;
	basicMirrorPSO.stencilRef = 1;

	// semiAlphaPSO
	semiAlphaPSO = basicPSO;
	semiAlphaPSO.rasterizerState = noneCullRS;
	semiAlphaPSO.pixelShader = basicAlphaClipPS;

	// skyboxPSO
	skyboxPSO = basicPSO;
	skyboxPSO.inputLayout = skyboxIL;
	skyboxPSO.vertexShader = skyboxVS;
	skyboxPSO.pixelShader = skyboxPS;

	// cloudPSO
	cloudPSO = basicPSO;
	cloudPSO.inputLayout = cloudIL;
	cloudPSO.vertexShader = cloudVS;
	cloudPSO.pixelShader = cloudPS;
	cloudPSO.blendState = alphaBS;

	// cloudMirrorPSO
	cloudMirrorPSO = cloudPSO;
	cloudMirrorPSO.rasterizerState = mirrorRS;
	cloudMirrorPSO.depthStencilState = mirrorDrawMaskedDSS;
	cloudMirrorPSO.stencilRef = 1;

	// fogFilterPSO
	fogFilterPSO = basicPSO;
	fogFilterPSO.inputLayout = samplingIL;
	fogFilterPSO.vertexShader = samplingVS;
	fogFilterPSO.pixelShader = fogFilterPS;

	// instancePSO
	instancePSO = basicPSO;
	instancePSO.inputLayout = instanceIL;
	instancePSO.vertexShader = instanceVS;
	instancePSO.rasterizerState = noneCullRS;
	instancePSO.pixelShader = basicAlphaClipPS;

	// instanceMirrorPSO
	instanceMirrorPSO = instancePSO;
	instanceMirrorPSO.pixelShader = basicAlphaDepthClipPS;
	instanceMirrorPSO.depthStencilState = mirrorDrawMaskedDSS;
	instanceMirrorPSO.stencilRef = 1;

	// mirrorMaskingPSO
	mirrorMaskingPSO = basicPSO;
	mirrorMaskingPSO.pixelShader = mirrorMaskingPS;
	mirrorMaskingPSO.depthStencilState = mirrorMaskingDSS;
	mirrorMaskingPSO.stencilRef = 1;

	// blurPSO
	blurPSO = basicPSO;
	blurPSO.inputLayout = samplingIL;
	blurPSO.vertexShader = samplingVS;
	blurPSO.pixelShader = nullptr;

	// waterPlanePSO
	waterPlanePSO = basicPSO;
	waterPlanePSO.rasterizerState = noneCullRS;
	waterPlanePSO.pixelShader = waterPlanePS;
	waterPlanePSO.blendState = alphaBS;

	// waterFilterPSO
	waterFilterPSO = basicPSO;
	waterFilterPSO.inputLayout = samplingIL;
	waterFilterPSO.vertexShader = samplingVS;
	waterFilterPSO.pixelShader = waterFilterPS;

	// basicshadowPSO
	basicShadowPSO = basicPSO;
	basicShadowPSO.vertexShader = basicShadowVS;
	basicShadowPSO.geometryShader = basicShadowGS;
	basicShadowPSO.pixelShader = nullptr;

	// ssaoNormalPSO
	ssaoNormalPSO = basicPSO;
	ssaoNormalPSO.inputLayout = samplingIL;
	ssaoNormalPSO.vertexShader = samplingVS;
	ssaoNormalPSO.pixelShader = ssaoNormalPS;
	ssaoNormalPSO.depthStencilState = stencilEqualDrawDSS;
	ssaoNormalPSO.stencilRef = 0;

	// ssaoEdgePSO
	ssaoEdgePSO = ssaoNormalPSO;
	ssaoEdgePSO.pixelShader = ssaoEdgePS;
	ssaoEdgePSO.stencilRef = 1;

	// edgeMaskingPSO
	edgeMaskingPSO = basicPSO;
	edgeMaskingPSO.inputLayout = samplingIL;
	edgeMaskingPSO.vertexShader = samplingVS;
	edgeMaskingPSO.pixelShader = edgeMaskingPS;
	edgeMaskingPSO.depthStencilState = edgeMaskingDSS;
	edgeMaskingPSO.stencilRef = 1;
}

void Graphics::SetPipelineStates(GraphicsPSO& pso)
{
	context->IASetInputLayout(pso.inputLayout.Get());
	context->IASetPrimitiveTopology(pso.topology);

	context->VSSetShader(pso.vertexShader.Get(), nullptr, 0);

	context->GSSetShader(pso.geometryShader.Get(), nullptr, 0);

	context->RSSetState(pso.rasterizerState.Get());

	context->PSSetShader(pso.pixelShader.Get(), nullptr, 0);

	if (pso.samplerStates.empty())
		context->PSSetSamplers(0, 0, nullptr);
	else
		context->PSSetSamplers(0, (UINT)pso.samplerStates.size(), pso.samplerStates.data());

	context->OMSetDepthStencilState(pso.depthStencilState.Get(), pso.stencilRef);

	context->OMSetBlendState(pso.blendState.Get(), pso.blendFactor, 0xffffffff);
}