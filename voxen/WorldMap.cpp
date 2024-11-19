#include "WorldMap.h"
#include "Graphics.h"
#include "SimpleQuadRenderer.h"
#include "Terrain.h"

WorldMap::WorldMap()
	: m_mapData(WORLD_MAP_BUFFER_SIZE * WORLD_MAP_BUFFER_SIZE * WORLD_MAP_PIXEL_SIZE, 0),
	  m_offsetPosition(0.0f)
{
}

WorldMap::~WorldMap() {}

bool WorldMap::Initialize(Vector3 cameraPosition)
{
	m_offsetPosition = Utils::CalcOffsetPos(cameraPosition, WORLD_SIZE_PER_PIXEL);
	for (int z = 0; z < WORLD_MAP_BUFFER_SIZE; ++z) {
		for (int x = 0; x < WORLD_MAP_BUFFER_SIZE; ++x) {
			Vector3 color = GenerateWorldMapColor(x, z);

			int xz = (x + z * WORLD_MAP_BUFFER_SIZE) * WORLD_MAP_PIXEL_SIZE;
			m_mapData[xz + 0] = (UINT)color.x;
			m_mapData[xz + 1] = (UINT)color.y;
			m_mapData[xz + 2] = (UINT)color.z;
			m_mapData[xz + 3] = 255;
		}
	}

	if (!UpdateBuffer()) {
		return false;
	}

	return true;
}

void WorldMap::Update(Vector3 cameraPosition) 
{
	Vector3 newOffsetPosition = Utils::CalcOffsetPos(cameraPosition, WORLD_SIZE_PER_PIXEL);
	Vector3 offsetDiff = m_offsetPosition - newOffsetPosition;
	if (offsetDiff.Length() > 0) {
		m_offsetPosition = newOffsetPosition;

		int dx = Utils::Signf(offsetDiff.x);
		int dz = -Utils::Signf(offsetDiff.z);

		ShiftMapData(dx, dz);

		UpdateMapData(dx, dz);

		UpdateBuffer();
	}
}

void WorldMap::Render()
{
	Graphics::context->RSSetViewports(1, &Graphics::worldMapViewport);

	std::vector<ID3D11ShaderResourceView*> ppSRVs;
	ppSRVs.push_back(Graphics::worldMapSRV.Get());
	ppSRVs.push_back(Graphics::worldPointSRV.Get());
	Graphics::context->PSSetShaderResources(0, (UINT)ppSRVs.size(), ppSRVs.data());

	Graphics::SetPipelineStates(Graphics::worldMapPSO);
	SimpleQuadRenderer::GetInstance()->Render();

	Graphics::context->RSSetViewports(1, &Graphics::basicViewport);
}

bool WorldMap::UpdateBuffer()
{
	D3D11_MAPPED_SUBRESOURCE ms;
	HRESULT ret =
		Graphics::context->Map(Graphics::worldMapBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
	if (FAILED(ret)) {
		std::cout << "failed to update world map buffer" << std::endl;
		return false;
	}

	uint8_t* pData = (uint8_t*)ms.pData;
	for (UINT h = 0; h < WORLD_MAP_BUFFER_SIZE; ++h) {
		memcpy(&pData[h * ms.RowPitch],
			&m_mapData[h * WORLD_MAP_BUFFER_SIZE * WORLD_MAP_PIXEL_SIZE],
			WORLD_MAP_BUFFER_SIZE * WORLD_MAP_PIXEL_SIZE);
	}
	Graphics::context->Unmap(Graphics::worldMapBuffer.Get(), NULL);

	return true;
}

void WorldMap::ShiftMapData(int dx, int dz)
{
	int startX = (dx <= 0) ? 0 : WORLD_MAP_BUFFER_SIZE - 1;
	int endX = (dx <= 0) ? WORLD_MAP_BUFFER_SIZE : -1;
	int stepX = (dx <= 0) ? 1 : -1;

	int startZ = (dz <= 0) ? 0 : WORLD_MAP_BUFFER_SIZE - 1;
	int endZ = (dz <= 0) ? WORLD_MAP_BUFFER_SIZE : -1;
	int stepZ = (dz <= 0) ? 1 : -1;

	for (int z = startZ; z != endZ; z += stepZ) {
		for (int x = startX; x != endX; x += stepX) {
			int nx = x + dx;
			int nz = z + dz;

			if (nx < 0 || nx >= WORLD_MAP_BUFFER_SIZE || nz < 0 || nz >= WORLD_MAP_BUFFER_SIZE)
				continue;

			int xz = (x + z * WORLD_MAP_BUFFER_SIZE) * WORLD_MAP_PIXEL_SIZE;
			int nxz = (nx + nz * WORLD_MAP_BUFFER_SIZE) * WORLD_MAP_PIXEL_SIZE;

			m_mapData[nxz + 0] = m_mapData[xz + 0];
			m_mapData[nxz + 1] = m_mapData[xz + 1];
			m_mapData[nxz + 2] = m_mapData[xz + 2];
			m_mapData[nxz + 3] = m_mapData[xz + 3];
		}
	}
}

void WorldMap::UpdateMapData(int dx, int dz)
{ 
	if (dx != 0) {
		int x = (dx < 0) ? WORLD_MAP_BUFFER_SIZE - 1 : 0;
		for (int z = 0; z < WORLD_MAP_BUFFER_SIZE; ++z) {
			Vector3 color = GenerateWorldMapColor(x, z);
			
			int xz = (x + z * WORLD_MAP_BUFFER_SIZE) * WORLD_MAP_PIXEL_SIZE;
			m_mapData[xz + 0] = (UINT)color.x;
			m_mapData[xz + 1] = (UINT)color.y;
			m_mapData[xz + 2] = (UINT)color.z;
			m_mapData[xz + 3] = 255;
		}
	}

	if (dz != 0) {
		int z = (dz < 0) ? WORLD_MAP_BUFFER_SIZE - 1 : 0;
		for (int x = 0; x < WORLD_MAP_BUFFER_SIZE; ++x) {
			Vector3 color = GenerateWorldMapColor(x, z);

			int xz = (x + z * WORLD_MAP_BUFFER_SIZE) * WORLD_MAP_PIXEL_SIZE;
			m_mapData[xz + 0] = (UINT)color.x;
			m_mapData[xz + 1] = (UINT)color.y;
			m_mapData[xz + 2] = (UINT)color.z;
			m_mapData[xz + 3] = 255;
		}
	}
}

Vector3 WorldMap::GenerateWorldMapColor(int x, int z)
{
	int worldX = (int)m_offsetPosition.x - (WORLD_SIZE / 2) + WORLD_SIZE_PER_PIXEL * x;
	int worldZ = (int)m_offsetPosition.z + (WORLD_SIZE / 2) - WORLD_SIZE_PER_PIXEL * z;

	float continentalness = Terrain::GetContinentalness(worldX, worldZ);
	float erosion = Terrain::GetErosion(worldX, worldZ);
	float peaksValley = Terrain::GetPeaksValley(worldX, worldZ);
	
	float elevation = Terrain::GetElevation(continentalness, erosion, peaksValley);

	if (elevation <= Terrain::WATER_HEIGHT_LEVEL) {
		return Vector3(0, 0, elevation);
	}
	else {
		return Vector3(elevation, elevation, elevation);
	}
}