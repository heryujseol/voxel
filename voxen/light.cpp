#include "Light.h"
#include "Graphics.h"
#include "DXUtils.h"

#include <algorithm>

using namespace DirectX::SimpleMath;

Light::Light() : m_dateTime(0), m_up(0.0f, 1.0f, 0.0f), m_dir(-1.0f, 0.0f, 0.0f) {}

Light::~Light() {}

bool Light::Initialize()
{
	if (!DXUtils::CreateConstantBuffer(m_constantBuffer, m_constantData)) {
		std::cout << "failed create constant buffer in light" << std::endl;
		return false;
	}

	return true;
}

void Light::Update(float dt, Camera& camera)
{
	static float acc = 0.0f;

	// dateTime
	acc += DATE_TIME_SPEED * dt;
	m_dateTime = (uint32_t)acc;
	m_dateTime %= DATE_CYCLE_AMOUNT;

	float angle = (float)m_dateTime / DATE_CYCLE_AMOUNT * 2.0f * Utils::PI;
	
	m_dir = Vector3::Transform(Vector3(-1.0f, 0.0f, 0.0f), Matrix::CreateRotationZ(angle));
	m_up = XMVector3TransformNormal(Vector3(0.0f, 1.0f, 0.0f), Matrix::CreateRotationZ(angle));

	float cascade[CASCADE_NUM] = { 0.125f, 0.25f, 0.5f, 1.0f };
	float topLX[CASCADE_NUM] = { 0.0f, 1024.0f, 1536.0f, 1792.0f };

	for (int i = 0; i < CASCADE_NUM; i++) {
		m_lightPos = Vector4::Transform(
			Vector4(550.0f * cascade[i], 0.0f, 0.0f, 1.0f), Matrix::CreateRotationZ(angle));

		float viewWidth = 1024.0f * cascade[CASCADE_NUM - 1 - i];
		float viewHeight = viewWidth;

		Matrix viewRow = XMMatrixLookToLH(m_lightPos, m_dir, m_up);
		Matrix projRow = XMMatrixOrthographicLH(viewWidth, viewHeight, 0.1f, 3000.0f);
		//XMMatrixPerspectiveFovLH(3.141592f / 2.0f, 1.0f, 0.1f, 3000.0f);

		m_constantData.view[i] = viewRow.Transpose();
		m_constantData.proj[i] = projRow.Transpose();
		m_constantData.invProj[i] = projRow.Invert().Transpose();
		m_constantData.topLX[i] = topLX[i];
		m_constantData.viewWith[i] = viewWidth;

		DXUtils::UpdateViewport(m_viewPorts[i], topLX[i], 0.0f, viewWidth, viewHeight);
	}

	DXUtils::UpdateConstantBuffer(m_constantBuffer, m_constantData);

	//Vector3 frustumCorner[8] = {
	//	Vector3(-1.0f, 1.0f, 0.0f),
	//	Vector3(1.0f, 1.0f, 0.0f),
	//	Vector3(1.0f, -1.0f, 0.0f),
	//	Vector3(-1.0f, -1.0f, 0.0f),
	//	Vector3(-1.0f, 1.0f, 1.0f),
	//	Vector3(1.0f, 1.0f, 1.0f),
	//	Vector3(1.0f, -1.0f, 1.0f),
	//	Vector3(-1.0f, -1.0f, 1.0f),
	//};

	//Matrix invViewProj =
	//	(camera.GetViewMatrix() * camera.GetProjectionMatrix()).Invert().Transpose();

	//for (int i = 0; i < 8; ++i)
	//	frustumCorner[i].Transform(frustumCorner[i], invViewProj);

	//for (int i = 0; i < 4; ++i) {

	//	for (int j = 0; j < 4; ++j) {
	//		Vector3 cornerRay = frustumCorner[j + 4] - frustumCorner[j];
	//		Vector3 nearRay = cornerRay * cascade[i];
	//		Vector3 farRay = cornerRay * cascade[i + 1];
	//		frustumCorner[j + 4] = frustumCorner[j] + farRay;
	//		frustumCorner[j] = frustumCorner[j] + nearRay;
	//	}

	//	Vector3 center(0.0f);
	//	for (int i = 0; i < 8; ++i)
	//		center += frustumCorner[i];
	//	center *= (1.0f / 8.0f);

	//	float radius = 0.0f;
	//	for (int i = 0; i < 8; ++i) {
	//		float dist = (frustumCorner[i] - center).Length();
	//		radius = max(radius, dist);
	//	}

	//	Vector3 mins(FLT_MAX);
	//	Vector3 maxes(FLT_MIN);

	//	radius = ceil(radius * 16.0f) / 16.0f;
	//	maxes = Vector3(radius, radius, radius);
	//	mins = -maxes;

	//	Vector3 cascadeExtents = maxes - mins;

	//	Vector3 lightPos = center - m_dir * fabs(mins.z);
	//	m_constantData.lightPos[i] = lightPos;

	//	Matrix viewRow = XMMatrixLookToLH(lightPos, lightPos + m_dir, m_up);
	//	Matrix projRow = XMMatrixOrthographicLH(radius * 2.0f, radius * 2.0f, 0.0f, radius);

	//	m_constantData.view[i] = viewRow.Transpose();
	//	m_constantData.proj[i] = projRow.Transpose();
	//	m_constantData.invProj[i] = projRow.Invert().Transpose();

	//	// DXUtils::UpdateViewport(m_viewPorts[i], 0, 0, radius * 2.0f, radius * 2.0f);
	//}

	//DXUtils::UpdateConstantBuffer(m_constantBuffer, m_constantData);
}





