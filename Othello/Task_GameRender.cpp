//-------------------------------------------------------------------
//駒の描画
//-------------------------------------------------------------------
#include  "MyPG.h"
#include  "Task_GameRender.h"
#include  "Task_GameBoard.h"

namespace  Render
{
	Resource::WP  Resource::instance;
	//-------------------------------------------------------------------
	//リソースの初期化
	bool  Resource::Initialize()
	{
		//駒画像を読み込む
		imgStone = DG::Image::Create("./data/image/Koma.png");

		Board_Initialize();
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

		//★データ初期化
		
		//★タスクの生成

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
	}
	//-------------------------------------------------------------------
	//「２Ｄ描画」１フレーム毎に行う処理
	void  Object::Render2D_AF()
	{
		res->Board_Render();
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
	//-------------------------------------------------------------------
	//駒の切り出し範囲を設定する
	void Resource::Board_Initialize()
	{
		for (int c = 0; c < 3; c++)
		{
			int x = c % 3;
			chip[c] = ML::Box2D(x * 32, 0, 32, 32);
		}
	}
	//-------------------------------------------------------------------
	//盤面情報をもとに駒を描画する
	void Resource::Board_Render()
	{
		auto boardRes = Board::Resource::Create();
		if (!boardRes)return;

		for (int y = 0; y < 8; y++)
		{
			for (int x = 0; x < 8; x++)
			{
				//盤面左上とマスサイズから描画位置を計算する
				int drawX = boardRes->boardOffset.x + (x * boardRes->cellSize);
				int drawY = boardRes->boardOffset.y + (y * boardRes->cellSize);

				//1マス分の描画範囲を作る
				ML::Box2D draw(drawX, drawY, boardRes->cellSize, boardRes->cellSize);

				//盤面データを画像番号として取り出す
				int stoneType = (int)boardRes->boardData[y][x];

				//対応する駒画像をそのマスに描画する
				imgStone->Draw(draw, chip[stoneType]);
			}
		}
	}
}