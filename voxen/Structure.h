#pragma once

#include <directxtk/SimpleMath.h>

using namespace DirectX::SimpleMath;

enum DIR : uint8_t {
	LEFT = 0,
	RIGHT = 1, 
	BOTTOM = 2,
	TOP = 3,
	FRONT = 4,
	BACK = 5
};

enum BLOCK_TYPE : uint8_t {
	B_AIR = 0,
	B_WATER = 1,
	B_BEDROCK = 2,
	B_GRASS = 3,
	B_SNOW_GRASS = 4,
	B_DIRT = 5,
	B_STONE = 6,
	B_SAND = 7,
	B_SNOW = 8,
};

enum TEXTURE_INDEX : uint8_t {
	// block
	T_WATER = 0,
	T_GRASS_TOP = 1,
	T_GRASS_OVERLAY = 2,
	T_DIRT = 3,
	T_SAND = 4,
	T_BEDROCK = 5,
	T_STONE = 6,
	T_SNOW_GRASS_TOP = 7,
	T_SNOW_GRASS_SIDE = 8,
	T_SNOW = 9,

	// instance
	T_SHORT_GRASS = 128
};

enum INSTANCE_TYPE : uint8_t {
	I_CROSS = 0,
	I_FENCE = 1,
	I_SQUARE = 2,
	I_NONE = 3,
};

struct VoxelVertex {
	uint32_t data;
};

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
	uint32_t texIndex;
};

struct CameraConstantData {
	Matrix view;
	Matrix proj;
	Matrix invProj;
	Vector3 eyePos;
	float maxRenderDistance;
	Vector3 eyeDir;
	float lodRenderDistance;
	int isUnderWater;
	Vector3 dummy;
};

struct ChunkConstantData {
	Matrix world;
};

struct SkyboxConstantData {
	Vector3 normalHorizonColor;
	float skyScale;
	Vector3 normalZenithColor;
	float dummy1;
	Vector3 sunHorizonColor;
	float dummy2;
	Vector3 sunZenithColor;
	float dummy3;
};

struct LightConstantData {
	Vector3 lightDir;
	float radianceWeight;
	Vector3 radianceColor;
	float maxRadianceWeight;
};

struct CloudConstantData {
	Matrix world;
	Vector3 volumeColor;
	float cloudScale;
};

struct BlurConstantData {
	float dx;
	float dy;
	Vector2 dummy;
};

struct ShadowConstantData {
	Matrix viewProj[3];
	float topLX[4];
	float viewportWidth[4];
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

struct AppConstantData {
	float appWidth;
	float appHeight;
	float mirrorWidth;
	float mirrorHeight;
};