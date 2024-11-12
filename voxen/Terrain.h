#pragma once

#include "Utils.h"

#include <directxtk/SimpleMath.h>

using namespace DirectX::SimpleMath;

namespace Terrain {
	static const int MAX_HEIGHT_LEVEL = 256;
	static const int MIN_HEIGHT_LEVEL = 0;
	static const int WATER_HEIGHT_LEVEL = 63;

	static Vector2 Hash(uint32_t x, uint32_t y)
	{
		// https://www.shadertoy.com/view/3dVXDc
		uint32_t u0 = 1597334673U;
		uint32_t u1 = 3812015801U;
		float uf = (1.0f / float(0xffffffffU));

		uint32_t qi = x * u0;
		uint32_t qj = y * u1;

		uint32_t qx = (qi ^ qj) * u0;
		uint32_t qy = (qi ^ qj) * u1;

		float rx = -1.0f + 2.0f * qx * uf;
		float ry = -1.0f + 2.0f * qy * uf;

		return Vector2(rx, ry);
	}

	static Vector3 Hash(uint32_t x, uint32_t y, uint32_t z)
	{
		// https://www.shadertoy.com/view/3dVXDc
		uint32_t u0 = 1597334673U;
		uint32_t u1 = 3812015801U;
		uint32_t u2 = 2798796415U;
		float uf = (1.0f / float(0xffffffffU));

		uint32_t qi = x * u0;
		uint32_t qj = y * u1;
		uint32_t qk = z * u2;

		uint32_t qx = (qi ^ qj ^ qk) * u0;
		uint32_t qy = (qi ^ qj ^ qk) * u1;
		uint32_t qz = (qi ^ qj ^ qk) * u2;

		float rx = -1.0f + 2.0f * qx * uf;
		float ry = -1.0f + 2.0f * qy * uf;
		float rz = -1.0f + 2.0f * qz * uf;

		return Vector3(rx, ry, rz);
	}

	static float GetPerlinNoiseFbm(float x, float y)
	{
		Vector2 p = Vector2(x, y);
		int x0 = (int)floor(x);
		int x1 = x0 + 1;
		int y0 = (int)floor(y);
		int y1 = y0 + 1;

		float n0 = Hash(x0, y0).Dot(p - Vector2((float)x0, (float)y0));
		float n1 = Hash(x1, y0).Dot(p - Vector2((float)x1, (float)y0));
		float n2 = Hash(x0, y1).Dot(p - Vector2((float)x0, (float)y1));
		float n3 = Hash(x1, y1).Dot(p - Vector2((float)x1, (float)y1));

		float i0 = Utils::CubicLerp(n0, n1, p.x - (float)x0);
		float i1 = Utils::CubicLerp(n2, n3, p.x - (float)x0);

		return Utils::CubicLerp(i0, i1, p.y - (float)y0);
	}

	static float GetPerlinNoiseFbm(float x, float y, float z)
	{
		Vector3 p = Vector3(x, y, z);

		int x0 = (int)floor(x);
		int x1 = x0 + 1;
		int y0 = (int)floor(y);
		int y1 = y0 + 1;
		int z0 = (int)floor(z);
		int z1 = z0 + 1;

		float n0 = Hash(x0, y0, z0).Dot(p - Vector3((float)x0, (float)y0, (float)z0));
		float n1 = Hash(x0, y0, z1).Dot(p - Vector3((float)x0, (float)y0, (float)z1));
		float n2 = Hash(x0, y1, z0).Dot(p - Vector3((float)x0, (float)y1, (float)z0));
		float n3 = Hash(x0, y1, z1).Dot(p - Vector3((float)x0, (float)y1, (float)z1));
		float n4 = Hash(x1, y0, z0).Dot(p - Vector3((float)x1, (float)y0, (float)z0));
		float n5 = Hash(x1, y0, z1).Dot(p - Vector3((float)x1, (float)y0, (float)z1));
		float n6 = Hash(x1, y1, z0).Dot(p - Vector3((float)x1, (float)y1, (float)z0));
		float n7 = Hash(x1, y1, z1).Dot(p - Vector3((float)x1, (float)y1, (float)z1));

		float i0 = Utils::CubicLerp(n0, n1, p.z - z0);
		float i1 = Utils::CubicLerp(n2, n3, p.z - z0);
		float i2 = Utils::CubicLerp(i0, i1, p.y - y0);
		float i3 = Utils::CubicLerp(n4, n5, p.z - z0);
		float i4 = Utils::CubicLerp(n6, n7, p.z - z0);
		float i5 = Utils::CubicLerp(i3, i4, p.y - y0);

		return Utils::CubicLerp(i2, i5, p.x - x0);
	}

