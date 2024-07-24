#include "PostEffect.h"
#include "Graphics.h"
#include "DXUtils.h"
#include "MeshGenerator.h"
#include "App.h"
#include "Utils.h"

#include <algorithm>
#include <random>

PostEffect::PostEffect()
	: m_fogFilterConstantBuffer(nullptr), m_waterFilterConstantBuffer(nullptr),
	  m_ssaoConstantBuffer(nullptr), m_stride(sizeof(SamplingVertex)), m_offset(0),
	  m_vertexBuffer(nullptr), m_indexBuffer(nullptr), m_waterAdaptationTime(0.0f),
	  m_waterMaxDuration(2.5f){};

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

	std::uniform_real_distribution<float> randomFloats(0.0001f, 1.0f);
	std::default_random_engine generator;
	for (int i = 0; i < 64; ++i) {
		Vector4 sampleKernel;

		sampleKernel.x = randomFloats(generator) * 2.0f - 1.0f;
		sampleKernel.y = randomFloats(generator) * 2.0f - 1.0f;
		sampleKernel.z = randomFloats(generator);
		sampleKernel.w = 0.0f;

		sampleKernel.Normalize();
		sampleKernel *= randomFloats(generator);

		float scale = (float)i / 64;
		sampleKernel *= Utils::Lerp(0.1f, 1.0f, scale * scale);

		m_ssaoConstantData.sampleKernel[i] = sampleKernel;
	}
	if (!DXUtils::CreateConstantBuffer(m_ssaoConstantBuffer, m_ssaoConstantData)) {
		std::cout << "failed create ssao constant buffer" << std::endl;
		return false;
	}

	for (int i = 0; i < 16; ++i) {
		Vector4 rotationNoise;

		rotationNoise.x = randomFloats(generator) * 2.0f - 1.0f;
		rotationNoise.y = randomFloats(generator) * 2.0f - 1.0f;
		rotationNoise.z = 0.0f;
		rotationNoise.w = 0.0f;

		m_ssaoNoiseConstantData.rotationNoise[i] = rotationNoise;
	}
	if (!DXUtils::CreateConstantBuffer(m_ssaoNoiseConstantBuffer, m_ssaoNoiseConstantData)) {
		std::cout << "failed create ssao noise constant buffer" << std::endl;
		return false;
	}

	return true;
}

void PostEffect::Update(float dt, Camera& camera)
{
	if (camera.IsUnderWater()) {
		m_waterAdaptationTime += dt;
		m_waterAdaptationTime = min(m_waterMaxDuration, m_waterAdaptationTime);

		float percetage = m_waterAdaptationTime / m_waterMaxDuration;

		m_waterFilterConstantData.filterColor.x = 0.075f + 0.075f * percetage;
		m_waterFilterConstantData.filterColor.y = 0.125f + 0.125f * percetage;
		m_waterFilterConstantData.filterColor.z = 0.48f + 0.48f * percetage;
		m_waterFilterConstantData.filterStrength = 0.7f - (0.3f * percetage);

		m_fogFilterConstantData.fogDistMin = 15.0f + (15.0f * percetage);
		m_fogFilterConstantData.fogDistMax = 30.0f + (90.0f * percetage);
		m_fogFilterConstantData.fogStrength = 5.0f - percetage;
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

void PostEffect::Blur(int count, ComPtr<ID3D11ShaderResourceView>& src,
	ComPtr<ID3D11RenderTargetView>& dst, ComPtr<ID3D11ShaderResourceView> blurSRV[2],
	ComPtr<ID3D11RenderTargetView> blurRTV[2], ComPtr<ID3D11PixelShader> blurPS[2])
{
	for (int i = 0; i < count; ++i) {
		Graphics::context->OMSetRenderTargets(1, blurRTV[0].GetAddressOf(), nullptr);

		if (i == 0)
			Graphics::context->PSSetShaderResources(0, 1, src.GetAddressOf());
		else
			Graphics::context->PSSetShaderResources(0, 1, blurSRV[1].GetAddressOf());

		Graphics::context->PSSetShader(blurPS[0].Get(), nullptr, 0);
		Render();

		if (i == count - 1)
			Graphics::context->OMSetRenderTargets(1, dst.GetAddressOf(), nullptr);
		else
			Graphics::context->OMSetRenderTargets(1, blurRTV[1].GetAddressOf(), nullptr);

		Graphics::context->PSSetShaderResources(0, 1, blurSRV[0].GetAddressOf());

		Graphics::context->PSSetShader(blurPS[1].Get(), nullptr, 0);
		Render();
	}
}

void PostEffect::Bloom()
{
	// bloom Down 0 -> 1 -> 2 -> 3
	{
		Graphics::SetPipelineStates(Graphics::bloomDownPSO);
		for (int i = 0; i <= 3; ++i) {
			int div = (int)std::pow(2, i + 1);

			DXUtils::UpdateViewport(
				Graphics::bloomViewport, 0, 0, App::WIDTH / div, App::HEIGHT / div);
			Graphics::context->RSSetViewports(1, &Graphics::bloomViewport);

			Graphics::context->OMSetRenderTargets(
				1, Graphics::bloomRTV[i + 1].GetAddressOf(), nullptr);

			Graphics::context->PSSetShaderResources(0, 1, Graphics::bloomSRV[i].GetAddressOf());

			Render();
		}
	}

	// bloom Up
	{
		Graphics::SetPipelineStates(Graphics::bloomUpPSO);
		for (int i = 3; i >= 0; --i) {
			int div = (int)std::pow(2, i);

			DXUtils::UpdateViewport(
				Graphics::bloomViewport, 0, 0, App::WIDTH / div, App::HEIGHT / div);
			Graphics::context->RSSetViewports(1, &Graphics::bloomViewport);

			Graphics::context->OMSetRenderTargets(1, Graphics::bloomRTV[i].GetAddressOf(), nullptr);

			Graphics::context->PSSetShaderResources(0, 1, Graphics::bloomSRV[i + 1].GetAddressOf());

			Render();
		}
	}

	// Combine & Tonemapping
	{
		Graphics::context->OMSetRenderTargets(1, Graphics::backBufferRTV.GetAddressOf(), nullptr);

		ID3D11ShaderResourceView* ppSRVs[2] = { Graphics::basicSRV.Get(),
			Graphics::bloomSRV[0].Get() };
		Graphics::context->PSSetShaderResources(0, 2, ppSRVs);

		Graphics::SetPipelineStates(Graphics::combineBloomPSO);
		Render();
	}
}