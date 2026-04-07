//-------------------------------------------------------------------
//マウスの座標を取得する
//-------------------------------------------------------------------
#include  "MyPG.h"
#include  "Task_Input.h"

namespace  Input
{
	Resource::WP  Resource::instance;
	//-------------------------------------------------------------------
	//リソースの初期化
	bool  Resource::Initialize()
	{
		mouse = ge->mouse;
		return true;
	}
	//-------------------------------------------------------------------
	//リソースの解放
	bool  Resource::Finalize()
	{
		mouse.reset();
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

		//★データ初期化
		
		//★タスクの生成
		res->posX = 0;
		res->posY = 0;
		res->rawX = 0;
		res->rawY = 0;
		return  true;
	}
	//-------------------------------------------------------------------
	//「終了」タスク消滅時に１回だけ行う処理
	bool  Object::Finalize()
	{
		//★データ＆タスク解放


		if (!ge->QuitFlag() && this->nextTaskCreate) {
			//★引き継ぎタスクの生成
		}

		return  true;
	}
	//-------------------------------------------------------------------
	//「更新」１フレーム毎に行う処理
	void  Object::UpDate()
	{
		//実際のウィンドウ情報を取る
		auto inputSystem = XI::Obj::GetInst();
		if (!inputSystem) return;

		HWND gameWindow = inputSystem->Wnd();

		POINT cursorScreenPoint;
		GetCursorPos(&cursorScreenPoint);

		POINT cursorClientPoint = cursorScreenPoint;
		ScreenToClient(gameWindow, &cursorClientPoint);

		//raw = 補正前のクライアント座標
		res->rawX = cursorClientPoint.x;
		res->rawY = cursorClientPoint.y;

		RECT clientRect;
		GetClientRect(gameWindow, &clientRect);

		int clientWidth = clientRect.right - clientRect.left;
		int clientHeight = clientRect.bottom - clientRect.top;

		if (clientWidth <= 0 || clientHeight <= 0)
		{
			return;
		}

		//基準ゲーム画面の比率
		float gameAspect = (float)ge->screenWidth / (float)ge->screenHeight;
		float clientAspect = (float)clientWidth / (float)clientHeight;

		//実際にゲームが描画されている範囲
		int viewX = 0;
		int viewY = 0;
		int viewWidth = clientWidth;
		int viewHeight = clientHeight;

		if (clientAspect > gameAspect)
		{
			//横が余る → 左右に余白
			viewHeight = clientHeight;
			viewWidth = (int)(viewHeight * gameAspect);
			viewX = (clientWidth - viewWidth) / 2;
			viewY = 0;
		}
		else
		{
			//縦が余る → 上下に余白
			viewWidth = clientWidth;
			viewHeight = (int)(viewWidth / gameAspect);
			viewX = 0;
			viewY = (clientHeight - viewHeight) / 2;
		}

		//pos = 補正後のゲーム内座標
		//余白分を引いてから、1280x720基準へ変換
		res->posX = (res->rawX - viewX) * ge->screenWidth / viewWidth;
		res->posY = (res->rawY - viewY) * ge->screenHeight / viewHeight;
	}
	//-------------------------------------------------------------------
	//「２Ｄ描画」１フレーム毎に行う処理
	void  Object::Render2D_AF()
	{
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