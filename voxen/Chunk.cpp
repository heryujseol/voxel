#include "Chunk.h"
#include "DXUtils.h"
#include "MeshGenerator.h"

#include <future>
#include <algorithm>
#include <unordered_map>

Chunk::Chunk(UINT id)
	: m_id(id), m_isLoaded(false), m_offsetPosition(0.0f, 0.0f, 0.0f), m_position(0.0f, 0.0f, 0.0f),
	  m_isUpdateRequired(true)
{
}

Chunk::~Chunk() { Clear(); }

ChunkInitMemory* Chunk::Initialize(ChunkInitMemory* memory)
{
	////////////////////////////////////
	// check start time
	static long long sum = 0;
	static long long count = 0;
	auto start_time = std::chrono::steady_clock::now();
	////////////////////////////////////

	m_isLoaded = false;
	m_isUpdateRequired = false;

	// 0. initialize chunk data
	InitChunkData();

	// 1. intialize instance vertcie data
	InitInstanceInfoData();

	// 2. initialize world(opaque & water & semiAlpha) vertice data by greedy meshing
	InitWorldVerticesData(memory);

	// 3. initialize constant data
	m_position = Vector3(m_offsetPosition.x, -2.0f * CHUNK_SIZE, m_offsetPosition.z);
	m_constantData.world = Matrix::CreateTranslation(m_position);

	////////////////////////////////////
	// check end time
	auto end_time = std::chrono::steady_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
	sum += duration.count();
	count++;
	std::cout << "duration: " << duration.count() << " micro s"
			  << " | "
			  << "average: " << (float)sum / (float)count << " micro s" << std::endl;
	////////////////////////////////////

	return memory;
}

void Chunk::Update(float dt)
{
	m_position.y += 50.0f * dt;
	if (m_position.y > m_offsetPosition.y) {
		m_position.y = m_offsetPosition.y;
	}
	m_constantData.world = Matrix::CreateTranslation(m_position);
}

void Chunk::Clear()
{
	m_lowLodVertices.clear();
	m_lowLodIndices.clear();

	m_opaqueVertices.clear();
	m_opaqueIndices.clear();

	m_transparencyVertices.clear();
	m_transparencyIndices.clear();

	m_semiAlphaVertices.clear();
	m_semiAlphaIndices.clear();

	m_instanceMap.clear();
}

void Chunk::InitChunkData()
{
	for (int x = 0; x < CHUNK_SIZE_P; ++x) {
		for (int z = 0; z < CHUNK_SIZE_P; ++z) {
			int worldX = (int)m_offsetPosition.x + x - 1;
			int worldZ = (int)m_offsetPosition.z + z - 1;

			float continentalness = Terrain::GetContinentalness(worldX, worldZ);
			float erosion = Terrain::GetErosion(worldX, worldZ);
			float peaksValley = Terrain::GetPeaksValley(worldX, worldZ);
			float baseLevel = Terrain::GetBaseLevel(continentalness, erosion, peaksValley);

			float temperature = Terrain::GetTemperature(worldX, worldZ);
			float humidity = Terrain::GetHumidity(worldX, worldZ);
			BIOME_TYPE biome = Terrain::GetBiome(temperature, humidity);
			m_biomes[x][z] = biome;

			for (int y = 0; y < CHUNK_SIZE_P; ++y) {
				int worldY = (int)m_offsetPosition.y + y - 1;

				// set static transparency type
				if (worldY == 256) {
					m_blocks[x][y][z].SetType(BLOCK_TYPE::B_AIR);
					continue;
				}
				if (worldY == 0) {
					m_blocks[x][y][z].SetType(BLOCK_TYPE::B_BEDROCK);
					continue;
				}
				BLOCK_TYPE transparencyType =
					worldY <= 63 ? BLOCK_TYPE::B_WATER : BLOCK_TYPE::B_AIR;
				m_blocks[x][y][z].SetType(transparencyType);

				if (worldY <= baseLevel) {
					// set block type
					BLOCK_TYPE type = Terrain::GetBlockType(biome, baseLevel, worldY);
					m_blocks[x][y][z].SetType(type);

					// cave
					float d1 = Terrain::GetDensity(worldX, worldY, worldZ, 3.0f, 256.0f);
					float d2 = Terrain::GetDensity2(worldX, worldY, worldZ, 123.0f, 512.0f);
					if (d1 * d1 + d2 * d2 <= 0.004f) {
						m_blocks[x][y][z].SetType(transparencyType);
					}
				}
			}
		}
	}
}

