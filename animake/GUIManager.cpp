#include "GUIManager.hpp"
#include "SasaGUI.hpp"

namespace siapp
{
	GUIManager::GUIManager(void) :
		m_pGui(nullptr),
		m_SelectListNo(0),
		m_AnimationArray(),
		m_AnimNameArray(),
		m_AnimationName(U""),
		m_SelectPattern(0),
		m_AllFrame(1.0),
		m_CurrentDir(),
		m_TextureFilePath(),
		m_AnimFilePath(),
		m_TextFilePath(),
		m_bDeleteAssert(false),
		m_Texture(),
		m_TexScale(1.0),
		m_AnimScale(1.0),
		m_EditScale(1.0),
		m_MotionSpeedRate(1.0),
		m_MotionTime(0.0),
		m_Pattern(0),
		m_PatternCount(30),
		m_bTextureScaleWindow(true),
		m_bAnimationScaleWindow(true),
		m_bEditScaleWindow(true),
		m_TextureOffset(0.0, 0.0),
		m_AnimationOffset(0.0, 0.0),
		m_EditOffset(0.0, 0.0),
		m_bGrid(false),
		m_GridScale(16)
	{
		m_CurrentDir = FileSystem::CurrentDirectory();
	}

	GUIManager::~GUIManager(void)
	{
		SAFE_DELETE(m_pGui);
	}

	void GUIManager::Initialize(void)
	{
		m_pGui = new SasaGUI::GUIManager();

		//初期化時にデータを一つ追加しておく
		m_AnimNameArray << U"NewAnimation1";
		m_AnimationArray << AnimationInfo();
		
		m_SelectListNo = 0;
		
		m_AnimationName = m_AnimNameArray[m_SelectListNo];
	}

	void GUIManager::Update(void)
	{
		//アニメーションの時間をフレームに依存なく進める
		AnimationAddTimer(Scene::DeltaTime());

		m_pGui->frameBegin();
		{
			//アニメーション描画ウィンドウの制御
			AnimationViewWindow();
			
			//アニメーションデータグループの制御
			if (m_bDeleteAssert)
			{
				AnimationDeleteWindow();
			}
			else
			{
				//アニメーションリストの制御
				AnimationListWindow();
				AnimationDataWindow();
			}
			
			//アニメーションボタングループの制御
			AnimationBtnWindow();
		}
		m_pGui->frameEnd();
	}

	void GUIManager::AnimationAddTimer(const double& s)
	{
		//アニメーションを進めるための経過時間を加算していく。
		m_MotionTime += s * m_MotionSpeedRate;
		
		//アニメ―ションデータの参照
		AnimationInfo* pAnim = &(m_AnimationArray[m_SelectListNo]);
		Array<AnimationPattern>* ptn = &(pAnim->pattern);

		//アニメーションパターンの待機フレームからフレームに依存しない時間を計算
		double sec = (*ptn)[m_Pattern].wait * s;

		if (m_MotionTime >= sec)
		{
			//モーション時間のリセットして次のパターンにする。
			m_MotionTime = 0.0;
			m_Pattern++;
			
			//最後のパターンを見ていて、ループフラグが立っているなら最初に戻す。
			int size = static_cast<int>(ptn->size());
			if (m_Pattern >= size)
			{
				m_Pattern = pAnim->bLoop ? 0 : size - 1;
			}
		}
	}

	void GUIManager::ResetAnimTimer(void)
	{
		//アニメーションの状態を最初に戻す。
		m_Pattern = 0;
		m_MotionTime = 0.0;
		m_SelectPattern = Clamp(m_SelectPattern, 0, static_cast<int>(m_AnimationArray[m_SelectListNo].pattern.size()));
	}

	void GUIManager::LoadTexture(const FilePath& path)
	{
		//受け取ったファイルパスをミニマップありで作成する。
		m_Texture = Texture(path, TextureDesc::Mipped);
		
		//相対パスに変換して保存しておく。
		m_TextureFilePath = FileSystem::RelativePath(path);
	}

	double GUIManager::RectScale(const Vec2& rectSize, const Vec2& drawSize)
	{
		double scale = 1.0;
		
		//0除算を防ぐためにリターンする。
		if (drawSize.x <= 0 || drawSize.y <= 0)
		{
			return scale;
		}
		
		//縦横別々に比率を求める。
		double sx = rectSize.x / drawSize.x;
		double sy = rectSize.y / drawSize.y;

		//小さいほうを入れることで、枠内に収まるようにする。
		scale = Min(sx, sy);

		//1を超えても、引き延ばしてきたなくなるだけなのでクリップしておく。
		scale = Clamp(scale, 0.1, 1.0);

		return scale;
	}

