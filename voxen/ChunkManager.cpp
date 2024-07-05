#include "ChunkManager.h"
#include "Graphics.h"
#include "Utils.h"
#include "DXUtils.h"
#include "MeshGenerator.h"
#include "Block.h"
#include "Instance.h"

#include <iostream>

ChunkManager::ChunkManager() {}

ChunkManager::~ChunkManager()
{
	for (int i = 0; i < CHUNK_POOL_SIZE; ++i) {
		ClearChunkBuffer(m_chunkPool[i]);
	}
}

bool ChunkManager::Initialize(Vector3 cameraChunkPos)
{
	for (int i = 0; i < CHUNK_POOL_SIZE; ++i) {
		m_chunkPool.push_back(new Chunk(i));
	}

	m_lowLodVertexBuffers.resize(CHUNK_POOL_SIZE, nullptr);
	m_lowLodIndexBuffers.resize(CHUNK_POOL_SIZE, nullptr);

	m_opaqueVertexBuffers.resize(CHUNK_POOL_SIZE, nullptr);
	m_opaqueIndexBuffers.resize(CHUNK_POOL_SIZE, nullptr);

	m_transparencyVertexBuffers.resize(CHUNK_POOL_SIZE, nullptr);
	m_transparencyIndexBuffers.resize(CHUNK_POOL_SIZE, nullptr);

	m_semiAlphaVertexBuffers.resize(CHUNK_POOL_SIZE, nullptr);
	m_semiAlphaIndexBuffers.resize(CHUNK_POOL_SIZE, nullptr);

	m_constantBuffers.resize(CHUNK_POOL_SIZE, nullptr);

	m_instanceVertexBuffers.resize(Instance::INSTANCE_TYPE_COUNT, nullptr);
	m_instanceIndexBuffers.resize(Instance::INSTANCE_TYPE_COUNT, nullptr);
	m_instanceInfoBuffers.resize(Instance::INSTANCE_TYPE_COUNT, nullptr);
	m_instanceInfoList.resize(Instance::INSTANCE_TYPE_COUNT);
	if (!MakeInstanceVertexBuffer())
		return false;
	if (!MakeInstanceInfoBuffer())
		return false;

	UpdateChunkList(cameraChunkPos);

	return true;
}

void ChunkManager::Update(Camera& camera)
{
	if (camera.m_isOnChunkDirtyFlag) {
		UpdateChunkList(camera.GetChunkPosition());
		camera.m_isOnChunkDirtyFlag = false;
	}

	UpdateLoadChunkList();
	UpdateUnloadChunkList();
	UpdateRenderChunkList(camera);
	UpdateInstanceInfoList(camera);
}

void ChunkManager::RenderOpaqueChunk(Chunk* chunk)
{
	if (chunk->IsEmptyOpaque())
		return;

	UINT id = chunk->GetID();
	UINT stride = sizeof(VoxelVertex);
	UINT offset = 0;

	Graphics::context->IASetIndexBuffer(m_opaqueIndexBuffers[id].Get(), DXGI_FORMAT_R32_UINT, 0);
	Graphics::context->IASetVertexBuffers(
		0, 1, m_opaqueVertexBuffers[id].GetAddressOf(), &stride, &offset);
	Graphics::context->VSSetConstantBuffers(1, 1, m_constantBuffers[id].GetAddressOf());

	Graphics::context->DrawIndexed((UINT)chunk->GetOpaqueIndices().size(), 0, 0);
}

void ChunkManager::RenderSemiAlphaChunk(Chunk* chunk)
{
	if (chunk->IsEmptySemiAlpha())
		return;

	UINT id = chunk->GetID();
	UINT stride = sizeof(VoxelVertex);
	UINT offset = 0;

	Graphics::context->IASetIndexBuffer(m_semiAlphaIndexBuffers[id].Get(), DXGI_FORMAT_R32_UINT, 0);
	Graphics::context->IASetVertexBuffers(
		0, 1, m_semiAlphaVertexBuffers[id].GetAddressOf(), &stride, &offset);
	Graphics::context->VSSetConstantBuffers(1, 1, m_constantBuffers[id].GetAddressOf());

	Graphics::context->DrawIndexed((UINT)chunk->GetSemiAlphaIndices().size(), 0, 0);
}

