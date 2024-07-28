#include "Light.h"
#include "DXUtils.h"
#include "MeshGenerator.h"
#include "App.h"

#include <algorithm>

using namespace DirectX::SimpleMath;

Light::Light()
	: m_dir(1.0f, 0.0f, 0.0f), m_scale(0.0f), m_radianceColor(1.0f), m_radianceWeight(1.0f),
	  m_lightConstantBuffer(nullptr)
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
	const float MAX_RADIANCE_WEIGHT = 2.0;

	// m_dir
	float angle = (float)dateTime / App::DAY_CYCLE_AMOUNT * 2.0f * Utils::PI;
	m_dir = Vector3::Transform(Vector3(1.0f, 0.0f, 0.0f), Matrix::CreateRotationZ(angle));
	m_dir.Normalize();

	// radiance
	if (dateTime < App::DAY_START)
		dateTime += App::DAY_CYCLE_AMOUNT;

	if (App::DAY_START <= dateTime && dateTime < App::DAY_END) {
		m_radianceWeight = MAX_RADIANCE_WEIGHT;
	}
	else if (App::DAY_END <= dateTime && dateTime < App::NIGHT_START) {
		float w = (float)(dateTime - App::DAY_END) / (App::NIGHT_START - App::DAY_END);

		m_radianceWeight = Utils::Smootherstep(0.0f, MAX_RADIANCE_WEIGHT, 1.0f - w);
	}
	else if (App::NIGHT_START <= dateTime && dateTime < App::NIGHT_END) {
		m_radianceWeight = 0.0f;
	}
	else if (App::NIGHT_END <= dateTime && dateTime <= App::DAY_START + App::DAY_CYCLE_AMOUNT) {
		float w = (float)(dateTime - App::NIGHT_END) /
				  (App::DAY_START + App::DAY_CYCLE_AMOUNT - App::NIGHT_END);

		m_radianceWeight = Utils::Smootherstep(0.0f, MAX_RADIANCE_WEIGHT, w);
	}

	m_lightConstantData.lightDir = m_dir;
	m_lightConstantData.radianceWeight = m_radianceWeight;
	m_lightConstantData.radianceColor = m_radianceColor;
	m_lightConstantData.maxRadianceWeight = MAX_RADIANCE_WEIGHT;
	
	DXUtils::UpdateConstantBuffer(m_lightConstantBuffer, m_lightConstantData);
}