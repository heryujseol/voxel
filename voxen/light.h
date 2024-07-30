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
	void Update(UINT dateTime, Vector3 cameraPos);

	inline float GetRadianceWeight() const { return m_radianceWeight; }

	ComPtr<ID3D11Buffer> m_lightConstantBuffer;
	LightConstantData m_lightConstantData;

	ComPtr<ID3D11Buffer> m_shadowConstantBuffer;
	ShadowConstantData m_shadowConstantData;

	D3D11_VIEWPORT m_shadowViewPorts[3];

	inline Matrix GetViewMatrix(int i) { return m_view[i]; };
	inline Matrix GetProjectionMatrix(int i) { return m_proj[i]; };

private:
	Vector3 m_dir;
	float m_scale;
	Vector3 m_radianceColor;
	float m_radianceWeight;

	Vector3 m_up;
	Vector3 m_sunPos;

	Matrix m_view[3];
	Matrix m_proj[3];

	static const int CASCADE_NUM = 3;
	const Vector3 RADIANCE_DAY_COLOR = Vector3(1.0f, 1.0f, 1.0f);
	const Vector3 RADIANCE_SUNRISE_COLOR = Vector3(0.72f, 0.60f, 0.34f);
	const Vector3 RADIANCE_SUNSET_COLOR = Vector3(0.64f, 0.26f, 0.04f);
	const Vector3 RADIANCE_NIGHT_COLOR = Vector3(0.0f, 0.0f, 0.0f);
};