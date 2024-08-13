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
		float i2 = Utils::CubicLerp(i0, i1, p.y - (float)y0);
		float i3 = Utils::CubicLerp(n4, n5, p.z - z0);
		float i4 = Utils::CubicLerp(n6, n7, p.z - z0);
		float i5 = Utils::CubicLerp(i3, i4, p.y - (float)y0);

		return Utils::CubicLerp(i2, i5, p.x - (float)x0);
	}

	/*
	 * x, y : 좌표
	 * - 현재 x, y가 중요한게 아니라, 해당 함수를 호출할 때 x, y의 증감 폭이 중요한 것
	 * - 증감이 작아야 함, 크면 점진적인 변화율을 체크할 수 없음
	 * freq : 진동수, 주파수
	 * - x, y의 범위를 재조정하게 됨
	 * - freq만큼 구간을 잘게 나누게 될 것
	 * - 반복 횟수가 올라가면 구간을 더욱 잘게 나눔 (2배씩)
	 * octave : 옥타브
	 * - 노이즈의 반복 횟수
	 * - 해당 반복에 따라서 freq, amp의 영향을 미침
	 * amp : 진폭
	 * - 노이즈 연산에 대한 결과의 영향력이라고 생각하면 됨
	 */
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

	static float PerlinFbm(float x, float y, float z, float freq, float octave)
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
			return Utils::Smootherstep(0.0f, 0.14f, w);
		}
		else if (value <= -0.21f) {
			float w = (value - -0.51f) / (-0.21f - -0.51f);
			return Utils::Smootherstep(0.14f, 0.31f, w);
		}
		else if (value <= -0.10f) {
			float w = (value - -0.21f) / (-0.10f - -0.21f);
			return Utils::Smootherstep(0.31f, 0.43f, w);
		}
		else if (value <= 0.09f) {
			float w = (value - -0.10f) / (0.09f - -0.10f);
			return Utils::Smootherstep(0.43f, 0.57f, w);
		}
		else if (value <= 0.42f) {
			float w = (value - 0.09f) / (0.42f - 0.09f);
			return Utils::Smootherstep(0.57f, 0.86f, w);
		}
		else {
			float w = (value - 0.42f) / (1.0f - 0.42f);
			return Utils::Smootherstep(0.86f, 1.0f, w);
		}
	}

	static float GetContinentalness(float x, float z) 
	{ 
		float scale = 1024.0f;
		
		float freq = 3.0f;
		int octave = 6;

		float cNoise = PerlinFbm(x / scale, z / scale, freq, octave);
		float cValue = SplineContinentalness(cNoise);

		return cValue;
	}

	static float SplineErosion(float value) 
	{
		value = std::clamp(value * 1.5f, -1.0f, 1.0f);

		if (value <= -0.54f) {
			float w = (value - -1.0f) / (-0.54f - -1.0f);
			return Utils::Smootherstep(0.01f, 0.14f, w);
		}
		else if (value <= 0.03f) {
			float w = (value - -0.54f) / (0.03f - -0.54f);
			return Utils::Smootherstep(0.14f, 0.43f, w);
		}
		else if (value <= 0.32f) {
			float w = (value - 0.03f) / (0.32f - 0.03f);
			return Utils::Smootherstep(0.43f, 0.57f, w);
		}
		else if (value <= 0.39f) {
			float w = (value - 0.32f) / (0.39f - 0.32f);
			return Utils::Smootherstep(0.57f, 0.71f, w);
		}
		else if (value <= 0.78f) {
			float w = (value - 0.39f) / (0.78f - 0.39f);
			return Utils::Smootherstep(0.71f, 0.86f, w);
		}
		else {
			float w = (value - 0.78f) / (1.0f - 0.78f);
			return Utils::Smootherstep(0.86f, 1.0f, w);
		}
	}

	static float GetErosion(float x, float z) 
	{
		float scale = 1024.0f;
		float bias = 123.0f;

		float freq = 2.0f;
		int octave = 3;

		float cNoise = PerlinFbm(x / scale + bias, z / scale + bias, freq, octave);
		float cValue = SplineErosion(cNoise);
		
		return cValue;
	}

	static float SplinePeaksValley(float value) 
	{ 
		value = abs(std::clamp(value * 1.5f, -1.0f, 1.0f));

		if (value <= 0.05f) {
			float w = (value - 0.0f) / (0.05f - 0.0f);
			return Utils::Smootherstep(0.01f, 0.07f, w);
		}
		else if (value <= 0.10f) {
			float w = (value - 0.05f) / (0.10f - 0.05f);
			return Utils::Smootherstep(0.07f, 0.12f, w);
		}
		else if (value <= 0.24f) {
			float w = (value - 0.05f) / (0.24f - 0.05f);
			return Utils::Smootherstep(0.12f, 0.52f, w);
		}
		else if (value <= 0.42f) {
			float w = (value - 0.24f) / (0.42f - 0.24f);
			return Utils::Smootherstep(0.52f, 0.64f, w);
		}
		else if (value <= 0.80f) {
			float w = (value - 0.42f) / (0.80f - 0.42f);
			return Utils::Smootherstep(0.64f, 1.0f, w);
		}
		else {
			float w = (value - 0.80f) / (1.0f - 0.80f);
			return Utils::Smootherstep(0.75f, 1.0f, 1.0f - w);
		}
	}

	static float GetPeaksValley(float x, float z)
	{
		float scale = 512.0f;
		float bias = 2.0f;

		float freq = 3.0f;
		int octave = 5;

		float cNoise = PerlinFbm(x / scale + bias, z / scale + bias, freq, octave);
		float cValue = SplinePeaksValley(cNoise);

		return cValue;
	}

	static float GetBaseLevel(float c, float e, float pv) 
	{	
		if (c <= 0.3f)
			c = c / 0.3f - 1.0f; // [-1.0f, 0.0f]
		else
			c = (c - 0.3f) / 0.7f; // [0.0f, 1.0f]
	
		pv = (pv - 0.5f) * 2.0f;
		                              // erosion이 높을 때만 c에 영향           // erosion이 작을 때만 pv에 영향
		float baseLevel = 64.0f + 64.0f * c * (1.0f - e * e) + 64.0f * pv * powf((1.0f - e), 2.0f);
		
		return baseLevel;
	}

	static float GetSquishFactor(float c, float e, float pv)
	{
		float w = c * e * (1.0f - pv);
		
		return Utils::Smootherstep(0.01f, 0.2f, w);
	}

	static float GetDensity(int x, int y, int z)
	{
		// 압축 인수
		// - 침식과 압축인수는 비례
		// 높이 편향 기준
		// - 대륙성에 따른 높이 편향

		float scale = 512.0f;
		float bias = 3.0f;

		float freq = 3.0f;
		int octave = 5.0f;

		float cNoise =
			PerlinFbm(x / scale + bias, y / scale + bias, z / scale + bias, freq, octave);

		return cNoise;
	}
}