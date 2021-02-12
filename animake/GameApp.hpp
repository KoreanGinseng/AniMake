#pragma once
#include <Siv3D.hpp> // OpenSiv3D v0.4.3
#include "Define.hpp"

namespace siapp 
{
	class GUIManager;

	class GameApp
	{
	private:
		FMT_DISALLOW_COPY_AND_ASSIGN(GameApp);
		GUIManager* m_pGuiManager;
	public:
		GameApp(void);
		~GameApp(void);
		BOOL Initialize(void);
		BOOL Update(void);
	};
}

