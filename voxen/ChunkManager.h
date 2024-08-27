#pragma once

#include <map>
#include <queue>
#include <future>

#include "Chunk.h"
#include "Camera.h"
#include "Light.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

class ChunkManager {

public:
	static const int CHUNK_COUNT = 2 * (Camera::MAX_RENDER_DISTANCE / Chunk::CHUNK_SIZE) + 1;
	static const int MAX_HEIGHT = 256;
	static const int MAX_HEIGHT_CHUNK_COUNT = 8;
	static const int CHUNK_COUNT_P = CHUNK_COUNT + 2;
	static const int MAX_HEIGHT_CHUNK_COUNT_P = MAX_HEIGHT_CHUNK_COUNT + 2;
	static const int CHUNK_POOL_SIZE = CHUNK_COUNT_P * CHUNK_COUNT_P * MAX_HEIGHT_CHUNK_COUNT_P;
	static const int MAX_INSTANCE_BUFFER_SIZE = 1024 * 1024 * 8;
	static const int MAX_INSTANCE_BUFFER_COUNT =
		MAX_INSTANCE_BUFFER_SIZE / sizeof(InstanceInfoVertex);

	static ChunkManager* GetInstance();

	bool Initialize(Vector3 cameraChunkPos);
	void Update(float dt, Camera& camera, Light& light);

	void RenderOpaqueChunk(Chunk* chunk);
	void RenderSemiAlphaChunk(Chunk* chunk);
	void RenderLowLodChunk(Chunk* chunk);
	void RenderTransparencyChunk(Chunk* chunk);
	void RenderInstance();

	void RenderBasic(Vector3 cameraPos);
	void RenderMirrorWorld();
	void RenderTransparency();
	void RenderShadowMap();

	Chunk* GetChunkByPosition(int x, int y, int z);
	

private:
	static ChunkManager* chunkManager;

	ChunkManager();
	~ChunkManager();
	ChunkManager(const ChunkManager& other);
	void operator=(const ChunkManager& rhs);

	void UpdateChunkList(Vector3 cameraChunkPos);
	void UpdateLoadChunkList(Camera& camera);
	void UpdateUnloadChunkList();
	void UpdateRenderChunkList(Camera& camera, Light& light);
	void UpdateInstanceInfoList(Camera& camera);
	void UpdateChunkConstant(float dt);

	bool FrustumCulling(
		Vector3 position, Camera& camera, Light& light, bool useMirror, bool useShadow);

	void InitChunkBuffer(Chunk* chunk);
	
	Chunk* GetChunkFromPool();
	void ReleaseChunkToPool(Chunk* chunk);

	bool MakeInstanceVertexBuffer();
	bool MakeInstanceInfoBuffer();

	std::vector<Chunk*> m_chunkPool;
	std::map<std::tuple<int, int, int>, Chunk*> m_chunkMap;

	std::vector<Chunk*> m_loadChunkList;
	std::vector<Chunk*> m_unloadChunkList;
	std::vector<Chunk*> m_renderChunkList;
	std::vector<Chunk*> m_renderMirrorChunkList;
	std::vector<Chunk*> m_renderShadowChunkList;

	std::vector<ComPtr<ID3D11Buffer>> m_lowLodVertexBuffers;
	std::vector<ComPtr<ID3D11Buffer>> m_lowLodIndexBuffers;

	std::vector<ComPtr<ID3D11Buffer>> m_opaqueVertexBuffers;
	std::vector<ComPtr<ID3D11Buffer>> m_opaqueIndexBuffers;

	std::vector<ComPtr<ID3D11Buffer>> m_transparencyVertexBuffers;
	std::vector<ComPtr<ID3D11Buffer>> m_transparencyIndexBuffers;

	std::vector<ComPtr<ID3D11Buffer>> m_semiAlphaVertexBuffers;
	std::vector<ComPtr<ID3D11Buffer>> m_semiAlphaIndexBuffers;

	std::vector<ComPtr<ID3D11Buffer>> m_constantBuffers;

	std::vector<ComPtr<ID3D11Buffer>> m_instanceVertexBuffers;
	std::vector<ComPtr<ID3D11Buffer>> m_instanceIndexBuffers;
	std::vector<ComPtr<ID3D11Buffer>> m_instanceInfoBuffers;
	std::vector<std::vector<InstanceInfoVertex>> m_instanceInfoList;
	std::vector<UINT> m_instanceIndexCount;
	
	unsigned int m_initThreadCount;
	std::vector<std::pair<Chunk*, std::future<ChunkInitMemory*>>> m_futures;
	std::vector<ChunkInitMemory*> m_chunkInitMemoryPool;
};