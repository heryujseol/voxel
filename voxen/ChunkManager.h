#pragma once

#include <map>
#include <queue>
#include <future>

#include "Chunk.h"
#include "Camera.h"

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
	static const int MAX_ASYNC_LOAD_COUNT = 1;
	static const int MAX_INSTANCE_BUFFER_SIZE = 1024 * 1024 * 8;
	static const int MAX_INSTANCE_BUFFER_COUNT =
		MAX_INSTANCE_BUFFER_SIZE / sizeof(InstanceInfoVertex);

	ChunkManager();
	~ChunkManager();

	bool Initialize(Vector3 cameraChunkPos);
	void Update(Camera& camera, float dt);

	void RenderOpaqueChunk(Chunk* chunk);
	void RenderSemiAlphaChunk(Chunk* chunk);
	void RenderLowLodChunk(Chunk* chunk);
	void RenderTransparencyChunk(Chunk* chunk);
	void RenderInstance();

	void RenderBasic(Vector3 cameraPos, bool useMasking);
	void RenderMirrorWorld();
	void RenderTransparency(bool useBlending);
	

private:
	void UpdateChunkList(Vector3 cameraChunkPos);
	void UpdateLoadChunkList(Camera& camera);
	void UpdateUnloadChunkList();
	void UpdateRenderChunkList(Camera& camera);
	void UpdateInstanceInfoList(Camera& camera);
	void UpdateChunkConstant(float dt);
	void UpdateIsInWater(Camera& camera);

	bool FrustumCulling(Vector3 position, Camera& camera, bool useMirror);

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
};