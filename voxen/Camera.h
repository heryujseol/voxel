#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "directxtk/SimpleMath.h"

#include "Chunk.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

class Camera {
public:
	static const int MAX_RENDER_DISTANCE = 260;
	static const int LOD_RENDER_DISTANCE = 160;

	Camera();
	~Camera();

	bool Initialize(Vector3 pos);

	void Update(float dt, bool keyPressed[256], float mouseX, float mouseY);

	ComPtr<ID3D11Buffer> m_constantBuffer;
	ComPtr<ID3D11Buffer> m_mirrorConstantBuffer;

	inline Vector3 GetPosition() { return m_eyePos; }
	inline Vector3 GetChunkPosition() { return m_chunkPos; }
	inline Vector3 GetForward() { return m_forward; }
	inline Matrix GetViewMatrix() { return XMMatrixLookToLH(m_eyePos, m_forward, m_up); }
	inline Matrix GetProjectionMatrix()
	{
		return XMMatrixPerspectiveFovLH(
			XMConvertToRadians(m_projFovAngleY), m_aspectRatio, m_nearZ, m_farZ);
	}
	inline Matrix GetMirrorPlaneMatrix() { return m_mirrorPlaneMatrix; }
	inline bool IsUnderWater() { return m_isUnderWater; }

	bool m_isOnConstantDirtyFlag;
	bool m_isOnChunkDirtyFlag;
	
	CameraConstantData m_constantData;

private:
	void UpdatePosition(bool keyPressed[256], float dt);
	void UpdateViewDirection(float mouseX, float mouseY);

	void MoveForward(float dt);
	void MoveRight(float dt);

	void SetIsUnderWater();

	float m_projFovAngleY;
	float m_nearZ;
	float m_farZ;
	float m_aspectRatio;

	Vector3 m_eyePos;
	Vector3 m_chunkPos;
	Vector3 m_forward;
	Vector3 m_up;
	Vector3 m_right;
	Matrix m_mirrorPlaneMatrix;

	float m_viewNdcX;
	float m_viewNdcY;

	float m_speed;

	bool m_isUnderWater;

	//CameraConstantData m_constantData;

	Vector3 lookTo[6] = {
		Vector3(1.0f, 0.0f, 0.0f),
		Vector3(-1.0f, 0.0f, 0.0f),
		Vector3(0.0f, 1.0f, 0.0f),
		Vector3(0.0f, -1.0f, 0.0f),
		Vector3(0.0f, 0.0f, 1.0f),
		Vector3(0.0f, 0.0f, -1.0f),
	};
	Vector3 up[6] = {
		Vector3(0.0f, 1.0f, 0.0f),
		Vector3(0.0f, 1.0f, 0.0f),
		Vector3(0.0f, 0.0f, -1.0f),
		Vector3(0.0f, 0.0f, 1.0f),
		Vector3(0.0f, 1.0f, 0.0f),
		Vector3(0.0f, 1.0f, 0.0f),
	};
};