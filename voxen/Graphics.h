#pragma once

#include <d3d11.h>
#include <wrl.h>

#include "GraphicsPSO.h"

using namespace Microsoft::WRL;

namespace Graphics {
	// Graphics Core
	extern ComPtr<ID3D11Device> device;
	extern ComPtr<ID3D11DeviceContext> context;
	extern ComPtr<IDXGISwapChain> swapChain;


	// Input Layout
	extern ComPtr<ID3D11InputLayout> basicIL;
	extern ComPtr<ID3D11InputLayout> skyboxIL;
	extern ComPtr<ID3D11InputLayout> cloudIL;
	extern ComPtr<ID3D11InputLayout> samplingIL;
	extern ComPtr<ID3D11InputLayout> instanceIL;


	// Vertex Shader
	extern ComPtr<ID3D11VertexShader> basicVS;
	extern ComPtr<ID3D11VertexShader> skyboxVS;
	extern ComPtr<ID3D11VertexShader> skyboxEnvMapVS;
	extern ComPtr<ID3D11VertexShader> cloudVS;
	extern ComPtr<ID3D11VertexShader> samplingVS;
	extern ComPtr<ID3D11VertexShader> instanceVS;


	// Geometry Shader
	extern ComPtr<ID3D11GeometryShader> skyoxEnvMapGS;


	// Pixel Shader
	extern ComPtr<ID3D11PixelShader> basicPS;
	extern ComPtr<ID3D11PixelShader> basicAlphaClipPS;
	extern ComPtr<ID3D11PixelShader> basicDepthClipPS;
	extern ComPtr<ID3D11PixelShader> skyboxPS;
	extern ComPtr<ID3D11PixelShader> skyboxEnvMapPS;
	extern ComPtr<ID3D11PixelShader> cloudPS;
	extern ComPtr<ID3D11PixelShader> samplingPS;
	extern ComPtr<ID3D11PixelShader> instancePS;
	extern ComPtr<ID3D11PixelShader> instanceDepthClipPS;
	extern ComPtr<ID3D11PixelShader> fogPS;
	extern ComPtr<ID3D11PixelShader> mirrorMaskingPS;
	extern ComPtr<ID3D11PixelShader> mirrorBlendingPS;
	extern ComPtr<ID3D11PixelShader> blurXPS;
	extern ComPtr<ID3D11PixelShader> blurYPS;


	// Rasterizer State
	extern ComPtr<ID3D11RasterizerState> solidRS;
	extern ComPtr<ID3D11RasterizerState> wireRS;
	extern ComPtr<ID3D11RasterizerState> noneCullRS;
	extern ComPtr<ID3D11RasterizerState> mirrorRS;

	// Sampler State
	extern ComPtr<ID3D11SamplerState> pointWrapSS;
	extern ComPtr<ID3D11SamplerState> linearWrapSS;
	extern ComPtr<ID3D11SamplerState> linearClampSS;


	// Depth Stencil State
	extern ComPtr<ID3D11DepthStencilState> basicDSS;
	extern ComPtr<ID3D11DepthStencilState> basicMaskingDSS;
	extern ComPtr<ID3D11DepthStencilState> basicMaskingExceptDepthDSS;
	extern ComPtr<ID3D11DepthStencilState> mirrorDrawMaskedDSS;
	extern ComPtr<ID3D11DepthStencilState> postDrawMaskedDSS;

	
	// Blend State
	extern ComPtr<ID3D11BlendState> alphaBS;


	// RTV & Buffer
	extern ComPtr<ID3D11Texture2D> backBuffer;
	extern ComPtr<ID3D11RenderTargetView> backBufferRTV;

	extern ComPtr<ID3D11Texture2D> basicRenderBuffer;
	extern ComPtr<ID3D11RenderTargetView> basicRTV;

	extern ComPtr<ID3D11Texture2D> backgroundRenderBuffer;
	extern ComPtr<ID3D11RenderTargetView> backgroundRTV;

	extern ComPtr<ID3D11Texture2D> envMapRenderBuffer;
	extern ComPtr<ID3D11RenderTargetView> envMapRTV;

	extern ComPtr<ID3D11Texture2D> mirrorWorldRenderBuffer;
	extern ComPtr<ID3D11RenderTargetView> mirrorWorldRTV;
	
	extern ComPtr<ID3D11Texture2D> mirrorWorldBlurBuffer[2];
	extern ComPtr<ID3D11RenderTargetView> mirrorWorldBlurRTV[2];
	

	// DSV & Buffer
	extern ComPtr<ID3D11Texture2D> basicDepthBuffer;
	extern ComPtr<ID3D11DepthStencilView> basicDSV;