	static float PerlinFbm(float x, float y, float freq, int octave)
	{
		float amp = 1.0f;
		float noise = 0.0f;
		float aFactor = exp2(-0.85f);

		for (int i = 0; i < octave; ++i) {
			noise += amp * GetPerlinNoiseFbm(x * freq, y * freq);

			freq *= 2.0f;
			amp *= aFactor;
		}

		return noise;
	}

	static float PerlinFbm(float x, float y, float z, float freq, int octave)
	{
		float amp = 1.0f;
		float noise = 0.0f;
		float aFactor = exp2(-0.85f);

		for (int i = 0; i < octave; ++i) {
			noise += amp * GetPerlinNoiseFbm(x * freq, y * freq, z * freq);

			freq *= 2.0f;
			amp *= aFactor;
		}

		return noise;
	}

	static float SplineContinentalness(float value)
	{
		value = std::clamp(value * 1.5f, -1.0f, 1.0f);

		if (value <= -0.51f) {
			float w = (value - -1.0f) / (-0.51f - -1.0f);
			return Utils::CubicLerp(0.0f, 0.14f, w);
		}
		else if (value <= -0.25f) {
			float w = (value - -0.51f) / (-0.25f - -0.51f);
			return Utils::CubicLerp(0.14f, 0.31f, w);
		}
		else if (value <= -0.10f) {
			float w = (value - -0.25f) / (-0.10f - -0.25f);
			return Utils::CubicLerp(0.31f, 0.43f, w);
		}
		else if (value <= 0.09f) {
			float w = (value - -0.10f) / (0.09f - -0.10f);
			return Utils::CubicLerp(0.43f, 0.57f, w);
		}
		else if (value <= 0.42f) {
			float w = (value - 0.09f) / (0.42f - 0.09f);
			return Utils::CubicLerp(0.57f, 0.92f, w);
		}
		else {
			float w = (value - 0.42f) / (1.0f - 0.42f);
			return Utils::CubicLerp(0.92f, 1.0f, w);
		}
	}

	static float GetContinentalness(int x, int z)
	{
		float scale = 512.0f;

		float cNoise = PerlinFbm(x / scale, z / scale, 2.0f, 6);
		float cValue = SplineContinentalness(cNoise);

		if (cValue <= 0.3f)
			return cValue / 0.3f - 1.0f; // [-1.0f, 0.0f]
		else
			return (cValue - 0.3f) / 0.7f; // [0.0f, 1.0f]
	}

	static float SplineErosion(float value)
	{
		value = std::clamp(value * 1.5f, -1.0f, 1.0f);

		if (value <= -0.78f) { // 0.02
			float w = (value - -1.0f) / (-0.78f - -1.0f);
			return Utils::CubicLerp(0.0f, 0.02f, w);
		}
		else if (value <= -0.57f) { // 0.14
			float w = (value - -0.78f) / (-0.57f - -0.78f);
			return Utils::CubicLerp(0.02f, 0.14f, w);
		}
		else if (value <= -0.36f) { // 0.29
			float w = (value - -0.57f) / (-0.36f - -0.57f);
			return Utils::CubicLerp(0.14f, 0.29f, w);
		}
		else if (value <= 0.03f) { // 0.43
			float w = (value - -0.36f) / (0.03f - -0.36f);
			return Utils::CubicLerp(0.29f, 0.43f, w);
		}
		else if (value <= 0.32f) { // 0.57
			float w = (value - 0.03f) / (0.32f - 0.03f);
			return Utils::CubicLerp(0.43f, 0.57f, w);
		}
		else if (value <= 0.39f) { // 0.71
			float w = (value - 0.32f) / (0.39f - 0.32f);
			return Utils::CubicLerp(0.57f, 0.71f, w);
		}
		else if (value <= 0.78f) { // 0.86
			float w = (value - 0.39f) / (0.78f - 0.39f);
			return Utils::CubicLerp(0.71f, 0.86f, w);
		}
		else { // 1.0
			float w = (value - 0.78f) / (1.0f - 0.78f);
			return Utils::CubicLerp(0.86f, 1.0f, w);
		}
	}