	void GUIManager::LoadData(const FilePath& path)
	{
		//ファイルパスを相対パスに変換する。
		FilePath animPath = FileSystem::RelativePath(path);

		//ファイルの拡張子がテキストファイルか調べる。
		bool isText = FileSystem::Extension(path) == U"txt";

		//テキストファイルならデータの一行目からファイル名を取得する。
		if (isText)
		{
			TextReader tr;
			if (!tr.open(animPath))
			{
				return;
			}
			tr.readLine(animPath);
			tr.close();
		}


		//アニメーションファイルを開く。失敗した場合読み込みせず終了。
		BinaryReader br;
		if (!br.open(animPath))
		{
			return;
		}

		m_AnimFilePath = animPath;
		
		//読み込む前にデータを一度消しておく。
		m_AnimationArray.clear();
		m_AnimNameArray.clear();

		//書き出した順に読み込む。書き出し順はSaveData関数を参照。
		
		int textureNameLength;
		
		br.read(&textureNameLength, sizeof(int));
		
		char* textureName = new char[textureNameLength + 1];
		
		br.read(textureName, sizeof(char) * textureNameLength);
		
		textureName[textureNameLength] = '\0';

		LoadTexture(Unicode::Widen(textureName));

		SAFE_DELETEARRAY(textureName);

		int animCount;
		
		br.read(&animCount, sizeof(int));

		for (int i : step(animCount))
		{
			m_AnimationArray << AnimationInfo();
			m_AnimNameArray << String();
			
			AnimationInfo* pAnimInfo = &(m_AnimationArray[i]);
			
			int animNameLength;

			br.read(&animNameLength, sizeof(int));
			
			char* animName = new char[animNameLength + 1];

			br.read(animName, sizeof(char) * animNameLength);
			
			animName[animNameLength] = '\0';
			
			m_AnimNameArray[i] = Unicode::Widen(animName);

			SAFE_DELETEARRAY(animName);

			float offsetX;
			float offsetY;
			float width;
			float height;

			br.read(&offsetX, sizeof(float));
			br.read(&offsetY, sizeof(float));
			br.read(&width  , sizeof(float));
			br.read(&height , sizeof(float));

			pAnimInfo->offsetX = static_cast<double>(offsetX);
			pAnimInfo->offsetY = static_cast<double>(offsetY);
			pAnimInfo->width   = static_cast<double>(width  );
			pAnimInfo->height  = static_cast<double>(height );

			char loop;
			br.read(&loop, sizeof(char));
			pAnimInfo->bLoop = loop;

			int patternCount;

			br.read(&patternCount, sizeof(int));

			//パターンデータもクリアしておく。
			pAnimInfo->pattern.clear();

			for (int j : step(patternCount))
			{
				pAnimInfo->pattern << AnimationPattern();

				AnimationPattern* pPattern = &(pAnimInfo->pattern[j]);

				float wait;

				br.read(&wait, sizeof(float));

				pPattern->wait = static_cast<double>(wait);

				br.read(&(pPattern->no)  , sizeof(int));
				br.read(&(pPattern->step), sizeof(int));
			}
		}

		int textNameLength;

		br.read(&textNameLength, sizeof(int));

		char* textName = new char[textNameLength + 1];

		br.read(textName, sizeof(char) * textNameLength);

		textName[textNameLength] = '\0';

		m_TextFilePath = Unicode::Widen(textName);

		SAFE_DELETEARRAY(textName);

		br.close();

		//アニメーションパターン参照でエラーを回避するためリセットしておく。
		ResetAnimTimer();

		//テキストボックスのアニメーション名が変更されていないので強制的に変更させる。
		m_AnimationName = m_AnimNameArray[0];

		//ファイルを読み込んだ時はいつもリストの先頭を見るようにしておく。(初代ポケモンのミュウバグを防ぐため)
		m_SelectListNo = 0;
	}

	void GUIManager::SaveData(const FilePath& path)
	{
		//ファイルパスを相対パスに変換する。
		FilePath animPath = FileSystem::RelativePath(path);

		//ファイルの拡張子がテキストファイルか調べる。
		bool isText = FileSystem::Extension(animPath) == U"txt";

		//テキストファイルなら同じファイル名でアニメーションデータを作成する。
		if (isText)
		{
			size_t pathLength = animPath.length();
			animPath = animPath.substr(0, pathLength - 4);
			animPath += U".anim";
		}

		m_AnimFilePath = animPath;

		//アニメーションファイルを開く。失敗した場合書き込みせず終了。
		BinaryWriter bw;
		if (!bw.open(m_AnimFilePath))
		{
			return;
		}

		/// ###          書き込むデータの順番は以下の通り          ###
		//     ・テクスチャのファイル名の長さ( int   )
		//     ・テクスチャのファイル名      ( char  )
		//     ・アニメーションの数          ( int   )
		/// ##ここからアニメーションの数だけループ
		//     ・アニメーション名の長さ      ( int   )
		//     ・アニメーション名            ( char  )
		//     ・オフセットX                 ( float )
		//     ・オフセットY                 ( float )
		//     ・横幅                        ( float )
		//     ・高さ                        ( float )
		//     ・ループフラグ                ( char  )
		//     ・アニメーションパターン数    ( int   )
		///  #ここからパターンの数だけループ
		//     ・待機フレーム                ( float ) 
		//     ・No                          ( int   )
		//     ・Step                        ( int   )
		///  #ここまでパターンの数だけループ
		/// ##ここまでアニメーションの数だけループ
		//     ・テキストファイル名の長さ    ( int   )
		//     ・テキストファイル名          ( char  )
		//     ・EOF
		/// ###          ここまで.animファイル                     ###
		/// ###          .txtファイルを別に出力                    ###
		/// ###          .txtファイルなので読めばわかる            ###
		
		std::string textureName = m_TextureFilePath.narrow();
		int textureNameLength   = static_cast<int>(m_TextureFilePath.length());
		int animCount           = static_cast<int>(m_AnimationArray.size());

		bw.write(&textureNameLength , sizeof(int) );
		bw.write(textureName.c_str(), sizeof(char) * textureNameLength);
		bw.write(&animCount         , sizeof(int) );
		
		for (int i : step(animCount))
		{
			AnimationInfo* pAnimInfo = &(m_AnimationArray[i]);
			std::string animName     = m_AnimNameArray[i].narrow();
			int animNameLength       = static_cast<int>  (animName.size());
			float offsetX            = static_cast<float>(pAnimInfo->offsetX);
			float offsetY            = static_cast<float>(pAnimInfo->offsetY);
			float width              = static_cast<float>(pAnimInfo->width  );
			float height             = static_cast<float>(pAnimInfo->height );
			int   patternCount       = static_cast<int>  (pAnimInfo->pattern.size());

			char loop = pAnimInfo->bLoop ? 1 : 0;
			bw.write(&animNameLength    , sizeof(int)  );
			bw.write(animName.c_str()   , sizeof(char) * animNameLength);
			bw.write(&offsetX           , sizeof(float));
			bw.write(&offsetY           , sizeof(float));
			bw.write(&width             , sizeof(float));
			bw.write(&height            , sizeof(float));
			bw.write(&loop              , sizeof(char) );
			bw.write(&patternCount      , sizeof(int)  );

			for (int j : step(patternCount))
			{
				AnimationPattern* pPattern = &(pAnimInfo->pattern[j]);
				float wait = static_cast<float>(pPattern->wait);
				bw.write(&wait            , sizeof(float));
				bw.write(&(pPattern->no)  , sizeof(int)  );
				bw.write(&(pPattern->step), sizeof(int)  );
			}
		}

		size_t animPathLength = animPath.length();
		FilePath txtPath = animPath.substr(0, animPathLength - 5);
		txtPath += U".txt";
		int txtPathLength = static_cast<int>(txtPath.length());
		std::string textName = txtPath.narrow();
		bw.write(&txtPathLength  , sizeof(int) );
		bw.write(textName.c_str(), sizeof(char) * txtPathLength);

		bw.close();

		TextWriter tw;
		if (!tw.open(txtPath))
		{
			return;
		}

		m_TextFilePath = txtPath;

		tw.write(animPath + U'\n');
		tw.write(U"上記データファイル名を消すとツールで読み込めなくなります。\n");
		tw.write(U"対応画像ファイル名：" + m_TextureFilePath + U"\n");
		tw.write(U"以下コピペ用\n");
		//例：
		//    {
		//        {
		//                "NewAnimaiton1",
		//                0,0,
		//                60,70,
		//                TRUE,{{5,0,0},{5,1,0},{5,2,0},{5,3,0}}
		//        },
		//    };
		tw.write(U"{\n");
		for (int i : step(animCount))
		{
			tw.write(U"\t{\n");
			{
				AnimationInfo* pAnimInfo = &(m_AnimationArray[i]);
				
				//アニメーション名の書き出し
				tw.write(U"\t\t\"" + m_AnimNameArray[i]         + U"\",\n");
				
				//オフセット位置の書き出し
				tw.write(U"\t\t"   + Format(pAnimInfo->offsetX) + U"," + Format(pAnimInfo->offsetY) + U"," + U"\n");
				
				//幅、高さの書き出し
				tw.write(U"\t\t"   + Format(pAnimInfo->width)   + U"," + Format(pAnimInfo->height)  + U"," + U"\n");
				
				//ループフラグの書き出し
				String sLoop = pAnimInfo->bLoop ? U"TRUE" : U"FALSE";
				tw.write(U"\t\t"   + sLoop + U",{");
				
				size_t patternCount = pAnimInfo->pattern.size();
				for (auto j : step(patternCount))
				{
					AnimationPattern* pPattern = &(pAnimInfo->pattern[j]);
					
					//アニメーションパターンの書き出し
					tw.write(U"{" + Format(pPattern->wait) + U"," + Format(pPattern->no) + U"," + Format(pPattern->step) + U"}");
					
					//最後のパターン以外はカンマで区切る。
					if (j != patternCount - 1)
					{
						tw.write(U",");
					}
				}
				tw.write(U"}\n\t},\n");
			}
		}
		tw.write(U"};");
		tw.close();
	}

