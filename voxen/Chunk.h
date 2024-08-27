#pragma once

#include "Block.h"
#include "Instance.h"
#include "Structure.h"

#include <d3d11.h>
#include <wrl.h>
#include <directxtk/SimpleMath.h>
#include <vector>
#include <map>

using namespace Microsoft::WRL;
using namespace DirectX::SimpleMath;

struct ChunkInitMemory;

class Chunk {

public:
	static const int CHUNK_SIZE = 32;
	static const int CHUNK_SIZE2 = CHUNK_SIZE * CHUNK_SIZE;
	static const int CHUNK_SIZE_P = CHUNK_SIZE + 2;
	static const int CHUNK_SIZE_P2 = CHUNK_SIZE_P * CHUNK_SIZE_P;

	Chunk(UINT id);
	~Chunk();

	ChunkInitMemory* Initialize(ChunkInitMemory* memory);
	void Update(float dt);
	void Clear();

	inline UINT GetID() { return m_id; }

	inline void SetLoad(bool isLoaded) { m_isLoaded = isLoaded; }
	inline bool IsLoaded() { return m_isLoaded; }
	inline bool IsEmpty() { return IsEmptyOpaque() && IsEmptyTransparency() && IsEmptySemiAlpha(); }

	inline Vector3 GetOffsetPosition() { return m_offsetPosition; }
	inline void SetOffsetPosition(Vector3 offsetPosition) { m_offsetPosition = offsetPosition; }
	inline Vector3 GetPosition() { return m_position; }
	inline void SetUpdateRequired(bool isRequired) { m_isUpdateRequired = isRequired; }
	inline bool IsUpdateRequired() { return m_isUpdateRequired; }

	inline bool IsEmptyLowLod() { return m_lowLodVertices.empty(); }
	inline bool IsEmptyOpaque() { return m_opaqueVertices.empty(); }
	inline bool IsEmptyTransparency() { return m_transparencyVertices.empty(); }
	inline bool IsEmptySemiAlpha() { return m_semiAlphaVertices.empty(); }

	inline const std::vector<VoxelVertex>& GetLowLodVertices() const { return m_lowLodVertices; }
	inline const std::vector<uint32_t>& GetLowLodIndices() const { return m_lowLodIndices; }

	inline const std::vector<VoxelVertex>& GetOpaqueVertices() const { return m_opaqueVertices; }
	inline const std::vector<uint32_t>& GetOpaqueIndices() const { return m_opaqueIndices; }

	inline const std::vector<VoxelVertex>& GetTransparencyVertices() const
	{
		return m_transparencyVertices;
	}
	inline const std::vector<uint32_t>& GetTransparencyIndices() const
	{
		return m_transparencyIndices;
	}

	inline const std::vector<VoxelVertex>& GetSemiAlphaVertices() const
	{
		return m_semiAlphaVertices;
	}
	inline const std::vector<uint32_t>& GetSemiAlphaIndices() const { return m_semiAlphaIndices; }

	inline const std::map<std::tuple<int, int, int>, Instance>& GetInstanceMap() const
	{
		return m_instanceMap;
	}

	inline const ChunkConstantData& GetConstantData() const { return m_constantData; }

	uint8_t GetBlockTypeByPosition(Vector3 pos);

private:
	void InitChunkData();
	void InitInstanceInfoData();
	void InitWorldVerticesData(ChunkInitMemory* memory);

	void MakeFaceSliceColumnBit(uint64_t cullColBit[Chunk::CHUNK_SIZE_P2 * 6],
		uint64_t sliceColBit[Block::BLOCK_TYPE_COUNT][Chunk::CHUNK_SIZE2 * 6]);
	void GreedyMeshing(uint64_t faceColBit[Chunk::CHUNK_SIZE2 * 6],
		std::vector<VoxelVertex>& vertices,
		std::vector<uint32_t>& indices, uint8_t type);

	Block m_blocks[CHUNK_SIZE_P][CHUNK_SIZE_P][CHUNK_SIZE_P];
	std::map<std::tuple<int, int, int>, Instance> m_instanceMap;

	UINT m_id;
	bool m_isLoaded;
	bool m_isUpdateRequired;
	Vector3 m_offsetPosition;
	Vector3 m_position;
	
	std::vector<VoxelVertex> m_lowLodVertices;
	std::vector<uint32_t> m_lowLodIndices;

	std::vector<VoxelVertex> m_opaqueVertices;
	std::vector<uint32_t> m_opaqueIndices;

	std::vector<VoxelVertex> m_transparencyVertices;
	std::vector<uint32_t> m_transparencyIndices;

	std::vector<VoxelVertex> m_semiAlphaVertices;
	std::vector<uint32_t> m_semiAlphaIndices;

	ChunkConstantData m_constantData;
};

struct ChunkInitMemory {
	uint64_t llColBit[Chunk::CHUNK_SIZE_P2 * 3];
	uint64_t opColBit[Chunk::CHUNK_SIZE_P2 * 3];

	uint64_t llCullColBit[Chunk::CHUNK_SIZE_P2 * 6];
	uint64_t opCullColBit[Chunk::CHUNK_SIZE_P2 * 6];
	uint64_t tpCullColBit[Chunk::CHUNK_SIZE_P2 * 6];
	uint64_t saCullColBit[Chunk::CHUNK_SIZE_P2 * 6];

	uint64_t llSliceColBit[Block::BLOCK_TYPE_COUNT][Chunk::CHUNK_SIZE2 * 6];
	uint64_t opSliceColBit[Block::BLOCK_TYPE_COUNT][Chunk::CHUNK_SIZE2 * 6];
	uint64_t tpSliceColBit[Block::BLOCK_TYPE_COUNT][Chunk::CHUNK_SIZE2 * 6];
	uint64_t saSliceColBit[Block::BLOCK_TYPE_COUNT][Chunk::CHUNK_SIZE2 * 6];

	ChunkInitMemory()
		: llColBit{ 0 }, opColBit{ 0 }, llCullColBit{ 0 }, opCullColBit{ 0 }, tpCullColBit{ 0 },
		  saCullColBit{ 0 }, llSliceColBit{ 0 }, opSliceColBit{ 0 }, tpSliceColBit{ 0 },
		  saSliceColBit{ 0 }
	{
	}

	void Clear()
	{
		std::fill(std::begin(llColBit), std::end(llColBit), 0);
		std::fill(std::begin(opColBit), std::end(opColBit), 0);

		std::fill(std::begin(llCullColBit), std::end(llCullColBit), 0);
		std::fill(std::begin(opCullColBit), std::end(opCullColBit), 0);
		std::fill(std::begin(tpCullColBit), std::end(tpCullColBit), 0);
		std::fill(std::begin(saCullColBit), std::end(saCullColBit), 0);

		for (int i = 0; i < Block::BLOCK_TYPE_COUNT; ++i) {
			std::fill(std::begin(llSliceColBit[i]), std::end(llSliceColBit[i]), 0);
			std::fill(std::begin(opSliceColBit[i]), std::end(opSliceColBit[i]), 0);
			std::fill(std::begin(tpSliceColBit[i]), std::end(tpSliceColBit[i]), 0);
			std::fill(std::begin(saSliceColBit[i]), std::end(saSliceColBit[i]), 0);
		}
	}
};