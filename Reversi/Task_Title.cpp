//-------------------------------------------------------------------
//タイトル画面
//-------------------------------------------------------------------
#include  "MyPG.h"
#include  "Task_Title.h"
#include  "Task_Game.h"
#include  "Task_Input.h"
#include  "sound.h"

namespace  Title
{
	Resource::WP  Resource::instance;
	//-------------------------------------------------------------------
	//リソースの初期化
	bool  Resource::Initialize()
	{
		//タイトル文字表示用のフォントを作成
		this->titleFont = DG::Font::Create("ＭＳ ゴシック", 16, 40);

		//タイトル背景画像を読み込む
		this->img = DG::Image::Create("./data/image/Title.png");

		//スタートボタンの下に使う画像を読み込む
		this->buttonFillImg = DG::Image::Create("./data/effect/black.png");

		return true;
	}
	//-------------------------------------------------------------------
	//リソースの解放
	bool  Resource::Finalize()
	{
		this->img.reset();
		this->buttonFillImg.reset();
		this->titleFont.reset();
		return true;
	}
	//-------------------------------------------------------------------
	//「初期化」タスク生成時に１回だけ行う処理
	bool  Object::Initialize()
	{
		//スーパークラス初期化
		__super::Initialize(defGroupName, defName, true);

		//リソースクラス生成orリソース共有
		this->res = Resource::Create();

		//マウス入力用のタスクを生成
		auto input = Input::Object::Create(true);

		int buttonWidth = 260;
		int buttonHeight = 70;
		int buttonX = (ge->screenWidth - buttonWidth) / 2;
		int buttonY = ge->screenHeight - 210;

		this->startButtonRect = ML::Box2D(buttonX, buttonY, buttonWidth, buttonHeight);
		this->continueButtonRect = ML::Box2D(buttonX, buttonY + 90, buttonWidth, buttonHeight);

		this->yesButtonRect = ML::Box2D(ge->screenWidth / 2 - 150, ge->screenHeight / 2 + 80, 120, 60);
		this->noButtonRect = ML::Box2D(ge->screenWidth / 2 + 30, ge->screenHeight / 2 + 80, 120, 60);

		ifstream fin("./data/Resource/SaveData.txt");
		this->hasSaveData = (bool)fin;

		this->isMouseOnButton = false;
		this->isMouseOnContinueButton = false;
		this->isStartingGame = false;
		this->isConfirmNewGame = false;
		this->blinkTimer = 0;

		//BGM関連
		bgm::AllStop();
		bgm::Play("title_bgm");

		return  true;
	}
	//-------------------------------------------------------------------
	//「終了」タスク消滅時に１回だけ行う処理
	bool  Object::Finalize()
	{
		if (!ge->QuitFlag() && this->nextTaskCreate) {
			//★引き継ぎタスクの生成
			auto  nextTask = Game::Object::Create(true);
		}
		return  true;
	}
	//-------------------------------------------------------------------
	//「更新」１フレーム毎に行う処理
	void  Object::UpDate()
	{
		//現在のマウスボタン状態を取得
		auto mouseState = ge->mouse->GetState();

		//補正済みマウス座標を取得
		auto inputRes = Input::Resource::Create();
		if (!inputRes) return;

		ML::Point mousePoint = { inputRes->posX, inputRes->posY };

		//マウスがスタートボタンの上にあるかどうか判定
		this->isMouseOnButton = this->startButtonRect.Hit(mousePoint);
		this->isMouseOnContinueButton = this->continueButtonRect.Hit(mousePoint);

		this->blinkTimer++;
		if (this->blinkTimer >= 60)
		{
			this->blinkTimer = 0;
		}

		if (this->isStartingGame)
		{
			if (ge->getCounterFlag("TitleFadeOut") == MyPG::MyGameEngine::COUNTER_FLAGS::LIMIT)
			{
				this->Kill();
			}
			return;
		}

		if (this->isConfirmNewGame)
		{
			if (mouseState.LB.down && this->yesButtonRect.Hit(mousePoint))
			{
				remove("./data/Resource/SaveData.txt");

				Board::Resource::loadSaveData = false;
				this->hasSaveData = false;
				this->isConfirmNewGame = false;
				this->isStartingGame = true;

				ge->CreateEffect(99, ML::Vec2(0, 0));
				ge->StartCounter("TitleFadeOut", 50);
				return;
			}

			if (mouseState.LB.down && this->noButtonRect.Hit(mousePoint))
			{
				this->isConfirmNewGame = false;
				return;
			}

			return;
		}

		if (mouseState.LB.down && this->isMouseOnButton)
		{
			if (this->hasSaveData)
			{
				this->isConfirmNewGame = true;
				return;
			}

			Board::Resource::loadSaveData = false;
			this->isStartingGame = true;
			ge->CreateEffect(99, ML::Vec2(0, 0));
			ge->StartCounter("TitleFadeOut", 50);
			return;
		}

		if (this->hasSaveData && mouseState.LB.down && this->isMouseOnContinueButton)
		{
			Board::Resource::loadSaveData = true;
			this->isStartingGame = true;
			ge->CreateEffect(99, ML::Vec2(0, 0));
			ge->StartCounter("TitleFadeOut", 50);
			return;
		}
	}
	//-------------------------------------------------------------------
	//「２Ｄ描画」１フレーム毎に行う処理
	void  Object::Render2D_AF()
	{
		//背景の表示
		if (this->res->img)
		{
			ML::Box2D draw(0, 0, ge->screenWidth, ge->screenHeight);
			ML::Box2D src(0, 0, 1920, 1080);
			this->res->img->Draw(draw, src);
		}

		//マウスが乗っているときだけ点滅表示
		bool showButton = true;
		bool showContinueButton = true;
		if (this->isMouseOnButton)
		{
			if (this->blinkTimer >= 30)
			{
				showButton = false;
			}
		}
		if (this->hasSaveData && this->isMouseOnContinueButton)
		{
			if (this->blinkTimer >= 30)
			{
				showContinueButton = false;
			}
		}

		if (showButton && this->res->buttonFillImg)
		{
			ML::Box2D src(0, 0, 256, 256);
			this->res->buttonFillImg->Draw(this->startButtonRect,src,ML::Color(0.4f, 0.4f, 0.4f, 0.5f));
		}

		//スタートボタンの文字を表示する
		ML::Box2D draw(this->startButtonRect.x + 90, this->startButtonRect.y + 20, 220, 60);
		this->res->titleFont->Draw(
			this->startButtonRect,
			"START",
			ML::Color(1, 1, 1, 1),
			DT_CENTER | DT_VCENTER | DT_SINGLELINE
		);

		ML::Box2D continueDraw(this->continueButtonRect.x + 45, this->continueButtonRect.y + 15, 260, 60);

		if (showContinueButton && this->res->buttonFillImg)
		{
			ML::Box2D src(0, 0, 256, 256);

			ML::Color continueColor;
			if (this->hasSaveData)
			{
				continueColor = ML::Color(0.4f, 0.4f, 0.4f, 0.5f);
			}
			else
			{
				continueColor = ML::Color(0.1f, 0.1f, 0.1f, 0.7f);
			}

			this->res->buttonFillImg->Draw(this->continueButtonRect, src, continueColor);
		}

		ML::Color continueTextColor;
		if (this->hasSaveData)
		{
			continueTextColor = ML::Color(1, 1, 1, 1);
		}
		else
		{
			continueTextColor = ML::Color(0.4f, 0.4f, 0.4f, 1);
		}

		this->res->titleFont->Draw(
			this->continueButtonRect,
			"CONTINUE",
			continueTextColor,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE
		);

		if (this->isConfirmNewGame)
		{
			ML::Box2D src(0, 0, 256, 256);

			//後ろの四角い背景
			ML::Box2D box(ge->screenWidth / 2 - 320, ge->screenHeight / 2 - 130, 640, 280);
			this->res->buttonFillImg->Draw(box, src, ML::Color(0.85f, 0, 0, 0));

			//メッセージ
			ML::Box2D msg(ge->screenWidth / 2 - 300, ge->screenHeight / 2 - 80, 600, 50);
			this->res->titleFont->Draw(msg, "SAVE DATA EXISTS", ML::Color(1, 1, 1, 1), DT_CENTER | DT_VCENTER | DT_SINGLELINE);

			ML::Box2D msg2(ge->screenWidth / 2 - 300, ge->screenHeight / 2 - 25, 600, 50);
			this->res->titleFont->Draw(msg2, "START NEW GAME?", ML::Color(1, 1, 1, 1), DT_CENTER | DT_VCENTER | DT_SINGLELINE);

			//YES/NO ボタン
			this->res->buttonFillImg->Draw(this->yesButtonRect, src, ML::Color(0.8f, 0.2f, 0.2f, 0.2f));
			this->res->buttonFillImg->Draw(this->noButtonRect, src, ML::Color(0.8f, 0.2f, 0.2f, 0.2f));

			this->res->titleFont->Draw(this->yesButtonRect, "YES", ML::Color(1, 1, 1, 1), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			this->res->titleFont->Draw(this->noButtonRect, "NO", ML::Color(1, 1, 1, 1), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}
	}

	//★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★
	//以下は基本的に変更不要なメソッド
	//★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★
	//-------------------------------------------------------------------
	//タスク生成窓口
	Object::SP  Object::Create(bool  flagGameEnginePushBack_)
	{
		Object::SP  ob = Object::SP(new  Object());
		if (ob) {
			ob->me = ob;
			if (flagGameEnginePushBack_) {
				ge->PushBack(ob);//ゲームエンジンに登録
			}
			if (!ob->B_Initialize()) {
				ob->Kill();//イニシャライズに失敗したらKill
			}
			return  ob;
		}
		return nullptr;
	}
	//-------------------------------------------------------------------
	bool  Object::B_Initialize()
	{
		return  this->Initialize();
	}
	//-------------------------------------------------------------------
	Object::~Object() { this->B_Finalize(); }
	bool  Object::B_Finalize()
	{
		auto  rtv = this->Finalize();
		return  rtv;
	}
	//-------------------------------------------------------------------
	Object::Object() {	}
	//-------------------------------------------------------------------
	//リソースクラスの生成
	Resource::SP  Resource::Create()
	{
		if (auto sp = instance.lock()) {
			return sp;
		}
		else {
			sp = Resource::SP(new  Resource());
			if (sp) {
				sp->Initialize();
				instance = sp;
			}
			return sp;
		}
	}
	//-------------------------------------------------------------------
	Resource::Resource() {}
	//-------------------------------------------------------------------
	Resource::~Resource() { this->Finalize(); }
}