	void GUIManager::AnimationViewWindow(void)
	{
		size_t tabNo = m_pGui->tab({ U"All", U"AnimOnly", U"EditPatternOnly", U"TextureOnly" });
		m_pGui->newLine();
		
		//GUIの次設置座標を取得する。
		Vec2 winPos = m_pGui->windowGetNextItemPos();
		
		//            (GUIのcursor座標, 矩形サイズ(Vec2(幅) - Vec2(マージン * 2)))
		RectF viewRect(         winPos, Vec2(animViewWindowWidth, animViewWindowHeight) - Vec2(20, 49));
		viewRect.drawFrame(2.0, Palette::Gray);
		
		//テクスチャを描画する用範囲の矩形(座標は後でセットする)
		RectF textureRect(Vec2(), Vec2(viewRect.w * 0.6, viewRect.h));
		
		//アニメーションを描画する用範囲の矩形(座標は後でセットする)
		RectF animRect(Vec2(), Vec2(viewRect.w * 0.4, viewRect.h * 0.5));
		
		//編集中のパターンを描画する用範囲の矩形(座標は後でセットする)
		RectF patternRect(animRect);

		switch (tabNo)
		{
		case 0:
		{
			//描画範囲内の左半分に設定する。
			textureRect.setPos(Vec2(viewRect.x, viewRect.y));

			//描画範囲内の右上に設定する。
			animRect.setPos(Vec2(viewRect.x + textureRect.w, viewRect.y));

			//描画範囲内の左下に設定する。
			patternRect.setPos(Vec2(viewRect.x + textureRect.w, viewRect.y + animRect.h));
			
			//各矩形の枠を描画する。
			textureRect.drawFrame(2.0, Palette::Gray);
			animRect.drawFrame(2.0, Palette::Gray);
			patternRect.drawFrame(2.0, Palette::Gray);

			//テクスチャの幅と描画枠から収まるように計算する。
			m_TexScale  = RectScale(textureRect.size, m_Texture.size());
			m_AnimScale = RectScale(animRect.size   , GetSrcRect().size);
			m_EditScale = RectScale(patternRect.size, GetPtnRect().size);

			//各枠内に画像を描画する。
			m_Texture.scaled(m_TexScale).draw(textureRect.pos);
			m_Texture(GetSrcRect()).scaled(m_AnimScale).drawAt(animRect.center());
			m_Texture(GetPtnRect()).scaled(m_EditScale).drawAt(patternRect.center());
			
			//画像を縁取る矩形の描画
			RectF textureFrame = m_Texture.region(textureRect.pos);
			textureFrame.setSize(textureFrame.size * m_TexScale);
			textureFrame.drawFrame(1.0, Palette::Green);

			//編集枠の描画
			RectF editFrame = GetPtnRect();
			editFrame.pos *= m_TexScale;
			editFrame.pos += textureRect.pos;
			editFrame.setSize(editFrame.size * m_TexScale);
			editFrame.drawFrame(1.0, Palette::Greenyellow);

			//アニメーション枠の描画
			RectF animationFrame = GetSrcRect();
			animationFrame.pos *= m_TexScale;
			animationFrame.pos += textureRect.pos;
			animationFrame.setSize(animationFrame.size * m_TexScale);
			animationFrame.drawFrame(1.0, Palette::Red);
			break;
		}
		case 1:
		{
			//描画範囲内を占有するようにリサイズする。
			animRect.setPos(viewRect.pos);
			animRect.setSize(viewRect.size);

			//テクスチャの幅と描画枠から収まるように計算する。
			double defScale = RectScale(animRect.size, GetSrcRect().size);

			//ウィンドウのマウスオーバーしてるか入れるための変数。
			bool bWindowOver = false;

			//ウィンドウが表示設定になっていれば描画する。
			if (m_bAnimationScaleWindow)
			{
				AnimaitonScaleWindow(animRect, defScale, bWindowOver);
			}

			//矩形内でウィンドウ上にマウスがない場合、マウスホイールで拡大縮小ができる。
			if (animRect.mouseOver() && !bWindowOver)
			{
				Cursor::RequestStyle(CursorStyle::Hand);
				m_AnimScale += Mouse::Wheel() * -0.08;
			}

			//矩形内でウィンドウ上にマウスがない場合、左ドラッグで画像の移動ができる。
			if (animRect.leftPressed() && !bWindowOver)
			{
				m_AnimationOffset += Cursor::DeltaF();
			}

			m_Texture(GetSrcRect()).scaled(m_AnimScale).draw(animRect.pos + m_AnimationOffset);

			//グリッドの描画
			if (m_bGrid)
			{
				animRect.setPos((animRect.pos) + m_AnimationOffset);
				animRect.setSize(GetSrcRect().size);
				GridView(animRect, m_AnimScale);
			}

			break;
		}
		case 2:
		{
			//描画範囲内を占有するようにリサイズする。
			patternRect.setPos(viewRect.pos);
			patternRect.setSize(viewRect.size);
			
			//テクスチャの幅と描画枠から収まるように計算する。
			double defScale = RectScale(patternRect.size, GetPtnRect().size);

			//ウィンドウのマウスオーバーしてるか入れるための変数。
			bool bWindowOver = false;

			//ウィンドウが表示設定になっていれば描画する。
			if (m_bEditScaleWindow)
			{
				EditScaleWindow(patternRect, defScale, bWindowOver);
			}

			//矩形内でウィンドウ上にマウスがない場合、マウスホイールで拡大縮小ができる。
			if (patternRect.mouseOver() && !bWindowOver)
			{
				Cursor::RequestStyle(CursorStyle::Hand);
				m_EditScale += Mouse::Wheel() * -0.08;
			}

			//矩形内でウィンドウ上にマウスがない場合、左ドラッグで画像の移動ができる。
			if (patternRect.leftPressed() && !bWindowOver)
			{
				m_EditOffset += Cursor::DeltaF();
			}

			m_Texture(GetPtnRect()).scaled(m_EditScale).draw(patternRect.pos + m_EditOffset);

			//グリッドの描画
			if (m_bGrid)
			{
				patternRect.setPos((patternRect.pos) + m_EditOffset);
				patternRect.setSize(GetPtnRect().size);
				GridView(patternRect, m_EditScale);
			}

			break;
		}
		case 3:
			//描画範囲内を占有するようにリサイズする。
			textureRect.setPos(viewRect.pos);
			textureRect.setSize(viewRect.size);

			//テクスチャの幅と描画枠から収まるように計算する。
			double defScale = RectScale(textureRect.size, m_Texture.size());

			//ウィンドウのマウスオーバーしてるか入れるための変数。
			bool bWindowOver = false;

			//ウィンドウが表示設定になっていれば描画する。
			if (m_bTextureScaleWindow)
			{
				TextureScaleWindow(textureRect, defScale, bWindowOver);
			}

			//矩形内でウィンドウ上にマウスがない場合、マウスホイールで拡大縮小ができる。
			if (textureRect.mouseOver() && !bWindowOver)
			{
				Cursor::RequestStyle(CursorStyle::Hand);
				m_TexScale += Mouse::Wheel() * -0.08;
			}
			
			//矩形内でウィンドウ上にマウスがない場合、左ドラッグで画像の移動ができる。
			if (textureRect.leftPressed() && !bWindowOver)
			{
				m_TextureOffset += Cursor::DeltaF();
			}

			m_Texture.scaled(m_TexScale).draw(textureRect.pos + m_TextureOffset);

			//グリッドの描画
			if (m_bGrid)
			{
				textureRect.setPos(textureRect.pos + m_TextureOffset);
				GridView(textureRect, m_TexScale);
			}
			
			break;
		}
	}

