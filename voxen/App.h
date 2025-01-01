#pragma once

#include <windows.h>

#include "ChunkManager.h"
#include "Camera.h"
#include "Skybox.h"
#include "Cloud.h"
#include "Light.h"
#include "PostEffect.h"
#include "WorldMap.h"

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace DirectX::SimpleMath;

class App {

public:
	App();
	~App();

	LRESULT EventHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool Initialize();
	void Run();

	static const UINT WIDTH = 1920;
	static const UINT HEIGHT = 1080;
	static const UINT SHADOW_WIDTH = 3072;
	static const UINT SHADOW_HEIGHT = 1024;
	static const UINT MIRROR_WIDTH = WIDTH / 2;
	static const UINT MIRROR_HEIGHT = HEIGHT / 2;

	static const UINT DAY_CYCLE_AMOUNT = 24000;
	static const UINT DAY_CYCLE_REAL_TIME = 30;
	static const UINT DAY_CYCLE_TIME_SPEED = DAY_CYCLE_AMOUNT / DAY_CYCLE_REAL_TIME;
	static const UINT DAY_START = 1000;
	static const UINT DAY_END = 11000;
	static const UINT MAX_SUNSET = 12500;
	static const UINT NIGHT_START = 13700;
	static const UINT NIGHT_END = 22300;
	static const UINT MAX_SUNRISE = 23500;

private:
	bool InitWindow();
	bool InitDirectX();
	bool InitGUI();
	bool InitScene();

	void Update(float dt);
	void Render();

	void FillGBuffer();
	void MaskMSAAEdge();
	void RenderSSAO();
	void ShadingBasic();

	void ConvertToMSAA();

	void RenderSkybox();
	void RenderCloud();

	void RenderFogFilter();
	void RenderWaterFilter();

	void RenderMirrorWorld();
	void RenderWaterPlane();

	void RenderShadowMap();

	void LockCursor();
	void UnlockCursor();

	HWND m_hwnd;
	ComPtr<ID3D11Buffer> m_constantBuffer;
	AppConstantData m_constantData;

	Camera m_camera;
	Skybox m_skybox;
	Cloud m_cloud;
	Light m_light;
	PostEffect m_postEffect;
	WorldMap m_worldMap;

	UINT m_dateTime;
	bool m_keyPressed[256];
	bool m_keyToggled[256];

	LONG m_mouseDeltaX;
	LONG m_mouseDeltaY;

	bool m_isActive;
};
