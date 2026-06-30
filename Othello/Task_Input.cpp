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
		//マウス座標初期化
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
		//入力システム本体を取得
		auto inputSystem = XI::Obj::GetInst();
		if (!inputSystem) return;

		//ゲームウィンドウのハンドルを取得
		HWND gameWindow = inputSystem->Wnd();

		//画面全体でのカーソル位置を取得
		POINT cursorScreenPoint;
		GetCursorPos(&cursorScreenPoint);

		//取得した座標をウィンドウ内座標へ変換
		POINT cursorClientPoint = cursorScreenPoint;
		ScreenToClient(gameWindow, &cursorClientPoint);

		//補正前のクライアント座標を保存
		res->rawX = cursorClientPoint.x;
		res->rawY = cursorClientPoint.y;

		//現在のクライアント領域サイズを取得
		RECT clientRect;
		GetClientRect(gameWindow, &clientRect);

		//ウィンドウ内の表示サイズを計算
		int clientWidth = clientRect.right - clientRect.left;
		int clientHeight = clientRect.bottom - clientRect.top;

		if (clientWidth <= 0 || clientHeight <= 0)
		{
			return;
		}

		//基準となるゲーム画面のアスペクト比
		float gameAspect = (float)ge->screenWidth / (float)ge->screenHeight;

		//実際のウィンドウのアスペクト比
		float clientAspect = (float)clientWidth / (float)clientHeight;

		//実際にゲームが描画されている範囲を初期化
		int viewX = 0;
		int viewY = 0;
		int viewWidth = clientWidth;
		int viewHeight = clientHeight;

		//ウィンドウが横長なら左右に余白ができる
		if (clientAspect > gameAspect)
		{
			//高さを基準に描画範囲を決める
			viewHeight = clientHeight;
			viewWidth = (int)(viewHeight * gameAspect);

			//左右の余白分だけXをずらす
			viewX = (clientWidth - viewWidth) / 2;
			viewY = 0;
		}
		//ウィンドウが縦長なら上下に余白ができる
		else
		{
			//幅を基準に描画範囲を決める
			viewWidth = clientWidth;
			viewHeight = (int)(viewWidth / gameAspect);
			//上下の余白分だけYをずらす
			viewX = 0;
			viewY = (clientHeight - viewHeight) / 2;
		}

		//余白を除いた位置をゲーム基準の解像度へ変換する
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