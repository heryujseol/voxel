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
	B_GRASS = 2,
	B_DIRT = 3,
	B_STONE = 4,
	B_SAND = 5,
	B_BEDROCK = 6,
	B_SANDSTONE = 7,
	B_REDSAND = 8,
	B_REDSANDSTONE = 9,
	B_TERRACOTTA = 10,
	B_TERRACOTTA_ORANGE = 11,
	B_TERRACOTTA_YELLOW = 12,
	B_TERRACOTTA_WHITE = 13,
	B_TERRACOTTA_RED = 14,
	B_TERRACOTTA_BROWN = 15,
	B_GRAVEL = 16,
};

enum TEXTURE_INDEX : uint8_t {
	T_WATER = 0,
	T_GRASS_TOP = 1,
	T_GRASS_OVERLAY = 2,


	T_DIRT = 16,
	T_SAND = 17,
	T_SANDSTONE = 18,
	T_SANDSTONE_BOTTOM = 19,
	T_SANDSTONE_TOP = 20,


	T_REDSAND = 21,
	T_REDSANDSTONE = 22,
	T_REDSANDSTONE_BOTTOM = 23,
	T_REDSANDSTONE_TOP = 24,
	T_TERRACOTTA = 25,
	T_TERRACOTTA_ORANGE = 26,
	T_TERRACOTTA_YELLOW = 27,
	T_TERRACOTTA_WHITE = 28,
	T_TERRACOTTA_RED = 29,
	T_TERRACOTTA_BROWN = 30,
	T_GRAVEL = 31,


	T_BEDROCK = 32,
	T_STONE = 33,

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