	void GUIManager::TextureScaleWindow(const RectF& rect, const double& def, bool& outOver)
	{
		m_pGui->windowBegin(U"画像詳細ウィンドウ", SasaGUI::WindowFlag::AlwaysForeground, SizeF(400, 200), rect.bottomCenter() - Vec2(200, 200));
		{
			m_pGui->label(U"画像スケール");
			m_pGui->newLine();
			
			//      slider(    データ, 最小値, 最大値)
			m_pGui->slider(m_TexScale,   0.01,   10.0);

			//      spinbox(    データ, 最小値, 最大値, 加算値, 幅)
			m_pGui->spinBox(m_TexScale,   0.01,   10.0,    0.1, 80);
			
			if (m_pGui->button(U"リセット"))
			{
				m_TexScale = def;
			}
			
			m_pGui->newLine();
			m_pGui->label(U"表示座標位置");
			m_pGui->newLine();
			
			//      spinbox(           データ,  最小値, 最大値, 加算値,  幅)
			m_pGui->spinBox(m_TextureOffset.x, -1000.0, 1000.0,    0.1, 120);
			m_pGui->spinBox(m_TextureOffset.y, -1000.0, 1000.0,    0.1, 120);
			
			if (m_pGui->button(U"座標リセット"))
			{
				m_TextureOffset = Vec2();
			}
			
			m_pGui->newLine();
			m_pGui->label(U"画像サイズ：" + Format(m_Texture.width()) + U"x" + Format(m_Texture.height()) + U"(pix)");

			outOver = m_pGui->windowHovered();
		}
		m_pGui->windowEnd();
	}

