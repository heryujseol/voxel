#pragma once

#include <stdint.h>

#include "Structure.h"

class Block {
public:
	static const int BLOCK_TYPE_COUNT = 256;
	static inline bool IsOpaqua(BLOCK_TYPE type) { return (1 < type && type < 32);  } // �ӽ� ������
	static inline bool IsSemiAlpha(BLOCK_TYPE type) { return (32 <= type && type < 128); } // �ӽ� ������
	static inline bool IsTransparency(BLOCK_TYPE type) { return (type <= 1); } // �ӽ� ������

	Block() : m_type(BLOCK_TYPE::BLOCK_AIR) {}
	~Block() {}

	inline BLOCK_TYPE GetType() const { return m_type; }
	inline void SetType(BLOCK_TYPE type) { m_type = type; }

private:
	BLOCK_TYPE m_type;
};