void ChunkManager::RenderLowLodChunk(Chunk* chunk)
{
	if (chunk->IsEmptyLowLod())
		return;

	UINT id = chunk->GetID();
	UINT stride = sizeof(VoxelVertex);
	UINT offset = 0;

	Graphics::context->IASetIndexBuffer(m_lowLodIndexBuffers[id].Get(), DXGI_FORMAT_R32_UINT, 0);
	Graphics::context->IASetVertexBuffers(
		0, 1, m_lowLodVertexBuffers[id].GetAddressOf(), &stride, &offset);
	Graphics::context->VSSetConstantBuffers(1, 1, m_constantBuffers[id].GetAddressOf());

	Graphics::context->DrawIndexed((UINT)chunk->GetLowLodIndices().size(), 0, 0);
}

void ChunkManager::RenderTransparencyChunk(Chunk* chunk)
{
	if (chunk->IsEmptyTransparency())
		return;

	UINT id = chunk->GetID();
	UINT stride = sizeof(VoxelVertex);
	UINT offset = 0;

	Graphics::context->IASetIndexBuffer(
		m_transparencyIndexBuffers[id].Get(), DXGI_FORMAT_R32_UINT, 0);
	Graphics::context->IASetVertexBuffers(
		0, 1, m_transparencyVertexBuffers[id].GetAddressOf(), &stride, &offset);
	Graphics::context->VSSetConstantBuffers(1, 1, m_constantBuffers[id].GetAddressOf());

	Graphics::context->DrawIndexed((UINT)chunk->GetTransparencyIndices().size(), 0, 0);
}

void ChunkManager::RenderInstance()
{
	UINT indexCountPerInstance[3] = { 12, 24, 6 };

	for (int i = 0; i < Instance::INSTANCE_TYPE_COUNT; ++i) {
		Graphics::context->IASetIndexBuffer(
			m_instanceIndexBuffers[i].Get(), DXGI_FORMAT_R32_UINT, 0);

		std::vector<UINT> strides = { sizeof(InstanceVertex), sizeof(InstanceInfoVertex) };
		std::vector<UINT> offsets = { 0, 0 };
		std::vector<ID3D11Buffer*> buffers = { m_instanceVertexBuffers[i].Get(),
			m_instanceInfoBuffers[i].Get() };
		Graphics::context->IASetVertexBuffers(0, 2, buffers.data(), strides.data(), offsets.data());
		Graphics::context->DrawIndexedInstanced(
			indexCountPerInstance[i], (UINT)m_instanceInfoList[i].size(), 0, 0, 0);
	}
}

void ChunkManager::RenderBasic(Vector3 cameraPos, bool useMasking)
{
	std::vector<ID3D11ShaderResourceView*> pptr = { Graphics::atlasMapSRV.Get(),
		Graphics::grassColorMapSRV.Get(), Graphics::shadowSRV.Get() };
	Graphics::context->PSSetShaderResources(0, 3, pptr.data());

	for (auto& c : m_renderChunkList) {
		Vector3 chunkOffset = c->GetPosition();
		Vector3 chunkCenterPosition = chunkOffset + Vector3(Chunk::CHUNK_SIZE * 0.5);
		Vector3 diffPosition = chunkCenterPosition - cameraPos;

		Graphics::SetPipelineStates(useMasking ? Graphics::basicMaskingPSO : Graphics::basicPSO);
		if (diffPosition.Length() > (float)Camera::LOD_RENDER_DISTANCE) {
			RenderLowLodChunk(c);
		}
		else {
			RenderOpaqueChunk(c);

			Graphics::SetPipelineStates(
				useMasking ? Graphics::semiAlphaMaskingPSO : Graphics::semiAlphaPSO);
			RenderSemiAlphaChunk(c);
		}
	}

	Graphics::SetPipelineStates(useMasking ? Graphics::instanceMaskingPSO : Graphics::instancePSO);
	RenderInstance();
}

