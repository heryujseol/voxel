#pragma once

#include "Utils.h"

#include <directxtk/SimpleMath.h>

using namespace DirectX::SimpleMath;

namespace Terrain {

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
		float scale = 1024.0f;

		float freq = 2.0f;
		int octave = 6;

		float cNoise = PerlinFbm(x / scale, z / scale, freq, octave);
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
		float scale = 1024.0f;
		float bias = 123.0f;

		float freq = 1.0f;
		int octave = 4;

		float eNoise = PerlinFbm(x / scale + bias, z / scale + bias, freq, octave);
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
		float scale = 512.0f;
		float bias = 4.0f;

		float freq = 1.5f;
		int octave = 6;

		float pvNoise = PerlinFbm(x / scale + bias, z / scale + bias, freq, octave);
		float pvValue = SplinePeaksValley(pvNoise);

		return (pvValue - 0.5f) * 2.0f;
	}

	static float GetBaseLevel(float c, float e, float pv)
	{
		float baseLevel = 64.0f + 64.0f * c * (1.0f - e * e) + 64.0f * pv * powf((1.0f - e), 1.25f);

		return max(baseLevel, 1.0f);
	}

	static float GetDensity(int x, int y, int z, float bias, float scale)
	{
		float freq = 2.0f;
		int octave = 4;

		float dNoise =
			PerlinFbm(x / scale + bias, y / scale + bias, z / scale + bias, freq, octave);

		return dNoise;
	}

	static float GetDensity2(int x, int y, int z, float bias, float scale)
	{
		float freq = 2.0f;
		int octave = 4;

		float dNoise =
			PerlinFbm(x / scale + bias, y / (scale / 2.0f) + bias, z / scale + bias, freq, octave);

		return dNoise;
	}

	static float GetTemperature(int x, int z) { return 1.0; }

	static float GetHumidity(int x, int z) { return 1.0; }

	static BIOME_TYPE GetBiome(float temperature, float humidity) { return BIOME_TYPE::PLAINS; }
	
	static BLOCK_TYPE GetBlockType(BIOME_TYPE biome, float currentHeight, float maxHeight) {
		return BLOCK_TYPE::B_DIRT;
	}

	static TEXTURE_INDEX GetBlockTextureIndex(BLOCK_TYPE blockType, int face) {
		return TEXTURE_INDEX::T_DIRT;
	}
}