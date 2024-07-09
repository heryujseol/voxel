#include "Camera.h"
#include "DXUtils.h"

Camera::Camera()
	: m_projFovAngleY(80.0f), m_nearZ(0.1f), m_farZ(1000.0f), m_aspectRatio(16.0f / 9.0f),
	  m_eyePos(0.0f, 0.0f, 0.0f), m_chunkPos(0.0f, 0.0f, 0.0f), m_forward(0.0f, 0.0f, 1.0f),
	  m_up(0.0f, 1.0f, 0.0f), m_right(1.0f, 0.0f, 0.0f), m_viewNdcX(0.0f), m_viewNdcY(0.0f),
	  m_speed(20.0f), m_isOnConstantDirtyFlag(false), m_isOnChunkDirtyFlag(false),
	  m_isInWater(false)
{
}

Camera::~Camera() {}

bool Camera::Initialize(Vector3 pos)
{
	m_eyePos = pos;
	m_chunkPos = Utils::CalcOffsetPos(m_eyePos, Chunk::CHUNK_SIZE);

	m_constantData.view = GetViewMatrix();
	m_constantData.proj = GetProjectionMatrix();
	m_constantData.invProj = m_constantData.proj.Invert();
	m_constantData.eyePos = m_eyePos;
	m_constantData.eyeDir = m_forward;
	m_constantData.maxRenderDistance = (float)MAX_RENDER_DISTANCE;
	m_constantData.lodRenderDistance = (float)LOD_RENDER_DISTANCE;

	CameraConstantData tempConstantData;
	tempConstantData.view = m_constantData.view.Transpose();
	tempConstantData.proj = m_constantData.proj.Transpose();
	tempConstantData.invProj = m_constantData.invProj.Transpose();
	tempConstantData.eyePos = m_constantData.eyePos;
	tempConstantData.eyeDir = m_constantData.eyeDir;
	tempConstantData.maxRenderDistance = (float)MAX_RENDER_DISTANCE;
	tempConstantData.lodRenderDistance = (float)LOD_RENDER_DISTANCE;
	if (!DXUtils::CreateConstantBuffer(m_constantBuffer, tempConstantData)) {
		std::cout << "failed create camera constant buffer" << std::endl;
		return false;
	}

	Plane mirrorPlane = Plane(Vector3(0.0f, 62.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
	m_mirrorPlaneMatrix = Matrix::CreateReflection(mirrorPlane);
	tempConstantData.view = m_mirrorPlaneMatrix * m_constantData.view;
	tempConstantData.view = tempConstantData.view.Transpose();
	if (!DXUtils::CreateConstantBuffer(m_mirrorConstantBuffer, tempConstantData)) {
		std::cout << "failed create camera mirror constant buffer" << std::endl;
		return false;
	}

	for (int i = 0; i < 6; ++i) {
		m_envMapConstantData.view[i] = XMMatrixLookToLH(Vector3(0.0f), lookTo[i], up[i]);
		m_envMapConstantData.view[i] = m_envMapConstantData.view[i].Transpose();
	}
	m_envMapConstantData.proj =
		XMMatrixPerspectiveFovLH(XMConvertToRadians(90.0f), 1.0, m_nearZ, m_farZ);
	m_envMapConstantData.proj = m_envMapConstantData.proj.Transpose();
	if (!DXUtils::CreateConstantBuffer(m_envMapConstantBuffer, m_envMapConstantData)) {
		std::cout << "failed create env map constant buffer" << std::endl;
		return false;
	}

	return true;
}

void Camera::Update(float dt, bool keyPressed[256], float mouseX, float mouseY)
{
	UpdatePosition(keyPressed, dt);
	UpdateViewDirection(mouseX, mouseY);

	if (m_isOnConstantDirtyFlag) {
		m_constantData.view = GetViewMatrix();
		m_constantData.proj = GetProjectionMatrix();
		m_constantData.invProj = m_constantData.proj.Invert();
		m_constantData.eyePos = m_eyePos;
		m_constantData.eyeDir = m_forward;

		CameraConstantData tempConstantData;
		tempConstantData.view = m_constantData.view.Transpose();
		tempConstantData.proj = m_constantData.proj.Transpose();
		tempConstantData.invProj = m_constantData.invProj.Transpose();
		tempConstantData.eyePos = m_constantData.eyePos;
		tempConstantData.eyeDir = m_constantData.eyeDir;
		tempConstantData.maxRenderDistance = (float)MAX_RENDER_DISTANCE;
		tempConstantData.lodRenderDistance = (float)LOD_RENDER_DISTANCE;
		DXUtils::UpdateConstantBuffer(m_constantBuffer, tempConstantData);

		tempConstantData.view = m_mirrorPlaneMatrix * m_constantData.view;
		tempConstantData.view = tempConstantData.view.Transpose();
		DXUtils::UpdateConstantBuffer(m_mirrorConstantBuffer, tempConstantData);

		m_isOnConstantDirtyFlag = false;
	}
	// static float acc = 0.0f;

	//// dateTime
	// acc += DATE_TIME_SPEED * dt;
	// m_dateTime = (uint32_t)acc;
	// m_dateTime %= DATE_CYCLE_AMOUNT;

	// float angle = (float)m_dateTime / DATE_CYCLE_AMOUNT * 2.0f * Utils::PI;
	// m_constantData.eyePos = Vector3::Transform(Vector3(450.0f, 0.0f, 0.0f),
	// Matrix::CreateRotationZ(angle)); m_eyePos = m_constantData.eyePos; m_forward =
	// Vector3::Transform(Vector3(-1.0f, 0.0f, 0.0f), Matrix::CreateRotationZ(angle));
	// m_constantData.eyeDir = m_forward;

	//// Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
	// m_up = XMVector3TransformNormal(Vector3(0.0f, 1.0f, 0.0f), Matrix::CreateRotationZ(angle));

	// Matrix viewRow = XMMatrixLookToLH(
	//	m_constantData.eyePos, m_constantData.eyeDir, m_up);
	////Matrix projRow = XMMatrixPerspectiveFovLH(XMConvertToRadians(120.0f), 1.0f, 0.1f, 1000.0f);
	// Matrix projRow = XMMatrixOrthographicLH(1080.0f, 1080.0f, 0.1f, 3000.0f);

	// m_constantData.invProj = projRow.Invert().Transpose();

	// m_constantData.view = viewRow.Transpose();
	// m_constantData.proj = projRow.Transpose();

	// DXUtils::UpdateConstantBuffer(m_constantBuffer, m_constantData);
}

void Camera::UpdatePosition(bool keyPressed[256], float dt)
{
	if (keyPressed['W']) {
		MoveForward(dt);
		m_isOnConstantDirtyFlag = true;
	}
	if (keyPressed['S']) {
		MoveForward(-dt);
		m_isOnConstantDirtyFlag = true;
	}
	if (keyPressed['D']) {
		MoveRight(dt);
		m_isOnConstantDirtyFlag = true;
	}
	if (keyPressed['A']) {
		MoveRight(-dt);
		m_isOnConstantDirtyFlag = true;
	}

	if (m_isOnConstantDirtyFlag) {
		Vector3 newChunkPos = Utils::CalcOffsetPos(m_eyePos, Chunk::CHUNK_SIZE);
		if (newChunkPos != m_chunkPos) {
			m_isOnChunkDirtyFlag = true;
			m_chunkPos = newChunkPos;
		}
	}
}

void Camera::UpdateViewDirection(float ndcX, float ndcY)
{
	if (m_viewNdcX == ndcX && m_viewNdcY == ndcY)
		return;

	m_isOnConstantDirtyFlag = true;

	m_viewNdcX = ndcX;
	m_viewNdcY = ndcY;

	float thetaHorizontal = DirectX::XM_PI * m_viewNdcX;
	float thetaVertical = -DirectX::XM_PIDIV2 * m_viewNdcY;

	// using Quaternion not Euler
	Vector3 basisX = Vector3(1.0f, 0.0f, 0.0f);
	Vector3 basisY = Vector3(0.0f, 1.0f, 0.0f);
	Vector3 basisZ = Vector3(0.0f, 0.0f, 1.0f);

	Quaternion q = Quaternion(basisY * sinf(thetaHorizontal * 0.5f), cosf(thetaHorizontal * 0.5f));
	m_forward = Vector3::Transform(basisZ, Matrix::CreateFromQuaternion(q));
	m_right = Vector3::Transform(basisX, Matrix::CreateFromQuaternion(q));

	q = Quaternion(m_right * sinf(thetaVertical * 0.5f), cosf(thetaVertical * 0.5f));
	m_forward = Vector3::Transform(m_forward, Matrix::CreateFromQuaternion(q));
	m_up = Vector3::Transform(basisY, Matrix::CreateFromQuaternion(q));
}

void Camera::MoveForward(float dt) { m_eyePos += m_forward * m_speed * dt; }

void Camera::MoveRight(float dt) { m_eyePos += m_right * m_speed * dt; }