void ChunkManager::RenderMirrorWorld()
{
	std::vector<ID3D11ShaderResourceView*> pptr = { Graphics::atlasMapSRV.Get(),
		Graphics::grassColorMapSRV.Get() };
	Graphics::context->PSSetShaderResources(0, 2, pptr.data());

	Graphics::SetPipelineStates(Graphics::basicMirrorPSO);
	for (auto& c : m_renderMirrorChunkList) {
		RenderLowLodChunk(c);
	}

	Graphics::SetPipelineStates(Graphics::instanceMirrorPSO);
	RenderInstance();
}

void ChunkManager::RenderTransparency(bool useBlending)
{
	if (useBlending) {
		std::vector<ID3D11ShaderResourceView*> pptr = { Graphics::atlasMapSRV.Get(),
			Graphics::mirrorWorldSRV.Get(), Graphics::depthOnlySRV.Get(),
			Graphics::basicResolvedSRV.Get() };
		Graphics::context->PSSetShaderResources(0, 4, pptr.data());
	}
	else {
		Graphics::context->PSSetShaderResources(0, 1, Graphics::envMapSRV.GetAddressOf());
	}

	for (auto& c : m_renderChunkList) {
		RenderTransparencyChunk(c);
	}
}

void ChunkManager::UpdateChunkList(Vector3 cameraChunkPos)
{
	std::map<std::tuple<int, int, int>, bool> loadedChunkMap;
	for (int i = 0; i < MAX_HEIGHT_CHUNK_COUNT; ++i) {
		for (int j = 0; j < CHUNK_COUNT; ++j) {
			for (int k = 0; k < CHUNK_COUNT; ++k) {
				int y = Chunk::CHUNK_SIZE * (i - 2);
				int x = (int)cameraChunkPos.x + Chunk::CHUNK_SIZE * (j - CHUNK_COUNT / 2);
				int z = (int)cameraChunkPos.z + Chunk::CHUNK_SIZE * (k - CHUNK_COUNT / 2);

				if (m_chunkMap.find(std::make_tuple(x, y, z)) ==
					m_chunkMap.end()) { // found chunk to be loaded
					Chunk* chunk = GetChunkFromPool();
					if (chunk) {
						chunk->SetPosition(Vector3((float)x, (float)y, (float)z));

						m_chunkMap[std::make_tuple(x, y, z)] = chunk;
						m_loadChunkList.push_back(chunk);
					}
				}
				else
					loadedChunkMap[std::make_tuple(x, y, z)] = true;
			}
		}
	}

	for (auto& p : m_chunkMap) { // { 1, 2, 3 } -> { 1, 2 } : 3 unload
		if (loadedChunkMap.find(p.first) == loadedChunkMap.end() &&
			m_chunkMap[p.first]->IsLoaded()) {

			m_unloadChunkList.push_back(p.second);
		}
	}
}

void ChunkManager::UpdateLoadChunkList()
{
	int loadCount = 0;

	while (!m_loadChunkList.empty() && loadCount < MAX_ASYNC_LOAD_COUNT) {
		Chunk* chunk = m_loadChunkList.back();
		m_loadChunkList.pop_back();

		////////////////////////////////////
		// check start time
		static long long sum = 0;
		static long long count = 0;
		auto start_time = std::chrono::steady_clock::now();
		////////////////////////////////////


		// load chunk
		chunk->Initialize();
		InitChunkBuffer(chunk);

		// set load value
		loadCount++;
		chunk->SetLoad(true);


		////////////////////////////////////
		// check end time
		auto end_time = std::chrono::steady_clock::now();
		auto duration =
			std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
		sum += duration.count();
		count++;
		std::cout << "Function Average duration: " << (double)sum / (double)count << " microseconds"
				  << std::endl;
		////////////////////////////////////
	}
}

void ChunkManager::UpdateUnloadChunkList()
{
	while (!m_unloadChunkList.empty()) {
		Chunk* chunk = m_unloadChunkList.back();
		m_unloadChunkList.pop_back();

		Vector3 pos = chunk->GetPosition();
		int x = (int)pos.x;
		int y = (int)pos.y;
		int z = (int)pos.z;
		m_chunkMap.erase(std::make_tuple(x, y, z));

		ReleaseChunkToPool(chunk);

		chunk->Clear();
		chunk->SetLoad(false);
	}
}