	void GUIManager::AnimaitonScaleWindow(const RectF& rect, const double& def, bool& outOver)
	{
		m_pGui->windowBegin(U"アニメーション詳細ウィンドウ", SasaGUI::WindowFlag::AlwaysForeground, SizeF(430, 240), rect.bottomCenter() - Vec2(215, 240));
		{
			m_pGui->label(U"アニメーションスケール");
			m_pGui->newLine();

			//      slider(     データ, 最小値, 最大値)
			m_pGui->slider(m_AnimScale,   0.01,   10.0);

			//      spinbox(     データ, 最小値, 最大値, 加算値, 幅)
			m_pGui->spinBox(m_AnimScale,   0.01,   10.0,    0.1, 80);
			
			if (m_pGui->button(U"リセット"))
			{
				m_AnimScale = def;
			}

			m_pGui->newLine();
			m_pGui->label(U"表示座標位置");
			m_pGui->newLine();

			//      spinbox(             データ,  最小値, 最大値, 加算値,  幅)
			m_pGui->spinBox(m_AnimationOffset.x, -1000.0, 1000.0,    0.1, 120);
			m_pGui->spinBox(m_AnimationOffset.y, -1000.0, 1000.0,    0.1, 120);

			if (m_pGui->button(U"座標リセット"))
			{
				m_AnimationOffset = Vec2();
			}

			m_pGui->newLine();
			m_pGui->label(U"再生速度");
			m_pGui->newLine();

			//      slider(           データ, 最小値, 最大値)
			m_pGui->slider(m_MotionSpeedRate,   0.01,   10.0);

			//      spinbox(           データ,  最小値, 最大値, 加算値, 幅)
			m_pGui->spinBox(m_MotionSpeedRate,    0.01,   10.0,    0.1, 80);

			if (m_pGui->button(U"速度リセット"))
			{
				m_MotionSpeedRate = 1.0;
			}

			outOver = m_pGui->windowHovered();
		}
		m_pGui->windowEnd();
	}

	void GUIManager::EditScaleWindow(const RectF& rect, const double& def, bool& outOver)
	{
		m_pGui->windowBegin(U"編集詳細ウィンドウ", SasaGUI::WindowFlag::AlwaysForeground, SizeF(400, 200), rect.bottomCenter() - Vec2(200, 200));
		{
			m_pGui->label(U"編集スケール");
			m_pGui->newLine();

			//      slider(     データ, 最小値, 最大値)
			m_pGui->slider(m_EditScale,   0.01,   10.0);

			//      spinbox(     データ, 最小値, 最大値, 加算値, 幅)
			m_pGui->spinBox(m_EditScale,   0.01,   10.0,    0.1, 80);

			if (m_pGui->button(U"リセット"))
			{
				m_EditScale = def;
			}

			m_pGui->newLine();
			m_pGui->label(U"表示座標位置");
			m_pGui->newLine();

			//      spinbox(        データ,  最小値, 最大値, 加算値,  幅)
			m_pGui->spinBox(m_EditOffset.x, -1000.0, 1000.0,    0.1, 120);
			m_pGui->spinBox(m_EditOffset.y, -1000.0, 1000.0,    0.1, 120);

			if (m_pGui->button(U"座標リセット"))
			{
				m_EditOffset = Vec2();
			}

			outOver = m_pGui->windowHovered();
		}
		m_pGui->windowEnd();
	}

	void GUIManager::GridGroup(void)
	{
		m_pGui->groupBegin(U"", true, true);
		{
			m_pGui->checkBox(m_bGrid, U"グリッドの表示");
			m_pGui->newLine();
			m_pGui->label(U"グリッドスケール");
			m_pGui->newLine();
			
			//      slider(     データ, 最小値, 最大値)
			m_pGui->slider(m_GridScale,      1,    256);
			
			//      spinbox(     データ,  最小値, 最大値, 加算値, 幅)
			m_pGui->spinBox(m_GridScale,       1,    256,      1, 60);
		}
		m_pGui->groupEnd();
	}

	void GUIManager::GridView(const RectF& rect, const double& scale)
	{
		int y_max = Clamp(((int)rect.h / m_GridScale) + 1, 1, 128);
		for (int y : step(y_max))
		{
			int x_max = Clamp(((int)rect.w / m_GridScale) + 1, 1, 128);
			for (int x : step(x_max))
			{
				Line(Vec2(x * m_GridScale * scale + rect.pos.x, rect.pos.y), Vec2(x * m_GridScale * scale + rect.pos.x, rect.pos.y + rect.size.y * scale)).draw(Palette::Gray);
				Line(Vec2(rect.pos.x, y * m_GridScale * scale + rect.pos.y), Vec2(rect.pos.x + rect.size.x * scale, y * m_GridScale * scale + rect.pos.y)).draw(Palette::Gray);
			}
		}
	}

	void GUIManager::AnimationDataWindow(void)
	{
		//                 (           ウィンドウ名,                                          ウインドウのタイプ,
		m_pGui->windowBegin(U"アニメーションデータ", SasaGUI::WindowFlag::NoMove | SasaGUI::WindowFlag::NoResize,
		//                                 ウィンドウサイズ,                                               ウインドウの表示座標)
			Size(animDataWindowWidth, animDataWindowHeight), Vec2(windowWidth - (animDataWindowWidth + animListWindowWidth), 0));
		{
			//アニメーション矩形データ情報の管理、描画
			AnimationDataGroup();
			m_pGui->newLine();
			
			//アニメーション矩形のパターン情報の管理、描画
			AnimationPatternGroup();
			m_pGui->newLine();
			
			//アニメーションデータの追加、削除の管理、描画
			AnimationAddGroup();
		}
		m_pGui->windowEnd();
	}

