#pragma once

#include "Structure.h"
#include "Camera.h"

#include <d3d11.h>
#include <wrl.h>
#include <vector>

using namespace Microsoft::WRL;

class Light {
public:
	static const int CASCADE_NUM = 4;

	Light();
	~Light();

	bool Initialize();
	void Update(float dt, Camera& camera);

	ComPtr<ID3D11Buffer> m_constantBuffer;
	//D3D11_VIEWPORT m_viewPorts[4];

private:

	uint32_t m_dateTime;
	Vector3 m_up;
	Vector3 m_dir;
	Vector4 m_lightPos;

	const uint32_t DATE_CYCLE_AMOUNT = 24000;
	const uint32_t DATE_REAL_TIME = 30; // 60
	const float DATE_TIME_SPEED = (float)DATE_CYCLE_AMOUNT / DATE_REAL_TIME;

	
	LightConstantData m_constantData;
	
};