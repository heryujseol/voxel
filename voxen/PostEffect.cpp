#include "PostEffect.h"
#include "Graphics.h"
#include "DXUtils.h"
#include "MeshGenerator.h"
#include "App.h"

#include <algorithm>

PostEffect::PostEffect()
	: m_stride(sizeof(SamplingVertex)), m_offset(0), m_vertexBuffer(nullptr),
	  m_indexBuffer(nullptr), m_waterAdaptationTime(0.0f){};

PostEffect::~PostEffect(){};

bool PostEffect::Initialize()
{
	MeshGenerator::CreateSampleSquareMesh(m_vertices, m_indices);

	if (!DXUtils::CreateVertexBuffer(m_vertexBuffer, m_vertices)) {
		std::cout << "failed create fog vertex buffer" << std::endl;
		return false;
	}

	if (!DXUtils::CreateIndexBuffer(m_indexBuffer, m_indices)) {
		std::cout << "failed create fog index buffer" << std::endl;
		return false;
	}

	m_blurConstantData.dx = 1.0f / (float)App::MIRROR_WIDTH;
	m_blurConstantData.dy = 1.0f / (float)App::MIRROR_HEIGHT;
	if (!DXUtils::CreateConstantBuffer(m_blurConstantBuffer, m_blurConstantData)) {
		std::cout << "failed create blur constant buffer" << std::endl;
		return false;
	}

	m_fogFilterConstantData.fogDistMin = 0.0f;
	m_fogFilterConstantData.fogDistMax = 0.0f;
	m_fogFilterConstantData.fogStrength = 1.0f;
	m_fogFilterConstantData.fogColor = Vector3(0.0f);
	if (!DXUtils::CreateConstantBuffer(m_fogFilterConstantBuffer, m_fogFilterConstantData)) {
		std::cout << "failed create fog filter constant buffer" << std::endl;
		return false;
	}

	m_waterFilterConstantData.filterColor = Vector3(0.0f);
	m_waterFilterConstantData.filterStrength = 0.0f;
	if (!DXUtils::CreateConstantBuffer(m_waterFilterConstantBuffer, m_waterFilterConstantData)) {
		std::cout << "failed create water filter constant buffer" << std::endl;
		return false;
	}

	return true;
}

void PostEffect::Update(float dt, Camera& camera)
{
	if (camera.IsUnderWater()) {
		m_waterAdaptationTime += dt;
		m_waterAdaptationTime = min(2.0f, m_waterAdaptationTime);

		m_waterFilterConstantData.filterColor.x = 0.15f * 0.5f * m_waterAdaptationTime;
		m_waterFilterConstantData.filterColor.y = 0.25f * 0.5f * m_waterAdaptationTime;
		m_waterFilterConstantData.filterColor.z = 0.96f * 0.5f * m_waterAdaptationTime;
		m_waterFilterConstantData.filterStrength = 0.8f - (0.4f * 0.5f * m_waterAdaptationTime);

		m_fogFilterConstantData.fogDistMin = 15.0f + (15.0f * 0.5f * m_waterAdaptationTime);
		m_fogFilterConstantData.fogDistMax = 30.0f + (90.0f * 0.5f * m_waterAdaptationTime);
		m_fogFilterConstantData.fogStrength = 5.0f - (0.5f * m_waterAdaptationTime);
		
	}
	else {
		m_waterAdaptationTime = 0.0f;

		m_fogFilterConstantData.fogDistMin = Camera::LOD_RENDER_DISTANCE;
		m_fogFilterConstantData.fogDistMax = Camera::MAX_RENDER_DISTANCE;
		m_fogFilterConstantData.fogStrength = 3.0f;
	}
	DXUtils::UpdateConstantBuffer(m_fogFilterConstantBuffer, m_fogFilterConstantData);
	DXUtils::UpdateConstantBuffer(m_waterFilterConstantBuffer, m_waterFilterConstantData);
}

void PostEffect::Render()
{
	Graphics::context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	Graphics::context->IASetVertexBuffers(
		0, 1, m_vertexBuffer.GetAddressOf(), &m_stride, &m_offset);

	Graphics::context->DrawIndexed((UINT)m_indices.size(), 0, 0);
}