	void GUIManager::AnimationNameList(void)
	{
		m_pGui->label(U"アニメーション名");
		m_pGui->newLine();

		//テキストボックスに変更があった場合リスト内の文字列を更新する。
		if (m_pGui->textBox(m_AnimationName, U"", SasaGUI::TextInputFlag::All, 265.0))
		{
			m_AnimNameArray[m_SelectListNo] = m_AnimationName;
		}
	}

	void GUIManager::AnimationDataGroup(void)
	{
		m_pGui->groupBegin(U"", /*frame = */ true, /*enable = */ true);
		{
			//アニメーションリストとアニメーション名の管理、描画
			AnimationNameList();

			//選択中のアニメーションデータの参照
			AnimationInfo* pSelectInfo = &(m_AnimationArray[m_SelectListNo]);
			
			//      label( 表示テキスト,      表示色, 有効フラグ, 表示座標)
			m_pGui->label(U"OffsetX : ", unspecified,       true, Vec2(15.0, 100.0));
			m_pGui->label(U"OffsetY : ", unspecified,       true, Vec2(15.0, 140.0));
			m_pGui->label(U"Width   : ", unspecified,       true, Vec2(15.0, 180.0));
			m_pGui->label(U"Height  : ", unspecified,       true, Vec2(15.0, 220.0));
			
			//      spinBox(          扱うデータ, 最小値, 最大値, 加算値,  横幅, 有効フラグ, 表示座標)
			m_pGui->spinBox(pSelectInfo->offsetX,    0.0, 9999.0,    0.1, 150.0,       true, Vec2(130.0, 100.0));
			m_pGui->spinBox(pSelectInfo->offsetY,    0.0, 9999.0,    0.1, 150.0,       true, Vec2(130.0, 140.0));
			m_pGui->spinBox(pSelectInfo->width  ,    0.0, 9999.0,    0.1, 150.0,       true, Vec2(130.0, 180.0));
			m_pGui->spinBox(pSelectInfo->height ,    0.0, 9999.0,    0.1, 150.0,       true, Vec2(130.0, 220.0));
			
			//      checkBox(        扱うデータ,    表示テキスト, 有効フラグ, 表示座標)
			m_pGui->checkBox(pSelectInfo->bLoop, U"ループ有効化",       true, Vec2(15.0, 260.0));
			
			//ループ設定がfalseの時アニメーションを再生するためのボタン
			if (m_pGui->button(U"再生", /*enable = */!pSelectInfo->bLoop))
			{
				ResetAnimTimer();
			}
		}
		m_pGui->groupEnd();
	}

	void GUIManager::AnimationPatternGroup(void)
	{
		m_pGui->groupBegin(U"", /*frame = */ true, /*enable = */ true);
		{
			//パターン配列の参照
			Array<AnimationPattern>* pSelectPatternArray = &(m_AnimationArray[m_SelectListNo].pattern);

			//      label(      表示テキスト,      表示色, 有効フラグ, 表示座標)
			m_pGui->label(U"パターン番号 : ", unspecified,       true, Vec2(15.0, 320.0));
			m_pGui->label(U"待機フレーム : ", unspecified,       true, Vec2(15.0, 360.0));
			m_pGui->label(U"横カウント   : ", unspecified,       true, Vec2(15.0, 400.0));
			m_pGui->label(U"縦カウント   : ", unspecified,       true, Vec2(15.0, 440.0));

			//パターン最大値の取得
			int patternMax = (int)(pSelectPatternArray->size()) - 1;

			//      spinBox(     扱うデータ, 最小値,     最大値, 加算値,  横幅, 有効フラグ, 表示座標)
			m_pGui->spinBox(m_SelectPattern,      0, patternMax,      1, 150.0,       true, Vec2(130.0, 320.0));

			//編集中のパターンを参照
			AnimationPattern* pSelectPattern = (&(*pSelectPatternArray)[m_SelectPattern]);
		
			//      spinBox(          扱うデータ, 最小値, 最大値, 加算値,  横幅, 有効フラグ, 表示座標)
			m_pGui->spinBox(pSelectPattern->wait,    0.0, 1024.0,    0.1, 150.0,       true, Vec2(130.0, 360.0));
			m_pGui->spinBox(pSelectPattern->no  ,      0,   1024,      1, 150.0,       true, Vec2(130.0, 400.0));
			m_pGui->spinBox(pSelectPattern->step,      0,   1024,      1, 150.0,       true, Vec2(130.0, 440.0));
		}
		m_pGui->groupEnd();
	}

	void GUIManager::AnimationAddGroup(void)
	{
		m_pGui->groupBegin(U"", /*frame = */ true, /*enable = */ true);
		{
			if (m_pGui->button(U"追加"))
			{
				//アニメーションデータと名前を追加する。名前にはリストの数を添える。
				m_AnimationArray << AnimationInfo();
				int size = static_cast<int>(m_AnimationArray.size());

				//同じ名前があると表示がおかしくなるので後ろに付け足す。(ライブラリ側の仕様のため)
				String addName = U"NewAnimation" + Format(size);
				while (m_AnimNameArray.count(addName) != 0)
				{
					addName += U"+";
				}
				m_AnimNameArray << addName;
			}

			//アニメーション数が2以上で削除できるようにする。(0になるのを防ぐ)
			bool delflag = ((int)m_AnimationArray.size() > 1);
			if (m_pGui->button(U"削除", /*enable = */ delflag))
			{
				m_bDeleteAssert = true;
			}
		}
		m_pGui->groupEnd();
	}