void ChunkManager::UpdateRenderChunkList(Camera& camera)
{
	m_renderChunkList.clear();
	m_renderMirrorChunkList.clear();

	for (auto& p : m_chunkMap) {
		if (!p.second->IsLoaded())
			continue;

		if (p.second->IsEmpty())
			continue;

		Vector3 chunkOffsetPos = p.second->GetPosition();
		if (FrustumCulling(chunkOffsetPos, camera, false)) {
			m_renderChunkList.push_back(p.second);
		}

		Vector3 mirrorChunkOffsetPos =
			Vector3::Transform(chunkOffsetPos, camera.GetMirrorPlaneMatrix());
		if (FrustumCulling(mirrorChunkOffsetPos, camera, true)) {
			m_renderMirrorChunkList.push_back(p.second);
		}
	}
}

void ChunkManager::UpdateInstanceInfoList(Camera& camera)
{
	// clear all info
	for (int i = 0; i < Instance::INSTANCE_TYPE_COUNT; ++i)
		m_instanceInfoList[i].clear();

	// check instance in chunk managerList
	for (auto& c : m_renderChunkList) {
		// check distance
		Vector3 chunkOffset = c->GetPosition();
		Vector3 chunkCenterPosition = chunkOffset + Vector3(Chunk::CHUNK_SIZE * 0.5);
		Vector3 diffPosition = chunkCenterPosition - camera.GetPosition();
		if (diffPosition.Length() > (float)Camera::LOD_RENDER_DISTANCE)
			continue;

		// set info
		const std::map<std::tuple<int, int, int>, Instance>& instanceMap = c->GetInstanceMap();
		for (auto& p : instanceMap) {
			InstanceInfoVertex info;
			info.type = p.second.GetType();

			info.instanceWorld = p.second.GetWorld().Transpose();

			m_instanceInfoList[Instance::GetInstanceType(info.type)].push_back(info);
		}
	}

	for (int i = 0; i < Instance::INSTANCE_TYPE_COUNT; ++i) {
		DXUtils::ResizeBuffer(m_instanceInfoBuffers[i], m_instanceInfoList[i],
			(UINT)D3D11_BIND_VERTEX_BUFFER, m_instanceInfoList[i].size() + 1024);
		DXUtils::UpdateBuffer(m_instanceInfoBuffers[i], m_instanceInfoList[i]);
	}
}

bool ChunkManager::FrustumCulling(Vector3 position, Camera& camera, bool useMirror)
{
	Matrix invMat = (camera.GetViewMatrix() * camera.GetProjectionMatrix()).Invert();

	std::vector<Vector3> worldPos = { Vector3::Transform(Vector3(-1.0f, 1.0f, 0.0f), invMat),
		Vector3::Transform(Vector3(1.0f, 1.0f, 0.0f), invMat),
		Vector3::Transform(Vector3(1.0f, -1.0f, 0.0f), invMat),
		Vector3::Transform(Vector3(-1.0f, -1.0f, 0.0f), invMat),
		Vector3::Transform(Vector3(-1.0f, 1.0f, 1.0f), invMat),
		Vector3::Transform(Vector3(1.0f, 1.0f, 1.0f), invMat),
		Vector3::Transform(Vector3(1.0f, -1.0f, 1.0f), invMat),
		Vector3::Transform(Vector3(-1.0f, -1.0f, 1.0f), invMat) };

	std::vector<Vector4> planes = { DirectX::XMPlaneFromPoints(
										worldPos[0], worldPos[1], worldPos[2]),
		DirectX::XMPlaneFromPoints(worldPos[7], worldPos[6], worldPos[5]),
		DirectX::XMPlaneFromPoints(worldPos[4], worldPos[5], worldPos[1]),
		DirectX::XMPlaneFromPoints(worldPos[3], worldPos[2], worldPos[6]),
		DirectX::XMPlaneFromPoints(worldPos[4], worldPos[0], worldPos[3]),
		DirectX::XMPlaneFromPoints(worldPos[1], worldPos[5], worldPos[6]) };

	float x = (float)Chunk::CHUNK_SIZE;
	float y = (float)Chunk::CHUNK_SIZE;
	float z = (float)Chunk::CHUNK_SIZE;
	if (useMirror)
		y *= -1;
	for (int i = 0; i < 6; ++i) {
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position)) < 0.0f)
			continue;
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position + Vector3(x, 0.0f, 0.0f))) <= 0.0f)
			continue;
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position + Vector3(0.0f, y, 0.0f))) <= 0.0f)
			continue;
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position + Vector3(x, y, 0.0f))) <= 0.0f)
			continue;
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position + Vector3(0.0f, 0.0f, z))) <= 0.0f)
			continue;
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position + Vector3(x, 0.0f, z))) <= 0.0f)
			continue;
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position + Vector3(0.0f, y, z))) <= 0.0f)
			continue;
		if (XMVectorGetX(XMPlaneDotCoord(planes[i], position + Vector3(x, y, z))) <= 0.0f)
			continue;
		return false;
	}

	return true;
}

