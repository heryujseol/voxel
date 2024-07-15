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

void Chunk::Initialize()
{
	// 0. initialize chunk data
	InitChunkData();

	// 1. intialize instance vertcie data
	InitInstanceInfoData();

	// 2. initialize world(opaque & water & semiAlpha) vertice data by greedy meshing
	InitWorldVerticesData();

	// 3. initialize constant data
	m_position = Vector3(m_offsetPosition.x, -2.0f * CHUNK_SIZE, m_offsetPosition.z);
	m_constantData.world = Matrix::CreateTranslation(m_position);
	m_isUpdateRequired = true;
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
			int nx = (int)m_offsetPosition.x + x - 1;
			int nz = (int)m_offsetPosition.z + z - 1;

			int height = Terrain::GetHeight(nx, nz);
			float t = Terrain::GetPerlinNoise2((float)nx / 182.0f, (float)nz / 182.0f);

			for (int y = 0; y < CHUNK_SIZE_P; ++y) {
				m_blocks[x][y][z].SetType(0);

				int ny = (int)m_offsetPosition.y + y - 1;
				if (-64 <= ny && (ny <= height || height <= 62)) {
					uint8_t type = Terrain::GetType(nx, ny, nz, height, t);

					m_blocks[x][y][z].SetType(type);
				}

				/////////////////////////////
				// for testing
				if (height + 3 <= ny && ny <= height + 6 && 14 <= x && x <= 20 && 14 <= z &&
					z <= 20) {
					m_blocks[x][y][z].SetType(10);
				}
				/////////////////////////////
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
			}
		}
	}
}