	void GUIManager::AnimationDeleteWindow(void)
	{
		//                 (ウィンドウ名, ウインドウのタイプ,
		m_pGui->windowBegin(U"アニメーションの削除", SasaGUI::WindowFlag::None,
		//ウィンドウサイズ, ウインドウの表示座標)
			Size(500, 120), Vec2(windowWidth * 0.5 - 250, windowHeight * 0.5 - 60));
		{
			m_pGui->label(m_AnimationName + U"を消してもよろしいですか？");
			m_pGui->newLine();
			if (m_pGui->button(U"はい"))
			{
				m_bDeleteAssert = false;
				
				//選択中のデータを消してリサイズする。
				m_AnimNameArray.remove_at(m_SelectListNo);
				m_AnimationArray.remove_at(m_SelectListNo);
				uint16 size = (uint16)m_AnimationArray.size();
				m_AnimationArray.resize(size);
				m_AnimNameArray.resize(size);
				
				//消したときに配列の境界外を参照しないようにクリップしておく。
				m_SelectListNo = Clamp(m_SelectListNo, uint16(0), uint16(size - 1));

				//アニメーション名テキストが反映されていないので書き換えておく。
				m_AnimationName = m_AnimNameArray[m_SelectListNo];

				//アニメーションが変更されるのでアニメーションをリセットしておく。
				m_SelectPattern = 0;
				ResetAnimTimer();
			}

			if (m_pGui->button(U"いいえ"))
			{
				m_bDeleteAssert = false;
			}
		}
		m_pGui->windowEnd();
	}

	void GUIManager::AnimationBtnWindow(void)
	{
		//                 (ウィンドウ名, ウインドウのタイプ,
		//ウィンドウサイズ(ウィンドウ幅,   画面高さ - データウィンドウの高さ), ウインドウの表示座標)
		m_pGui->windowBegin(         U"", SasaGUI::WindowFlag::NoTitlebar | SasaGUI::WindowFlag::NoMove | SasaGUI::WindowFlag::NoResize,
			          Size( windowWidth, windowHeight - animDataWindowHeight), Vec2(0, animDataWindowHeight));
		{
			//ファイルの管理、描画
			AnimationFileGroup();

			//フレーム一括変更の管理、描画
			AnimationAllFrameGroup();
		}
		m_pGui->windowEnd();
	}

	void GUIManager::AnimationFileGroup(void)
	{
		m_pGui->groupBegin(U"", /*frame = */ true, /*enable = */ true);
		{
			size_t tab = m_pGui->tab({ U"データファイル", U"カレントディレクトリ", U"ウィンドウ", U"グリッド", U"背景色", U"情報" });
			m_pGui->newLine();
			switch (tab)
			{
			case 0:
			{
				//データファイル関連の制御、描画
				AnimationFileDataGroup();
				break;
			}
			case 1:
			{
				m_pGui->textBox(m_CurrentDir, U"", SasaGUI::WindowFlag::NoResize, 600.0, 3, false);
				m_pGui->newLine();

				if (m_pGui->button(U"カレントディレクトリ変更"))
				{
					Optional<FilePath> path = Dialog::SelectFolder(m_CurrentDir, U"カレントディレクトリの選択");
					if (path != none)
					{
						m_CurrentDir = path.value();
						FileSystem::ChangeCurrentDirectory(m_CurrentDir);
					}
				}

				break;
			}
			case 2:
			{
				m_pGui->checkBox(m_bTextureScaleWindow  , U"画像スケールウィンドウ表示");           m_pGui->newLine();
				m_pGui->checkBox(m_bAnimationScaleWindow, U"アニメーションスケールウィンドウ表示");	m_pGui->newLine();
				m_pGui->checkBox(m_bEditScaleWindow     , U"編集スケールウィンドウ表示");           m_pGui->newLine();
				break;
			}
			case 3:
				//グリッドグループの制御、描画
				GridGroup();
				break;
			case 4:
			{
				if (m_pGui->colorPicker(m_Color))
				{
					Scene::SetBackground(m_Color.toColorF());
				}
				break;
			}
			case 5:
			{
				m_pGui->label(U"Release Ver. " + Format(appVersion));
				m_pGui->newLine();
				break;
			}
			}
		}
		m_pGui->groupEnd();
	}

	void GUIManager::AnimationSaveBtnGroup(void)
	{
		m_pGui->label(U"データファイル (*.anim)");
		m_pGui->textBox(m_AnimFilePath, U"", SasaGUI::WindowFlag::NoResize, 300.0, 3, false);

		if (m_pGui->button(U"開く"))
		{
			Array<FileFilter> filter;
			FileFilter animFilter;
			animFilter.name = U"アニメーションデータ(*.anim;*.txt)";
			animFilter.patterns << U"anim;";
			animFilter.patterns << U"txt;";
			filter << animFilter;
			filter << FileFilter::AllFiles();
			Optional<FilePath> path = Dialog::OpenFile(filter, m_CurrentDir, U"アニメーションデータを開く");
			if (path != none)
			{
				LoadData(path.value());
			}
		}

		if (m_pGui->button(U"保存"))
		{
			Array<FileFilter> filter;
			FileFilter animFilter;
			animFilter.name = U"アニメーションデータ(*.anim;*.txt)";
			animFilter.patterns << U"anim;";
			animFilter.patterns << U"txt;";
			filter << animFilter;
			filter << FileFilter::AllFiles();
			Optional<FilePath> path = Dialog::SaveFile(filter, m_CurrentDir, U"アニメーションデータを保存");
			if (path != none)
			{
				SaveData(path.value());
			}
		}

		if (m_pGui->button(U"上書き"))
		{
			if (m_AnimFilePath == U"")
			{
				Array<FileFilter> filter;
				filter << FileFilter({ U"アニメーションデータ(*.anim;*.txt;)", {U"*.anim;*.txt;"} });
				Optional<FilePath> path = Dialog::SaveFile(filter, m_CurrentDir, U"アニメーションデータを保存");
				if (path != none)
				{
					SaveData(path.value());
				}
			}
			else
			{
				SaveData(m_AnimFilePath);
			}
		}
	}

