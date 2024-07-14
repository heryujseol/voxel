#pragma once

#include <d3d11.h>
#include <vector>
#include <wrl.h>

#include "Structure.h"
#include "Utils.h"
#include "Camera.h"

using namespace Microsoft::WRL;

class PostEffect {
public:
	PostEffect();
	~PostEffect();

	bool Initialize();
	void Update(float dt, Camera& camera);
	void Render();

	ComPtr<ID3D11Buffer> m_blurConstantBuffer;
	ComPtr<ID3D11Buffer> m_fogFilterConstantBuffer;
	ComPtr<ID3D11Buffer> m_waterFilterConstantBuffer;

	BlurConstantData m_blurConstantData;
	FogFilterConstantData m_fogFilterConstantData;
	WaterFilterConstantData m_waterFilterConstantData;

private:
	std::vector<SamplingVertex> m_vertices;
	std::vector<uint32_t> m_indices;
	UINT m_stride;
	UINT m_offset;
	ComPtr<ID3D11Buffer> m_vertexBuffer;
	ComPtr<ID3D11Buffer> m_indexBuffer;
};