	extern ComPtr<ID3D11Texture2D> backgroundDepthBuffer;
	extern ComPtr<ID3D11DepthStencilView> backgroundDSV;

	extern ComPtr<ID3D11Texture2D> depthOnlyBuffer;
	extern ComPtr<ID3D11DepthStencilView> depthOnlyDSV;

	extern ComPtr<ID3D11Texture2D> envMapDepthBuffer;
	extern ComPtr<ID3D11DepthStencilView> envMapDSV;

	extern ComPtr<ID3D11Texture2D> mirrorPlaneDepthBuffer;
	extern ComPtr<ID3D11DepthStencilView> mirrorPlaneDepthDSV;

	extern ComPtr<ID3D11Texture2D> mirrorWorldDepthBuffer;
	extern ComPtr<ID3D11DepthStencilView> mirrorWorldDSV;


	// SRV & Buffer
	extern ComPtr<ID3D11Texture2D> atlasMapBuffer;
	extern ComPtr<ID3D11ShaderResourceView> atlasMapSRV;

	extern ComPtr<ID3D11Texture2D> grassColorMapBuffer;
	extern ComPtr<ID3D11ShaderResourceView> grassColorMapSRV;

	extern ComPtr<ID3D11Texture2D> sunBuffer;
	extern ComPtr<ID3D11ShaderResourceView> sunSRV;
	extern ComPtr<ID3D11Texture2D> moonBuffer;
	extern ComPtr<ID3D11ShaderResourceView> moonSRV;

	extern ComPtr<ID3D11Texture2D> cloudResolvedBuffer;
	extern ComPtr<ID3D11ShaderResourceView> cloudSRV;

	extern ComPtr<ID3D11ShaderResourceView> depthOnlySRV;

	extern ComPtr<ID3D11Texture2D> copiedDepthOnlyBuffer;
	extern ComPtr<ID3D11ShaderResourceView> copiedDepthOnlySRV;

	extern ComPtr<ID3D11Texture2D> basicResolvedBuffer;
	extern ComPtr<ID3D11ShaderResourceView> basicResolvedSRV;

	extern ComPtr<ID3D11ShaderResourceView> envMapSRV;

	extern ComPtr<ID3D11ShaderResourceView> mirrorWorldSRV;
	extern ComPtr<ID3D11ShaderResourceView> mirrorWorldBlurSRV[2];
	extern ComPtr<ID3D11ShaderResourceView> mirrorPlaneDepthSRV;


	// Viewport
	extern D3D11_VIEWPORT basicViewport;
	extern D3D11_VIEWPORT envMapViewPort;
	extern D3D11_VIEWPORT mirrorWorldViewPort;


	// device, context, swapChain
	extern bool InitGraphicsCore(DXGI_FORMAT pixelFormat, HWND& hwnd);
	

	// RTV, DSV, SRV (+ UAV ...)
	extern bool InitGraphicsBuffer();
	extern bool InitRenderTargetBuffers();
	extern bool InitDepthStencilBuffers();
	extern bool InitShaderResourceBuffers();
	

	// VS, IL, PS, RS, SS, DSS (+ HS, DS, GS, BS ...)
	extern bool InitGraphicsState();
	extern bool InitVertexShaderAndInputLayouts();
	extern bool InitGeometryShaders();
	extern bool InitPixelShaders();
	extern bool InitRasterizerStates();
	extern bool InitSamplerStates();
	extern bool InitDepthStencilStates();
	extern bool InitBlendStates();


	// PSO
	extern void InitGraphicsPSO();
	extern void SetPipelineStates(GraphicsPSO& pso);
	extern GraphicsPSO basicPSO;
	extern GraphicsPSO basicWirePSO;
	extern GraphicsPSO basicMirrorPSO;
	extern GraphicsPSO basicMaskingPSO;
	extern GraphicsPSO semiAlphaPSO;
	extern GraphicsPSO semiAlphaMaskingPSO;
	extern GraphicsPSO skyboxPSO;
	extern GraphicsPSO skyboxEnvMapPSO;
	extern GraphicsPSO cloudPSO;
	extern GraphicsPSO cloudMirrorPSO;
	extern GraphicsPSO fogPSO;
	extern GraphicsPSO instancePSO;
	extern GraphicsPSO instanceMirrorPSO;
	extern GraphicsPSO instanceMaskingPSO;
	extern GraphicsPSO mirrorDepthPSO;
	extern GraphicsPSO mirrorMaskingPSO;
	extern GraphicsPSO mirrorMaskingExceptDepthPSO;
	extern GraphicsPSO mirrorBlendPSO;
	extern GraphicsPSO mirrorBlurPSO;
}