void ChunkManager::InitChunkBuffer(Chunk* chunk)
{
	if (chunk->IsEmpty())
		return;

	UINT id = chunk->GetID();

	// constant data
	ChunkConstantData tempConstantData = chunk->GetConstantData();
	tempConstantData.world = tempConstantData.world.Transpose();
	if (!m_constantBuffers[id])
		DXUtils::CreateConstantBuffer(m_constantBuffers[id], tempConstantData);
	else
		DXUtils::UpdateConstantBuffer(m_constantBuffers[id], tempConstantData);

	// lowLod
	if (!chunk->IsEmptyLowLod()) {
		DXUtils::ResizeBuffer(
			m_lowLodVertexBuffers[id], chunk->GetLowLodVertices(), (UINT)D3D11_BIND_VERTEX_BUFFER);
		DXUtils::ResizeBuffer(
			m_lowLodIndexBuffers[id], chunk->GetLowLodIndices(), (UINT)D3D11_BIND_INDEX_BUFFER);

		DXUtils::UpdateBuffer(m_lowLodVertexBuffers[id], chunk->GetLowLodVertices());
		DXUtils::UpdateBuffer(m_lowLodIndexBuffers[id], chunk->GetLowLodIndices());
	}

	// opaque
	if (!chunk->IsEmptyOpaque()) {
		DXUtils::ResizeBuffer(
			m_opaqueVertexBuffers[id], chunk->GetOpaqueVertices(), (UINT)D3D11_BIND_VERTEX_BUFFER);
		DXUtils::ResizeBuffer(
			m_opaqueIndexBuffers[id], chunk->GetOpaqueIndices(), (UINT)D3D11_BIND_INDEX_BUFFER);

		DXUtils::UpdateBuffer(m_opaqueVertexBuffers[id], chunk->GetOpaqueVertices());
		DXUtils::UpdateBuffer(m_opaqueIndexBuffers[id], chunk->GetOpaqueIndices());
	}

	// transparency
	if (!chunk->IsEmptyTransparency()) {
		DXUtils::ResizeBuffer(m_transparencyVertexBuffers[id], chunk->GetTransparencyVertices(),
			(UINT)D3D11_BIND_VERTEX_BUFFER);
		DXUtils::ResizeBuffer(m_transparencyIndexBuffers[id], chunk->GetTransparencyIndices(),
			(UINT)D3D11_BIND_INDEX_BUFFER);

		DXUtils::UpdateBuffer(m_transparencyVertexBuffers[id], chunk->GetTransparencyVertices());
		DXUtils::UpdateBuffer(m_transparencyIndexBuffers[id], chunk->GetTransparencyIndices());
	}

	// semiAlpha
	if (!chunk->IsEmptySemiAlpha()) {
		DXUtils::ResizeBuffer(m_semiAlphaVertexBuffers[id], chunk->GetSemiAlphaVertices(),
			(UINT)D3D11_BIND_VERTEX_BUFFER);
		DXUtils::ResizeBuffer(m_semiAlphaIndexBuffers[id], chunk->GetSemiAlphaIndices(),
			(UINT)D3D11_BIND_INDEX_BUFFER);

		DXUtils::UpdateBuffer(m_semiAlphaVertexBuffers[id], chunk->GetSemiAlphaVertices());
		DXUtils::UpdateBuffer(m_semiAlphaIndexBuffers[id], chunk->GetSemiAlphaIndices());
	}
}