void Chunk::InitInstanceInfoData()
{
	for (int x = 0; x < CHUNK_SIZE; ++x) {
		for (int y = 0; y < CHUNK_SIZE; ++y) {
			for (int z = 0; z < CHUNK_SIZE; ++z) {

				// instance testing
				if ((m_blocks[x + 1][y + 1][z + 1].GetType() == BLOCK_TYPE::B_AIR ||
						m_blocks[x + 1][y + 1][z + 1].GetType() == BLOCK_TYPE::B_WATER) &&
					Block::IsOpaqua(m_blocks[x + 1][y][z + 1].GetType()) && x % 3 == 0 &&
					z % 3 == 0) {
					Instance instance;

					instance.SetTextureIndex(TEXTURE_INDEX::T_SHORT_GRASS);

					instance.SetBiome(m_biomes[x + 1][z + 1]);

					Vector3 pos = Vector3((float)x, (float)y, (float)z) + Vector3(0.5f);
					instance.SetWorld(Matrix::CreateTranslation(pos));

					m_instanceMap[std::make_tuple(x, y, z)] = instance;
				}
			}
		}
	}
}

void Chunk::InitWorldVerticesData(ChunkInitMemory* memory)
{
	memory->Clear();

	// 1. make axis column bit data
	std::unordered_map<BLOCK_TYPE, bool> llTypeMap;
	std::unordered_map<BLOCK_TYPE, bool> opTypeMap;
	std::unordered_map<BLOCK_TYPE, bool> tpTypeMap;
	std::unordered_map<BLOCK_TYPE, bool> saTypeMap;

	std::unordered_map<BIOME_TYPE, bool> biomeTypeMap;

	// 2. cull face column bit
	// 0: x axis & left->right side (- => + : dir +)
	// 1: x axis & right->left side (+ => - : dir -)
	// 2: y axis & bottom->top side (- => + : dir +)
	// 3: y axis & top->bottom side (+ => - : dir -)
	// 4: z axis & front->back side (- => + : dir +)
	// 5: z axis & back->front side (+ => - : dir -)
	for (int x = 0; x < CHUNK_SIZE_P; ++x) {
		for (int y = 0; y < CHUNK_SIZE_P; ++y) {
			for (int z = 0; z < CHUNK_SIZE_P; ++z) {
				biomeTypeMap[m_biomes[x][z]] = true;
				BLOCK_TYPE type = m_blocks[x][y][z].GetType();

				if (type == BLOCK_TYPE::B_AIR)
					continue;

				if (Block::IsTransparency(type)) {
					tpTypeMap[type] = true;

					// 타입이 같거나 불투명 블록이면 메쉬를 생성하지 않음
					if (x - 1 >= 0 && type != m_blocks[x - 1][y][z].GetType() &&
						!Block::IsOpaqua(m_blocks[x - 1][y][z].GetType())) {
						memory->tpCullColBit[Utils::GetIndexFrom3D(0, y, z, CHUNK_SIZE_P)] |=
							(1ULL << x);
					}
					if (x + 1 < CHUNK_SIZE_P && type != m_blocks[x + 1][y][z].GetType() &&
						!Block::IsOpaqua(m_blocks[x + 1][y][z].GetType())) {
						memory->tpCullColBit[Utils::GetIndexFrom3D(1, y, z, CHUNK_SIZE_P)] |=
							(1ULL << x);
					}

					if (y - 1 >= 0 && type != m_blocks[x][y - 1][z].GetType() &&
						!Block::IsOpaqua(m_blocks[x][y - 1][z].GetType())) {
						memory->tpCullColBit[Utils::GetIndexFrom3D(2, z, x, CHUNK_SIZE_P)] |=
							(1ULL << y);
					}
					if (y + 1 < CHUNK_SIZE_P && type != m_blocks[x][y + 1][z].GetType() &&
						!Block::IsOpaqua(m_blocks[x][y + 1][z].GetType())) {
						memory->tpCullColBit[Utils::GetIndexFrom3D(3, z, x, CHUNK_SIZE_P)] |=
							(1ULL << y);
					}

					if (z - 1 >= 0 && type != m_blocks[x][y][z - 1].GetType() &&
						!Block::IsOpaqua(m_blocks[x][y][z - 1].GetType())) {
						memory->tpCullColBit[Utils::GetIndexFrom3D(4, y, x, CHUNK_SIZE_P)] |=
							(1ULL << z);
					}
					if (z + 1 < CHUNK_SIZE_P && type != m_blocks[x][y][z + 1].GetType() &&
						!Block::IsOpaqua(m_blocks[x][y][z + 1].GetType())) {
						memory->tpCullColBit[Utils::GetIndexFrom3D(5, y, x, CHUNK_SIZE_P)] |=
							(1ULL << z);
					}
				}
				else if (Block::IsSemiAlpha(type)) {
					saTypeMap[type] = true;
					// - -> + : 불투명이 아니면 페이스 존재 -> 같은 타입을 고려하지 않음
					if (x + 1 < CHUNK_SIZE_P && !Block::IsOpaqua(m_blocks[x + 1][y][z].GetType())) {
						memory->saCullColBit[Utils::GetIndexFrom3D(1, y, z, CHUNK_SIZE_P)] |=
							(1ULL << x);
					}
					if (y + 1 < CHUNK_SIZE_P && !Block::IsOpaqua(m_blocks[x][y + 1][z].GetType())) {
						memory->saCullColBit[Utils::GetIndexFrom3D(3, z, x, CHUNK_SIZE_P)] |=
							(1ULL << y);
					}
					if (z + 1 < CHUNK_SIZE_P && !Block::IsOpaqua(m_blocks[x][y][z + 1].GetType())) {
						memory->saCullColBit[Utils::GetIndexFrom3D(5, y, x, CHUNK_SIZE_P)] |=
							(1ULL << z);
					}

					// + -> - : 투명일 때만 페이스 존재 -> 같은 타입을 고려하지 않음
					if (x - 1 >= 0 && Block::IsTransparency(m_blocks[x - 1][y][z].GetType())) {
						memory->saCullColBit[Utils::GetIndexFrom3D(0, y, z, CHUNK_SIZE_P)] |=
							(1ULL << x);
					}
					if (y - 1 >= 0 && Block::IsTransparency(m_blocks[x][y - 1][z].GetType())) {
						memory->saCullColBit[Utils::GetIndexFrom3D(2, z, x, CHUNK_SIZE_P)] |=
							(1ULL << y);
					}
					if (z - 1 >= 0 && Block::IsTransparency(m_blocks[x][y][z - 1].GetType())) {
						memory->saCullColBit[Utils::GetIndexFrom3D(4, y, x, CHUNK_SIZE_P)] |=
							(1ULL << z);
					}

					llTypeMap[type] = true;
					memory->llColBit[Utils::GetIndexFrom3D(0, y, z, CHUNK_SIZE_P)] |= (1ULL << x);
					memory->llColBit[Utils::GetIndexFrom3D(1, z, x, CHUNK_SIZE_P)] |= (1ULL << y);
					memory->llColBit[Utils::GetIndexFrom3D(2, y, x, CHUNK_SIZE_P)] |= (1ULL << z);
				}
				else {
					opTypeMap[type] = true;
					memory->opColBit[Utils::GetIndexFrom3D(0, y, z, CHUNK_SIZE_P)] |= (1ULL << x);
					memory->opColBit[Utils::GetIndexFrom3D(1, z, x, CHUNK_SIZE_P)] |= (1ULL << y);
					memory->opColBit[Utils::GetIndexFrom3D(2, y, x, CHUNK_SIZE_P)] |= (1ULL << z);

					llTypeMap[type] = true;
					memory->llColBit[Utils::GetIndexFrom3D(0, y, z, CHUNK_SIZE_P)] |= (1ULL << x);
					memory->llColBit[Utils::GetIndexFrom3D(1, z, x, CHUNK_SIZE_P)] |= (1ULL << y);
					memory->llColBit[Utils::GetIndexFrom3D(2, y, x, CHUNK_SIZE_P)] |= (1ULL << z);
				}
			}
		}
	}


	// 3. lowlod & opaque face culling
	for (int axis = 0; axis < 3; ++axis) {
		for (int h = 1; h < CHUNK_SIZE_P - 1; ++h) {
			for (int w = 1; w < CHUNK_SIZE_P - 1; ++w) {
				uint64_t llBit = memory->llColBit[Utils::GetIndexFrom3D(axis, h, w, CHUNK_SIZE_P)];
				memory->llCullColBit[Utils::GetIndexFrom3D(axis * 2 + 0, h, w, CHUNK_SIZE_P)] =
					llBit & ~(llBit << 1);
				memory->llCullColBit[Utils::GetIndexFrom3D(axis * 2 + 1, h, w, CHUNK_SIZE_P)] =
					llBit & ~(llBit >> 1);

				uint64_t opBit = memory->opColBit[Utils::GetIndexFrom3D(axis, h, w, CHUNK_SIZE_P)];
				memory->opCullColBit[Utils::GetIndexFrom3D(axis * 2 + 0, h, w, CHUNK_SIZE_P)] =
					opBit & ~(opBit << 1);
				memory->opCullColBit[Utils::GetIndexFrom3D(axis * 2 + 1, h, w, CHUNK_SIZE_P)] =
					opBit & ~(opBit >> 1);
			}
		}
	}


	// 4. face cull column bit -> face slice column bit
	std::map<std::pair<BIOME_TYPE, BLOCK_TYPE>, std::vector<uint64_t>> llSliceColBit;
	std::map<std::pair<BIOME_TYPE, BLOCK_TYPE>, std::vector<uint64_t>> opSliceColBit;
	std::map<std::pair<BIOME_TYPE, BLOCK_TYPE>, std::vector<uint64_t>> tpSliceColBit;
	std::map<std::pair<BIOME_TYPE, BLOCK_TYPE>, std::vector<uint64_t>> saSliceColBit;

	MakeFaceSliceColumnBit(memory->llCullColBit, llSliceColBit);
	MakeFaceSliceColumnBit(memory->opCullColBit, opSliceColBit);
	MakeFaceSliceColumnBit(memory->tpCullColBit, tpSliceColBit);
	MakeFaceSliceColumnBit(memory->saCullColBit, saSliceColBit);


	// 5. make vertices by bit slices column (greedy meshing)
	for (const auto& b : biomeTypeMap) {

		for (const auto& t : llTypeMap) {
			const auto& p = std::make_pair(b.first, t.first);
			if (llSliceColBit.find(p) != llSliceColBit.end()) {
				GreedyMeshing(llSliceColBit[p], m_lowLodVertices, m_lowLodIndices, p);
			}
		}

		for (const auto& t : opTypeMap) {
			const auto& p = std::make_pair(b.first, t.first);
			if (opSliceColBit.find(p) != opSliceColBit.end()) {
				GreedyMeshing(opSliceColBit[p], m_opaqueVertices, m_opaqueIndices, p);
			}
		}

		for (const auto& t : tpTypeMap) {
			const auto& p = std::make_pair(b.first, t.first);
			if (tpSliceColBit.find(p) != tpSliceColBit.end()) {
				GreedyMeshing(tpSliceColBit[p], m_transparencyVertices, m_transparencyIndices, p);
			}
		}

		for (const auto& t : saTypeMap) {
			const auto& p = std::make_pair(b.first, t.first);
			if (saSliceColBit.find(p) != saSliceColBit.end()) {
				GreedyMeshing(saSliceColBit[p], m_semiAlphaVertices, m_semiAlphaIndices, p);
			}
		}
	}
}

