#pragma once

#include <stdint.h>

enum BLOCK_TYPE : uint8_t {
	AIR = 0,
	WATER = 1,
	LEAF = 10,
};

class Block {
public:
	static const int BLOCK_TYPE_COUNT = 256;
	static inline bool IsOpaqua(uint8_t type) { return (1 < type && type < 10);  } // 임시 데이터
	static inline bool IsSemiAlpha(uint8_t type) { return (10 <= type && type < 20); } // 임시 데이터
	static inline bool IsTransparency(uint8_t type) { return (type <= 1); } // 임시 데이터

	Block() : m_type(0) {}
	~Block() {}

	inline uint8_t GetType() const { return m_type; }
	inline void SetType(uint8_t type) { m_type = type; }

private:
	uint8_t m_type;
};