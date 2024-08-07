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
		float scale = 1080.0f;
		float bias = 101.0f;
		
		float freq = 3.0f;
		float cNoise = PerlinFbm(x / scale + bias, z / scale + bias, freq, 6);
		
		float cValue = SplineContinentalness(cNoise);

		return cValue;
	}

	static float SplineErosion(float value) 
	{
		value = std::clamp(value * 1.5f, -1.0f, 1.0f);

		if (value <= -0.78f) { // 0.14
			float w = (value - -1.0f) / (-0.78f - -1.0f);
			return Utils::Smootherstep(0.0f, 0.14f, w);
		}
		else if (value <= -0.64f) { // 0.14
			float w = (value - -0.78f) / (-0.64f - -0.78f);
			return Utils::Smootherstep(0.14f, 0.24f, w);
		}
		else if (value <= -0.48f) { // 0.29
			float w = (value - -0.64f) / (-0.48f - -0.64f);
			return Utils::Smootherstep(0.24f, 0.29f, w);
		}
		else if (value <= 0.03f) { // 0.43
			float w = (value - -0.48f) / (0.03f - -0.48f);
			return Utils::Smootherstep(0.29f, 0.43f, w);
		}
		else if (value <= 0.32f) { // 0.57
			float w = (value - 0.03f) / (0.32f - 0.03f);
			return Utils::Smootherstep(0.43f, 0.57f, w);
		}
		else if (value <= 0.39f) { // 0.71
			float w = (value - 0.32f) / (0.39f - 0.32f);
			return Utils::Smootherstep(0.57f, 0.71f, w);
		}
		else if (value <= 0.78f) { // 0.86
			float w = (value - 0.39f) / (0.78f - 0.39f);
			return Utils::Smootherstep(0.71f, 0.86f, w);
		}
		else { // 1.0
			float w = (value - 0.78f) / (1.0f - 0.78f);
			return Utils::Smootherstep(0.86f, 1.0f, w);
		}
	}

	static float GetErosion(float x, float z) 
	{
		float scale = 1080.0f;
		float bias = 283.0f;

		float freq = 2.0f;
		float cNoise = PerlinFbm(x / scale + bias, z / scale + bias, freq, 5);

		float cValue = SplineErosion(cNoise);

		return cValue;
	}

	static float SplinePeaksValley(float value) 
	{ 
		value = abs(std::clamp(value * 1.5f, -1.0f, 1.0f));

		if (value <= 0.12f) {
			float w = value / 0.12f;
			return Utils::Smootherstep(0.01f, 0.20f, w);
		}
		else if (value <= 0.24f) {
			return 0.20f;
		}
		else if (value <= 0.32f) {
			float w = (value - 0.24f) / (0.32f - 0.24f);
			return Utils::Smootherstep(0.21f, 0.4f, w);
		}
		else if (value <= 0.73f) {
			float w = (value - 0.28f) / (0.73f - 0.28f);
			return Utils::Smootherstep(0.4f, 1.0f, w);
		}
		else if (value <= 0.86f) {
			float w = (value - 0.73f) / (0.86f - 0.73f);
			return Utils::Smootherstep(1.0f, 0.7f, w);
		}
		else {
			float w = (value - 0.86f) / (1.0f - 0.86f);
			return Utils::Smootherstep(0.4f, 0.7f, 1.0f - w);
		}
	}

	static float GetPeaksValley(float x, float z)
	{
		float scale = 540.0f;
		float bias = 797.0f;

		float freq = 3.0f;
		float cNoise = PerlinFbm(x / scale + bias, z / scale + bias, freq, 3);

		float cValue = SplinePeaksValley(cNoise);

		return cValue;
	}

	static uint8_t GetHeight(int x, int z) 
	{
		float c = GetContinentalness(x, z);
		float e = GetErosion(x, z);
		float pv = GetPeaksValley(x, z);
		
		float retH;

		if (c <= 0.3f)
			c = c / 0.3f - 1.0f; // [-1.0f, 0.0f]
		else
			c = (c - 0.3f) / 0.7f; // [0.0f, 1.0f]

		if (pv <= 0.18f) 
			pv = pv / 0.18f - 1.0f; // [-1.0f, 0.0f]
		else
			pv = (pv - 0.18f) / 0.8f; // [0,0f, 1.0f]

		// 대륙성 값
		// 대륙성이 크다는 것은 대륙 내부
		// 대륙성이 작다는 것은 바다 라고 판단
		// 대륙성 기준 값을 기준으로 바다 or 대륙 판단
		
		// 침식 값
		// 침식 값이 크다는 것은 바다와 공기 근처 사이가 평평함
		// 침식 값이 작다는 것은 바다와 공기 근처 사이가 차이남
		// 침식 기준 값을 기준으로 어디에 가까워질지 판단

		// 봉우리, 계곡 값
		// 값이 높으면 올라가고 값이 낮으면 내려감
		// 침식의 영향을 당연히 받음

		float baseHeight = 64.0f + 64.0f * c * (1.0f - e);
		float retHeight = baseHeight * (Utils::Sigmoid(2.0f * (1.0f - e) * pv) + 0.5f);
		
		return (uint8_t)(max(retHeight, 1.0f));
	}
}