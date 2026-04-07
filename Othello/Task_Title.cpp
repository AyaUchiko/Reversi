//-------------------------------------------------------------------
//タイトル画面
//-------------------------------------------------------------------
#include  "MyPG.h"
#include  "Task_Title.h"
#include  "Task_Game.h"
#include  "Task_Input.h"

namespace  Title
{
	Resource::WP  Resource::instance;
	//-------------------------------------------------------------------
	//リソースの初期化
	bool  Resource::Initialize()
	{
		this->img = DG::Image::Create("./data/image/Title.jpg");
		this->buttonFillImg = DG::Image::Create("./data/effect/black.png");
		return true;
	}
	//-------------------------------------------------------------------
	//リソースの解放
	bool  Resource::Finalize()
	{
		this->img.reset();
		this->buttonFillImg.reset();
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
		auto input = Input::Object::Create(true);

		//★データ初期化

		//★タスクの生成
		int buttonWidth = 260;
		int buttonHeight = 80;
		int buttonX = (ge->screenWidth - buttonWidth) / 2;
		int buttonY = ge->screenHeight - 160;

		this->startButtonRect = ML::Box2D(buttonX, buttonY, buttonWidth, buttonHeight);

		this->isMouseOnButton = false;
		this->isStartingGame = false;
		this->blinkTimer = 0;
		return  true;
	}
	//-------------------------------------------------------------------
	//「終了」タスク消滅時に１回だけ行う処理
	bool  Object::Finalize()
	{
		//★データ＆タスク解放


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
		auto mouseState = ge->mouse->GetState();

		auto inputSystem = XI::Obj::GetInst();
		if (!inputSystem) return;

		HWND gameWindow = inputSystem->Wnd();

		POINT cursorScreenPoint;
		GetCursorPos(&cursorScreenPoint);

		POINT cursorClientPoint = cursorScreenPoint;
		ScreenToClient(gameWindow, &cursorClientPoint);

		int rawX = cursorClientPoint.x;
		int rawY = cursorClientPoint.y;

		RECT clientRect;
		GetClientRect(gameWindow, &clientRect);

		int clientWidth = clientRect.right - clientRect.left;
		int clientHeight = clientRect.bottom - clientRect.top;

		if (clientWidth <= 0 || clientHeight <= 0)
		{
			return;
		}

		float gameAspect = (float)ge->screenWidth / (float)ge->screenHeight;
		float clientAspect = (float)clientWidth / (float)clientHeight;

		int viewX = 0;
		int viewY = 0;
		int viewWidth = clientWidth;
		int viewHeight = clientHeight;

		if (clientAspect > gameAspect)
		{
			viewHeight = clientHeight;
			viewWidth = (int)(viewHeight * gameAspect);
			viewX = (clientWidth - viewWidth) / 2;
			viewY = 0;
		}
		else
		{
			viewWidth = clientWidth;
			viewHeight = (int)(viewWidth / gameAspect);
			viewX = 0;
			viewY = (clientHeight - viewHeight) / 2;
		}

		int mouseX = (rawX - viewX) * ge->screenWidth / viewWidth;
		int mouseY = (rawY - viewY) * ge->screenHeight / viewHeight;

		ML::Point mousePoint = { mouseX, mouseY };

		//ボタン上にマウスがあるか
		this->isMouseOnButton = this->startButtonRect.Hit(mousePoint);

		//点滅用タイマー
		this->blinkTimer++;
		if (this->blinkTimer >= 60)
		{
			this->blinkTimer = 0;
		}

		//すでに開始演出中なら暗転終了を待つ
		if (this->isStartingGame)
		{
			if (ge->getCounterFlag("TitleFadeOut") == MyPG::MyGameEngine::COUNTER_FLAGS::LIMIT)
			{
				this->Kill();
			}
			return;
		}

		//ボタン内クリックで暗転開始
		if (mouseState.LB.down && this->isMouseOnButton)
		{
			this->isStartingGame = true;
			ge->CreateEffect(99, ML::Vec2(0, 0));
			ge->StartCounter("TitleFadeOut", 45);
			return;
		}

		//STキーでも進める
		auto inp = ge->in1->GetState();
		if (inp.ST.down)
		{
			this->isStartingGame = true;
			ge->CreateEffect(99, ML::Vec2(0, 0));
			ge->StartCounter("TitleFadeOut", 45);
			return;
		}
	}
	//-------------------------------------------------------------------
	//「２Ｄ描画」１フレーム毎に行う処理
	void  Object::Render2D_AF()
	{
		//背景
		if (this->res->img)
		{
			ML::Box2D draw(0, 0, ge->screenWidth, ge->screenHeight);
			ML::Box2D src(0, 0, 1920, 1080);
			this->res->img->Draw(draw, src);
		}

		//マウスが乗っているときだけ点滅表示
		bool showButton = true;
		if (this->isMouseOnButton)
		{
			if (this->blinkTimer >= 30)
			{
				showButton = false;
			}
		}

		if (showButton && this->res->buttonFillImg)
		{
			ML::Box2D src(0, 0, 256, 256);
			this->res->buttonFillImg->Draw(this->startButtonRect,src,ML::Color(0.4f, 0.4f, 0.4f, 0.5f));
		}

		// ボタン文字
		ge->Dbg_ToDisplay(this->startButtonRect.x + 85,this->startButtonRect.y + 28,"START");
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