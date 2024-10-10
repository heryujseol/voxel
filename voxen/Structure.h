#pragma once

#include <directxtk/SimpleMath.h>

using namespace DirectX::SimpleMath;

enum BIOME_TYPE : uint8_t { 
	PLAINS = 0,
	JUNGLE = 1,
	DESERT = 2,
	SWAMP = 3,
	FOREST = 4,
	SAVANNA = 5,
	TAIGA = 6,
	TUNDRA = 7,
	BADLAND = 8
};

enum BLOCK_TYPE : uint8_t {
	B_AIR = 0,
	B_WATER = 1,
	B_GRASS = 2,
	B_DIRT = 3,
	B_STONE = 4,
	B_SAND = 5,
	B_BEDROCK = 6
};

enum TEXTURE_INDEX : uint8_t {
	T_WATER = 0,
	T_GRASS_TOP = 1,
	T_DIRT = 16,
	T_SHORT_GRASS = 128,
	
};

enum INSTANCE_TYPE : uint8_t {
	I_CROSS = 0,
	I_FENCE = 1,
	I_SQUARE = 2,
	I_NONE = 3,
};

struct VoxelVertex {
	uint32_t data;
	uint8_t biome;
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
	uint8_t biome;
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