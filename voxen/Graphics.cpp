#include "Graphics.h"
#include "DXUtils.h"
#include "App.h"
#include "Terrain.h"

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


	// Pixel Shader
	ComPtr<ID3D11PixelShader> basicPS;
	ComPtr<ID3D11PixelShader> basicAlphaClipPS;
	ComPtr<ID3D11PixelShader> basicMirrorPS;
	ComPtr<ID3D11PixelShader> basicMirrorAlphaClipPS;
	ComPtr<ID3D11PixelShader> skyboxPS;
	ComPtr<ID3D11PixelShader> skyboxMirrorPS;
	ComPtr<ID3D11PixelShader> cloudPS;
	ComPtr<ID3D11PixelShader> samplingPS;
	ComPtr<ID3D11PixelShader> fogFilterPS;
	ComPtr<ID3D11PixelShader> mirrorMaskingPS;
	ComPtr<ID3D11PixelShader> waterPlanePS;
	ComPtr<ID3D11PixelShader> waterFilterPS;
	ComPtr<ID3D11PixelShader> blurMirrorPS[2];
	ComPtr<ID3D11PixelShader> blurSsaoPS[2];
	ComPtr<ID3D11PixelShader> ssaoPS;
	ComPtr<ID3D11PixelShader> ssaoEdgePS;
	ComPtr<ID3D11PixelShader> edgeMaskingPS;
	ComPtr<ID3D11PixelShader> shadingBasicPS;
	ComPtr<ID3D11PixelShader> shadingBasicEdgePS;
	ComPtr<ID3D11PixelShader> bloomDownPS;
	ComPtr<ID3D11PixelShader> bloomUpPS;
	ComPtr<ID3D11PixelShader> combineBloomPS;


	// Rasterizer State
	ComPtr<ID3D11RasterizerState> solidRS;
	ComPtr<ID3D11RasterizerState> wireRS;
	ComPtr<ID3D11RasterizerState> noneCullRS;
	ComPtr<ID3D11RasterizerState> mirrorRS;
	ComPtr<ID3D11RasterizerState> frontCullRS;


	// Sampler State
	ComPtr<ID3D11SamplerState> pointWrapSS;
	ComPtr<ID3D11SamplerState> linearWrapSS;
	ComPtr<ID3D11SamplerState> linearClampSS;
	ComPtr<ID3D11SamplerState> shadowPointSS;
	ComPtr<ID3D11SamplerState> shadowCompareSS;
	ComPtr<ID3D11SamplerState> pointClampSS;


	// Depth Stencil State
	ComPtr<ID3D11DepthStencilState> basicDSS;
	ComPtr<ID3D11DepthStencilState> stencilMaskDSS;
	ComPtr<ID3D11DepthStencilState> stencilEqualDrawDSS;
	ComPtr<ID3D11DepthStencilState> mirrorDrawMaskedDSS;


	// Blend State
	ComPtr<ID3D11BlendState> alphaBS;


	// Render Target Buffer
	ComPtr<ID3D11Texture2D> backBuffer;
	ComPtr<ID3D11RenderTargetView> backBufferRTV;

	ComPtr<ID3D11Texture2D> basicBuffer;
	ComPtr<ID3D11RenderTargetView> basicRTV;
	ComPtr<ID3D11ShaderResourceView> basicSRV;

	ComPtr<ID3D11Texture2D> basicMSBuffer;
	ComPtr<ID3D11RenderTargetView> basicMSRTV;
	ComPtr<ID3D11ShaderResourceView> basicMSSRV;

	ComPtr<ID3D11Texture2D> normalEdgeBuffer;
	ComPtr<ID3D11RenderTargetView> normalEdgeRTV;
	ComPtr<ID3D11ShaderResourceView> normalEdgeSRV;

	ComPtr<ID3D11Texture2D> positionBuffer;
	ComPtr<ID3D11RenderTargetView> positionRTV;
	ComPtr<ID3D11ShaderResourceView> positionSRV;

	ComPtr<ID3D11Texture2D> albedoBuffer;
	ComPtr<ID3D11RenderTargetView> albedoRTV;
	ComPtr<ID3D11ShaderResourceView> albedoSRV;

	ComPtr<ID3D11Texture2D> coverageBuffer;
	ComPtr<ID3D11RenderTargetView> coverageRTV;
	ComPtr<ID3D11ShaderResourceView> coverageSRV;

	ComPtr<ID3D11Texture2D> ssaoBuffer;
	ComPtr<ID3D11RenderTargetView> ssaoRTV;
	ComPtr<ID3D11ShaderResourceView> ssaoSRV;

	ComPtr<ID3D11Texture2D> ssaoBlurBuffer[2];
	ComPtr<ID3D11RenderTargetView> ssaoBlurRTV[2];
	ComPtr<ID3D11ShaderResourceView> ssaoBlurSRV[2];

	ComPtr<ID3D11Texture2D> mirrorWorldBuffer;
	ComPtr<ID3D11RenderTargetView> mirrorWorldRTV;
	ComPtr<ID3D11ShaderResourceView> mirrorWorldSRV;

	ComPtr<ID3D11Texture2D> mirrorDepthRenderBuffer;
	ComPtr<ID3D11RenderTargetView> mirrorDepthRTV;
	ComPtr<ID3D11ShaderResourceView> mirrorDepthSRV;

	ComPtr<ID3D11Texture2D> mirrorBlurBuffer[2];
	ComPtr<ID3D11RenderTargetView> mirrorBlurRTV[2];
	ComPtr<ID3D11ShaderResourceView> mirrorBlurSRV[2];

	ComPtr<ID3D11Texture2D> bloomBuffer[5];
	ComPtr<ID3D11RenderTargetView> bloomRTV[5];
	ComPtr<ID3D11ShaderResourceView> bloomSRV[5];


	// Depth Stencil Buffer
	ComPtr<ID3D11Texture2D> basicDepthBuffer;
	ComPtr<ID3D11DepthStencilView> basicDSV;
	ComPtr<ID3D11ShaderResourceView> basicDepthSRV;

	ComPtr<ID3D11Texture2D> deferredDepthBuffer;
	ComPtr<ID3D11DepthStencilView> deferredDSV;

	ComPtr<ID3D11Texture2D> mirrorWorldDepthBuffer;
	ComPtr<ID3D11DepthStencilView> mirrorWorldDSV;

	ComPtr<ID3D11Texture2D> shadowBuffer;
	ComPtr<ID3D11DepthStencilView> shadowDSV;
	ComPtr<ID3D11ShaderResourceView> shadowSRV;


	// Shadow Resource Buffer
	ComPtr<ID3D11Texture2D> atlasMapBuffer;
	ComPtr<ID3D11ShaderResourceView> atlasMapSRV;

	ComPtr<ID3D11Texture2D> grassColorMapBuffer;
	ComPtr<ID3D11ShaderResourceView> grassColorMapSRV;

	ComPtr<ID3D11Texture2D> sunBuffer;
	ComPtr<ID3D11ShaderResourceView> sunSRV;

	ComPtr<ID3D11Texture2D> moonBuffer;
	ComPtr<ID3D11ShaderResourceView> moonSRV;

	ComPtr<ID3D11Texture2D> copyForwardRenderBuffer;
	ComPtr<ID3D11ShaderResourceView> copyForwardSRV;


	// Viewport
	D3D11_VIEWPORT basicViewport;
	D3D11_VIEWPORT mirrorWorldViewPort;
	D3D11_VIEWPORT bloomViewport;


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
	GraphicsPSO skyboxMirrorPSO;
	GraphicsPSO cloudPSO;
	GraphicsPSO cloudMirrorPSO;
	GraphicsPSO samplingPSO;
	GraphicsPSO fogFilterPSO;
	GraphicsPSO instancePSO;
	GraphicsPSO instanceMirrorPSO;
	GraphicsPSO mirrorMaskingPSO;
	GraphicsPSO waterPlanePSO;
	GraphicsPSO waterFilterPSO;
	GraphicsPSO basicDepthPSO;
	GraphicsPSO instanceDepthPSO;
	GraphicsPSO basicShadowPSO;
	GraphicsPSO ssaoPSO;
	GraphicsPSO ssaoEdgePSO;
	GraphicsPSO edgeMaskingPSO;
	GraphicsPSO shadingBasicPSO;
	GraphicsPSO shadingBasicEdgePSO;
	GraphicsPSO bloomDownPSO;
	GraphicsPSO bloomUpPSO;
	GraphicsPSO combineBloomPSO;
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

	// basic buffer
	DXGI_FORMAT format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	UINT bindFlag = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	if (!DXUtils::CreateTextureBuffer(
			basicBuffer, App::WIDTH, App::HEIGHT, false, format, bindFlag)) {
		std::cout << "failed create basic buffer" << std::endl;
		return false;
	}
	ret = device->CreateRenderTargetView(basicBuffer.Get(), nullptr, basicRTV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create basic rtv" << std::endl;
		return false;
	}
	ret = device->CreateShaderResourceView(basicBuffer.Get(), nullptr, basicSRV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create basic srv" << std::endl;
		return false;
	}

	// basic MS buffer
	format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	bindFlag = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	if (!DXUtils::CreateTextureBuffer(
			basicMSBuffer, App::WIDTH, App::HEIGHT, true, format, bindFlag)) {
		std::cout << "failed create basic MS buffer" << std::endl;
		return false;
	}
	ret = device->CreateRenderTargetView(basicMSBuffer.Get(), nullptr, basicMSRTV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create basic MS rtv" << std::endl;
		return false;
	}
	ret = device->CreateShaderResourceView(basicMSBuffer.Get(), nullptr, basicMSSRV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create basic MS srv" << std::endl;
		return false;
	}

	// normal edge
	format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	bindFlag = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	if (!DXUtils::CreateTextureBuffer(
			normalEdgeBuffer, App::WIDTH, App::HEIGHT, true, format, bindFlag)) {
		std::cout << "failed create normal edge buffer" << std::endl;
		return false;
	}
	ret = device->CreateRenderTargetView(
		normalEdgeBuffer.Get(), nullptr, normalEdgeRTV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create normal edge rtv" << std::endl;
		return false;
	}
	ret = device->CreateShaderResourceView(
		normalEdgeBuffer.Get(), nullptr, normalEdgeSRV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create normal edge srv" << std::endl;
		return false;
	}

	// position
	format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	bindFlag = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	if (!DXUtils::CreateTextureBuffer(
			positionBuffer, App::WIDTH, App::HEIGHT, true, format, bindFlag)) {
		std::cout << "failed create position buffer" << std::endl;
		return false;
	}
	ret = device->CreateRenderTargetView(positionBuffer.Get(), nullptr, positionRTV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create position rtv" << std::endl;
		return false;
	}
	ret =
		device->CreateShaderResourceView(positionBuffer.Get(), nullptr, positionSRV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create position srv" << std::endl;
		return false;
	}

	// albedo
	format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	bindFlag = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	if (!DXUtils::CreateTextureBuffer(
			albedoBuffer, App::WIDTH, App::HEIGHT, true, format, bindFlag)) {
		std::cout << "failed create albedo buffer" << std::endl;
		return false;
	}
	ret = device->CreateRenderTargetView(albedoBuffer.Get(), nullptr, albedoRTV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create albedo rtv" << std::endl;
		return false;
	}
	ret = device->CreateShaderResourceView(albedoBuffer.Get(), nullptr, albedoSRV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create albedo srv" << std::endl;
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

	// ssao blur
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

	// mirrorWorld
	format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	bindFlag = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	if (!DXUtils::CreateTextureBuffer(
			mirrorWorldBuffer, App::MIRROR_WIDTH, App::MIRROR_HEIGHT, false, format, bindFlag)) {
		std::cout << "failed create mirror world buffer" << std::endl;
		return false;
	}
	ret = device->CreateRenderTargetView(
		mirrorWorldBuffer.Get(), nullptr, mirrorWorldRTV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create mirror world rtv" << std::endl;
		return false;
	}
	ret = device->CreateShaderResourceView(
		mirrorWorldBuffer.Get(), nullptr, mirrorWorldSRV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create mirror world srv" << std::endl;
		return false;
	}

	// mirrorDepth render
	format = DXGI_FORMAT_R32_FLOAT;
	bindFlag = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	if (!DXUtils::CreateTextureBuffer(mirrorDepthRenderBuffer, App::MIRROR_WIDTH,
			App::MIRROR_HEIGHT, false, format, bindFlag)) {
		std::cout << "failed create mirror depth render buffer" << std::endl;
		return false;
	}
	ret = device->CreateRenderTargetView(
		mirrorDepthRenderBuffer.Get(), nullptr, mirrorDepthRTV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create mirror depth rtv" << std::endl;
		return false;
	}
	ret = device->CreateShaderResourceView(
		mirrorDepthRenderBuffer.Get(), nullptr, mirrorDepthSRV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create mirror depth srv" << std::endl;
		return false;
	}

	// mirror blur
	format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	bindFlag = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	for (int i = 0; i < 2; ++i) {
		if (!DXUtils::CreateTextureBuffer(mirrorBlurBuffer[i], App::MIRROR_WIDTH,
				App::MIRROR_HEIGHT, false, format, bindFlag)) {
			std::cout << "failed create mirror blur buffer" << std::endl;
			return false;
		}
		ret = device->CreateRenderTargetView(
			mirrorBlurBuffer[i].Get(), nullptr, mirrorBlurRTV[i].GetAddressOf());
		if (FAILED(ret)) {
			std::cout << "failed create mirror blur rtv" << std::endl;
			return false;
		}
		ret = device->CreateShaderResourceView(
			mirrorBlurBuffer[i].Get(), nullptr, mirrorBlurSRV[i].GetAddressOf());
		if (FAILED(ret)) {
			std::cout << "failed create mirror blur srv" << std::endl;
			return false;
		}
	}

	// bloom buffer
	UINT bloomWidth = App::WIDTH;
	UINT bloomHeight = App::HEIGHT;
	format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	bindFlag = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	for (int i = 0; i < 5; ++i) {
		if (!DXUtils::CreateTextureBuffer(
				bloomBuffer[i], bloomWidth, bloomHeight, false, format, bindFlag)) {
			std::cout << "failed create bloom buffer" << std::endl;
			return false;
		}
		ret = device->CreateRenderTargetView(
			bloomBuffer[i].Get(), nullptr, bloomRTV[i].GetAddressOf());
		if (FAILED(ret)) {
			std::cout << "failed create bloom rtv" << std::endl;
			return false;
		}
		ret = device->CreateShaderResourceView(
			bloomBuffer[i].Get(), nullptr, bloomSRV[i].GetAddressOf());
		if (FAILED(ret)) {
			std::cout << "failed create bloom srv" << std::endl;
			return false;
		}

		bloomWidth /= 2;
		bloomHeight /= 2;
	}

	return true;
}