void Chunk::MakeFaceSliceColumnBit(uint64_t cullColBit[Chunk::CHUNK_SIZE_P2 * 6],
	std::map<std::pair<BIOME_TYPE, BLOCK_TYPE>, std::vector<uint64_t>>& sliceColBit)
{
	/*
	 *     ---------------
	 *    / x    y    z /| <= 5:face, h:1, w:2 bits
	 *   / d    e    f / |
	 *  | 4    5    6 |  |
	 *  |			  |  |	     xi   jy  zk (3rd z slice - => +)
	 *  |   i    j    |k /  =>   da   eb  fc (2nd z slice - => +)   => bitPos->slice, h->shift, w->w
	 *  |  a    b    c| /        41   52  63 (1st z slice - => +)
	 *  | 1    2    3 |/
	 *  ---------------
	 */

	for (int face = 0; face < 6; ++face) {
		for (int h = 0; h < CHUNK_SIZE; ++h) {
			for (int w = 0; w < CHUNK_SIZE; ++w) {
				uint64_t colbit = // 34bit: P,CHUNK_SIZE,P
					cullColBit[Utils::GetIndexFrom3D(face, h + 1, w + 1, CHUNK_SIZE_P)];
				colbit = colbit >> 1;					 // 33bit: P,CHUNK_SIZE
				colbit = colbit & ~(1ULL << CHUNK_SIZE); // 32bit: CHUNK_SIZE

				while (colbit) {
					int bitPos = Utils::TrailingZeros(colbit); // 1110001000 -> trailing zero : 3
					colbit = colbit & (colbit - 1ULL);		   // 1110000000

					BIOME_TYPE biome = BIOME_TYPE::PLAINS;
					BLOCK_TYPE type = BLOCK_TYPE::B_AIR;
					if (face < 2) { // left right
						type = m_blocks[bitPos + 1][h + 1][w + 1].GetType();
						biome = m_biomes[bitPos + 1][w + 1];
					}
					else if (face < 4) { // top bottom
						type = m_blocks[w + 1][bitPos + 1][h + 1].GetType();
						biome = m_biomes[w + 1][h + 1];
					}
					else { //(face < 6) // front back
						type = m_blocks[w + 1][h + 1][bitPos + 1].GetType();
						biome = m_biomes[w + 1][bitPos + 1];
					}

					const auto& key = std::make_pair(biome, type);
					if (sliceColBit.find(key) == sliceColBit.end()) {
						sliceColBit[key] = std::vector<uint64_t>(Chunk::CHUNK_SIZE2 * 6, 0);
					}
					sliceColBit[key][Utils::GetIndexFrom3D(face, bitPos, w, CHUNK_SIZE)] |=
						(1ULL << h);
				}
			}
		}
	}
}

