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
		
		float inter_x0 = Utils::CubicLerp(n0, n1, p.x - (float)x0);
		float inter_x1 = Utils::CubicLerp(n2, n3, p.x - (float)x0);
		float inter_y = Utils::CubicLerp(inter_x0, inter_x1, p.y - (float)y0);

		return inter_y;
	}

	/*
	 * x, y : 좌표
	 * - 현재 x, y가 중요한게 아니라, 해당 함수를 호출할 때 x, y의 증감 폭이 중요한 것
	 * - 증감이 작아야 함, 크면 점진적인 변화율을 체크할 수 없음
	 * freq : 진동수
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

	static float SplineContinentalness(float value)
	{
		if (value <= -0.95f) {
			float w = (value + 1.0f) / 0.05f;
			return Utils::Smootherstep(1.0f, 0.05f, w);
		}
		else if (value <= -0.51f) {
			return 0.05f;
		}
		else if (value <= -0.43f) {
			float w = (value + 0.51f) / 0.08f;
			return Utils::Smootherstep(0.05f, 0.4f, w);
		}
		else if (value <= -0.19f) {
			return 0.4f;
		}
		else if (value <= -0.17f) {
			float w = (value + 0.19f) / 0.02f;
			return Utils::Smootherstep(0.4f, 0.875f, w);
		}
		else if (value <= -0.11f) {
			float w = (value + 0.17f) / 0.06f;
			return Utils::Smootherstep(0.875f, 0.9f, w);
		}
		else if (value < 0.23f) {
			float w = (value + 0.11f) / 0.34f;
			return Utils::Smootherstep(0.9f, 0.96f, w);
		}
		else {
			float w = (value - 0.23f) / 0.77f;
			return Utils::Smootherstep(0.96f, 0.99f, w);
		}
	}

	static float GetNoiseContinentalness(float x, float z) 
	{ 
		float scale = 720.0f;
		
		float freq = 4.0f;
		float cNoise = PerlinFbm(x / scale, z / scale, freq, 7);
		
		float cValue = SplineContinentalness(cNoise);
		//std::cout << cNoise << ", " << cValue << std::endl;

		return cValue;
	}

	static float SplineErosion(float value) 
	{
		if (value <= -0.8f) {
			float w = (value + 1.0f) / 0.2f;
			return Utils::Smootherstep(1.0f, 0.76f, w);
		}
		else if (value <= -0.5f) {
			float w = (value + 0.8f) / 0.3f;
			return Utils::Smootherstep(0.76f, 0.5f, w);
		}
		else if (value <= -0.45f) {
			float w = (value + 0.5f) / 0.05f;
			return Utils::Smootherstep(0.5f, 0.56f, w);
		}
		else if (value <= -0.05f) {
			float w = (value + 0.45f) / 0.4f;
			return Utils::Smootherstep(0.56f, 0.1f, w);
		}
		else if (value <= 0.6f) {
			float w = (value + 0.05f) / 0.65f;
			return Utils::Smootherstep(0.1f, 0.09f, w);
		}
		else if (value <= 0.65f) {
			float w = (value - 0.6f) / 0.05f;
			return Utils::Smootherstep(0.09f, 0.3f, w);
		}
		else if (value <= 0.75f) {
			float w = (value - 0.65f) / 0.1f;
			return Utils::Smootherstep(0.3f, 0.3f, w);
		}
		else if (value <= 0.8f) {
			float w = (value - 0.75f) / 0.05f;
			return Utils::Smootherstep(0.3f, 0.09f, w);
		}
		else {
			float w = (value - 0.8f) / 0.2f;
			return Utils::Smootherstep(0.09f, 0.01f, w);
		}
	}

	static float GetNoiseErosion(float x, float z) 
	{
		float scale = 1080.0f;

		float freq = 2.0f;
		float cNoise = PerlinFbm(x / scale, z / scale, freq, 5);

		float cValue = SplineErosion(cNoise);

		return cValue;
	}

	static float SplinePeaksValley(float value) 
	{ 
		if (value <= 0.0f) {
			float w = (value + 1.0f) / 1.0f;
			return Utils::Smootherstep(0.01f, 0.32f, w);
		}
		else if (value <= 0.4f) {
			float w = (value - 0.0f) / 0.4f;
			return Utils::Smootherstep(0.32f, 0.88f, w);
		}
		else if (value <= 0.6f) {
			float w = (value - 0.4f) / 0.2f;
			return Utils::Smootherstep(0.88f, 0.96f, w);
		}
		else if (value <= 0.85f) {
			float w = (value - 0.6f) / 0.25f;
			return Utils::Smootherstep(0.96f, 0.92f, w);
		}
		else {
			float w = (value - 0.85f) / 0.15f;
			return Utils::Smootherstep(0.92f, 0.99f, w);
		}
	}

	static float GetNoisePeaksValley(float x, float z)
	{
		float scale = 128.0f;

		float freq = 2.0f;
		float cNoise = PerlinFbm(x / scale, z / scale, freq, 3);

		float cValue = SplinePeaksValley(cNoise);

		return cValue;
	}

	static uint8_t GetHeight(int x, int z) 
	{
		float c = GetNoiseContinentalness(x, z);
		float e = GetNoiseErosion(x, z);
		float pv = GetNoisePeaksValley(x, z);
		
		float baseH = c * 64.0f;
		float detailH = (1.0f - e) * pv * 48.0f;

		return baseH + detailH;
	}
}