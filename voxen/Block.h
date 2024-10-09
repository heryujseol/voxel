#pragma once

#include <stdint.h>

#include "Structure.h"

class Block {
public:
	static const int BLOCK_TYPE_COUNT = 256;
	static inline bool IsOpaqua(BLOCK_TYPE type) { return (1 < type && type < 10);  } // �ӽ� ������
	static inline bool IsSemiAlpha(BLOCK_TYPE type) { return (10 <= type && type < 20); } // �ӽ� ������
	static inline bool IsTransparency(BLOCK_TYPE type) { return (type <= 1); } // �ӽ� ������

	Block() : m_type(BLOCK_TYPE::B_AIR) {}
	~Block() {}

	inline BLOCK_TYPE GetType() const { return m_type; }
	inline void SetType(BLOCK_TYPE type) { m_type = type; }

private:
	BLOCK_TYPE m_type;
};