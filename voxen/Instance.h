#pragma once

#include <stdint.h>
#include <directxtk/SimpleMath.h>

using namespace DirectX::SimpleMath;



class Instance {
public:
	static const int INSTANCE_TYPE_COUNT = 3;

	static inline INSTANCE_TYPE GetInstanceType(uint8_t texIndex)
	{
		if (128 <= texIndex && texIndex < 128 + 16)
			return INSTANCE_TYPE::I_CROSS;
		else if (128 + 16 <= texIndex && texIndex < 128 + 16 * 2)
			return INSTANCE_TYPE::I_FENCE;
		else if (128 + 16 * 2 <= texIndex && texIndex < 128 + 16 * 3)
			return INSTANCE_TYPE::I_SQUARE;
		else
			return INSTANCE_TYPE::I_NONE;
	}

	Instance() : m_world(Matrix()), m_texIndex(TEXTURE_INDEX::T_SHORT_GRASS) {}
	~Instance() {}

	inline TEXTURE_INDEX GetTextureIndex() const { return m_texIndex; }
	inline void SetTextureIndex(TEXTURE_INDEX index) { m_texIndex = index; }
	inline const Matrix& GetWorld() const { return m_world; }
	inline void SetWorld(const Matrix& world) { m_world = world; }

private:
	Matrix m_world; // scale rotate position
	TEXTURE_INDEX m_texIndex;
};