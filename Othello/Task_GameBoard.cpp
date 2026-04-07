//-------------------------------------------------------------------
//盤面管理(判定、ルール管理)
//-------------------------------------------------------------------
#include  "MyPG.h"
#include  "Task_GameBoard.h"
#include  "Task_Input.h"

namespace Board
{
	Resource::WP  Resource::instance;
	//-------------------------------------------------------------------
	// 盤面の位置とサイズを更新
	void Resource::UpdateBoardLayout()
	{
		cellSize = 90;
		boardSize = cellSize * 8;

		boardOffset.x = (ge->screenWidth - boardSize) / 2;
		boardOffset.y = (ge->screenHeight - boardSize) / 2;

		boardRect.left = boardOffset.x;
		boardRect.top = boardOffset.y;
		boardRect.right = boardOffset.x + boardSize;
		boardRect.bottom = boardOffset.y + boardSize;
	}

	//-------------------------------------------------------------------
	//リソースの初期化
	bool Resource::Initialize()
	{
		UpdateBoardLayout();
		Board_Load();
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
		res->UpdateBoardLayout();

		auto inputRes = Input::Resource::Create();
		auto mouseState = inputRes->mouse->GetState();

		if (mouseState.LB.down)
		{
			ML::Point mousePoint = { inputRes->posX, inputRes->posY };

			if (!(mousePoint.x >= res->boardRect.left &&mousePoint.x < res->boardRect.right &&
				mousePoint.y >= res->boardRect.top &&mousePoint.y < res->boardRect.bottom))
			{
				return;
			}

			ML::Point localPoint ={mousePoint.x - res->boardRect.left,mousePoint.y - res->boardRect.top};

			int x = localPoint.x / res->cellSize;
			int y = localPoint.y / res->cellSize;

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
	bool Resource::Board_Load()
	{
		int temp;
		string filePath = "./data/Resource/Board.txt";
		ifstream fin(filePath);

		if (!fin) { return false; }

		for (int y = 0; y < 8; ++y)
		{
			for (int x = 0; x < 8; ++x)
			{
				fin >> temp;
				this->boardData[y][x] = (Stone)temp;
			}
		}
		fin.close();
		return true;
	}
	//-------------------------------------------------------------------
	bool Resource::Board_Check(int x, int y, Stone t)//	駒を置けるか確認する
	{
		if (x < 0 || x >= 8 || y < 0 || y >= 8) return false;//盤面外ならfalse
		if (boardData[y][x] != Stone::Non) return false;//駒が置かれていたらfalse

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
		//二重ループで8方向を調べる
		for (int dy = -1; dy <= 1; ++dy)
		{
			for (int dx = -1; dx <= 1; ++dx)
			{
				if (dx == 0 && dy == 0) continue;//そのマスは見ない

				//置こうとしているマスの隣から調べていく
				int cx = x + dx;
				int cy = y + dy;
				bool existOpponent = false;//対戦相手の駒があったかどうか記録する

				while (cx >= 0 && cx < 8 && cy >= 0 && cy < 8)//盤面の外に出るまで調べる
				{
					if (boardData[cy][cx] == Stone::Non) break;//空マスに当たったらこの方向には置けない

					if (boardData[cy][cx] == opp)//相手の駒かどうか確認する。相手の駒だったら次のマスに進む
					{
						existOpponent = true;
						cx += dx;
						cy += dy;
						continue;
					}

					if (boardData[cy][cx] == t)//自分の駒か確認する
					{
						if (existOpponent) return true;//もし自分の駒にたどり着いたとき、間に相手の駒があったらtrue
						break;
					}
					break;
				}
			}
		}
		return false;
	}

	bool Resource::Board_Put(int x, int y, Stone t)//チェックをもとに駒を置きひっくり返す
	{
		if (!Board_Check(x, y, t)) return false;//Board_Checkがfalseの時は終了

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

		boardData[y][x] = t;//駒を置く

		for (int dy = -1; dy <= 1; ++dy)
		{
			for (int dx = -1; dx <= 1; ++dx)
			{
				if (dx == 0 && dy == 0) continue;

				int cx = x + dx;
				int cy = y + dy;
				bool existOpponent = false;

				while (cx >= 0 && cx < 8 && cy >= 0 && cy < 8)
				{
					if (boardData[cy][cx] == Stone::Non) { existOpponent = false; break; }//マスが空の時、この方向はひっくり返せないのでbreak

					if (boardData[cy][cx] == opp)
					{
						existOpponent = true;
						cx += dx;
						cy += dy;
						continue;
					}

					if (boardData[cy][cx] == t)
					{
						break;
					}

					existOpponent = false;
					break;
				}

				if (!existOpponent) continue;//相手の駒を記録していない場合、ひっくり返せないので次の方向へ
				if (!(cx >= 0 && cx < 8 && cy >= 0 && cy < 8)) continue;//盤面外に出た場合は次の方向へ
				if (boardData[cy][cx] != t) continue;//自分の駒にたどり着いていない場合は次の方向へ

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
		return true;
	}
	//-------------------------------------------------------------------
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
	bool Resource::IsBoardFull()
	{
		for (int y = 0; y < 8; ++y)
		{
			for (int x = 0; x < 8; ++x)
			{
				if (boardData[y][x] == Stone::Non)
				{
					return false;
				}
			}
		}
		return true;
	}
}