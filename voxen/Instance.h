#pragma once

#include <stdint.h>
#include <directxtk/SimpleMath.h>

using namespace DirectX::SimpleMath;

enum INSTANCE_TYPE : uint8_t {
	CROSS = 0,
	FENCE = 1,
	SQUARE = 2,
	NONE = 3,
};

class Instance {
public:
	static const int INSTANCE_TYPE_COUNT = 3;

	static inline INSTANCE_TYPE GetInstanceType(uint8_t type)
	{
		if (128 <= type && type < 128 + 16)
			return INSTANCE_TYPE::CROSS;
		else if (128 + 16 <= type && type < 128 + 16 * 2)
			return INSTANCE_TYPE::FENCE;
		else if (128 + 16 * 2 <= type && type < 128 + 16 * 3)
			return INSTANCE_TYPE::SQUARE;
		else
			return INSTANCE_TYPE::NONE;
	}

	Instance() : m_type(0), m_world(Matrix()) {}
	~Instance() {}

	inline uint8_t GetType() const { return m_type; }
	inline void SetType(uint8_t type) { m_type = type; }
	inline const Matrix& GetWorld() const { return m_world; }
	inline void SetWorld(const Matrix& world) { m_world = world; }

private:
	uint8_t m_type;
	Matrix m_world; // scale rotate position
};