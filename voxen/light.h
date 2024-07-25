#pragma once

#include "Structure.h"
#include "Camera.h"

#include <d3d11.h>
#include <wrl.h>
#include <vector>

using namespace Microsoft::WRL;

class Light {

public:
	Light();
	~Light();

	bool Initialize();
	void Update(UINT dateTime);

	ComPtr<ID3D11Buffer> m_lightConstantBuffer;
	LightConstantData m_lightConstantData;

private:
	Vector3 m_dir;
	float m_scale;
	Vector3 m_radianceColor;
	float m_radianceWeight;
};