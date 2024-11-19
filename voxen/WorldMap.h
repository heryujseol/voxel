#pragma once

#include <directxtk/SimpleMath.h>
#include <vector>

#include "ChunkManager.h"

using namespace DirectX::SimpleMath;

class WorldMap {

public:
	static const UINT WORLD_MAP_PIXEL_SIZE = sizeof(uint8_t) * 4;

	static const UINT WORLD_MAP_BUFFER_SIZE = 512;
	static const UINT WORLD_MAP_UI_SIZE = 720;

	static const UINT WORLD_SIZE_PER_PIXEL = (ChunkManager::CHUNK_COUNT * Chunk::CHUNK_SIZE * 2) / WORLD_MAP_BUFFER_SIZE;
	static const UINT WORLD_SIZE = WORLD_MAP_BUFFER_SIZE * WORLD_SIZE_PER_PIXEL;


	WorldMap();
	~WorldMap();

	bool Initialize(Vector3 cameraPosition);
	void Update(Vector3 cameraPosition);
	void Render();

private:
	std::vector<uint8_t> m_mapData;
	Vector3 m_offsetPosition;

	bool UpdateBuffer();
	void ShiftMapData(int dx, int dz);
	Vector3 GenerateWorldMapColor(int x, int z);
	void UpdateMapData(int dx, int dz);
};
