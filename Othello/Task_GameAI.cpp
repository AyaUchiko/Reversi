//-------------------------------------------------------------------
//ゲームAI
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
		__super::Initialize(defGroupName, defName, true);
		this->res = Resource::Create();

		searchDepth = 4;
		bestMove = { -1, -1 };
		bestScore = 0;
		candidateCount = 0;

		waitTimer = 0;
		isWaiting = false;

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
	Board::Resource::Stone Object::Opponent(Board::Resource::Stone color)//相手の色を返す
	{
		if (color == Board::Resource::Stone::Black) return Board::Resource::Stone::White;
		if (color == Board::Resource::Stone::White) return Board::Resource::Stone::Black;
		return Board::Resource::Stone::Non;
	}

	//-------------------------------------------------------------------
	void Object::CopyBoard(
		//盤面をコピーする関数、srcがコピー元、dstがコピー先
		Board::Resource::Stone src[8][8],
		Board::Resource::Stone dst[8][8])
	{
		for (int y = 0; y < 8; ++y)
		{
			for (int x = 0; x < 8; ++x)
			{
				dst[y][x] = src[y][x];
			}
		}
	}

	//-------------------------------------------------------------------
	//駒を置けるかどうかを判断する関数
	bool Object::IsValidMove(
		Board::Resource::Stone board[8][8],
		int x, int y,
		Board::Resource::Stone color)
	{
		if (x < 0 || x >= 8 || y < 0 || y >= 8) return false;//盤面外ならfalse
		if (board[y][x] != Board::Resource::Stone::Non) return false;//すでに石がある場合も置けない

		Board::Resource::Stone opp = Opponent(color);//相手の色を取得する
		//8方向探索
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
					if (board[cy][cx] == Board::Resource::Stone::Non)
					{
						break;
					}

					if (board[cy][cx] == opp)
					{
						existOpponent = true;
						cx += dx;
						cy += dy;
						continue;
					}

					if (board[cy][cx] == color)
					{
						if (existOpponent) return true;
						break;
					}

					break;
				}
			}
		}
		return false;
	}

	//-------------------------------------------------------------------
	//ひっくり返して駒を置く関数
	bool Object::PutStone(
		Board::Resource::Stone board[8][8],
		int x, int y,
		Board::Resource::Stone color)
	{
		if (!IsValidMove(board, x, y, color)) return false;//駒が置けなければ何もしない

		Board::Resource::Stone opp = Opponent(color);
		board[y][x] = color;

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
					if (board[cy][cx] == Board::Resource::Stone::Non)
					{
						existOpponent = false;
						break;
					}

					if (board[cy][cx] == opp)
					{
						existOpponent = true;
						cx += dx;
						cy += dy;
						continue;
					}

					if (board[cy][cx] == color)
					{
						break;
					}

					existOpponent = false;
					break;
				}

				if (!existOpponent) continue;
				if (!(cx >= 0 && cx < 8 && cy >= 0 && cy < 8)) continue;
				if (board[cy][cx] != color) continue;

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
	//置ける手があるかどうかを判断する関数
	bool Object::HasAnyMove(
		Board::Resource::Stone board[8][8],
		Board::Resource::Stone color)
	{
		for (int y = 0; y < 8; ++y)
		{
			for (int x = 0; x < 8; ++x)
			{
				if (IsValidMove(board, x, y, color))
				{
					return true;
				}
			}
		}
		return false;
	}

	//-------------------------------------------------------------------
	vector<Object::Move> Object::GetMoves(
		Board::Resource::Stone board[8][8],
		Board::Resource::Stone color)
	{
		vector<Move> moves;//置ける手のリスト

		for (int y = 0; y < 8; ++y)
		{
			for (int x = 0; x < 8; ++x)
			{
				if (IsValidMove(board, x, y, color))
				{
					moves.push_back({ x, y });//置ける手をリストに追加
				}
			}
		}

		return moves;
	}

	//-------------------------------------------------------------------
	//盤面に手を適用する関数、boardは適用前の盤面、moveは適用する手、colorは置く石の色
	void Object::ApplyMove(
		Board::Resource::Stone board[8][8],
		Move move,
		Board::Resource::Stone color)
	{
		PutStone(board, move.x, move.y, color);
	}

	//-------------------------------------------------------------------
	//盤面の評価関数、boardは評価する盤面、返り値は盤面のスコア（正の値はAIに有利、負の値はプレイヤーに有利）
	int Object::Evaluate(Board::Resource::Stone board[8][8])
	{
		int score = 0;

		for (int y = 0; y < 8; ++y)
		{
			for (int x = 0; x < 8; ++x)
			{
				int add = 1;

				//角
				if ((x == 0 && y == 0) ||
					(x == 7 && y == 0) ||
					(x == 0 && y == 7) ||
					(x == 7 && y == 7))
				{
					add = 30;
				}
				//角の隣は少し危険
				else if (
					(x == 1 && y == 0) || (x == 0 && y == 1) || (x == 1 && y == 1) ||
					(x == 6 && y == 0) || (x == 7 && y == 1) || (x == 6 && y == 1) ||
					(x == 0 && y == 6) || (x == 1 && y == 7) || (x == 1 && y == 6) ||
					(x == 6 && y == 7) || (x == 7 && y == 6) || (x == 6 && y == 6))
				{
					add = -8;
				}
				//辺
				else if (x == 0 || x == 7 || y == 0 || y == 7)
				{
					add = 5;
				}

				if (board[y][x] == aiColor)
				{
					score += add;
				}
				else if (board[y][x] == playerColor)
				{
					score -= add;
				}
			}
		}
		return score;
	}

	//-------------------------------------------------------------------
	//Minimax法の実装、boardは探索する盤面、depthは探索の深さ、turnは現在の手番、maximizingは最大化する側かどうか
	int Object::Minimax(
		Board::Resource::Stone board[8][8],
		int depth,
		Board::Resource::Stone turn,
		bool maximizing)
	{
		if (depth == 0)//探索の深さが0になったら盤面を評価してスコアを返す
		{
			return Evaluate(board);
		}

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

		if (maximizing)
		{
			int best = -1000000;

			for (auto& move : moves)
			{
				Board::Resource::Stone next[8][8];
				CopyBoard(board, next);
				ApplyMove(next, move, turn);

				int score = Minimax(next, depth - 1, Opponent(turn), false);

				if (score > best)
				{
					best = score;
				}
			}

			return best;
		}
		else
		{
			int best = 1000000;

			for (auto& move : moves)
			{
				Board::Resource::Stone next[8][8];
				CopyBoard(board, next);
				ApplyMove(next, move, turn);

				int score = Minimax(next, depth - 1, Opponent(turn), true);

				if (score < best)
				{
					best = score;
				}
			}

			return best;
		}
	}

	//-------------------------------------------------------------------
	Object::Move Object::FindBestMove(
		Board::Resource::Stone board[8][8],
		int depth)
	{
		vector<Move> moves = GetMoves(board, aiColor);
		candidateCount = (int)moves.size();

		Move best = { -1, -1 };
		int bestLocalScore = -1000000;

		for (auto& move : moves)
		{
			Board::Resource::Stone next[8][8];
			CopyBoard(board, next);
			ApplyMove(next, move, aiColor);

			int score = Minimax(next, depth - 1, playerColor, false);

			if (score > bestLocalScore)
			{
				bestLocalScore = score;
				best = move;
			}
		}

		bestScore = bestLocalScore;
		return best;
	}

	//-------------------------------------------------------------------
	void  Object::UpDate()
	{
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

		// 1秒待つ
		if (waitTimer > 0)
		{
			waitTimer--;
			return;
		}

		//待ち終わったら着手
		bestMove = FindBestMove(work, searchDepth);

		if (bestMove.x >= 0 && bestMove.y >= 0)
		{
			if (boardRes->Board_Put(bestMove.x, bestMove.y, aiColor))
			{
				boardRes->turn = playerColor;
			}
		}

		//次のAIターンに備えてリセット
		waitTimer = 0;
		isWaiting = false;
	}

	//-------------------------------------------------------------------
	void  Object::Render2D_AF()
	{
		ge->Dbg_ToDisplay(20, 20, "AI Auto Move");
		ge->Dbg_ToDisplay(20, 40, "depth : %d", searchDepth);
		ge->Dbg_ToDisplay(20, 60, "candidate : %d", candidateCount);
		ge->Dbg_ToDisplay(20, 80, "best score : %d", bestScore);
		ge->Dbg_ToDisplay(20, 100, "best move : (%d, %d)", bestMove.x, bestMove.y);
	}
}