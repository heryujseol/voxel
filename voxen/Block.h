#pragma once

#include <stdint.h>

#include "Structure.h"

class Block {
public:
	static const int BLOCK_TYPE_COUNT = 256;
	static inline bool IsOpaqua(BLOCK_TYPE type) { return (1 < type && type < 10);  } // 임시 데이터
	static inline bool IsSemiAlpha(BLOCK_TYPE type) { return (10 <= type && type < 20); } // 임시 데이터
	static inline bool IsTransparency(BLOCK_TYPE type) { return (type <= 1); } // 임시 데이터

	Block() : m_type(BLOCK_TYPE::B_AIR) {}
	~Block() {}

	inline BLOCK_TYPE GetType() const { return m_type; }
	inline void SetType(BLOCK_TYPE type) { m_type = type; }

private:
	BLOCK_TYPE m_type;
};