#include "Light.h"
#include "DXUtils.h"
#include "MeshGenerator.h"
#include "App.h"

#include <algorithm>

using namespace DirectX::SimpleMath;

Light::Light()
	: m_dir(cos(Utils::PI / 4.0f), 0.0f, cos(Utils::PI / 4.0f)), m_scale(0.0f), m_radianceColor(1.0f),
	  m_radianceWeight(1.0f), m_lightConstantBuffer(nullptr)
{
}

Light::~Light() {}

bool Light::Initialize()
{
	if (!DXUtils::CreateConstantBuffer(m_lightConstantBuffer, m_lightConstantData)) {
		std::cout << "failed create constant buffer in light" << std::endl;
		return false;
	}

	return true;
}

void Light::Update(UINT dateTime)
{
	const float MAX_RADIANCE_WEIGHT = 1.5;

	// m_dir
	float angle = (float)dateTime / App::DAY_CYCLE_AMOUNT * 2.0f * Utils::PI;
	m_dir = Vector3::Transform(Vector3(cos(Utils::PI / 4.0f), 0.0f, cos(Utils::PI / 4.0f)),
		Matrix::CreateFromAxisAngle(Vector3(-cos(Utils::PI / 4.0f), 0.0f, cos(Utils::PI / 4.0f)), angle));
	m_dir.Normalize();

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
			float radianceColorFactor = (float)(dateTime - App::MAX_SUNRISE) /
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