#include "Camera.h"
#include "DXUtils.h"
#include "ChunkManager.h"

Camera::Camera()
	: m_projFovAngleY(80.0f), m_nearZ(0.1f), m_farZ(1000.0f), m_aspectRatio(16.0f / 9.0f),
	  m_eyePos(0.0f, 0.0f, 0.0f), m_chunkPos(0.0f, 0.0f, 0.0f), m_forward(0.0f, 0.0f, 1.0f),
	  m_up(0.0f, 1.0f, 0.0f), m_right(1.0f, 0.0f, 0.0f), m_viewNdcX(0.0f), m_viewNdcY(0.0f),
	  m_speed(20.0f), m_isUnderWater(false), m_isOnConstantDirtyFlag(false),
	  m_isOnChunkDirtyFlag(false)
{
}

Camera::~Camera() {}

bool Camera::Initialize(Vector3 pos)
{
	m_eyePos = pos;
	m_chunkPos = Utils::CalcOffsetPos(m_eyePos, Chunk::CHUNK_SIZE);

	m_constantData.view = GetViewMatrix().Transpose();
	m_constantData.proj = GetProjectionMatrix().Transpose();
	m_constantData.invProj = GetProjectionMatrix().Invert().Transpose();
	m_constantData.eyePos = m_eyePos;
	m_constantData.eyeDir = m_forward;
	m_constantData.maxRenderDistance = (float)MAX_RENDER_DISTANCE;
	m_constantData.lodRenderDistance = (float)LOD_RENDER_DISTANCE;
	m_constantData.isUnderWater = m_isUnderWater;

	if (!DXUtils::CreateConstantBuffer(m_constantBuffer, m_constantData)) {
		std::cout << "failed create camera constant buffer" << std::endl;
		return false;
	}

	Plane mirrorPlane = Plane(Vector3(0.0f, 64.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
	m_mirrorPlaneMatrix = Matrix::CreateReflection(mirrorPlane);
	m_constantData.view = m_mirrorPlaneMatrix * GetViewMatrix();
	m_constantData.view = m_constantData.view.Transpose();
	if (!DXUtils::CreateConstantBuffer(m_mirrorConstantBuffer, m_constantData)) {
		std::cout << "failed create camera mirror constant buffer" << std::endl;
		return false;
	}

	return true;
}

void Camera::Update(float dt, bool keyPressed[256], float mouseX, float mouseY)
{
	UpdatePosition(keyPressed, dt);
	UpdateViewDirection(mouseX, mouseY);
	/////////////////////////////
	if (keyPressed['R']) {
		m_constantData.dummy.x = 0.0f;
		m_isOnConstantDirtyFlag = true;
	}
	else {
		m_constantData.dummy.x = 1.0f;
		m_isOnConstantDirtyFlag = true;
	}
	/////////////////////////////

	if (m_isOnConstantDirtyFlag) {
		m_constantData.view = GetViewMatrix().Transpose();
		m_constantData.proj = GetProjectionMatrix().Transpose();
		m_constantData.invProj = GetProjectionMatrix().Invert().Transpose();
		m_constantData.eyePos = m_eyePos;
		m_constantData.eyeDir = m_forward;
		m_constantData.maxRenderDistance = (float)MAX_RENDER_DISTANCE;
		m_constantData.lodRenderDistance = (float)LOD_RENDER_DISTANCE;
		m_constantData.isUnderWater = m_isUnderWater;

		DXUtils::UpdateConstantBuffer(m_constantBuffer, m_constantData);

		m_constantData.view = m_mirrorPlaneMatrix * GetViewMatrix();
		m_constantData.view = m_constantData.view.Transpose();
		DXUtils::UpdateConstantBuffer(m_mirrorConstantBuffer, m_constantData);

		m_isOnConstantDirtyFlag = false;
	}
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
		SetIsUnderWater();

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

void Camera::SetIsUnderWater()
{
	m_isUnderWater = false;

	int x = (int)m_chunkPos.x;
	int y = (int)m_chunkPos.y;
	int z = (int)m_chunkPos.z;

	Chunk* c = ChunkManager::GetInstance()->GetChunkByPosition(x, y, z);
	if (c != nullptr && c->IsLoaded()) {
		Vector3 chunkOffset = m_eyePos - m_chunkPos;
		if (c->GetBlockTypeByPosition(chunkOffset) == BLOCK_TYPE::WATER) {
			m_isUnderWater = true;
			return;
		}

		float bias = 0.15f;
		chunkOffset.y -= bias;
		if (c->GetBlockTypeByPosition(chunkOffset) == BLOCK_TYPE::WATER) {
			m_isUnderWater = true;
			return;
		}
	}
}