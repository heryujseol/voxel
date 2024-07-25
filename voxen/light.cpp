#include "Light.h"
#include "DXUtils.h"
#include "MeshGenerator.h"
#include "App.h"

#include <algorithm>

using namespace DirectX::SimpleMath;

Light::Light()
	: m_dir(1.0f, 0.0f, 0.0f), m_scale(0.0f), m_radianceColor(1.0f), m_radianceWeight(1.0f),
	  m_stride(sizeof(LightMeshVertex)), m_offset(0), m_vertexBuffer(nullptr),
	  m_indexBuffer(nullptr), m_lightConstantBuffer(nullptr)
{
}

Light::~Light() {}

bool Light::Initialize()
{
	/*
	// Mesh Create
	// MeshGenerator::CreateLightMeshGenerator();

	if (!DXUtils::CreateVertexBuffer(m_vertexBuffer, m_vertices)) {
		std::cout << "failed create vertex buffer in light" << std::endl;
		return false;
	}

	if (!DXUtils::CreateIndexBuffer(m_indexBuffer, m_indices)) {
		std::cout << "failed create index buffer in light" << std::endl;
		return false;
	}
	*/
	if (!DXUtils::CreateConstantBuffer(m_lightConstantBuffer, m_lightConstantData)) {
		std::cout << "failed create constant buffer in light" << std::endl;
		return false;
	}
	
	return true;
}

void Light::Update(UINT dateTime)
{
	std::cout << dateTime << std::endl;
	// m_dir
	float angle = (float)dateTime / App::DAY_CYCLE_AMOUNT * 2.0f * Utils::PI;
	m_dir = Vector3::Transform(Vector3(1.0f, 0.0f, 0.0f), Matrix::CreateRotationZ(angle));
	m_dir.Normalize();

	// radiance
	if (dateTime < App::DAY_START)
		dateTime += App::DAY_CYCLE_AMOUNT;

	if (App::DAY_START <= dateTime && dateTime < App::DAY_END) {
		m_radianceWeight = 1.0f;
	}
	else if (App::DAY_END <= dateTime && dateTime < App::NIGHT_START) {
		float w = (float)(dateTime - App::DAY_END) / (App::NIGHT_START - App::DAY_END);

		m_radianceWeight = Utils::Smootherstep(0.0f, 1.0f, 1.0f - w);
	}
	else if (App::NIGHT_START <= dateTime && dateTime < App::NIGHT_END) {
		m_radianceWeight = 0.0f;
	}
	else if (App::NIGHT_END <= dateTime && dateTime <= App::DAY_START + App::DAY_CYCLE_AMOUNT) {
		float w = (float)(dateTime - App::NIGHT_END) /
				  (App::DAY_START + App::DAY_CYCLE_AMOUNT - App::NIGHT_END);

		m_radianceWeight = Utils::Smootherstep(0.0f, 1.0f, w);
	}

	m_lightConstantData.lightDir = m_dir;
	m_lightConstantData.lightScale = m_scale;
	m_lightConstantData.radianceColor = m_radianceColor;
	m_lightConstantData.radianceWeight = m_radianceWeight;
	DXUtils::UpdateConstantBuffer(m_lightConstantBuffer, m_lightConstantData);
}