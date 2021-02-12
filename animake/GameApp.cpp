#include "GameApp.hpp"
#include "GUIManager.hpp"

namespace siapp 
{
	GameApp::GameApp(void) :
		m_pGuiManager(nullptr)
	{
	}

	GameApp::~GameApp(void)
	{
		SAFE_DELETE(m_pGuiManager);
	}

	BOOL GameApp::Initialize(void)
	{
		FileSystem::ChangeCurrentDirectory(U"Resource");
		m_pGuiManager = new GUIManager();
		m_pGuiManager->Initialize();
		TextureAsset(U"");
		return TRUE;
	}

	BOOL GameApp::Update(void)
	{
		SceneManager;
		m_pGuiManager->Update();
		return TRUE;
	}
}