//// Á¦ÀÏ Å« ¸Ê
	// m_constantData.lightPos[CASCADE_NUM - 1] =
	//	Vector3::Transform(Vector3(300.0f, 0.0f, 0.0f), Matrix::CreateRotationZ(angle));
	// m_dir = Vector3::Transform(Vector3(-1.0f, 0.0f, 0.0f), Matrix::CreateRotationZ(angle));
	// m_constantData.lightDir = m_dir;

	// m_up = XMVector3TransformNormal(Vector3(0.0f, 1.0f, 0.0f), Matrix::CreateRotationZ(angle));


	// Matrix viewRow =
	//	XMMatrixLookToLH(m_constantData.lightPos[CASCADE_NUM - 1], m_constantData.lightDir, m_up);
	// Matrix projRow = XMMatrixOrthographicLH(1080.0f, 1080.0f, 0.1f, 1000.f);

	// m_constantData.view[CASCADE_NUM - 1] = viewRow.Transpose();
	// m_constantData.proj[CASCADE_NUM - 1] = projRow.Transpose();
	// m_constantData.invProj[CASCADE_NUM - 1] = projRow.Invert().Transpose();

	// DXUtils::UpdateViewport(m_viewPorts[CASCADE_NUM - 1], 0, 0, 1080.0f, 1080.0f);

	// std::vector<float> casecade = { 0.0f, 0.2f, 0.4f, 0.6f };

	//// NDC ÁÂÇ¥°è
	// Vector3 frustumCorner[8] = {
	//	Vector3(-1.0f, 1.0f, 0.0f),
	//	Vector3(1.0f, 1.0f, 0.0f),
	//	Vector3(1.0f, -1.0f, 0.0f),
	//	Vector3(-1.0f, -1.0f, 0.0f),
	//	Vector3(-1.0f, 1.0f, 1.0f),
	//	Vector3(1.0f, 1.0f, 1.0f),
	//	Vector3(1.0f, -1.0f, 1.0f),
	//	Vector3(-1.0f, -1.0f, 1.0f),
	// };

	//// ¿ùµå ÁÂÇ¥°è º¯È¯
	// Matrix invViewProj =
	//	(camera.GetViewMatrix() * camera.GetProjectionMatrix()).Invert().Transpose();
	// for (auto& v : frustumCorner)
	//{
	//	v = Vector3::Transform(v, invViewProj);
	// }


	// for (int i = 0; i < casecade.size() - 1; ++i) {
	//	Vector3 tFrsutum[8];
	//	for (int j = 0; j < 8; ++j)
	//	{
	//		tFrsutum[j] = frustumCorner[j];
	//	}

	//	for (int j = 0; j < 4; ++j)
	//	{
	//		Vector3 v = tFrsutum[j + 4] - tFrsutum[j];

	//		Vector3 n = v * casecade[i];
	//		Vector3 f = v * casecade[i + 1];

	//		tFrsutum[j + 4] = tFrsutum[j] + f;
	//		tFrsutum[j] = tFrsutum[j] + n;
	//	}

	//	Vector3 center(0.0f);
	//	for (auto& v : tFrsutum)
	//	{
	//		center += v;
	//	}
	//	center *= (1.0f / 8.0f);

	//	float radius = 0.0f;
	//	for (auto& v : tFrsutum)
	//	{
	//		radius = max(radius, (v - center).Length());
	//	}

	//	Vector3 lightPos = center + (m_dir * -radius * 2.0f);
	//	m_constantData.lightPos[i] = lightPos;
	//	m_constantData.lightDir = m_dir;

	//	viewRow =
	//		//XMMatrixLookAtLH(lightPos, center, m_up);
	//	XMMatrixLookToLH(m_constantData.lightPos[i], m_constantData.lightDir, m_up);
	//	projRow = XMMatrixOrthographicLH(radius * 2.0f, radius * 2.0f, 0.0f, radius);

	//	m_constantData.view[i] = viewRow.Transpose();
	//	m_constantData.proj[i] = projRow.Transpose();
	//	m_constantData.invProj[i] = projRow.Invert().Transpose();

	//	DXUtils::UpdateViewport(m_viewPorts[i], 0, 0, 1080.0f * casecade[i + 1], 1080.0f *
	//casecade[i + 1]);
	//}
