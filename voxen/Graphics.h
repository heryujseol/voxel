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
	extern ComPtr<ID3D11VertexShader> cloudVS;
	extern ComPtr<ID3D11VertexShader> samplingVS;
	extern ComPtr<ID3D11VertexShader> instanceVS;
	extern ComPtr<ID3D11VertexShader> basicShadowVS;


	// Geometry Shader
	extern ComPtr<ID3D11GeometryShader> basicShadowGS;


	// Pixel Shader
	extern ComPtr<ID3D11PixelShader> basicPS;
	extern ComPtr<ID3D11PixelShader> basicAlphaClipPS;
	extern ComPtr<ID3D11PixelShader> basicDepthClipPS;
	extern ComPtr<ID3D11PixelShader> basicAlphaDepthClipPS;
	extern ComPtr<ID3D11PixelShader> skyboxPS;
	extern ComPtr<ID3D11PixelShader> cloudPS;
	extern ComPtr<ID3D11PixelShader> samplingPS;
	extern ComPtr<ID3D11PixelShader> fogFilterPS;
	extern ComPtr<ID3D11PixelShader> mirrorMaskingPS;
	extern ComPtr<ID3D11PixelShader> waterPlanePS;
	extern ComPtr<ID3D11PixelShader> waterFilterPS;
	extern ComPtr<ID3D11PixelShader> blurMirrorPS[2];
	extern ComPtr<ID3D11PixelShader> blurSsaoPS[2];
	extern ComPtr<ID3D11PixelShader> ssaoNormalPS;
	extern ComPtr<ID3D11PixelShader> ssaoEdgePS;
	extern ComPtr<ID3D11PixelShader> edgeMaskingPS;


	// Rasterizer State
	extern ComPtr<ID3D11RasterizerState> solidRS;
	extern ComPtr<ID3D11RasterizerState> wireRS;
	extern ComPtr<ID3D11RasterizerState> noneCullRS;
	extern ComPtr<ID3D11RasterizerState> mirrorRS;

	// Sampler State
	extern ComPtr<ID3D11SamplerState> pointWrapSS;
	extern ComPtr<ID3D11SamplerState> linearWrapSS;
	extern ComPtr<ID3D11SamplerState> linearClampSS;
	extern ComPtr<ID3D11SamplerState> shadowPointSS;
	extern ComPtr<ID3D11SamplerState> shadowCompareSS;
	extern ComPtr<ID3D11SamplerState> pointClampSS;



	// Depth Stencil State
	extern ComPtr<ID3D11DepthStencilState> basicDSS;
	extern ComPtr<ID3D11DepthStencilState> edgeMaskingDSS;
	extern ComPtr<ID3D11DepthStencilState> stencilEqualDrawDSS;
	extern ComPtr<ID3D11DepthStencilState> mirrorMaskingDSS;
	extern ComPtr<ID3D11DepthStencilState> mirrorDrawMaskedDSS;

	
	// Blend State
	extern ComPtr<ID3D11BlendState> alphaBS;


	// Render Target Buffer
	extern ComPtr<ID3D11Texture2D> backBuffer;
	extern ComPtr<ID3D11RenderTargetView> backBufferRTV;

	extern ComPtr<ID3D11Texture2D> basicRenderBuffer;
	extern ComPtr<ID3D11RenderTargetView> basicRTV;
	extern ComPtr<ID3D11ShaderResourceView> basicSRV;

	extern ComPtr<ID3D11Texture2D> normalBuffer;
	extern ComPtr<ID3D11RenderTargetView> normalRTV;
	extern ComPtr<ID3D11ShaderResourceView> normalSRV;

	extern ComPtr<ID3D11Texture2D> positionBuffer;
	extern ComPtr<ID3D11RenderTargetView> positionRTV;
	extern ComPtr<ID3D11ShaderResourceView> positionSRV;

	extern ComPtr<ID3D11Texture2D> albedoEdgeBuffer;
	extern ComPtr<ID3D11RenderTargetView> albedoEdgeRTV;
	extern ComPtr<ID3D11ShaderResourceView> albedoEdgeSRV;

	extern ComPtr<ID3D11Texture2D> coverageBuffer;
	extern ComPtr<ID3D11RenderTargetView> coverageRTV;
	extern ComPtr<ID3D11ShaderResourceView> coverageSRV;

	extern ComPtr<ID3D11Texture2D> resolvedEdgeBuffer;
	extern ComPtr<ID3D11ShaderResourceView> resolvedEdgeSRV;

	extern ComPtr<ID3D11Texture2D> ssaoBuffer;
	extern ComPtr<ID3D11RenderTargetView> ssaoRTV;
	extern ComPtr<ID3D11ShaderResourceView> ssaoSRV;

	extern ComPtr<ID3D11Texture2D> ssaoBlurBuffer[2];
	extern ComPtr<ID3D11RenderTargetView> ssaoBlurRTV[2];
	extern ComPtr<ID3D11ShaderResourceView> ssaoBlurSRV[2];


	// Depth Stencil Buffer
	extern ComPtr<ID3D11Texture2D> basicDepthBuffer;
	extern ComPtr<ID3D11DepthStencilView> basicDSV;

	extern ComPtr<ID3D11Texture2D> deferredDepthBuffer;
	extern ComPtr<ID3D11DepthStencilView> deferredDSV;


	// Shadow Resource Buffer
	extern ComPtr<ID3D11Texture2D> atlasMapBuffer;
	extern ComPtr<ID3D11ShaderResourceView> atlasMapSRV;

	extern ComPtr<ID3D11Texture2D> grassColorMapBuffer;
	extern ComPtr<ID3D11ShaderResourceView> grassColorMapSRV;

	extern ComPtr<ID3D11Texture2D> sunBuffer;
	extern ComPtr<ID3D11ShaderResourceView> sunSRV;

	extern ComPtr<ID3D11Texture2D> moonBuffer;
	extern ComPtr<ID3D11ShaderResourceView> moonSRV;

	

	// Viewport
	extern D3D11_VIEWPORT basicViewport;
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
	extern GraphicsPSO basicMirrorPSO;
	extern GraphicsPSO semiAlphaPSO;
	extern GraphicsPSO skyboxPSO;
	extern GraphicsPSO cloudPSO;
	extern GraphicsPSO cloudMirrorPSO;
	extern GraphicsPSO fogFilterPSO;
	extern GraphicsPSO instancePSO;
	extern GraphicsPSO instanceMirrorPSO;
	extern GraphicsPSO mirrorMaskingPSO;
	extern GraphicsPSO blurPSO;
	extern GraphicsPSO waterPlanePSO;
	extern GraphicsPSO waterFilterPSO;
	extern GraphicsPSO basicDepthPSO;
	extern GraphicsPSO instanceDepthPSO;
	extern GraphicsPSO basicShadowPSO;
	extern GraphicsPSO ssaoNormalPSO;
	extern GraphicsPSO ssaoEdgePSO;
	extern GraphicsPSO edgeMaskingPSO;
}
