//-------------------------------------------------------------------
//ゲーム本編
//-------------------------------------------------------------------
#include  "MyPG.h"
#include  "Task_Game.h"
#include  "Task_GameBG.h"
#include  "Task_Ending.h"
#include  "Task_Input.h"
#include  "Task_GameRender.h"
#include  "Task_GameBoard.h"
#include  "Task_GameAI.h"
#include  "sound.h"


namespace  Game
{
	Resource::WP  Resource::instance;
	//-------------------------------------------------------------------
	//リソースの初期化
	bool  Resource::Initialize()
	{
		return true;
	}
	//-------------------------------------------------------------------
	//リソースの解放
	bool  Resource::Finalize()
	{
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
		
		//★タスクの生成
		auto bg = GameBG::Object::Create(true);
		auto boa = Board::Object::Create(true);
		auto ren = Render::Object::Create(true);
		auto ai = GameAI::Object::Create(true);

		//BGM関連
		bgm::AllStop();
		bgm::Play("game_bgm");
		return  true;
	}
	//-------------------------------------------------------------------
	//「終了」タスク消滅時に１回だけ行う処理
	bool  Object::Finalize()
	{
		//盤面のリソースを取得
		auto boardRes = Board::Resource::Create();

		//黒白の駒数と結果文字列を準備
		int blackCount = 0;
		int whiteCount = 0;
		string resultText = "";

		//盤面のリソースがあれば駒数を数えて勝敗を判定
		if (boardRes)
		{
			//駒の数を数える
			blackCount = boardRes->CountStone(Board::Resource::Stone::Black);
			whiteCount = boardRes->CountStone(Board::Resource::Stone::White);

			//駒の数を比較して勝敗を決める
			if (blackCount > whiteCount)
			{
				resultText = "BLACK WIN";
			}
			else if (blackCount < whiteCount)
			{
				resultText = "WHITE WIN";
			}
			else
			{
				resultText = "DRAW";
			}
		}

		ge->KillAll_G("本編");

		//次のタスクを生成する前に、ゲームエンジンが終了していないか確認
		if (!ge->QuitFlag() && this->nextTaskCreate) {
			auto next = Ending::Object::Create(true);

			//エンディング側へ結果を渡す
			if (next && next->res)
			{
				next->res->blackCount = blackCount;
				next->res->whiteCount = whiteCount;
				next->res->resultText = resultText;
			}
		}
		return  true;
	}
	//-------------------------------------------------------------------
	//「更新」１フレーム毎に行う処理
	void  Object::UpDate()
	{
		//盤面のリソースを取得
		auto boardRes = Board::Resource::Create();
		if (!boardRes) return;

		//駒を置ける場所があるか確認
		bool blackCanMove = boardRes->HasAnyMove(Board::Resource::Stone::Black);
		bool whiteCanMove = boardRes->HasAnyMove(Board::Resource::Stone::White);

		// 両者置けない、または盤面が埋まったら終局
		if ((!blackCanMove && !whiteCanMove) || boardRes->IsBoardFull())
		{
			this->Kill();
			return;
		}

		// 今の手番が置けないならパス
		if (boardRes->turn == Board::Resource::Stone::Black)
		{
			if (!blackCanMove && whiteCanMove)
			{
				boardRes->turn = Board::Resource::Stone::White;
			}
		}
		else
		{
			if (!whiteCanMove && blackCanMove)
			{
				boardRes->turn = Board::Resource::Stone::Black;
			}
		}
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