	void GUIManager::AnimationFileDataGroup(void)
	{
		//アニメーション保存ボタングループの制御、描画
		AnimationSaveBtnGroup();
		m_pGui->newLine();

		m_pGui->label(U"画像ファイル      (*.png)");
		m_pGui->textBox(m_TextureFilePath, U"", SasaGUI::WindowFlag::NoResize, 300.0, 3, false);

		if (m_pGui->button(U"変更"))
		{
			Array<FileFilter> filter;
			filter << FileFilter::AllImageFiles();
			Optional<FilePath> path = Dialog::OpenFile(filter, m_CurrentDir, U"画像の選択");
			if (path != none)
			{
				LoadTexture(path.value());
			}
		}
		m_pGui->newLine();

		m_pGui->label(U"テキストファイル(*.txt)");
		m_pGui->textBox(m_TextFilePath, U"", SasaGUI::WindowFlag::NoResize, 300.0, 3, false);
	}

	void GUIManager::AnimationAllFrameGroup(void)
	{
		m_pGui->groupBegin(U"", /*frame = */ true, /*enable = */ true);
		{
			//
			m_pGui->spinBox(m_PatternCount, 1, 30, 1, 120);
			if (m_pGui->button(U"パターン数変更"))
			{
				Array<AnimationPattern> tmp = m_AnimationArray[m_SelectListNo].pattern;
				int oldSize = static_cast<int>(tmp.size());
				m_AnimationArray[m_SelectListNo].pattern.clear();
				for (int i : step(m_PatternCount))
				{
					m_AnimationArray[m_SelectListNo].pattern << AnimationPattern();
					if (i < oldSize)
					{
						m_AnimationArray[m_SelectListNo].pattern[i] = tmp[i];
					}
				}
				m_SelectPattern = Clamp(m_SelectPattern, 0, m_PatternCount - 1);
				m_Pattern = Clamp(m_Pattern, 0, m_PatternCount - 1);
			}
			m_pGui->newLine();

			//      spinBox(扱うデータ, 最小値, 最大値, 加算値,  横幅)
			m_pGui->spinBox(m_AllFrame,    0.0, 1024.0,    0.1, 120.0);

			//アニメーションパターンの待機フレームを一括で上書きするボタン
			if (m_pGui->button(U"フレーム一括変更"))
			{
				//編集中のアニメーションパターン配列を参照
				Array<AnimationPattern>* pSelectPatternArray = &(m_AnimationArray[m_SelectListNo].pattern);
				for (auto i : step(pSelectPatternArray->size()))
				{
					(*pSelectPatternArray)[i].wait = m_AllFrame;
				}
			}
			m_pGui->newLine();
			
			//
			if (m_pGui->button(U"横カウント = パターン番号"))
			{
				for (size_t i : step(m_AnimationArray[m_SelectListNo].pattern.size()))
				{
					AnimationPattern* pPattern = &(m_AnimationArray[m_SelectListNo].pattern[i]);
					pPattern->no = i;
				}
			}

			if (m_pGui->button(U"横カウントリセット"))
			{
				for (size_t i : step(m_AnimationArray[m_SelectListNo].pattern.size()))
				{
					AnimationPattern* pPattern = &(m_AnimationArray[m_SelectListNo].pattern[i]);
					pPattern->no = 0;
				}
			}
			m_pGui->newLine();
			
			//
			if (m_pGui->button(U"縦カウント = パターン番号"))
			{
				for (size_t i : step(m_AnimationArray[m_SelectListNo].pattern.size()))
				{
					AnimationPattern* pPattern = &(m_AnimationArray[m_SelectListNo].pattern[i]);
					pPattern->step = i;
				}
			}

			if (m_pGui->button(U"縦カウントリセット"))
			{
				for (size_t i : step(m_AnimationArray[m_SelectListNo].pattern.size()))
				{
					AnimationPattern* pPattern = &(m_AnimationArray[m_SelectListNo].pattern[i]);
					pPattern->step = 0;
				}
			}
		}
		m_pGui->groupEnd();
	}

	void GUIManager::AnimationListWindow(void)
	{
		m_pGui->windowBegin(U"アニメーションリスト", SasaGUI::WindowFlag::NoMove | SasaGUI::WindowFlag::NoResize, Size(animListWindowWidth, animListWindowHeight), Vec2(windowWidth - animListWindowWidth, 0));
		{
			for (auto i : step(m_AnimNameArray.size()))
			{
				if (m_pGui->button(m_AnimNameArray[i]))
				{
					m_SelectListNo = static_cast<uint16>(i);
					m_AnimationName = m_AnimNameArray[m_SelectListNo];
					
					//アニメーションパターン参照でエラーを回避するためリセットしておく。
					ResetAnimTimer();
				}
				m_pGui->newLine();
			}
		}
		m_pGui->windowEnd();
	}
	RectF GUIManager::GetSrcRect(void)
	{
		AnimationInfo* pAnim = &(m_AnimationArray[m_SelectListNo]);
		AnimationPattern* pPattern = &(pAnim->pattern[m_Pattern]);
		RectF animSrcRect(
			pAnim->offsetX + pPattern->no * pAnim->width,
			pAnim->offsetY + pPattern->step * pAnim->height,
			pAnim->width,
			pAnim->height
		);
		return animSrcRect;
	}
	RectF GUIManager::GetPtnRect(void)
	{
		AnimationInfo* pAnim = &(m_AnimationArray[m_SelectListNo]);
		AnimationPattern* pPattern = &(pAnim->pattern[m_SelectPattern]);
		RectF animSrcRect(
			pAnim->offsetX + pPattern->no * pAnim->width,
			pAnim->offsetY + pPattern->step * pAnim->height,
			pAnim->width,
			pAnim->height
		);
		return animSrcRect;
	}
}