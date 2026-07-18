//-------------------------------------------------------------------
//ゲームAI
//AIは白を担当する
//-------------------------------------------------------------------
#include  "MyPG.h"
#include  "Task_GameAI.h"
#include  "Task_Input.h"

namespace  GameAI
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

		//リソースクラス生成
		this->res = Resource::Create();

		//AIの初期化
		//Minimaxの探索深さを設定
		searchDepth = 2;
		//最善手の初期値を未選択に設定
		bestMove = { -1, -1 };
		//評価値
		bestScore = 0;
		//候補手数
		candidateCount = 0;

		//AIの手番になった時にすぐ駒を置いてしまうので、少し待つための変数
		waitTimer = 0;
		isWaiting = false;

		//AIは白、プレイヤーは黒
		aiColor = Board::Resource::Stone::White;
		playerColor = Board::Resource::Stone::Black;

		return true;
	}

	//-------------------------------------------------------------------
	//「終了」タスク消滅時に１回だけ行う処理
	bool  Object::Finalize()
	{
		if (!ge->QuitFlag() && this->nextTaskCreate)
		{
		}
		return true;
	}

	//-------------------------------------------------------------------
	//タスク生成窓口
	Object::SP  Object::Create(bool  flagGameEnginePushBack_)
	{
		Object::SP  ob = Object::SP(new  Object());
		if (ob)
		{
			ob->me = ob;
			if (flagGameEnginePushBack_)
			{
				ge->PushBack(ob);
			}
			if (!ob->B_Initialize())
			{
				ob->Kill();
			}
			return ob;
		}
		return nullptr;
	}

	//-------------------------------------------------------------------
	bool  Object::B_Initialize()
	{
		return this->Initialize();
	}

	//-------------------------------------------------------------------
	Object::~Object()
	{
		this->B_Finalize();
	}

	//-------------------------------------------------------------------
	bool  Object::B_Finalize()
	{
		auto rtv = this->Finalize();
		return rtv;
	}

	//-------------------------------------------------------------------
	Object::Object()
	{
	}

	//-------------------------------------------------------------------
	//リソースクラスの生成
	Resource::SP  Resource::Create()
	{
		if (auto sp = instance.lock())
		{
			return sp;
		}
		else
		{
			sp = Resource::SP(new Resource());
			if (sp)
			{
				sp->Initialize();
				instance = sp;
			}
			return sp;
		}
	}

	//-------------------------------------------------------------------
	Resource::Resource()
	{
	}

	//-------------------------------------------------------------------
	Resource::~Resource()
	{
		this->Finalize();
	}

	//-------------------------------------------------------------------
	//相手の色を返す
	Board::Resource::Stone Object::Opponent(Board::Resource::Stone color)
	{
		if (color == Board::Resource::Stone::Black) return Board::Resource::Stone::White;
		if (color == Board::Resource::Stone::White) return Board::Resource::Stone::Black;
		return Board::Resource::Stone::Non;
	}

	//-------------------------------------------------------------------
	//盤面をコピーする、srcがコピー元、dstがコピー先
	void Object::CopyBoard(
		Board::Resource::Stone src[8][8],
		Board::Resource::Stone dst[8][8])
	{
		//盤面を1マスずつコピーする
		for (int y = 0; y < 8; ++y)
		{
			for (int x = 0; x < 8; ++x)
			{
				dst[y][x] = src[y][x];
			}
		}
	}

	//-------------------------------------------------------------------
	//仮想盤面で駒を置けるかどうかを判断する
	bool Object::IsValidMove(
		Board::Resource::Stone board[8][8],
		int x, int y,
		Board::Resource::Stone color)
	{
		//盤面外ならfalse
		if (x < 0 || x >= 8 || y < 0 || y >= 8) return false;

		//すでに石がある場合も置けない
		if (board[y][x] != Board::Resource::Stone::Non) return false;

		//相手の色を取得
		Board::Resource::Stone opp = Opponent(color);

		//8方向探索
		for (int dy = -1; dy <= 1; ++dy)
		{
			for (int dx = -1; dx <= 1; ++dx)
			{
				if (dx == 0 && dy == 0) continue;

				//隣のマスから調べ始める
				int cx = x + dx;
				int cy = y + dy;

				//相手の駒があるかどうか記録する変数
				bool existOpponent = false;

				//盤面の外に出るまで調べる
				while (cx >= 0 && cx < 8 && cy >= 0 && cy < 8)
				{
					//空マスに当たったらこの方向には置けないのでbreak
					if (board[cy][cx] == Board::Resource::Stone::Non)
					{
						break;
					}

					//相手の駒なら挟める可能性があるため先へ進む
					if (board[cy][cx] == opp)
					{
						existOpponent = true;
						cx += dx;
						cy += dy;
						continue;
					}

					//自分の駒に当たった場合、相手の駒を挟んでいるかどうかを確認
					if (board[cy][cx] == color)
					{
						//相手の駒を挟んでいる場合はこの方向に置けるのでtrueを返す
						if (existOpponent) return true;
						break;
					}

					//それ以外は無効
					break;
				}
			}
		}
		return false;
	}

	//-------------------------------------------------------------------
	//仮想盤面で駒を置きひっくり返す
	bool Object::PutStone(
		Board::Resource::Stone board[8][8],
		int x, int y,
		Board::Resource::Stone color)
	{
		//駒が置けなければ何もしない
		if (!IsValidMove(board, x, y, color)) return false;

		//相手の色を取得する
		Board::Resource::Stone opp = Opponent(color);

		//自分の駒を置く
		board[y][x] = color;

		//8方向探索
		for (int dy = -1; dy <= 1; ++dy)
		{
			for (int dx = -1; dx <= 1; ++dx)
			{
				if (dx == 0 && dy == 0) continue;

				//隣のマスから調べ始める
				int cx = x + dx;
				int cy = y + dy;

				//相手の駒があるかどうか記録する変数
				bool existOpponent = false;

				//盤面の外に出るまで調べる
				while (cx >= 0 && cx < 8 && cy >= 0 && cy < 8)
				{
					//空マスに当たったらこの方向には置けないのでbreak
					if (board[cy][cx] == Board::Resource::Stone::Non)
					{
						existOpponent = false;
						break;
					}

					//相手の駒なら挟める可能性があるため先へ進む
					if (board[cy][cx] == opp)
					{
						existOpponent = true;
						cx += dx;
						cy += dy;
						continue;
					}

					//自分の駒にたどり着いたら反転候補として確定
					if (board[cy][cx] == color)
					{
						break;
					}

					//それ以外は反転不可
					existOpponent = false;
					break;
				}

				//相手の駒が1つもなかった方向はひっくり返さない
				if (!existOpponent) continue;

				//盤面の外まで進んでしまった方向は反転しない
				if (!(cx >= 0 && cx < 8 && cy >= 0 && cy < 8)) continue;
				if (board[cy][cx] != color) continue;

				// 置いた位置の隣から、最後の自分の駒の手前までひっくり返す
				int fx = x + dx;
				int fy = y + dy;
				while (!(fx == cx && fy == cy))
				{
					board[fy][fx] = color;
					fx += dx;
					fy += dy;
				}
			}
		}
		return true;
	}

	//-------------------------------------------------------------------
	//置ける手があるかどうかを判断する
	bool Object::HasAnyMove(
		Board::Resource::Stone board[8][8],
		Board::Resource::Stone color)
	{
		for (int y = 0; y < 8; ++y)
		{
			for (int x = 0; x < 8; ++x)
			{
				//1つでも置ける場所があれば true
				if (IsValidMove(board, x, y, color))
				{
					return true;
				}
			}
		}

		//どこにも置けなければ false
		return false;
	}

	//-------------------------------------------------------------------
	//現在置ける手をすべて列挙する
	vector<Object::Move> Object::GetMoves(
		Board::Resource::Stone board[8][8],
		Board::Resource::Stone color)
	{
		//置ける手のリスト
		vector<Move> moves;

		for (int y = 0; y < 8; ++y)
		{
			for (int x = 0; x < 8; ++x)
			{
				//置ける手をリストに追加
				if (IsValidMove(board, x, y, color))
				{
					moves.push_back({ x, y });
				}
			}
		}
		return moves;
	}

	//-------------------------------------------------------------------
	//盤面に手を適用する
	void Object::ApplyMove(
		Board::Resource::Stone board[8][8],
		Move move,
		Board::Resource::Stone color)
	{
		//PutStone を使って盤面に反映
		PutStone(board, move.x, move.y, color);
	}

	//-------------------------------------------------------------------
	//盤面の評価関数（正の値はAIに有利、負の値はプレイヤーに有利）
	int Object::Evaluate(Board::Resource::Stone board[8][8])
	{
		//評価値の合計
		int score = 0;

		for (int y = 0; y < 8; ++y)
		{
			for (int x = 0; x < 8; ++x)
			{
				//通常マスの基本点
				int add = 1;

				//角は強いので高め
				if ((x == 0 && y == 0) ||
					(x == 7 && y == 0) ||
					(x == 0 && y == 7) ||
					(x == 7 && y == 7))
				{
					add = 30;
				}
				//角の隣は少し危険なので低め
				else if (
					(x == 1 && y == 0) || (x == 0 && y == 1) || (x == 1 && y == 1) ||
					(x == 6 && y == 0) || (x == 7 && y == 1) || (x == 6 && y == 1) ||
					(x == 0 && y == 6) || (x == 1 && y == 7) || (x == 1 && y == 6) ||
					(x == 6 && y == 7) || (x == 7 && y == 6) || (x == 6 && y == 6))
				{
					add = -8;
				}
				//辺は少し高め
				else if (x == 0 || x == 7 || y == 0 || y == 7)
				{
					add = 5;
				}

				//AIの駒なら加点
				if (board[y][x] == aiColor)
				{
					score += add;
				}

				//プレイヤーの駒なら減点
				else if (board[y][x] == playerColor)
				{
					score -= add;
				}
			}
		}
		return score;
	}

	//-------------------------------------------------------------------
	//Minimax法で先読みして評価値を求める
	int Object::Minimax(
		Board::Resource::Stone board[8][8],
		int depth,
		Board::Resource::Stone turn,
		bool maximizing)
	{
		//指定した深さまで読んだら現在の盤面を評価して打ち切る
		if (depth == 0)
		{
			return Evaluate(board);
		}

		//現在の手番で置ける手を列挙する
		vector<Move> moves = GetMoves(board, turn);

		//置ける手がない=パス
		if (moves.empty())
		{
			Board::Resource::Stone opp = Opponent(turn);

			//両者置けないなら終局扱い
			if (!HasAnyMove(board, opp))
			{
				return Evaluate(board);
			}

			return Minimax(board, depth - 1, opp, !maximizing);
		}

		//AIの番では評価値が最大になる手を探す
		if (maximizing)
		{
			//初期値は小さくしておく
			int best = -1000000;

			//候補手を順番に試す
			for (auto& move : moves)
			{
				//仮想盤面にコピー
				Board::Resource::Stone next[8][8];
				CopyBoard(board, next);

				//手を適用
				ApplyMove(next, move, turn);

				//1手進めた先を再帰的に評価
				int score = Minimax(next, depth - 1, Opponent(turn), false);

				//より高い評価値なら更新
				if (score > best)
				{
					best = score;
				}
			}
			return best;
		}

		//プレイヤーの番ではAIにとって不利な手を想定する
		else
		{
			//初期値は大きくしておく
			int best = 1000000;

			//候補手を順番に試す
			for (auto& move : moves)
			{
				Board::Resource::Stone next[8][8];
				CopyBoard(board, next);

				ApplyMove(next, move, turn);

				int score = Minimax(next, depth - 1, Opponent(turn), true);

				//より低い評価値なら更新
				if (score < best)
				{
					best = score;
				}
			}
			return best;
		}
	}

	//-------------------------------------------------------------------
	//AIが実際に打つ最善手を探す
	Object::Move Object::FindBestMove(
		Board::Resource::Stone board[8][8],
		int depth)
	{
		//AIが今置ける手を列挙する
		vector<Move> moves = GetMoves(board, aiColor);

		//候補手数を保存しておく
		candidateCount = (int)moves.size();

		//最善手の初期値
		Move best = { -1, -1 };

		//最善評価値の初期値
		int bestLocalScore = -1000000;

		//候補手を1つずつ試す
		for (auto& move : moves)
		{
			Board::Resource::Stone next[8][8];

			CopyBoard(board, next);
			ApplyMove(next, move, aiColor);

			//相手番から先を再帰的に評価
			int score = Minimax(next, depth - 1, playerColor, false);

			//より高評価の手なら最善手を更新
			if (score > bestLocalScore)
			{
				bestLocalScore = score;
				best = move;
			}
		}
		//最終的な最善評価値を保存
		bestScore = bestLocalScore;

		return best;
	}

	//-------------------------------------------------------------------
	void  Object::UpDate()
	{
		//現在の盤面リソースを取得
		auto boardRes = Board::Resource::Create();
		if (!boardRes) return;

		//白(AI)の手番でなければ待機状態をリセット
		if (boardRes->turn != aiColor)
		{
			waitTimer = 0;
			isWaiting = false;
			return;
		}

		Board::Resource::Stone work[8][8];
		CopyBoard(boardRes->boardData, work);

		//AIが置けないならパス
		if (!HasAnyMove(work, aiColor))
		{
			boardRes->turn = playerColor;
			bestMove = { -1, -1 };
			bestScore = 0;
			candidateCount = 0;
			waitTimer = 0;
			isWaiting = false;
			return;
		}

		//白の手番になったら待機開始
		if (!isWaiting)
		{
			isWaiting = true;
			waitTimer = 60;
			return;
		}

		//1秒待つ
		if (waitTimer > 0)
		{
			waitTimer--;
			return;
		}

		//待ち終わったら最善手を探索
		bestMove = FindBestMove(work, searchDepth);

		//有効な手が見つかったら実際の盤面へ反映
		if (bestMove.x >= 0 && bestMove.y >= 0)
		{
			if (boardRes->Board_Put(bestMove.x, bestMove.y, aiColor))
			{
				boardRes->turn = playerColor;
				boardRes->Board_Save("./data/Resource/SaveData.txt");
			}
		}

		//次のAIターンに備えてリセット
		waitTimer = 0;
		isWaiting = false;
	}

	//-------------------------------------------------------------------
	void  Object::Render2D_AF()
	{
		auto input = ge->in1->GetState();
		if (GetAsyncKeyState(VK_F2) & 0x8000)
		{
			//AIの状態をデバッグ表示
			ge->Dbg_ToDisplay(20, 20, "AI Auto Move");
			//現在の探索深さ
			ge->Dbg_ToDisplay(20, 40, "depth : %d", searchDepth);
			//候補手数
			ge->Dbg_ToDisplay(20, 60, "candidate : %d", candidateCount);
			//最善手の評価値
			ge->Dbg_ToDisplay(20, 80, "best score : %d", bestScore);
			//最善手の座標
			ge->Dbg_ToDisplay(20, 100, "best move : (%d, %d)", bestMove.x, bestMove.y);
		}
	}
}