void Chunk::GreedyMeshing(std::vector<uint64_t>& faceColBit, std::vector<VoxelVertex>& vertices,
	std::vector<uint32_t>& indices, std::pair<BIOME_TYPE, BLOCK_TYPE> types)
{
	// face 0, 1 : left,right
	// face 2, 3 : top,bottom
	// face 4, 5 : front,back
	for (int face = 0; face < 6; ++face) {
		TEXTURE_INDEX textureIndex = Terrain::GetBlockTextureIndex(types.second, face);
		BIOME_TYPE biome = types.first;

		for (int s = 0; s < CHUNK_SIZE; ++s) {
			for (int i = 0; i < CHUNK_SIZE; ++i) {
				uint64_t faceBit = faceColBit[Utils::GetIndexFrom3D(face, s, i, CHUNK_SIZE)];
				int step = 0;
				while (step < CHUNK_SIZE) {						   // 111100011100
					step += Utils::TrailingZeros(faceBit >> step); // 1111000111|00| -> 2
					if (step >= CHUNK_SIZE)
						break;

					int ones = Utils::TrailingOnes((faceBit >> step));	// 1111000|111|00 -> 3
					uint64_t submask = ((1ULL << ones) - 1ULL) << step; // 111 << 2 -> 11100

					int w = 1;
					while (i + w < CHUNK_SIZE) {
						uint64_t cb =
							faceColBit[Utils::GetIndexFrom3D(face, s, i + w, CHUNK_SIZE)] & submask;
						if (cb != submask)
							break;

						faceColBit[Utils::GetIndexFrom3D(face, s, i + w, CHUNK_SIZE)] &= (~submask);
						w++;
					}

					if (face == 0)
						MeshGenerator::CreateQuadMesh(
							vertices, indices, s, step, i, w, ones, face, biome, textureIndex);
					else if (face == 1)
						MeshGenerator::CreateQuadMesh(
							vertices, indices, s + 1, step, i, w, ones, face, biome, textureIndex);
					else if (face == 2)
						MeshGenerator::CreateQuadMesh(
							vertices, indices, i, s, step, w, ones, face, biome, textureIndex);
					else if (face == 3)
						MeshGenerator::CreateQuadMesh(
							vertices, indices, i, s + 1, step, w, ones, face, biome, textureIndex);
					else if (face == 4)
						MeshGenerator::CreateQuadMesh(
							vertices, indices, i, step, s, w, ones, face, biome, textureIndex);
					else // face == 5
						MeshGenerator::CreateQuadMesh(
							vertices, indices, i, step, s + 1, w, ones, face, biome, textureIndex);

					step += ones;
				}
			}
		}
	}
}

BLOCK_TYPE Chunk::GetBlockTypeByPosition(Vector3 pos)
{
	int padding = 1;

	int fx = (int)std::floor(pos.x);
	int fy = (int)std::floor(pos.y);
	int fz = (int)std::floor(pos.z);

	return m_blocks[fx + padding][fy + padding][fz + padding].GetType();
}