bool Graphics::InitDepthStencilBuffers()
{
	// basic DSV
	DXGI_FORMAT format = DXGI_FORMAT_R32_TYPELESS;
	UINT bindFlag = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	if (!DXUtils::CreateTextureBuffer(
			basicDepthBuffer, App::WIDTH, App::HEIGHT, true, format, bindFlag)) {
		std::cout << "failed create basic depth stencil buffer" << std::endl;
		return false;
	}
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	HRESULT ret = Graphics::device->CreateDepthStencilView(
		basicDepthBuffer.Get(), &dsvDesc, basicDSV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create basic depth stencil view" << std::endl;
		return false;
	}
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
	ret = Graphics::device->CreateShaderResourceView(
		basicDepthBuffer.Get(), &srvDesc, basicDepthSRV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create basic depth srv" << std::endl;
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

	// mirrorWorld DSV
	format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	bindFlag = D3D11_BIND_DEPTH_STENCIL;
	if (!DXUtils::CreateTextureBuffer(mirrorWorldDepthBuffer, App::MIRROR_WIDTH, App::MIRROR_HEIGHT,
			false, format, bindFlag)) {
		std::cout << "failed create mirror world depth buffer" << std::endl;
		return false;
	}
	ret = Graphics::device->CreateDepthStencilView(
		mirrorWorldDepthBuffer.Get(), nullptr, mirrorWorldDSV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create mirror world dsv" << std::endl;
		return false;
	}

	// shadow DSV
	format = DXGI_FORMAT_R32_TYPELESS;
	bindFlag = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	if (!DXUtils::CreateTextureBuffer(
			shadowBuffer, App::SHADOW_WIDTH, App::SHADOW_HEIGHT, false, format, bindFlag)) {
		std::cout << "failed create basic depth stencil buffer" << std::endl;
		return false;
	}
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	ret = Graphics::device->CreateDepthStencilView(
		shadowBuffer.Get(), &dsvDesc, shadowDSV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create shadow depth stencil view" << std::endl;
		return false;
	}
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = 1;
	ret = Graphics::device->CreateShaderResourceView(
		Graphics::shadowBuffer.Get(), &srvDesc, shadowSRV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create shader resource view from shadow srv" << std::endl;
		return false;
	}

	return true;
}

bool Graphics::InitShaderResourceBuffers()
{
	// Asset Files
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	if (!DXUtils::CreateTextureArrayFromAtlasFile(
			atlasMapBuffer, atlasMapSRV, "../assets/blockatlas1.png", format)) {
		std::cout << "failed create texture from atlas file" << std::endl;
		return false;
	}

	/*if (!DXUtils::CreateTextureFromFile(
			grassColorMapBuffer, grassColorMapSRV, "../assets/grass_color_map.png", format)) {
		std::cout << "failed create texture from grass color map file" << std::endl;
		return false;
	}*/

	if (!DXUtils::CreateTexture2DFromFile(sunBuffer, sunSRV, "../assets/sun.png", format)) {
		std::cout << "failed create texture from sun file" << std::endl;
		return false;
	}

	if (!DXUtils::CreateTexture2DFromFile(moonBuffer, moonSRV, "../assets/moon.png", format)) {
		std::cout << "failed create texture from moon file" << std::endl;
		return false;
	}

	// forward render
	format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	UINT bindFlag = D3D11_BIND_SHADER_RESOURCE;
	if (!DXUtils::CreateTextureBuffer(
			copyForwardRenderBuffer, App::WIDTH, App::HEIGHT, true, format, bindFlag)) {
		std::cout << "failed create copy forward render buffer" << std::endl;
		return false;
	}
	HRESULT ret = device->CreateShaderResourceView(
		copyForwardRenderBuffer.Get(), nullptr, copyForwardSRV.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create copy forward srv" << std::endl;
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

bool Graphics::InitGeometryShaders()
{ 
	// BasicShadowGS
	if (!DXUtils::CreateGeometryShader(L"BasicShadowGS.hlsl", basicShadowGS)) {
		std::cout << "failed create basic shadow gs" << std::endl;
		return false;
	}

	return true;
}

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

	// basicMirrorPS
	if (!DXUtils::CreatePixelShader(L"BasicPS.hlsl", basicMirrorPS, nullptr, "mainMirror")) {
		std::cout << "failed create basic mirror ps" << std::endl;
		return false;
	}

	// basicMirrorAlphaClipPS
	macros.clear();
	macros.push_back({ "USE_ALPHA_CLIP", "1" });
	macros.push_back({ NULL, NULL });
	if (!DXUtils::CreatePixelShader(
			L"BasicPS.hlsl", basicMirrorAlphaClipPS, macros.data(), "mainMirror")) {
		std::cout << "failed create basic mirror alpha clip ps" << std::endl;
		return false;
	}

	// SkyboxPS
	if (!DXUtils::CreatePixelShader(L"SkyboxPS.hlsl", skyboxPS)) {
		std::cout << "failed create skybox ps" << std::endl;
		return false;
	}

	// Skybox Mirror PS
	macros.clear();
	macros.push_back({ "USE_MIRROR", "1" });
	macros.push_back({ NULL, NULL });
	if (!DXUtils::CreatePixelShader(L"SkyboxPS.hlsl", skyboxMirrorPS, macros.data())) {
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
	if (!DXUtils::CreatePixelShader(L"SsaoPS.hlsl", ssaoPS)) {
		std::cout << "failed create ssao ps" << std::endl;
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

	// ShadingBasicPS
	if (!DXUtils::CreatePixelShader(L"ShadingBasicPS.hlsl", shadingBasicPS)) {
		std::cout << "failed create shading basic ps" << std::endl;
		return false;
	}
	if (!DXUtils::CreatePixelShader(
			L"ShadingBasicPS.hlsl", shadingBasicEdgePS, nullptr, "mainMSAA")) {
		std::cout << "failed create shading basic edge ps" << std::endl;
		return false;
	}

	// combineBloomPS
	if (!DXUtils::CreatePixelShader(L"CombineBloomPS.hlsl", combineBloomPS)) {
		std::cout << "failed create combine bloom ps" << std::endl;
		return false;
	}

	// bloomDownPS
	if (!DXUtils::CreatePixelShader(L"BloomDownPS.hlsl", bloomDownPS)) {
		std::cout << "failed create bloom down ps" << std::endl;
		return false;
	}

	// bloomUpPS
	if (!DXUtils::CreatePixelShader(L"BloomUpPS.hlsl", bloomUpPS)) {
		std::cout << "failed create bloom up ps" << std::endl;
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

	// frontCullRS
	rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_FRONT;
	rastDesc.FrontCounterClockwise = false;
	ret = Graphics::device->CreateRasterizerState(&rastDesc, frontCullRS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create frontCULL RS" << std::endl;
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
	desc.BorderColor[0] = 1.0f;
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	device->CreateSamplerState(&desc, shadowPointSS.GetAddressOf());

	// shadowCompareSS
	desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.BorderColor[0] = 1.0f;
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

	// stencil mask DSS
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
	ret = Graphics::device->CreateDepthStencilState(&desc, stencilMaskDSS.GetAddressOf());
	if (FAILED(ret)) {
		std::cout << "failed create stencil mask DSS" << std::endl;
		return false;
	}

	// stencil equal draw DSS
	ZeroMemory(&desc, sizeof(desc));
	desc.DepthEnable = true;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;
	desc.StencilEnable = true;
	desc.StencilReadMask = 0xFF;
	desc.StencilWriteMask = 0xFF;
	desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;	   // stencil X
	desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP; // stencil O depth X
	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;	   // stencil O depth O
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
	basicMirrorPSO.pixelShader = basicMirrorPS;
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

	// skyboxMirrorPSO
	skyboxMirrorPSO = skyboxPSO;
	skyboxMirrorPSO.pixelShader = skyboxMirrorPS;
	skyboxMirrorPSO.depthStencilState = mirrorDrawMaskedDSS;
	skyboxMirrorPSO.stencilRef = 1;

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

	// samplingPSO
	samplingPSO = basicPSO;
	samplingPSO.inputLayout = samplingIL;
	samplingPSO.vertexShader = samplingVS;
	samplingPSO.pixelShader = samplingPS;

	// fogFilterPSO
	fogFilterPSO = samplingPSO;
	fogFilterPSO.pixelShader = fogFilterPS;

	// instancePSO
	instancePSO = basicPSO;
	instancePSO.inputLayout = instanceIL;
	instancePSO.vertexShader = instanceVS;
	instancePSO.rasterizerState = noneCullRS;
	instancePSO.pixelShader = basicAlphaClipPS;

	// instanceMirrorPSO
	instanceMirrorPSO = instancePSO;
	instanceMirrorPSO.pixelShader = basicMirrorAlphaClipPS;
	instanceMirrorPSO.depthStencilState = mirrorDrawMaskedDSS;
	instanceMirrorPSO.stencilRef = 1;

	// mirrorMaskingPSO
	mirrorMaskingPSO = basicPSO;
	mirrorMaskingPSO.pixelShader = mirrorMaskingPS;
	mirrorMaskingPSO.depthStencilState = stencilMaskDSS;
	mirrorMaskingPSO.stencilRef = 1;

	// waterPlanePSO
	waterPlanePSO = basicPSO;
	waterPlanePSO.rasterizerState = noneCullRS;
	waterPlanePSO.pixelShader = waterPlanePS;

	// waterFilterPSO
	waterFilterPSO = samplingPSO;
	waterFilterPSO.pixelShader = waterFilterPS;

	// basicshadowPSO
	basicShadowPSO = basicPSO;
	basicShadowPSO.rasterizerState = noneCullRS;
	basicShadowPSO.vertexShader = basicShadowVS;
	basicShadowPSO.geometryShader = basicShadowGS;
	basicShadowPSO.pixelShader = nullptr;

	// ssaoPSO
	ssaoPSO = samplingPSO;
	ssaoPSO.pixelShader = ssaoPS;
	ssaoPSO.depthStencilState = stencilEqualDrawDSS;
	ssaoPSO.stencilRef = 0;

	// ssaoEdgePSO
	ssaoEdgePSO = ssaoPSO;
	ssaoEdgePSO.pixelShader = ssaoEdgePS;
	ssaoEdgePSO.stencilRef = 1;

	// edgeMaskingPSO
	edgeMaskingPSO = samplingPSO;
	edgeMaskingPSO.pixelShader = edgeMaskingPS;
	edgeMaskingPSO.depthStencilState = stencilMaskDSS;
	edgeMaskingPSO.stencilRef = 1;

	// shadingBasicPSO
	shadingBasicPSO = samplingPSO;
	shadingBasicPSO.pixelShader = shadingBasicPS;
	shadingBasicPSO.depthStencilState = stencilEqualDrawDSS;
	shadingBasicPSO.stencilRef = 0;

	// shadingBasicEdgePSO
	shadingBasicEdgePSO = shadingBasicPSO;
	shadingBasicEdgePSO.pixelShader = shadingBasicEdgePS;
	shadingBasicEdgePSO.stencilRef = 1;

	// bloomDownPSO
	bloomDownPSO = samplingPSO;
	bloomDownPSO.pixelShader = bloomDownPS;

	// bloomUpPSO
	bloomUpPSO = samplingPSO;
	bloomUpPSO.pixelShader = bloomUpPS;

	// combineBloomPSO
	combineBloomPSO = samplingPSO;
	combineBloomPSO.pixelShader = combineBloomPS;
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