	static float GetErosion(int x, int z)
	{
		float scale = 512.0f;
		float bias = 123.0f;

		float eNoise = PerlinFbm(x / scale + bias, z / scale + bias, 2.0f, 4);
		float eValue = SplineErosion(eNoise);

		return eValue;
	}

	static float SplinePeaksValley(float value)
	{
		value = std::clamp(abs(value * 1.5f), 0.0f, 1.0f);

		if (value <= 0.05f) {
			float w = (value - 0.0f) / (0.05f - 0.0f);
			return Utils::CubicLerp(0.01f, 0.07f, w);
		}
		else if (value <= 0.10f) {
			float w = (value - 0.05f) / (0.10f - 0.05f);
			return Utils::CubicLerp(0.07f, 0.12f, w);
		}
		else if (value <= 0.24f) {
			float w = (value - 0.10f) / (0.24f - 0.10f);
			return Utils::CubicLerp(0.12f, 0.32f, w);
		}
		else if (value <= 0.36f) {
			float w = (value - 0.24f) / (0.36f - 0.24f);
			return Utils::CubicLerp(0.32f, 0.52f, w);
		}
		else if (value <= 0.42f) {
			float w = (value - 0.36f) / (0.42f - 0.36f);
			return Utils::CubicLerp(0.52f, 0.64f, w);
		}
		else if (value <= 0.80f) {
			float w = (value - 0.42f) / (0.80f - 0.42f);
			return Utils::CubicLerp(0.64f, 1.0f, w);
		}
		else {
			float w = (value - 0.80f) / (1.0f - 0.80f);
			return Utils::CubicLerp(0.86f, 1.0f, 1.0f - w);
		}
	}

	static float GetPeaksValley(int x, int z)
	{
		float scale = 256.0f;
		float bias = 4.0f;

		float pvNoise = PerlinFbm(x / scale + bias, z / scale + bias, 1.5f, 6);
		float pvValue = SplinePeaksValley(pvNoise);

		return (pvValue - 0.5f) * 2.0f;
	}

	static float GetElevation(float c, float e, float pv)
	{
		float elevation = 64.0f + 64.0f * c * (1.0f - e) + 64.0f * pv * powf((1.0f - e), 1.25f);

		return max(elevation, 1.0f);
	}

	static float GetCaveDensity(
		int x, int y, int z, float bias, float xScale, float yScale, float zScale)
	{
		float dNoise = PerlinFbm(x / xScale + bias, y / yScale + bias, z / zScale + bias, 2.0f, 4);

		return dNoise;
	}

	static bool IsCave(int x, int y, int z)
	{
		float threshold = 0.004f;

		float density1 = Terrain::GetCaveDensity(x, y, z, 3.0f, 256.0f, 256.0f, 256.0f);
		if (density1 * density1 > threshold)
			return false; // ealry return

		float density2 = Terrain::GetCaveDensity(x, y, z, 123.0f, 512.0f, 256.0f, 512.0f);
		if (density2 * density2 > threshold)
			return false;

		return (density1 * density1 + density2 * density2 <= threshold);
	}

	static float GetTemperature(int x, int z)
	{
		float scale = 128.0f;
		float bias = 157.0f;

		float tNoise = PerlinFbm(x / scale + bias, z / scale + bias, 2.0f, 4);
		tNoise = std::clamp(tNoise * 1.5f, -1.0f, 1.0f);

		return (tNoise + 1.0f) * 0.5f;
	}

	static float GetHumidity(int x, int z)
	{
		float scale = 128.0f;
		float bias = 653.0f;

		float hNoise = PerlinFbm(x / scale + bias, z / scale + bias, 2.0f, 4);
		hNoise = std::clamp(hNoise * 1.5f, -1.0f, 1.0f);

		return (hNoise + 1.0f) * 0.5f;
	}

	static BIOME_TYPE GetBiomeType(float elevation, float temperature, float humidity, float y)
	{
		if (y < 64.0f) {
			return BIOME_OCEAN;
		}
		else if (y < 68.0f) {
			return BIOME_BEACH;
		}
		else if (y < 76.0f) {
			if (temperature < 0.5f) {
				return BIOME_SWAMP;
			}
			else {
				return BIOME_SAVANNA;
			}
		}
		else {
			if (temperature < 0.3f) {
				if (humidity < 0.3f) {
					return BIOME_SNOWY_TUNDRA;
				}
				else {
					return BIOME_SNOWY_TAIGA;
				}
			}
			else if (temperature < 0.7f) {
				if (humidity < 0.3f) {
					return BIOME_MOUNTAINS;
				}
				else {
					return BIOME_PLAINS;
				}
			}
			else {
				if (humidity < 0.3f) {
					return BIOME_DESERT;
				}
				else if (humidity < 0.6f) {
					return BIOME_TAIGA;
				}
				else {
					return BIOME_JUNGLE;
				}
			}
		}
	}

