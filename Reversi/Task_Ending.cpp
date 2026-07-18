//-------------------------------------------------------------------
//エンディング
//-------------------------------------------------------------------
#include  "MyPG.h"
#include  "Task_Ending.h"
#include  "Task_Title.h"
#include  "Task_Input.h"
#include "sound.h"

namespace  Ending
{
	Resource::WP  Resource::instance;
	//-------------------------------------------------------------------
	//リソースの初期化
	bool  Resource::Initialize()
	{
		//駒数表示用の値を初期化
		blackCount = 0;
		whiteCount = 0;

		//勝敗表示用の文字列を初期化
		resultText = "";

		//結果表示用のフォントを作成
		endingFont = DG::Font::Create("ＭＳ ゴシック", 16, 40);

		return true;
	}
	//-------------------------------------------------------------------
	//リソースの解放
	bool  Resource::Finalize()
	{
		endingFont.reset();
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
		//他のBGMを止める
		bgm::AllStop();

		//エンディングBGMを再生する
		bgm::Play("ending_bgm");

		return  true;
	}
	//-------------------------------------------------------------------
	//「終了」タスク消滅時に１回だけ行う処理
	bool  Object::Finalize()
	{
		//★データ＆タスク解放


		if (!ge->QuitFlag() && this->nextTaskCreate) {
			//★引き継ぎタスクの生成
			auto  nextTask = Title::Object::Create(true);
		}

		return  true;
	}
	//-------------------------------------------------------------------
	//「更新」１フレーム毎に行う処理
	void  Object::UpDate()
	{
		auto mouseState = ge->mouse->GetState();


		if (mouseState.LB.down) {
			//自身に消滅要請
			this->Kill();
		}
	}
	//-------------------------------------------------------------------
	//「２Ｄ描画」１フレーム毎に行う処理
	void  Object::Render2D_AF()
	{
		//数値表示用の一時文字列
		char buf[64];

		//RESULTの見出しを表示
		ML::Box2D draw1(100, 100, 500, 60);
		res->endingFont->Draw(draw1, "RESULT", ML::Color(1, 1, 1, 1));

		//黒の数を文字列にして表示
		sprintf(buf, "Black : %d", res->blackCount);
		ML::Box2D draw2(100, 160, 500, 60);
		res->endingFont->Draw(draw2, buf, ML::Color(1, 1, 1, 1));

		//白の数を文字列にして表示
		sprintf(buf, "White : %d", res->whiteCount);
		ML::Box2D draw3(100, 220, 500, 60);
		res->endingFont->Draw(draw3, buf, ML::Color(1, 1, 1, 1));

		//勝敗結果を表示
		ML::Box2D draw4(100, 280, 500, 60);
		res->endingFont->Draw(draw4, res->resultText.c_str(), ML::Color(1, 1, 1, 1));

		//タイトルへ戻る操作説明を表示
		ML::Box2D draw5(100, 360, 700, 60);
		res->endingFont->Draw(draw5, "Click to Title", ML::Color(1, 1, 1, 1));
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