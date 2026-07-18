#pragma warning(disable:4996)
#pragma once
//-------------------------------------------------------------------
//ゲームAI
//-------------------------------------------------------------------
#include "GameEngine_Ver3_83.h"
#include "Task_GameBoard.h"
#include <vector>

namespace  GameAI
{
	//タスクに割り当てるグループ名と固有名
	const  string  defGroupName("本編");	//グループ名
	const  string  defName("AI");		//タスク名
	//-------------------------------------------------------------------
	class  Resource : public BResource
	{
		bool  Initialize()	override;
		bool  Finalize()	override;
		Resource();
	public:
		~Resource();
		typedef  shared_ptr<Resource>	SP;
		typedef  weak_ptr<Resource>		WP;
		static   WP  instance;
		static  Resource::SP  Create();
		//共有する変数はここに追加する
	};
	//-------------------------------------------------------------------
	class  Object : public  BTask
	{
	public:
		virtual  ~Object();
		typedef  shared_ptr<Object>		SP;
		typedef  weak_ptr<Object>		WP;
		static  Object::SP  Create(bool flagGameEnginePushBack_);
		Resource::SP	res;

	private:
		Object();
		bool  B_Initialize();
		bool  B_Finalize();
		bool  Initialize();
		void  UpDate()			override;
		void  Render2D_AF()		override;
		bool  Finalize();

	public:
		//AIが駒を置くかを判断するための構造体
		struct Move
		{
			int x, y;
		};
		int waitTimer = 0;
		bool isWaiting = false;

		//Minimax法の探索深さ
		int searchDepth = 0;

		Move bestMove = { -1, -1 };
		//Minimax法で見つかった最善手のスコア
		int bestScore = 0;

		//候補手の数
		int candidateCount = 0;

		//AIの色を白、プレイヤーの色を黒
		Board::Resource::Stone aiColor = Board::Resource::Stone::White;
		Board::Resource::Stone playerColor = Board::Resource::Stone::Black;

		Board::Resource::Stone Opponent(Board::Resource::Stone color);

		//盤面をコピーする
		void CopyBoard(
			Board::Resource::Stone src[8][8],
			Board::Resource::Stone dst[8][8]);

		bool IsValidMove(
			Board::Resource::Stone board[8][8],
			int x, int y,
			Board::Resource::Stone color);

		bool PutStone(
			Board::Resource::Stone board[8][8],
			int x, int y,
			Board::Resource::Stone color);

		bool HasAnyMove(
			Board::Resource::Stone board[8][8],
			Board::Resource::Stone color);

		vector<Move> GetMoves(
			Board::Resource::Stone board[8][8],
			Board::Resource::Stone color);

		void ApplyMove(
			Board::Resource::Stone board[8][8],
			Move move,
			Board::Resource::Stone color);

		int Evaluate(Board::Resource::Stone board[8][8]);

		int Minimax(
			Board::Resource::Stone board[8][8],
			int depth,
			Board::Resource::Stone turn,
			bool maximizing);

		Move FindBestMove(
			Board::Resource::Stone board[8][8],
			int depth);
	};
}