	static BLOCK_TYPE GetBlockTypeByBiome(BIOME_TYPE biomeType)
	{ 
		switch (biomeType) {
		case BIOME_OCEAN:
			return BLOCK_A; // 예시로 해저에 특정한 블록을 설정

		case BIOME_BEACH:
			return BLOCK_B;

		case BIOME_SWAMP:
			return BLOCK_C;

		case BIOME_SAVANNA:
			return BLOCK_D;

		case BIOME_SNOWY_TUNDRA:
			return BLOCK_E;

		case BIOME_SNOWY_TAIGA:
			return BLOCK_F;

		case BIOME_MOUNTAINS:
			return BLOCK_G;

		case BIOME_PLAINS:
			return BLOCK_H;

		case BIOME_DESERT:
			return BLOCK_I;

		case BIOME_TAIGA:
			return BLOCK_J;

		case BIOME_JUNGLE:
			return BLOCK_K;

		default:
			return BLOCK_BEDROCK; // 예외적으로 기본 블록을 설정
		}
	}

	static BLOCK_TYPE GetBlockType(int x, int y, int z, float elevation, float temperature,
		float moisture, float erosion, float peaksValley)
	{
		if (y == MIN_HEIGHT_LEVEL)
			return BLOCK_BEDROCK;

		BLOCK_TYPE blockType = (y <= WATER_HEIGHT_LEVEL) ? BLOCK_WATER : BLOCK_AIR;

		if (y <= elevation && !IsCave(x, y, z)) {
			int biomeLayer =
				1 + (int)(4.0f * (1.0f - erosion) * powf(((-peaksValley + 1.0f) * 0.5f), 0.5f));

			if (y <= elevation - biomeLayer) {
				blockType = BLOCK_STONE;
				// 3D noise density
			}
			else {
				// Biome Block
				float ry = y - (int)(32.0f * peaksValley * powf((1.0f - erosion), 1.25f));

				BIOME_TYPE biomeType = GetBiomeType(elevation, temperature, moisture, ry);
				blockType = GetBlockTypeByBiome(biomeType);
			}
		}

		return blockType;
	}

	static TEXTURE_INDEX GetBlockTextureIndex(BLOCK_TYPE blockType, uint8_t face)
	{

		switch (blockType) {

		case BLOCK_WATER:
			return TEXTURE_WATER;

		case BLOCK_BEDROCK:
			return TEXTURE_BEDROCK;

		case BLOCK_GRASS:
			if (face == DIR::TOP)
				return TEXTURE_GRASS_TOP;
			else if (face == DIR::BOTTOM)
				return TEXTURE_DIRT;
			else
				return TEXTURE_GRASS_OVERLAY;

		case BLOCK_SNOW_GRASS:
			if (face == DIR::TOP)
				return TEXTURE_SNOW_GRASS_TOP;
			else if (face == DIR::BOTTOM)
				return TEXTURE_DIRT;
			else
				return TEXTURE_SNOW_GRASS_SIDE;

		case BLOCK_DIRT:
			return TEXTURE_DIRT;

		case BLOCK_STONE:
			return TEXTURE_STONE;

		case BLOCK_SAND:
			return TEXTURE_SAND;

		case BLOCK_SNOW:
			return TEXTURE_SNOW;

		// TESTING
		case BLOCK_A:
			return TEXTURE_A;
		case BLOCK_B:
			return TEXTURE_B;
		case BLOCK_C:
			return TEXTURE_C;
		case BLOCK_D:
			return TEXTURE_D;
		case BLOCK_E:
			return TEXTURE_E;
		case BLOCK_F:
			return TEXTURE_F;
		case BLOCK_G:
			return TEXTURE_G;
		case BLOCK_H:
			return TEXTURE_H;
		case BLOCK_I:
			return TEXTURE_I;
		case BLOCK_J:
			return TEXTURE_J;
		case BLOCK_K:
			return TEXTURE_K;

		default:
			return TEXTURE_STONE;
		}
	}
}