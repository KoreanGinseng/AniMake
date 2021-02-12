#pragma once
#include <Siv3D.hpp>
#include "Define.hpp"

namespace s3d
{
	namespace SasaGUI
	{
		class GUIManager;	
	}
}

namespace siapp 
{
	struct AnimationPattern
	{
		double                       wait    = 1.0;
		int                          no      = 0;
		int                          step    = 0;

		AnimationPattern(void) :
			wait(1.0),
			no(0),
			step(0)
		{
		}

		AnimationPattern(const AnimationPattern& obj)
		{
			wait = obj.wait;
			no   = obj.no;
			step = obj.step;
		}

		void operator= (const AnimationPattern& obj)
		{
			wait = obj.wait;
			no   = obj.no;
			step = obj.step;
		}
	};

	struct AnimationInfo
	{
		double                       offsetX = 0.0;
		double                       offsetY = 0.0;
		double                       width   = 0.0;
		double                       height  = 0.0;
		bool                         bLoop   = false;
		Array<AnimationPattern>      pattern;

		AnimationInfo(void) :
			offsetX(0.0),
			offsetY(0.0),
			width(0.0),
			height(0.0),
			bLoop(false)
		{
			for (int i : step(30))
			{
				pattern << AnimationPattern();
			}
		}
		
		AnimationInfo(const AnimationInfo& obj)
		{
			offsetX = obj.offsetX;
			offsetY = obj.offsetY;
			width   = obj.width;
			height  = obj.height;
			bLoop   = obj.bLoop;
			pattern.clear();
			for (size_t i : step(obj.pattern.size()))
			{
				pattern << AnimationPattern();
				pattern[i] = obj.pattern[i];
			}
		}

		void operator= (const AnimationInfo& obj)
		{
			offsetX = obj.offsetX;
			offsetY = obj.offsetY;
			width   = obj.width;
			height  = obj.height;
			bLoop   = obj.bLoop;
			pattern.clear();
			for (size_t i : step(obj.pattern.size()))
			{
				pattern << AnimationPattern();
				pattern[i] = obj.pattern[i];
			}
		}
	};

	class GUIManager
	{
	private:
		SasaGUI::GUIManager*         m_pGui;
		uint16                       m_SelectListNo;
		Array<AnimationInfo>         m_AnimationArray;
		Array<String>                m_AnimNameArray;
		String                       m_AnimationName;
		int                          m_SelectPattern;
		double                       m_AllFrame;
		FilePath                     m_CurrentDir;
		FilePath                     m_TextureFilePath;
		FilePath                     m_AnimFilePath;
		FilePath                     m_TextFilePath;
		bool                         m_bDeleteAssert;
		Texture                      m_Texture;
		double                       m_TexScale;
		double                       m_AnimScale;
		double                       m_EditScale;
		double                       m_MotionSpeedRate;
		int                          m_PatternCount;

		double                       m_MotionTime;
		int                          m_Pattern;

		bool                         m_bTextureScaleWindow;
		bool                         m_bAnimationScaleWindow;
		bool                         m_bEditScaleWindow;
		Vec2                         m_TextureOffset;
		Vec2                         m_AnimationOffset;
		Vec2                         m_EditOffset;

		bool                         m_bGrid;
		int                          m_GridScale;

		HSV                          m_Color;
	public:
		GUIManager(void);
		~GUIManager(void);
		void Initialize(void);
		void Update(void);
	private:
		void AnimationAddTimer(const double& s);
		void ResetAnimTimer(void);
		void LoadTexture(const FilePath& path);
		double RectScale(const Vec2& rectSize, const Vec2& drawSize);

		void LoadData(const FilePath& path);
		void SaveData(const FilePath& path);

		void AnimationViewWindow(void);

		void TextureScaleWindow(const RectF& rect, const double& def, bool& outOver);
		void AnimaitonScaleWindow(const RectF& rect, const double& def, bool& outOver);
		void EditScaleWindow(const RectF& rect, const double& def, bool& outOver);

		void GridGroup(void);
		void GridView(const RectF& rect, const double& scale);

		void AnimationDataWindow(void);
		void AnimationNameList(void);
		void AnimationDataGroup(void);
		void AnimationPatternGroup(void);
		void AnimationAddGroup(void);

		void AnimationDeleteWindow(void);

		void AnimationBtnWindow(void);
		void AnimationFileGroup(void);
		void AnimationSaveBtnGroup(void);
		void AnimationFileDataGroup(void);
		void AnimationAllFrameGroup(void);

		void AnimationListWindow(void);

		RectF GetSrcRect(void);
		RectF GetPtnRect(void);
	};
}


