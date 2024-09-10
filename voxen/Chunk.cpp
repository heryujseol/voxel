#include "Chunk.h"
#include "DXUtils.h"
#include "MeshGenerator.h"
#include "Terrain.h"

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
	// static long long sum = 0;
	// static long long count = 0;
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
	// sum += duration.count();
	// count++;
	std::cout << "Function duration: "
			  << "duration: " << duration.count() << " microseconds" << std::endl;
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

			for (int y = 0; y < CHUNK_SIZE_P; ++y) {

				int worldY = (int)m_offsetPosition.y + y - 1;

				if (worldY == 256) {
					m_blocks[x][y][z].SetType(0);
					continue;
				}
				if (worldY == 0) {
					m_blocks[x][y][z].SetType(4);
					continue;
				}

				float d1 = Terrain::GetDensity(worldX, worldY, worldZ, 3.0f, 256.0f);
				float d2 = Terrain::GetDensity2(worldX, worldY, worldZ, 123.0f, 512.0f);

				uint8_t transparencyType = worldY <= 63 ? 1 : 0;

				m_blocks[x][y][z].SetType(transparencyType);
				if (worldY <= baseLevel) {
					m_blocks[x][y][z].SetType(3);

					if (d1 * d1 + d2 * d2 <= 0.004f) {
						//m_blocks[x][y][z].SetType(transparencyType);
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
				/*
				// instance testing
				int choose[4] = { 128, 129, 130, 144 };
				static int loop = 0;
				if ((m_blocks[x + 1][y + 1][z + 1].GetType() == BLOCK_TYPE::AIR ||
						m_blocks[x + 1][y + 1][z + 1].GetType() == BLOCK_TYPE::WATER) &&
					Block::IsOpaqua(m_blocks[x + 1][y][z + 1].GetType()) && x % 3 == 0 &&
					z % 3 == 0) {
					Instance instance;

					uint8_t type = choose[loop++ % 4];
					instance.SetType(type);

					Vector3 pos = Vector3((float)x, (float)y, (float)z) + Vector3(0.5f);
					instance.SetWorld(Matrix::CreateTranslation(pos));

					m_instanceMap[std::make_tuple(x, y, z)] = instance;
				}
				*/
			}
		}
	}
}

void Chunk::InitWorldVerticesData(ChunkInitMemory* memory)
{
	memory->Clear();

	// 1. make axis column bit data
	std::unordered_map<uint8_t, bool> llTypeMap;
	std::unordered_map<uint8_t, bool> opTypeMap;
	std::unordered_map<uint8_t, bool> tpTypeMap;
	std::unordered_map<uint8_t, bool> saTypeMap;

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
				uint8_t type = m_blocks[x][y][z].GetType();
				if (type == BLOCK_TYPE::AIR)
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

	MakeFaceSliceColumnBit(memory->llCullColBit, memory->llSliceColBit);
	MakeFaceSliceColumnBit(memory->opCullColBit, memory->opSliceColBit);
	MakeFaceSliceColumnBit(memory->tpCullColBit, memory->tpSliceColBit);
	MakeFaceSliceColumnBit(memory->saCullColBit, memory->saSliceColBit);


	// 4. make vertices by bit slices column
	for (const auto& p : llTypeMap)
		GreedyMeshing(memory->llSliceColBit[p.first], m_lowLodVertices, m_lowLodIndices, p.first);
	for (const auto& p : opTypeMap)
		GreedyMeshing(memory->opSliceColBit[p.first], m_opaqueVertices, m_opaqueIndices, p.first);
	for (const auto& p : tpTypeMap)
		GreedyMeshing(
			memory->tpSliceColBit[p.first], m_transparencyVertices, m_transparencyIndices, p.first);
	for (const auto& p : saTypeMap)
		GreedyMeshing(
			memory->saSliceColBit[p.first], m_semiAlphaVertices, m_semiAlphaIndices, p.first);
}

void Chunk::MakeFaceSliceColumnBit(uint64_t cullColBit[Chunk::CHUNK_SIZE_P2 * 6],
	uint64_t sliceColBit[Block::BLOCK_TYPE_COUNT][Chunk::CHUNK_SIZE2 * 6])
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

					uint8_t type = 0;
					if (face < 2) {
						type = m_blocks[bitPos + 1][h + 1][w + 1].GetType();
					}
					else if (face < 4) {
						type = m_blocks[w + 1][bitPos + 1][h + 1].GetType();
					}
					else { // face < 6
						type = m_blocks[w + 1][h + 1][bitPos + 1].GetType();
					}

					sliceColBit[type][Utils::GetIndexFrom3D(face, bitPos, w, CHUNK_SIZE)] |=
						(1ULL << h);
				}
			}
		}
	}
}

void Chunk::GreedyMeshing(uint64_t faceColBit[Chunk::CHUNK_SIZE2 * 6],
	std::vector<VoxelVertex>& vertices, std::vector<uint32_t>& indices, uint8_t type)
{
	// face 0, 1 : left-right
	// face 2, 3 : top-bottom
	// face 4, 5 : front-back
	for (int face = 0; face < 6; ++face) {
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
							vertices, indices, s, step, i, w, ones, face, type);
					else if (face == 1)
						MeshGenerator::CreateQuadMesh(
							vertices, indices, s + 1, step, i, w, ones, face, type);
					else if (face == 2)
						MeshGenerator::CreateQuadMesh(
							vertices, indices, i, s, step, w, ones, face, type);
					else if (face == 3)
						MeshGenerator::CreateQuadMesh(
							vertices, indices, i, s + 1, step, w, ones, face, type);
					else if (face == 4)
						MeshGenerator::CreateQuadMesh(
							vertices, indices, i, step, s, w, ones, face, type);
					else // face == 5
						MeshGenerator::CreateQuadMesh(
							vertices, indices, i, step, s + 1, w, ones, face, type);

					step += ones;
				}
			}
		}
	}
}

uint8_t Chunk::GetBlockTypeByPosition(Vector3 pos)
{
	int padding = 1;

	int fx = (int)std::floor(pos.x);
	int fy = (int)std::floor(pos.y);
	int fz = (int)std::floor(pos.z);

	return m_blocks[fx + padding][fy + padding][fz + padding].GetType();
}