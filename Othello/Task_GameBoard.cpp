//-------------------------------------------------------------------
//盤面管理(判定、ルール管理)
//-------------------------------------------------------------------
#include  "MyPG.h"
#include  "Task_GameBoard.h"
#include  "Task_Input.h"
#include "sound.h"

namespace Board
{
	Resource::WP  Resource::instance;
	//-------------------------------------------------------------------
	// 盤面の位置とサイズを更新
	void Resource::UpdateBoardLayout()
	{
		//1マスの大きさを設定
		cellSize = 90;

		//盤面全体の大きさを計算
		boardSize = cellSize * 8;

		//盤面が画面中央に来るように左上座標を計算
		boardOffset.x = (ge->screenWidth - boardSize) / 2;
		boardOffset.y = (ge->screenHeight - boardSize) / 2;

		//盤面全体の当たり判定範囲を更新
		boardRect.left = boardOffset.x;
		boardRect.top = boardOffset.y;
		boardRect.right = boardOffset.x + boardSize;
		boardRect.bottom = boardOffset.y + boardSize;
	}

	//-------------------------------------------------------------------
	//リソースの初期化
	bool Resource::Initialize()
	{
		//盤面位置とサイズを計算する
		UpdateBoardLayout();

		//テキストファイルから初期盤面を読み込む
		Board_Load("./data/Resource/Board.txt");
		Board_Load("./data/Resource/SaveData.txt");

		//最初の手番を黒にする
		turn = Stone::Black;

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

		return  true;
	}
	//-------------------------------------------------------------------
	//「終了」タスク消滅時に１回だけ行う処理
	bool  Object::Finalize()
	{
		if (!ge->QuitFlag() && this->nextTaskCreate) {
			
		}

		return  true;
	}
	//-------------------------------------------------------------------
	//「更新」１フレーム毎に行う処理
	void  Object::UpDate()
	{
		//画面サイズ変更に対応できるよう毎フレーム盤面配置を更新する
		res->UpdateBoardLayout();

		//入力リソースを取得
		auto inputRes = Input::Resource::Create();

		//現在のマウスボタン状態を取得
		auto mouseState = inputRes->mouse->GetState();

		if (res->turn != Resource::Stone::Black)
		{
			return;
		}
		//左クリックされたときだけ盤面入力を受け付ける
		if (mouseState.LB.down)
		{
			ML::Point mousePoint = { inputRes->posX, inputRes->posY };

			//盤面の外をクリックした場合は何もしない
			if (!(mousePoint.x >= res->boardRect.left &&mousePoint.x < res->boardRect.right &&
				mousePoint.y >= res->boardRect.top &&mousePoint.y < res->boardRect.bottom))
			{
				return;
			}

			//画面座標を盤面左上基準の座標に変換する
			ML::Point localPoint ={mousePoint.x - res->boardRect.left,mousePoint.y - res->boardRect.top};

			//クリック位置を盤面の添字に変換する
			int x = localPoint.x / res->cellSize;
			int y = localPoint.y / res->cellSize;

			//置けたときだけ駒を置き、その後に手番を交代する
			if (res->Board_Put(x, y, res->turn))
			{
				if (res->turn == Resource::Stone::Black)
				{
					res->turn = Resource::Stone::White;
				}
				else
				{
					res->turn = Resource::Stone::Black;
				}
			}
		}
	}
	//-------------------------------------------------------------------
	//「２Ｄ描画」１フレーム毎に行う処理
	void  Object::Render2D_AF()
	{
		//盤面管理タスクのため、直接描画は行わない
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
	//テキストファイルから盤面を読み込む
	bool Resource::Board_Load(string filePath)
	{
		//一時的に読み込む整数値
		int temp;

		//ファイルを開く、開けなければ失敗
		ifstream fin(filePath);

		if (!fin) { return false; }

		//盤面データを順番に読み込む
		for (int y = 0; y < 8; ++y)
		{
			for (int x = 0; x < 8; ++x)
			{
				fin >> temp;
				this->boardData[y][x] = (Stone)temp;
			}
		}
		//ファイルを閉じる
		fin.close();

		return true;
	}
	//-------------------------------------------------------------------
	//指定したマスに駒を置けるかを判定する
	bool Resource::Board_Check(int x, int y, Stone t)
	{
		//盤面外ならfalse
		if (x < 0 || x >= 8 || y < 0 || y >= 8) return false;
		;//駒が置かれていたらfalse
		if (boardData[y][x] != Stone::Non) return false;

		//相手の駒の色を決める
		Stone opp;
		if (t == Stone::Black)
		{
			opp = Stone::White;
		}
		else
		{
			opp = Stone::Black;
		}

		//8方向を順番に調べる
		for (int dy = -1; dy <= 1; ++dy)
		{
			for (int dx = -1; dx <= 1; ++dx)
			{
				if (dx == 0 && dy == 0) continue;

				//隣のマスから調べ始る
				int cx = x + dx;
				int cy = y + dy;

				//相手の駒があったかを記録する
				bool existOpponent = false;

				//盤面の外に出るまで調べる
				while (cx >= 0 && cx < 8 && cy >= 0 && cy < 8)
				{
					//空マスに当たったらこの方向には置けない
					if (boardData[cy][cx] == Stone::Non) break;

					//相手の駒ならさらに先へ進む
					if (boardData[cy][cx] == opp)
					{
						existOpponent = true;
						cx += dx;
						cy += dy;
						continue;
					}

					//自分の駒にたどり着いたらはさんでいるか確認する
					if (boardData[cy][cx] == t)
					{
						//間に相手の駒があれば有効
						if (existOpponent) return true;
						break;
					}
					//それ以外は無効
					break;
				}
			}
		}
		//どの方向も挟めなければ置けない
		return false;
	}

	//-------------------------------------------------------------------
	//駒を置きひっくり返す
	bool Resource::Board_Put(int x, int y, Stone t)
	{
		//Board_Checkがfalseの時は終了
		if (!Board_Check(x, y, t)) return false;

		Stone opp;
		//相手の駒の色を決める
		if (t == Stone::Black)
		{
			opp = Stone::White;
		}
		else
		{
			opp = Stone::Black;
		}

		//駒を置く
		boardData[y][x] = t;

		//ひっくり返せる範囲を調べる
		for (int dy = -1; dy <= 1; ++dy)
		{
			for (int dx = -1; dx <= 1; ++dx)
			{
				if (dx == 0 && dy == 0) continue;

				//隣のマスから調べ始る
				int cx = x + dx;
				int cy = y + dy;

				//相手の駒があったかを記録する
				bool existOpponent = false;

				//盤面の外に出るまで調べる
				while (cx >= 0 && cx < 8 && cy >= 0 && cy < 8)
				{
					//マスが空の時、この方向はひっくり返せないのでbreak
					if (boardData[cy][cx] == Stone::Non) { existOpponent = false; break; }

					//相手の駒ならさらに先へ進む
					if (boardData[cy][cx] == opp)
					{
						existOpponent = true;
						cx += dx;
						cy += dy;
						continue;
					}

					//自分の駒にたどり着いたら反転候補として確定
					if (boardData[cy][cx] == t)
					{
						break;
					}

					// それ以外は無効
					existOpponent = false;
					break;
				}

				//相手の駒を記録していない場合、ひっくり返せないので次の方向へ
				if (!existOpponent) continue;

				//盤面外に出た場合は次の方向へ
				if (!(cx >= 0 && cx < 8 && cy >= 0 && cy < 8)) continue;

				//自分の駒にたどり着いていない場合は次の方向へ
				if (boardData[cy][cx] != t) continue;

				//置いたマスの隣から最後の自分の駒の手前までひっくり返す
				int fx = x + dx;
				int fy = y + dy;
				while (!(fx == cx && fy == cy))//最後の自分の駒に着く手前までひっくり返す
				{
					boardData[fy][fx] = t;
					fx += dx;
					fy += dy;
				}
			}
		}

		// 駒を置いた効果音を再生
		se::Play("put_se");

		return true;
	}
	//-------------------------------------------------------------------
	//置ける場所が1つでもあるかを調べる
	bool Resource::HasAnyMove(Stone color)
	{
		for (int y = 0; y < 8; ++y)
		{
			for (int x = 0; x < 8; ++x)
			{
				if (Board_Check(x, y, color))
				{
					return true;
				}
			}
		}
		return false;
	}
	//-------------------------------------------------------------------
	//駒数を数える
	int Resource::CountStone(Stone color)
	{
		int count = 0;

		for (int y = 0; y < 8; ++y)
		{
			for (int x = 0; x < 8; ++x)
			{
				if (boardData[y][x] == color)
				{
					count++;
				}
			}
		}
		return count;
	}
	//-------------------------------------------------------------------
	//盤面がすべて埋まっているかを調べる
	bool Resource::IsBoardFull()
	{
		for (int y = 0; y < 8; ++y)
		{
			for (int x = 0; x < 8; ++x)
			{
				//空きマスが1つでもあればまだ埋まっていない
				if (boardData[y][x] == Stone::Non)
				{
					return false;
				}
			}
		}
		return true;
	}
}