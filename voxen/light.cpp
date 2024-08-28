#include "Light.h"
#include "DXUtils.h"
#include "MeshGenerator.h"
#include "App.h"

#include <algorithm>

using namespace DirectX::SimpleMath;

Light::Light()
	: m_dir(cos(Utils::PI / 4.0f), 0.0f, cos(Utils::PI / 4.0f)), m_scale(0.0f),
	  m_radianceColor(1.0f), m_radianceWeight(1.0f), m_lightConstantBuffer(nullptr),
	  m_shadowConstantBuffer(nullptr)
{
}

Light::~Light() {}

bool Light::Initialize()
{
	if (!DXUtils::CreateConstantBuffer(m_lightConstantBuffer, m_lightConstantData)) {
		std::cout << "failed create constant buffer in light" << std::endl;
		return false;
	}

	if (!DXUtils::CreateConstantBuffer(m_shadowConstantBuffer, m_shadowConstantData)) {
		std::cout << "failed create constant buffer in shadow" << std::endl;
		return false;
	}

	return true;
}

void Light::Update(UINT dateTime, Camera& camera)
{
	const float MAX_RADIANCE_WEIGHT = 1.5;
	float angle = (float)dateTime / App::DAY_CYCLE_AMOUNT * 2.0f * Utils::PI;

	// light
	{
		Matrix rotationAxisMatrix = Matrix::CreateFromAxisAngle(
			Vector3(-cos(Utils::PI / 4.0f), 0.0f, cos(Utils::PI / 4.0f)), angle);

		m_dir = Vector3::Transform(
			Vector3(cos(Utils::PI / 4.0f), 0.0f, cos(Utils::PI / 4.0f)), rotationAxisMatrix);
		m_dir.Normalize();

		m_up = XMVector3TransformNormal(Vector3(0.0f, 1.0f, 0.0f), rotationAxisMatrix);

		// radiance
		if (dateTime < App::DAY_START)
			dateTime += App::DAY_CYCLE_AMOUNT;

		if (App::DAY_START <= dateTime && dateTime < App::DAY_END) {
			m_radianceWeight = MAX_RADIANCE_WEIGHT;
			m_radianceColor = RADIANCE_DAY_COLOR;
		}
		else if (App::DAY_END <= dateTime && dateTime < App::NIGHT_START) {
			float radianceWeightFactor =
				(float)(dateTime - App::DAY_END) / (App::NIGHT_START - App::DAY_END);

			m_radianceWeight =
				Utils::Smootherstep(0.0f, MAX_RADIANCE_WEIGHT, 1.0f - radianceWeightFactor);

			if (App::DAY_END <= dateTime && dateTime < App::MAX_SUNSET) {
				float radianceColorFactor =
					(float)(dateTime - App::DAY_END) / (App::MAX_SUNSET - App::DAY_END);

				m_radianceColor =
					Utils::Lerp(RADIANCE_DAY_COLOR, RADIANCE_SUNSET_COLOR, radianceColorFactor);
			}

			if (App::MAX_SUNSET <= dateTime && dateTime < App::NIGHT_START) {
				float radianceColorFactor =
					(float)(dateTime - App::MAX_SUNSET) / (App::NIGHT_START - App::MAX_SUNSET);

				m_radianceColor =
					Utils::Lerp(RADIANCE_SUNSET_COLOR, RADIANCE_NIGHT_COLOR, radianceColorFactor);
			}
		}
		else if (App::NIGHT_START <= dateTime && dateTime < App::NIGHT_END) {
			m_radianceWeight = 0.0f;
			m_radianceColor = RADIANCE_NIGHT_COLOR;
		}
		else if (App::NIGHT_END <= dateTime && dateTime <= App::DAY_START + App::DAY_CYCLE_AMOUNT) {
			float radianceWeightFactor = (float)(dateTime - App::NIGHT_END) /
										 (App::DAY_START + App::DAY_CYCLE_AMOUNT - App::NIGHT_END);

			m_radianceWeight = Utils::Smootherstep(0.0f, MAX_RADIANCE_WEIGHT, radianceWeightFactor);

			if (App::NIGHT_END <= dateTime && dateTime < App::MAX_SUNRISE) {
				float radianceColorFactor =
					(float)(dateTime - App::NIGHT_END) / (App::MAX_SUNRISE - App::NIGHT_END);

				m_radianceColor =
					Utils::Lerp(RADIANCE_NIGHT_COLOR, RADIANCE_SUNRISE_COLOR, radianceColorFactor);
			}

			if (App::MAX_SUNRISE <= dateTime && dateTime < App::DAY_START + App::DAY_CYCLE_AMOUNT) {
				float radianceColorFactor =
					(float)(dateTime - App::MAX_SUNRISE) /
					(App::DAY_START + App::DAY_CYCLE_AMOUNT - App::MAX_SUNRISE);

				m_radianceColor =
					Utils::Lerp(RADIANCE_SUNRISE_COLOR, RADIANCE_DAY_COLOR, radianceColorFactor);
			}
		}

		m_lightConstantData.lightDir = m_dir;
		m_lightConstantData.radianceWeight = m_radianceWeight;
		m_lightConstantData.radianceColor = m_radianceColor;
		m_lightConstantData.maxRadianceWeight = MAX_RADIANCE_WEIGHT;

		DXUtils::UpdateConstantBuffer(m_lightConstantBuffer, m_lightConstantData);
	}


	// shadow
	{
		float cascade[CASCADE_NUM + 1] = { 0.0f, 0.01f, 0.03f, 0.1f, 0.3f };
		float topLX[CASCADE_NUM] = { 0.0f, 1024.0f, 1536.0f, 1792.0f };
		float viewportWidth[CASCADE_NUM] = { 1024.0f, 512.0f, 256.0f, 128.0f };

		if (angle >= Utils::PI / 2.0f) {
			m_up *= -1;
		}

		Vector3 frustum[8]{ { -1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f }, { 1.0f, -1.0f, 0.0f },
			{ -1.0f, -1.0f, 0.0f },

			{ -1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, -1.0f, 1.0f },
			{ -1.0f, -1.0f, 1.0f } };

		Matrix toWorld = (camera.GetViewMatrix() * camera.GetProjectionMatrix()).Invert();

		for (auto& v : frustum)
			v = Vector3::Transform(v, toWorld);

		for (int i = 0; i < CASCADE_NUM; ++i) {
			Vector3 tFrustum[8];

			for (int j = 0; j < 4; ++j) {
				Vector3 v = frustum[j + 4] - frustum[j];

				Vector3 n = v * cascade[i];
				Vector3 f = v * cascade[i + 1];

				tFrustum[j] = frustum[j] + n;
				tFrustum[j + 4] = frustum[j] + f;
			}

			Vector3 center = Vector3(0.0f);
			for (auto& v : tFrustum)
				center += v;
			center *= 1.0f / 8.0f;

			float radius = 0.0f;
			for (auto& v : tFrustum)
				radius = max(radius, (v - center).Length());

			radius = std::ceil(radius * 16.0f) / 16.0f;
			
			Vector3 sunPos = center + (m_dir * radius * 1.0f);

			float farZ = (sunPos - center).Length() + radius;

			m_view[i] = XMMatrixLookAtLH(sunPos, center, m_up);
			m_proj[i] = XMMatrixOrthographicLH(radius * 2.0f, radius * 2.0f, 0.0f, farZ);
			Matrix viewProj = m_view[i] * m_proj[i];

			m_shadowConstantData.viewProj[i] = viewProj.Transpose();
			m_shadowConstantData.topLX[i] = topLX[i];
			m_shadowConstantData.viewWidth[i] = viewportWidth[i];

			DXUtils::UpdateViewport(m_shadowViewPorts[i], m_shadowConstantData.topLX[i], 0.0f,
				m_shadowConstantData.viewWidth[i], m_shadowConstantData.viewWidth[i]);
		}
		std::cout << std::endl;
		DXUtils::UpdateConstantBuffer(m_shadowConstantBuffer, m_shadowConstantData);
	}
}