void ChunkManager::ClearChunkBuffer(Chunk* chunk)
{
	UINT id = chunk->GetID();

	m_lowLodVertexBuffers[id].Reset();
	m_lowLodIndexBuffers[id].Reset();
	m_opaqueVertexBuffers[id].Reset();
	m_opaqueIndexBuffers[id].Reset();
	m_transparencyVertexBuffers[id].Reset();
	m_transparencyIndexBuffers[id].Reset();
	m_semiAlphaVertexBuffers[id].Reset();
	m_semiAlphaIndexBuffers[id].Reset();
	m_constantBuffers[id].Reset();

	m_lowLodVertexBuffers[id] = nullptr;
	m_lowLodIndexBuffers[id] = nullptr;
	m_opaqueVertexBuffers[id] = nullptr;
	m_opaqueIndexBuffers[id] = nullptr;
	m_transparencyVertexBuffers[id] = nullptr;
	m_transparencyIndexBuffers[id] = nullptr;
	m_semiAlphaVertexBuffers[id] = nullptr;
	m_semiAlphaIndexBuffers[id] = nullptr;
	m_constantBuffers[id] = nullptr;
}

Chunk* ChunkManager::GetChunkFromPool()
{
	if (!m_chunkPool.empty()) {
		Chunk* chunk = m_chunkPool.back();
		m_chunkPool.pop_back();
		return chunk;
	}
	return nullptr;
}

void ChunkManager::ReleaseChunkToPool(Chunk* chunk) { m_chunkPool.push_back(chunk); }

bool ChunkManager::MakeInstanceVertexBuffer()
{
	std::vector<InstanceVertex> instanceVertices;
	std::vector<uint32_t> instanceIndices;

	// Instance Type 0 : CROSS
	MeshGenerator::CreateCrossInstanceMesh(instanceVertices, instanceIndices);
	if (!DXUtils::CreateVertexBuffer(
			m_instanceVertexBuffers[INSTANCE_TYPE::CROSS], instanceVertices)) {
		std::cout << "failed create cross instance vertex buffer in chunk manager" << std::endl;
		return false;
	}
	if (!DXUtils::CreateIndexBuffer(
			m_instanceIndexBuffers[INSTANCE_TYPE::CROSS], instanceIndices)) {
		std::cout << "failed create cross instance index buffer in chunk manager" << std::endl;
		return false;
	}
	instanceVertices.clear();
	instanceIndices.clear();


	// Instance Type 1 : FENCE
	MeshGenerator::CreateFenceInstanceMesh(instanceVertices, instanceIndices);
	if (!DXUtils::CreateVertexBuffer(
			m_instanceVertexBuffers[INSTANCE_TYPE::FENCE], instanceVertices)) {
		std::cout << "failed create fence instance vertex buffer in chunk manager" << std::endl;
		return false;
	}
	if (!DXUtils::CreateIndexBuffer(
			m_instanceIndexBuffers[INSTANCE_TYPE::FENCE], instanceIndices)) {
		std::cout << "failed create fence instance index buffer in chunk manager" << std::endl;
		return false;
	}
	instanceVertices.clear();
	instanceIndices.clear();


	// Instance Type 2 : SQUARE
	MeshGenerator::CreateSquareInstanceMesh(instanceVertices, instanceIndices);
	if (!DXUtils::CreateVertexBuffer(
			m_instanceVertexBuffers[INSTANCE_TYPE::SQUARE], instanceVertices)) {
		std::cout << "failed create SQUARE instance vertex buffer in chunk manager" << std::endl;
		return false;
	}
	if (!DXUtils::CreateIndexBuffer(
			m_instanceIndexBuffers[INSTANCE_TYPE::SQUARE], instanceIndices)) {
		std::cout << "failed create SQUARE instance index buffer in chunk manager" << std::endl;
		return false;
	}

	return true;
}

bool ChunkManager::MakeInstanceInfoBuffer()
{
	for (auto& instanceBuffer : m_instanceInfoBuffers) {
		if (!DXUtils::CreateDynamicBuffer(instanceBuffer, MAX_INSTANCE_BUFFER_COUNT,
				sizeof(InstanceInfoVertex),
				(UINT)D3D11_BIND_VERTEX_BUFFER)) { // ¾à 8MB
			std::cout << "failed create instance info buffer in chunk manager" << std::endl;
			return false;
		}
	}

	return true;
}