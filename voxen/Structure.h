#pragma once

#include <directxtk/SimpleMath.h>

using namespace DirectX::SimpleMath;

typedef uint32_t VoxelVertex;

struct SkyboxVertex {
	Vector3 position;
};

struct CloudVertex {
	Vector3 position;
	uint8_t face;
};

struct SamplingVertex {
	Vector3 position;
	Vector2 texcoord;
};

struct InstanceVertex {
	Vector3 position;
	Vector3 normal;
	Vector2 texcoord;
};

struct InstanceInfoVertex {
	Matrix instanceWorld;
	uint32_t type;
};

struct CameraConstantData {
	Matrix view;
	Matrix proj;
	Vector3 eyePos;
	float maxRenderDistance;
	Vector3 eyeDir;
	float lodRenderDistance;
	Matrix invProj;
	int isUnderWater;
	Vector3 dummy;
	Matrix invTransposeView;
};

struct ChunkConstantData {
	Matrix world;
};

struct SkyboxConstantData {
	Vector3 sunDir;
	float skyScale;
	Vector3 normalHorizonColor;
	uint32_t dateTime;
	Vector3 normalZenithColor;
	float sunStrength;
	Vector3 sunHorizonColor; 
	float moonStrength;
	Vector3 sunZenithColor;
	float dummy;
};

struct CloudConstantData {
	Matrix world;
	Vector3 volumeColor;
	float cloudScale;
};

struct EnvMapConstantData {
	Matrix view[6];
	Matrix proj;
};

struct BlurConstantData {
	float dx;
	float dy;
	Vector2 dummy;
};

struct LightConstantData {
	Matrix view[4];
	Matrix proj[4];
	Matrix invProj[4];
	float topLX[4];
	float viewWith[4];
};

struct FogFilterConstantData {
	float fogDistMin;
	float fogDistMax;
	float fogStrength;
	float dummy1;
	Vector3 fogColor;
	float dummy2;
};

struct WaterFilterConstantData {
	Vector3 filterColor;
	float filterStrength;
};

struct SsaoConstantData {
	Vector4 sampleKernel[64];
};

struct SsaoNoiseConstantData {
	Vector4 rotationNoise[16];
};