void Chunk::InitWorldVerticesData()
{
	// 1. make axis column bit data
	std::unordered_map<uint8_t, bool> llTypeMap;
	std::unordered_map<uint8_t, bool> opTypeMap;
	std::unordered_map<uint8_t, bool> tpTypeMap;
	std::unordered_map<uint8_t, bool> saTypeMap;

	static uint64_t llColBit[CHUNK_SIZE_P2 * 3];
	static uint64_t opColBit[CHUNK_SIZE_P2 * 3];
	static uint64_t tpCullColBit[CHUNK_SIZE_P2 * 6];
	static uint64_t saCullColBit[CHUNK_SIZE_P2 * 6];

	std::fill(llColBit, llColBit + CHUNK_SIZE_P2 * 3, 0);
	std::fill(opColBit, opColBit + CHUNK_SIZE_P2 * 3, 0);
	std::fill(tpCullColBit, tpCullColBit + CHUNK_SIZE_P2 * 6, 0);
	std::fill(saCullColBit, saCullColBit + CHUNK_SIZE_P2 * 6, 0);

	// 1. cull face column bit
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
					// Ÿ���� ���ų� ������ ����̸� �޽��� �������� ����
					if (x - 1 >= 0 && type != m_blocks[x - 1][y][z].GetType() &&
						!Block::IsOpaqua(m_blocks[x - 1][y][z].GetType())) {
						tpCullColBit[Utils::GetIndexFrom3D(0, y, z, CHUNK_SIZE_P)] |= (1ULL << x);
					}
					if (x + 1 < CHUNK_SIZE_P && type != m_blocks[x + 1][y][z].GetType() &&
						!Block::IsOpaqua(m_blocks[x + 1][y][z].GetType())) {
						tpCullColBit[Utils::GetIndexFrom3D(1, y, z, CHUNK_SIZE_P)] |= (1ULL << x);
					}

					if (y - 1 >= 0 && type != m_blocks[x][y - 1][z].GetType() &&
						!Block::IsOpaqua(m_blocks[x][y - 1][z].GetType())) {
						tpCullColBit[Utils::GetIndexFrom3D(2, z, x, CHUNK_SIZE_P)] |= (1ULL << y);
					}
					if (y + 1 < CHUNK_SIZE_P && type != m_blocks[x][y + 1][z].GetType() &&
						!Block::IsOpaqua(m_blocks[x][y + 1][z].GetType())) {
						tpCullColBit[Utils::GetIndexFrom3D(3, z, x, CHUNK_SIZE_P)] |= (1ULL << y);
					}

					if (z - 1 >= 0 && type != m_blocks[x][y][z - 1].GetType() &&
						!Block::IsOpaqua(m_blocks[x][y][z - 1].GetType())) {
						tpCullColBit[Utils::GetIndexFrom3D(4, y, x, CHUNK_SIZE_P)] |= (1ULL << z);
					}
					if (z + 1 < CHUNK_SIZE_P && type != m_blocks[x][y][z + 1].GetType() &&
						!Block::IsOpaqua(m_blocks[x][y][z + 1].GetType())) {
						tpCullColBit[Utils::GetIndexFrom3D(5, y, x, CHUNK_SIZE_P)] |= (1ULL << z);
					}
				}
				else if (Block::IsSemiAlpha(type)) {
					saTypeMap[type] = true;
					// - -> + : �������� �ƴϸ� ���̽� ���� -> ���� Ÿ���� ������� ����
					if (x + 1 < CHUNK_SIZE_P && !Block::IsOpaqua(m_blocks[x + 1][y][z].GetType())) {
						saCullColBit[Utils::GetIndexFrom3D(1, y, z, CHUNK_SIZE_P)] |= (1ULL << x);
					}
					if (y + 1 < CHUNK_SIZE_P && !Block::IsOpaqua(m_blocks[x][y + 1][z].GetType())) {
						saCullColBit[Utils::GetIndexFrom3D(3, z, x, CHUNK_SIZE_P)] |= (1ULL << y);
					}
					if (z + 1 < CHUNK_SIZE_P && !Block::IsOpaqua(m_blocks[x][y][z + 1].GetType())) {
						saCullColBit[Utils::GetIndexFrom3D(5, y, x, CHUNK_SIZE_P)] |= (1ULL << z);
					}

					// + -> - : ������ ���� ���̽� ���� -> ���� Ÿ���� ������� ����
					if (x - 1 >= 0 && Block::IsTransparency(m_blocks[x - 1][y][z].GetType())) {
						saCullColBit[Utils::GetIndexFrom3D(0, y, z, CHUNK_SIZE_P)] |= (1ULL << x);
					}
					if (y - 1 >= 0 && Block::IsTransparency(m_blocks[x][y - 1][z].GetType())) {
						saCullColBit[Utils::GetIndexFrom3D(2, z, x, CHUNK_SIZE_P)] |= (1ULL << y);
					}
					if (z - 1 >= 0 && Block::IsTransparency(m_blocks[x][y][z - 1].GetType())) {
						saCullColBit[Utils::GetIndexFrom3D(4, y, x, CHUNK_SIZE_P)] |= (1ULL << z);
					}

					llTypeMap[type] = true;
					llColBit[Utils::GetIndexFrom3D(0, y, z, CHUNK_SIZE_P)] |= (1ULL << x);
					llColBit[Utils::GetIndexFrom3D(1, z, x, CHUNK_SIZE_P)] |= (1ULL << y);
					llColBit[Utils::GetIndexFrom3D(2, y, x, CHUNK_SIZE_P)] |= (1ULL << z);
				}
				else {
					opTypeMap[type] = true;
					opColBit[Utils::GetIndexFrom3D(0, y, z, CHUNK_SIZE_P)] |= (1ULL << x);
					opColBit[Utils::GetIndexFrom3D(1, z, x, CHUNK_SIZE_P)] |= (1ULL << y);
					opColBit[Utils::GetIndexFrom3D(2, y, x, CHUNK_SIZE_P)] |= (1ULL << z);

					llTypeMap[type] = true;
					llColBit[Utils::GetIndexFrom3D(0, y, z, CHUNK_SIZE_P)] |= (1ULL << x);
					llColBit[Utils::GetIndexFrom3D(1, z, x, CHUNK_SIZE_P)] |= (1ULL << y);
					llColBit[Utils::GetIndexFrom3D(2, y, x, CHUNK_SIZE_P)] |= (1ULL << z);
				}
			}
		}
	}

	// lowlod & opaque face culling
	static uint64_t llCullColBit[CHUNK_SIZE_P2 * 6];
	static uint64_t opCullColBit[CHUNK_SIZE_P2 * 6];

	std::fill(llCullColBit, llCullColBit + CHUNK_SIZE_P2 * 6, 0);
	std::fill(opCullColBit, opCullColBit + CHUNK_SIZE_P2 * 6, 0);

	for (int axis = 0; axis < 3; ++axis) {
		for (int h = 1; h < CHUNK_SIZE_P - 1; ++h) {
			for (int w = 1; w < CHUNK_SIZE_P - 1; ++w) {
				uint64_t llBit = llColBit[Utils::GetIndexFrom3D(axis, h, w, CHUNK_SIZE_P)];
				llCullColBit[Utils::GetIndexFrom3D(axis * 2 + 0, h, w, CHUNK_SIZE_P)] =
					llBit & ~(llBit << 1);
				llCullColBit[Utils::GetIndexFrom3D(axis * 2 + 1, h, w, CHUNK_SIZE_P)] =
					llBit & ~(llBit >> 1);

				uint64_t opBit = opColBit[Utils::GetIndexFrom3D(axis, h, w, CHUNK_SIZE_P)];
				opCullColBit[Utils::GetIndexFrom3D(axis * 2 + 0, h, w, CHUNK_SIZE_P)] =
					opBit & ~(opBit << 1);
				opCullColBit[Utils::GetIndexFrom3D(axis * 2 + 1, h, w, CHUNK_SIZE_P)] =
					opBit & ~(opBit >> 1);
			}
		}
	}


	// 2. build face culled bit slices column
	static uint64_t llSliceColBit[Block::BLOCK_TYPE_COUNT][CHUNK_SIZE2 * 6];
	static uint64_t opSliceColBit[Block::BLOCK_TYPE_COUNT][CHUNK_SIZE2 * 6];
	static uint64_t tpSliceColBit[Block::BLOCK_TYPE_COUNT][CHUNK_SIZE2 * 6];
	static uint64_t saSliceColBit[Block::BLOCK_TYPE_COUNT][CHUNK_SIZE2 * 6];

	for (const auto& p : llTypeMap)
		std::fill(llSliceColBit[p.first], llSliceColBit[p.first] + CHUNK_SIZE2 * 6, 0);
	for (const auto& p : opTypeMap)
		std::fill(opSliceColBit[p.first], opSliceColBit[p.first] + CHUNK_SIZE2 * 6, 0);
	for (const auto& p : tpTypeMap)
		std::fill(tpSliceColBit[p.first], tpSliceColBit[p.first] + CHUNK_SIZE2 * 6, 0);
	for (const auto& p : saTypeMap)
		std::fill(saSliceColBit[p.first], saSliceColBit[p.first] + CHUNK_SIZE2 * 6, 0);

	MakeFaceSliceColumnBit(llCullColBit, llSliceColBit);
	MakeFaceSliceColumnBit(opCullColBit, opSliceColBit);
	MakeFaceSliceColumnBit(tpCullColBit, tpSliceColBit);
	MakeFaceSliceColumnBit(saCullColBit, saSliceColBit);


	// 3. make vertices by bit slices column
	for (const auto& p : llTypeMap)
		GreedyMeshing(llSliceColBit[p.first], m_lowLodVertices, m_lowLodIndices, p.first);
	for (const auto& p : opTypeMap)
		GreedyMeshing(opSliceColBit[p.first], m_opaqueVertices, m_opaqueIndices, p.first);
	for (const auto& p : tpTypeMap)
		GreedyMeshing(
			tpSliceColBit[p.first], m_transparencyVertices, m_transparencyIndices, p.first);
	for (const auto& p : saTypeMap)
		GreedyMeshing(saSliceColBit[p.first], m_semiAlphaVertices, m_semiAlphaIndices, p.first);
}

void Chunk::MakeFaceSliceColumnBit(uint64_t cullColBit[CHUNK_SIZE_P2 * 6],
	uint64_t sliceColBit[Block::BLOCK_TYPE_COUNT][CHUNK_SIZE2 * 6])
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

void Chunk::GreedyMeshing(uint64_t faceColBit[CHUNK_SIZE2 * 6], std::vector<VoxelVertex>& vertices,
	std::vector<uint32_t>& indices, uint8_t type)
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