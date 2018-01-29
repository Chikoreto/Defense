#pragma warning(disable : 4996)

#include"DxLib.h"
#include<stdio.h>
#include<stdlib.h>
#include<math.h>




#define SCREEN_X 800   //画面の大きさ
#define SCREEN_Y 600   //     ﹆
#define MAX_SHOT 50    //最大弾数
#define MAX_ENE 50     //最大敵数
#define MAX_ENE_SHOT 100//最大敵弾数
#define MAX_ITEM 15		//最大アイテム数
#define TOWER_MAX_HP 1000.0	//タワー最大HP
#define KEN_MAX_COUNT 4

#define PI 3.1415926535897932384626433832795f     //円周率


int Key[256]; // キーが押されているフレーム数を格納する
int gamecount = 0;		//ゲームが始まったらカウント開始、敵の出現などに使用
int image_nam, time; //画像の番号と表示の時間
int muki; //歩行モーションの向き
int stage_num;//現在のステージ数
int last_game_count;///敵が出てくる最後のカウント
bool game_clear_flag = false;////ゲームクリアかどうかのフラグ
double damage_rate[6] = { 1.0,1.2,1.6,1.8,2.0,2.3 };	//キャラクタのレベルに応じだダメージレート
double enemovespeed[3] = { 1.0,0.5,0.7 };		//敵の種類ごとの移動スピード
double item_image_size[5] = { 0.5,0.7,1,1.3,1.7 };//アイテムの画像の大きさ

int se[8];//seを入れるハンドル(0決定音1クリア音2ゲームオーバー音3爆発音4ショット音5斬撃音6打撃音7選択上下移動音)

int naw_mode = 0;//現在どのモードか（メニュー画面だったりゲーム画面だったり）０タイトル、１ステージ選択、２ゲーム

				 ////ファイル読み込み系/////
char open_stage_name[20][30];	//読み込みファイル名格納用
char open_stage_name_temp[20][30];	//読み込みファイル名格納用/////中身が消えるバグ回避用
int naw_number = 0, all_number;	//現在選択されているステージ、全部のステージ数

								//int i = 0;				//何番目までロードしたか いらないかも
int set_ene_num = 0;
bool all_stage_num = false, ene_load_notend = true;		//ステージ数を読み込んだかどうか  敵の出現パターンデータの読み込みが最後まで行ったかのフラグ



														//キャラ構造体
typedef struct {  //キャラ構造体
				  //shotの間隔カウンタ、弾の進行距離制限フラグ、大きさ,遠距離レベル,近距離レベル,気絶時間,向きのフラグ ０正面 3左 6右 9後ろ、攻撃力、吹っ飛び距離,いくつでレベルアップするか
	int shot_count, tama_kyori_flag, renge, shot_lev, shot_lev_count, ken_lev, ken_lev_count, stantime, muki_num, motion_num, attackpoint_shot, attackpoint_ken, stan_meter, lev_up_grade;
	double x, y, xtemp, ytemp;		//キャラの座標,座標temp
	bool damege;		//ダメージを受けたかどうか

}ch_t;
ch_t ch;

//敵構造体
typedef struct {  //敵構造体
				  //向きのフラグ・現在向いている向き・画像ナンバー・動画像表示時間,当たり判定大きさ,どのアイテムを落とすか（０は落とさない）,どのダメージを喰らったか(0無1遠2近3特),敵の種類,爆発画像のナンバー,爆発画像カウンター,行動パターン(0爆、1遠、2近),ダメージ硬化時間,攻撃間隔,いつ敵が出るか,ゲーム内カウントテンプ,アイテムの量
	int muki_flag, naw_muki, g_namu, time, renge, item_num, damege_type, bakuimage_num, bakucount, pattern, damagecount, attack_count, spawn_count, gamecount_temp, plus_num;
	double x, y, xmove, ymove, angle, xtemp, ytemp, hp, hp_max, xtemp2, ytemp2;  //x,y座標,xy移動量,移動速度,,角度1つ前の座標を保存・HP
	bool deth_flag, muve_flag, baku, attack;								//生存フラグ(t生f死),画像ナンバー増減を判断するフラグ(t増f減),爆発させる,攻撃が可能かどうか
}ene_t;
ene_t ene[MAX_ENE];

typedef struct {
	double x, y, hp_max;
	int pattern, item_num, spwan_count, plus_num;

}ene_tmp_t;
ene_tmp_t ene_tmp[500];

//敵弾構造体
typedef struct {
	int renge, attack_taype, bakucount, bakuimage_num;				//当たり判定大きさ,近遠どちらの攻撃か、爆発のカウンタ、爆発画像ナンバー
	double  x, y, xmove, ymove, movespeed, angle;		//座標、移動量、移動速度、角度
	bool deth_flag, baku;			//生存フラグ爆発のフラグ
}ene_shot_t;
ene_shot_t ene_shot[MAX_ENE_SHOT];

//タワー構造体
typedef struct {//tower構造体
	int renge;		//,当たり判定大きさ
	double x, y, hp, hp_max;//座標,体力
	bool deth_flag;//生存フラグ
}tower_t;
tower_t tower;

//弾構造体
typedef struct {  //弾構造体
	int muki_flag, kyori_meter, renge;  //どの向きに進むかの判定フラグ、進行距離メーター,大きさ,弾のレベル
	double x, y, g_angle;  //x,y座標、弾の画像の回転角度
	bool deth_flag;    //生存フラグ
}tama_t;
tama_t tama[MAX_SHOT];

//剣構造体
typedef struct {
	int g_num, kaunt, renge;   //画像の表示番号,画像差し替えカウント、大きさ,剣レベル
	double x, y, g_angle, max_move;      //座標x,ｙ画像の回転角度,敵をどの程度移動させるか
	bool deth_flag;   //生存フラグ
}ken_t;
ken_t ken;

//特殊攻撃構造体
typedef struct {
	int g_num, gicon_num, count, renge, level, get_flag, coolcount, cool_maxcount, effectcount, effect_maxcount;	//画像表示番号、アイコン番号、画像差し替えカウント、当たり判定大きさ、レベル、どの特殊攻撃取得判定フラグ,クールタイム,効果時間
	double x, y, g_angle;					//座標、回転角度
	bool deth_flag, attack_flag;			//生存フラグ,攻撃してるかのフラグ
}special_t;
special_t special;

//アイテム構造体
typedef struct {
	int item_num, count, renge, plus_num;	//どのアイテムか,一定時間消失カウント用,当たり判定大きさ,一つ当たりどれぐらい増やすか
	double x, y, angle, xmove, ymove;		//座標
	bool deth_flag;	//生存フラグ
}item_t;
item_t item[MAX_ITEM];

/////////爆発構造体
typedef struct {
	int image_unum, count;
	double x, y;
	bool deth_flag;
}baku_t;
baku_t baku[MAX_ENE];



// キーの入力状態を更新する
int gpUpdateKey() {
	char tmpKey[256]; // 現在のキーの入力状態を格納する
	GetHitKeyStateAll(tmpKey); // 全てのキーの入力状態を得る
	for (int i = 0; i<256; i++) {
		if (tmpKey[i] != 0) { // i番のキーコードに対応するキーが押されていたら
			Key[i]++;     // 加算
		}
		else {              // 押されていなければ
			Key[i] = 0;   // 0にする
		}
	}
	return 0;
}

//初期化
void ini()
{
	gamecount = 0;
	ene_load_notend = true;
	game_clear_flag = false;

	image_nam = 0;
	time = 0;

	//自機キャラ初期化

	ch.shot_count = 0;
	ch.tama_kyori_flag = SCREEN_Y / 2;
	ch.renge = 8;
	ch.shot_lev = 1;
	ch.shot_lev_count = 0;
	ch.ken_lev = 1;
	ch.ken_lev_count = 0;
	ch.stantime = 0;
	ch.muki_num = 0;
	ch.motion_num = 1;

	ch.x = 100.0, ch.y = 100.0;
	ch.xtemp = 0.0;
	ch.ytemp = 0.0;

	ch.damege = false;



	//敵初期化
	for (int i = 0; i<MAX_ENE; i++)
	{
		ene[i].deth_flag = false;
		ene[i].muve_flag = true;
		ene[i].baku = false;
		ene[i].attack = false;

		ene[i].muki_flag = 0;
		ene[i].naw_muki = 0;
		ene[i].g_namu = 0;
		ene[i].time = 0;
		ene[i].renge = 20;
		ene[i].item_num = 1;
		ene[i].damege_type = 0;
		ene[i].bakuimage_num = 0;
		ene[i].bakucount = 0;
		ene[i].pattern = 1;
		ene[i].damagecount = 0;
		ene[i].attack_count = 0;
		ene[i].spawn_count = -1000;
		ene[i].gamecount_temp = 0;
		ene[i].plus_num;

		ene[i].x = 1.0;
		ene[i].y = 1.0;
		ene[i].xmove = 1.0;
		ene[i].ymove = 1.0;
		ene[i].angle = 0.0;
		ene[i].xtemp = 0.0;
		ene[i].ytemp = 0.0;
		ene[i].hp = 100.0;
		ene[i].hp_max = 100.0;
		ene[i].xtemp2 = 0.0;
		ene[i].ytemp2 = 0.0;
	}

	//敵弾初期化
	for (int i = 0; i < MAX_ENE_SHOT; i++)
	{
		ene_shot[i].deth_flag = false;
		ene_shot[i].baku = false;

		ene_shot[i].renge = 5;
		ene_shot[i].attack_taype = 1;
		ene_shot[i].bakucount = 0;
		ene_shot[i].bakuimage_num = 0;

		ene_shot[i].x = 0.0;
		ene_shot[i].y = 0.0;
		ene_shot[i].xmove = 0.0;
		ene_shot[i].ymove = 0.0;
		ene_shot[i].movespeed = 3.0;
		ene_shot[i].angle = 0.0;



	}

	//タワー初期化
	tower.deth_flag = true;

	tower.renge = 16;

	tower.x = SCREEN_X / 2 - 16;
	tower.y = SCREEN_Y / 2 - 16;
	tower.hp = 1000.0;
	tower.hp_max = 1000.0;


	//弾初期化
	for (int i = 0; i<MAX_SHOT; i++)
	{
		tama[i].deth_flag = false;//生存フラグ初期化

		tama[i].muki_flag = 0;
		tama[i].kyori_meter = 0;
		tama[i].renge = 8;

		tama[i].x = 0.0;
		tama[i].y = 0.0;
		tama[i].g_angle = 0.0;
	}



	//剣初期化
	ken.deth_flag = false;

	ken.g_num = 0;
	ken.kaunt = 0;
	ken.renge = 25;

	ken.x = 0.0;
	ken.y = 0.0;
	ken.g_angle = 0.0;
	ken.max_move = 30.0;


	//特殊攻撃初期化
	special.deth_flag = false;
	special.attack_flag = false;

	special.g_num = 0;
	special.gicon_num = 1;
	special.count = 0;
	special.renge = 8;
	special.level = 1;
	special.get_flag = 1;
	special.coolcount = 0;				//クールタイム
	special.cool_maxcount = 0;
	special.effectcount = 0;
	special.effect_maxcount = 0;

	special.x = 0.0;
	special.y = 0.0;
	special.g_angle = 0.0;

	//アイテム初期化
	for (int i = 0; i < MAX_ITEM; i++)
	{
		item[i].deth_flag = false;

		item[i].item_num = 0;
		item[i].count = 0;
		item[i].renge = 16;
		item[i].plus_num = 1;

		item[i].x = 0.0;
		item[i].y = 0.0;
		item[i].angle = 0.0;
		item[i].xmove = 0.0;
		item[i].ymove = 0.0;
	}

	//爆発構造体初期化
	for (int i = 0; i <= MAX_ENE; i++)
	{
		baku[i].deth_flag = false;

		baku[i].image_unum = 0;
		baku[i].count = 0;

		baku[i].x = 0.0;
		baku[i].y = 0.0;
	}

}

/////seを鳴らす(0決定音1クリア音2ゲームオーバー音3爆発音4ショット音5斬撃音6打撃音)//////
void sound_se(int num)
{
	PlaySoundMem(se[num], DX_PLAYTYPE_BACK);
}

//////メニュー関数
void title_menu(int title_g)
{
	static int select_num;//何番目を選んでいるか
	static int color[2] = { GetColor(0,0,0),GetColor(255,0,255) };
	int y[2] = { 350,400 };

	if (Key[KEY_INPUT_DOWN] == 1 || Key[KEY_INPUT_RIGHT] == 1)//下か右を押されたら
	{
		sound_se(7);
		select_num = ((select_num + 1) % 2);	//選択を一つ下へ
	}

	else if (Key[KEY_INPUT_UP] == 1 || Key[KEY_INPUT_LEFT] == 1)//上か左を押されたら
	{
		sound_se(7);
		select_num = ((select_num + 2 - 1) % 2);	//選択を一つ上へ
	}

	DrawGraph(250, 100, title_g, true);//タイトル画像表示

	DrawString(350, y[0], "ゲームスタート", color[0]);
	DrawString(350, y[1], "ゲームエンド", color[0]);
	DrawString(300, y[select_num], "→", color[1]);

	if (Key[KEY_INPUT_RETURN] == 1)//エンターを押されたら
	{
		sound_se(0);//se鳴らす
		if (select_num == 0)
			naw_mode = 1;	//モード変更

		else
			naw_mode = 10;
	}

}

/////ステージセレクト///////
void stage_select()
{
	static int select_num = 0;///何番目を選んでいるか
	static int color = GetColor(0, 0, 0);

	if (Key[KEY_INPUT_DOWN] == 1 || Key[KEY_INPUT_RIGHT] == 1)//下か右を押されたら
	{
		sound_se(7);
		select_num = (select_num + 1) % all_number;	//選択を一つ下へ
	}

	else if (Key[KEY_INPUT_UP] == 1 || Key[KEY_INPUT_LEFT] == 1)//上か左を押されたら
	{
		sound_se(7);
		select_num = (select_num + all_number - 1) % all_number;	//選択を一つ上へ
	}

	//文字表示
	DrawFormatString(200, 200, color, "ステージ%d", select_num + 1);
	DrawString(200, 350, "決定 エンター", color);
	DrawString(200, 400, "戻る エスケープ", color);

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 70);//スケさせる
	if (select_num == 0)
	{
		DrawFormatString(230, 230, color, "ステージ%d", select_num + 2);
		DrawFormatString(230, 170, color, "ステージ%d", all_number);
	}
	else if (select_num == all_number - 1)
	{
		DrawFormatString(230, 230, color, "ステージ%d", 1);
		DrawFormatString(230, 170, color, "ステージ%d", select_num);
	}
	else
	{
		DrawFormatString(230, 230, color, "ステージ%d", select_num + 2);
		DrawFormatString(230, 170, color, "ステージ%d", select_num);

	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	if (Key[KEY_INPUT_RETURN] == 1)//エンターを押されたら
	{
		sound_se(0);//se鳴らす
		naw_mode = 2;	//モードをゲームモードにする
		naw_number = select_num;//選択されたステージ数を現在のステージに入れる
	}

	if (Key[KEY_INPUT_ESCAPE] == 1)//エスケープを押されたら
	{
		naw_mode = 0;	//モードをタイトルモードにする
	}
}

/////ポーズ画面///////
void pose(int pose_menu_g)
{
	static int select_num = 0;///何番目を選んでいるか
	int y[2] = { 200,250 };
	static int color[2] = { GetColor(255,255,255),GetColor(255,0,255) };

	if (Key[KEY_INPUT_DOWN] == 1 || Key[KEY_INPUT_RIGHT] == 1)//下か右を押されたら
	{
		sound_se(7);
		select_num = (select_num + 1) % 2;	//選択を一つ下へ
	}

	else if (Key[KEY_INPUT_UP] == 1 || Key[KEY_INPUT_LEFT] == 1)//上か左を押されたら
	{
		sound_se(7);
		select_num = (select_num + 2 - 1) % 2;	//選択を一つ上へ
	}

	DrawGraph(280, 160, pose_menu_g, true);
	DrawString(350, y[0], "ゲームへ戻る", color[0]);
	DrawString(350, y[1], "ステージ選択へ戻る", color[0]);
	DrawString(300, y[select_num], "→", color[1]);
	if (Key[KEY_INPUT_RETURN] == 1)
	{
		sound_se(0);
		switch (select_num)
		{
			////ゲームへ戻る////
		case 0:
			naw_mode = 2;
			break;

			///ステージ選択に戻る

		case 1:
			ini();
			naw_mode = 1;
			break;
		}

	}
}

////ゲームオーバー///
void game_over(int gameover_g)
{
	static int select_num;//何番目を選んでいるか
	static int color[2] = { GetColor(0,50,255),GetColor(255,0,255) };
	int y[4] = { 300,350,400,450 };

	if (Key[KEY_INPUT_DOWN] == 1 || Key[KEY_INPUT_RIGHT] == 1)//下か右を押されたら
	{
		sound_se(7);
		select_num = ((select_num + 1) % 4);	//選択を一つ下へ
	}

	else if (Key[KEY_INPUT_UP] == 1 || Key[KEY_INPUT_LEFT] == 1)//上か左を押されたら
	{
		sound_se(7);
		select_num = ((select_num + 4 - 1) % 4);	//選択を一つ上へ
	}

	DrawGraph(150, 100, gameover_g, TRUE);//画像表示
	DrawString(350, y[0], "リトライ", color[0]);
	DrawString(350, y[1], "ステージセレクト", color[0]);
	DrawString(350, y[2], "タイトル", color[0]);
	DrawString(350, y[3], "ゲームエンド", color[0]);
	DrawString(300, y[select_num], "→", color[1]);

	//エンターが押されたら///
	if (Key[KEY_INPUT_RETURN] == 1)
	{
		sound_se(0);//se鳴らす
		ini();//初期化
		switch (select_num)
		{
		case 0:
			naw_mode = 2;
			break;

		case 1:	naw_mode = 1;
			break;

		case 2:	naw_mode = 0;
			break;

		case 3:	naw_mode = 10;
			break;

		}
	}
}

///ゲームクリア////
void game_clear(int cler_g)
{
	static int select_num;//何番目を選んでいるか
	static int color[2] = { GetColor(0,50,255),GetColor(255,0,255) };
	int y[3] = { 350,400,450 };

	if (Key[KEY_INPUT_DOWN] == 1 || Key[KEY_INPUT_RIGHT] == 1)//下か右を押されたら
	{
		sound_se(7);
		select_num = ((select_num + 1) % 3);	//選択を一つ下へ
	}

	else if (Key[KEY_INPUT_UP] == 1 || Key[KEY_INPUT_LEFT] == 1)//上か左を押されたら
	{
		sound_se(7);
		select_num = ((select_num + 3 - 1) % 3);	//選択を一つ上へ
	}

	DrawGraph(150, 100, cler_g, TRUE);//画像表示
	DrawString(350, y[0], "ステージセレクト", color[0]);
	DrawString(350, y[1], "タイトル", color[0]);
	DrawString(350, y[2], "ゲームエンド", color[0]);
	DrawString(300, y[select_num], "→", color[1]);

	//エンターが押されたら///
	if (Key[KEY_INPUT_RETURN] == 1)
	{
		sound_se(0);//se鳴らす
		ini();//初期化
		switch (select_num)
		{
		case 0:	naw_mode = 1;
			break;

		case 1:	naw_mode = 0;
			break;

		case 2:	naw_mode = 10;
			break;

		}
	}
}

////クリア判定//////
void game_clear_Judg()
{
	int count = 0;
	static int count_g = 255, stop_count = 0;
	if (!game_clear_flag)
	{
		for (int i = 0; i<MAX_ENE; i++)
		{
			if (!ene[i].deth_flag && ene[i].spawn_count < gamecount)
			{
				count++;
			}
			else
			{
				count = 0;
				break;
			}

			if (count >= MAX_ENE - 1)
			{
				count = 0;
				game_clear_flag = true;
			}
		}
	}
	if (stop_count <= 255 && game_clear_flag)
		stop_count += 2;

	if (game_clear_flag && stop_count >= 255)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, count_g);//段々スケさせる
		count_g -= 2;

		//スケさせ終わったら
		if (count_g <= 0)
		{
			sound_se(1);
			naw_mode = 4;
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}
}


//キャラクタ移動関数
void move()  //移動関数
{
	if (ken.deth_flag == false && !ch.damege) {	//剣を振ってなくてダメージを受けてなければ
		if (!Key[KEY_INPUT_LSHIFT])
		{
			if (Key[KEY_INPUT_UP]) { ch.y -= 3;	ch.muki_num = 9; }//上
			if (Key[KEY_INPUT_DOWN]) { ch.y += 3;	ch.muki_num = 0; }//下
			if (Key[KEY_INPUT_LEFT]) { ch.x -= 3; ch.muki_num = 3; }//左
			if (Key[KEY_INPUT_RIGHT]) { ch.x += 3; ch.muki_num = 6; }//右
		}
		else
		{
			if (Key[KEY_INPUT_UP]) { ch.y -= 3; }//上
			if (Key[KEY_INPUT_DOWN]) { ch.y += 3; }//下
			if (Key[KEY_INPUT_LEFT]) { ch.x -= 3; }//左
			if (Key[KEY_INPUT_RIGHT]) { ch.x += 3; }//右
		}


		//移動制限
		if (ch.x <= 16)	ch.x = 16;
		if (ch.x >= SCREEN_X - 16)	ch.x = SCREEN_X - 16;
		if (ch.y <= 16)	ch.y = 16;
		if (ch.y >= SCREEN_Y - 16)	ch.y = SCREEN_Y - 16;
	}
}

//歩行モーション関数
void ch_move_motion()
{
	if (muki != ch.muki_num) {
		muki = ch.muki_num;
		image_nam = ch.muki_num, ch.motion_num = 1;
	}//向きとフラグモーションが違ったら入れ込む


	if (time <= 6) {  //timeが十以下だったらtimeを増やし終わる画像が変わる間隔
		time++;
	}
	else if (ch.motion_num == 1) {  //timeが10以上でモーションフラグが1だったらimage_namを増やしてtimeを0にする
		image_nam++;
		time = 0;
		if (image_nam == 2 || image_nam == 5 || image_nam == 8 || image_nam == 11)
		{
			ch.motion_num = 0;
		} //iが2　5　8　11　だったらモーションフラグを0にする
	}
	else if (ch.motion_num == 0) {//timeが10以上でモーションフラグが0だったらimage_namを減らしtimeを0にする
		image_nam--;
		time = 0;
		if (image_nam % 3 == 0)
		{
			ch.motion_num = 1;
		}//iが0　3　6　9　だったらモーションフラグを1にする
	}
}

//敵歩行モーション
void ene_move_motion()
{
	for (int i = 0; i<MAX_ENE; i++)
	{
		if (ene[i].naw_muki != ene[i].muki_flag)		//現在の向きと次の向きが違ったら
		{
			ene[i].naw_muki = ene[i].muki_flag;		//現在の向きに次の向きを入れる
			ene[i].g_namu = ene[i].naw_muki;
			ene[i].muve_flag = true;
		}

		if (ene[i].time <= 8)
		{
			ene[i].time++;
		}
		else if (ene[i].muve_flag == true)//trueの場合画像ナンバーを増やす
		{
			ene[i].g_namu++;
			ene[i].time = 0;
			if (ene[i].g_namu == 2 || ene[i].g_namu == 5 || ene[i].g_namu == 8 || ene[i].g_namu == 11)
			{
				ene[i].muve_flag = false;
			} //g_namuが2　5　8　11　だったらモーションフラグをfalseにする
		}
		else if (ene[i].muve_flag == false) {//timeが10以上でモーションフラグが0だったらimage_namを減らしtimeを0にする

			ene[i].g_namu--;
			ene[i].time = 0;
			if (ene[i].g_namu % 3 == 0)
			{
				ene[i].muve_flag = true;
			}//iが0　3　6　9　だったらモーションフラグを1にする
		}
	}
}

//当たり判定計算   当たったらture　外れてたらfalse を返す
bool decision(double x1, double y1, int renge1, double x2, double y2, int renge2)
{
	int x, y, renge;
	x = abs(int(x1 - x2));
	y = abs(int(y1 - y2));
	renge = renge1 + renge2;

	if (x*x + y * y < renge*renge)//計算
	{
		return true;
	}
	else
	{
		return false;
	}

}

//敵出現関数
void ene_spawn()
{
	for (int i = 0; i < MAX_ENE; i++)
	{
		if (!ene[i].deth_flag && gamecount == ene[i].spawn_count)//生存フラグが立っていなければ
		{
			ene[i].deth_flag = true;
		}
	}
}

//敵移動関数
void ene_move()
{
	double xsa, ysa, xabs, yabs;
	for (int i = 0; i < MAX_ENE; i++)
	{
		ene[i].xtemp = ene[i].x;
		ene[i].ytemp = ene[i].y;


		//タワー最短移動

		//タワーに当たるまで
		if (ene[i].deth_flag && ene[i].damege_type <= 0)
		{

			ene[i].angle = atan2(tower.y - ene[i].y, tower.x - ene[i].x);//角度を求める
			ene[i].xmove = cos(ene[i].angle)*enemovespeed[ene[i].pattern];
			ene[i].ymove = sin(ene[i].angle)*enemovespeed[ene[i].pattern];
			//爆遠近でわける

			switch (ene[i].pattern)
			{
				//爆発
			case 0:
				if (decision(tower.x, tower.y, tower.renge, ene[i].x, ene[i].y, ene[i].renge))//当たったら移動しない爆パターン
				{

					ene[i].xmove = 0;
					ene[i].ymove = 0;
					ene[i].attack = true;
				}
				else
					ene[i].attack = false;

				break;
				//遠距離
			case 1:
				if (decision(tower.x, tower.y, tower.renge + 100, ene[i].x, ene[i].y, ene[i].renge))//当たったら移動しない遠パターン
				{
					ene[i].xmove = 0;
					ene[i].ymove = 0;
					ene[i].attack = true;
				}
				else
					ene[i].attack = false;

				break;
				//近距離
			case 2:
				if (decision(tower.x, tower.y, tower.renge, ene[i].x, ene[i].y, ene[i].renge))//当たったら攻撃近パターン
				{
					ene[i].attack = true;
					ene[i].gamecount_temp = gamecount;
				}
				break;


			}

			if (!ene[i].attack)
			{
				/////////特殊攻撃その２///////////
				if (special.attack_flag && special.get_flag == 4)
				{
					ene[i].x = ene[i].x + ene[i].xmove / 3;
					ene[i].y = ene[i].y + (ene[i].ymove / 3.0);
				}

				else
				{
					ene[i].x += ene[i].xmove;
					ene[i].y += ene[i].ymove;
				}
			}
		}

		//どの方向を向くか
		if (ene[i].damege_type <= 0)
		{
			xsa = ene[i].x - ene[i].xtemp;
			ysa = ene[i].y - ene[i].ytemp;
			xabs = abs(int(xsa));
			yabs = abs(int(ysa));

			if (xabs >= yabs && xabs > 0)
			{
				if (xsa > 0)
				{
					ene[i].muki_flag = 6;
				}
				else
				{
					ene[i].muki_flag = 3;
				}
			}
			else if (xabs <= yabs && yabs > 0)
			{
				if (ysa > 0)
				{
					ene[i].muki_flag = 0;
				}
				else
				{
					ene[i].muki_flag = 9;
				}
			}

		}
	}

}

//敵弾攻撃関数　　１つだけ設定して戻る
void ene_shot_set(int i)
{
	for (int s = 0; s < MAX_ENE_SHOT; s++)
	{
		//敵弾の生存フラグが立ってなかったら１つだけ立てる
		if (!ene_shot[s].deth_flag && ene[i].attack)
		{
			ene_shot[s].deth_flag = true;
			ene_shot[s].attack_taype = ene[i].pattern;//遠近どちらの弾かを判別させるため
			ene_shot[s].x = ene[i].x;
			ene_shot[s].y = ene[i].y;
			ene_shot[s].angle = atan2(tower.y - ene[i].y, tower.x - ene[i].x);//角度を求める
			ene_shot[s].xmove = cos(ene_shot[s].angle)*ene_shot[s].movespeed;
			ene_shot[s].ymove = sin(ene_shot[s].angle)*ene_shot[s].movespeed;
			break;
		}
	}
}

//敵弾の移動
void ene_shot_move()
{
	for (int s = 0; s < MAX_ENE_SHOT; s++)
	{
		if (ene_shot[s].deth_flag)
		{
			ene_shot[s].x += ene_shot[s].xmove;
			ene_shot[s].y += ene_shot[s].ymove;
		}
	}
}

//敵攻撃関数
void enemyattack()
{
	for (int i = 0; i < MAX_ENE; i++)
	{
		if (ene[i].deth_flag && ene[i].attack)
		{
			switch (ene[i].pattern)
			{
				//爆発
			case 0:
				ene[i].baku = true;
				ene[i].deth_flag = false;
				ene[i].attack = false;
				tower.hp -= 100;
				break;

				//遠距離
			case 1:
				if (ene[i].attack_count <= 0)
				{
					ene_shot_set(i);//弾を一つだけ設定する

					ene[i].attack_count = 150;	//攻撃間隔カウンタ再設定
				}
				else
					ene[i].attack_count--;
				break;

				//近距離
			case 2:


				if (ene[i].gamecount_temp + 50 >= gamecount)
				{
					ene[i].x -= ene[i].xmove*2.2;
					ene[i].y -= ene[i].ymove*2.4;
				}
				else if (ene[i].attack_count <= 0)
				{
					ene[i].attack_count = 100;
					ene[i].xmove = 0;
					ene[i].ymove = 0;
					ene[i].attack = false;
				}
				else
				{
					ene[i].attack_count--;
				}
				break;
			}
		}
	}
	ene_shot_move();
}

//プレイヤーノックバック状態
void chstan()
{
	if (ch.damege)
	{
		//向いている方向と逆方向に一定量下がった後少しの間スタン

		int xx, yy;
		xx = abs(int(ch.x - ch.xtemp));
		yy = abs(int(ch.y - ch.ytemp));
		if (xx < ch.stan_meter && yy == 0 || yy < ch.stan_meter && xx == 0)
		{
			switch (ch.muki_num)
			{
			case 0://正面
				ch.y -= 5;
				break;

			case 9://後ろ
				ch.y += 5;
				break;

			case 3://左
				ch.x += 5;
				break;

			case 6://右
				ch.x -= 5;
				break;

			}
		}
		else
			ch.damege = false;

	}
}

//敵ノックパック＆スタン
void enestan(int i)
{
	switch (ene[i].damege_type)
	{
		//遠距離攻撃の場合一定時間停止
	case 1:
		if (ene[i].damagecount <= 30)
		{
			ene[i].damagecount++;
		}
		else
		{
			ene[i].damagecount = 0;
			ene[i].damege_type = 0;
		}
		break;

		//近距離攻撃の場合一定距離下がらせる
	case 2:
		switch (ene[i].muki_flag)
		{
		case 0://正面向き
			ene[i].y += 8;
			break;

		case 3://左
			ene[i].x -= 8;
			break;

		case 6: //右
			ene[i].x += 8;
			break;

		case 9://後ろ
			ene[i].y -= 8;
			break;
		}

		if (abs(int(ene[i].xtemp2 - ene[i].x)) >= 50 || abs(int(ene[i].y - ene[i].ytemp2)) >= 50)
			ene[i].damege_type = 0;
		//break;


	}
}

//アイテム情報セット
void item_set(int ene_num)
{
	for (int i = 0; i < MAX_ITEM; i++)
	{
		if (!item[i].deth_flag)
		{
			item[i].deth_flag = true;
			item[i].x = ene[ene_num].x;
			item[i].y = ene[ene_num].y;
			item[i].item_num = ene[ene_num].item_num;
			item[i].plus_num = ene[ene_num].plus_num;//アイテムの増加量
			item[i].count = 510;
			break;
		}
	}
}

//敵爆発
void enebang()
{

	for (int i = 0; i < MAX_ENE; i++)
	{
		if (ene[i].deth_flag == false && ene[i].baku && ene[i].bakucount >= 6)
		{
			ene[i].bakuimage_num++;
			ene[i].bakucount = 0;
		}

		if (ene[i].bakucount < 6)
		{
			ene[i].bakucount++;
		}
		if (ene[i].bakuimage_num > 5)
		{
			ene[i].bakuimage_num = 0;
			ene[i].baku = false;
			ene[i].attack = false;
		}
	}

	for (int i = 0; i< MAX_ENE_SHOT; i++)
	{
		if (!ene_shot[i].deth_flag && ene_shot[i].baku && ene_shot[i].bakucount >= 6)
		{
			ene_shot[i].bakuimage_num++;
			ene_shot[i].bakucount = 0;
		}
		if (ene_shot[i].bakucount < 6)
		{
			ene_shot[i].bakucount++;
		}
		if (ene_shot[i].bakuimage_num > 5)
		{
			ene_shot[i].bakuimage_num = 0;
			ene_shot[i].baku = false;
		}
	}
}


//ダメージや当たり判定処理
void damage_check()
{
	//敵の数だけループ
	for (int i = 0; i < MAX_ENE; i++)
	{
		if (ene[i].deth_flag == true)//生きている敵だけ判定する
		{

			//敵と塔の判定//自爆攻撃ではない敵がとうに当たった時はこちらの処理
			if (decision(tower.x, tower.y, tower.renge, ene[i].x, ene[i].y, ene[i].renge) && ene[i].pattern > 0)
			{
				ene[i].baku = true;

				switch (ene[i].pattern)
				{
				case 1://遠距離は爆発
					sound_se(3);
					ene[i].deth_flag = false;
					tower.hp -= 10;
					break;

				case 2://近距離はdamageを与える
					sound_se(6);
					tower.hp -= 50;
					break;
				}
			}

			//敵と自機の判定
			if (decision(ch.x, ch.y, ch.renge, ene[i].x, ene[i].y, ene[i].renge) && !ch.damege)
			{
				ch.xtemp = ch.x;
				ch.ytemp = ch.y;
				ch.damege = true;
			}


			//敵と斬撃の判定
			if (decision(ken.x, ken.y, ken.renge, ene[i].x, ene[i].y, ene[i].renge) == true && ken.deth_flag && ene[i].damege_type == 0)
			{
				ene[i].damege_type = 2;
				ene[i].muki_flag = ch.muki_num;
				ene[i].xtemp2 = ene[i].x;//喰らった時の座標保存
				ene[i].ytemp2 = ene[i].y;
				ene[i].hp -= ch.attackpoint_ken*damage_rate[ch.ken_lev - 1];
			}


			//敵と矢の判定//////////////
			for (int j = 0; j < MAX_SHOT; j++)//矢の数回す
			{
				if (decision(tama[j].x, tama[j].y, tama[j].renge, ene[i].x, ene[i].y, ene[i].renge) == true && tama[j].deth_flag == true)
				{
					////////特殊攻撃１（貫通弾）の処理入れ込み
					if (!special.attack_flag || special.get_flag != 3)
					{	////specialの攻撃が発動してないなら
						tama[j].deth_flag = false;
					}

					if (tama[j].renge == 8)
					{
						ene[i].damege_type = 1; //遠距離喰らった
						ene[i].hp -= ch.attackpoint_shot*damage_rate[ch.shot_lev - 1];
					}
				}
			}

			//敵スタン＆ノックバック処理
			enestan(i);

			if (ene[i].hp <= 0.0)//HPがなくなったら
			{
				if (ene[i].item_num > 0)
					item_set(i);

				sound_se(3);
				ene[i].baku = true;
				ene[i].deth_flag = false;
				ene[i].hp = 100;

			}
		}

		//////////爆発処理
		if (!ene[i].deth_flag && ene[i].baku)
		{
			enebang();
		}

	}

	//敵の弾接触判定
	for (int i = 0; i < MAX_ENE_SHOT; i++)
	{
		if (ene_shot[i].deth_flag)
		{
			//塔と敵弾の判定
			if (decision(tower.x, tower.y, tower.renge, ene_shot[i].x, ene_shot[i].y, ene_shot[i].renge))
			{
				ene_shot[i].deth_flag = false;
				ene_shot[i].baku = true;

				switch (ene_shot[i].attack_taype)
				{
					//遠距離
				case 1:
					tower.hp -= 30;
					break;
				}
			}

			//自機と敵弾の判定
			if (decision(ch.x, ch.y, ch.renge, ene_shot[i].x, ene_shot[i].y, ene_shot[i].renge))
			{
				ene_shot[i].baku = true;
				ene_shot[i].deth_flag = false;
				ch.xtemp = ch.x;
				ch.ytemp = ch.y;
				ch.damege = true;
			}
		}
		if (!ene_shot[i].deth_flag && ene_shot[i].baku)
		{
			enebang();
		}
	}

	//アイテムの接触判定,消滅判定
	for (int i = 0; i < MAX_ITEM; i++)
	{
		//////アイテムをタワー方向に移動させる
		item[i].angle = atan2(tower.y - item[i].y, tower.x - item[i].x);
		item[i].xmove = cos(item[i].angle)*0.3;
		item[i].ymove = sin(item[i].angle)*0.3;

		item[i].x += item[i].xmove;
		item[i].y += item[i].ymove;

		if (item[i].deth_flag)
		{
			if (decision(ch.x, ch.y, ch.renge, item[i].x, item[i].y, item[i].renge))
			{
				item[i].deth_flag = false;
				switch (item[i].item_num)
				{
				case 1:

					//６以下で十個とったならレベルアップ
					if (ch.shot_lev < 6)
					{
						ch.shot_lev_count += item[i].plus_num;
						if (ch.shot_lev_count >= ch.lev_up_grade)
						{
							ch.shot_lev++;
							ch.shot_lev_count = ch.shot_lev_count % ch.lev_up_grade;
						}
					}
					break;

				case 2:
					if (ch.ken_lev < 6)
					{
						ch.ken_lev_count += item[i].plus_num;
						if (ch.ken_lev_count >= ch.lev_up_grade)
						{
							ch.ken_lev++;
							ch.ken_lev_count = ch.shot_lev_count % ch.lev_up_grade;
						}
					}
					break;

					///////特殊攻撃その１/////
				case 3:
					if (special.deth_flag && special.level >= ch.lev_up_grade)//レベルを上げる
					{
						special.level += item[i].plus_num;
					}
					special.deth_flag = true;				//フラグを立てる
					special.gicon_num = special.get_flag = item[i].item_num;	//どのタイプか

					special.effect_maxcount = 300;	//効果時間設定
					special.cool_maxcount = 500;//クールタイム設定種類ごとに変えるかも
					special.coolcount = 0;

					break;

					////特殊攻撃その２////
				case 4:
					if (special.deth_flag && special.level >= ch.lev_up_grade)//レベルを上げる
					{
						special.level += item[i].plus_num;
					}
					special.deth_flag = true;				//フラグを立てる
					special.gicon_num = special.get_flag = item[i].item_num;	//どのタイプか

					special.effect_maxcount = 200;//最大効果時間
					special.cool_maxcount = 600;//最大クールタイム
					special.coolcount = 0;
					break;



				default:
					special.deth_flag = true;
					special.gicon_num = item[i].item_num;
					break;
				}
			}

			if (item[i].count <= 0)
			{
				item[i].deth_flag = false;
			}
			else
			{
				item[i].count--;
			}
		}
	}

	if (tower.hp <= 0)
	{
		tower.deth_flag = false;
	}
}


//キャラクタショット攻撃
void c_shot()
{
	if (ch.shot_count == 0)   //Zが押されれば弾を向いている方向に打つ
	{


		if (Key[KEY_INPUT_Z] >= 1 && Key[KEY_INPUT_X] <= 0)
		{
			sound_se(4);
			for (int i = 0; i<MAX_SHOT; i++)
			{
				if (tama[i].deth_flag == false)//弾のフラグが立っていなければ
				{
					tama[i].deth_flag = true; //弾の生存フラグを立てる
					ch.shot_count = 8;  //shotの間隔カウンタを設定

					switch (ch.muki_num)
					{
					case 0: //正面向きの時
						tama[i].y = ch.y + 20;
						tama[i].x = ch.x;
						tama[i].g_angle = PI;
						tama[i].muki_flag = ch.muki_num;//向きを保存
						tama[i].kyori_meter = 0;  //kyori_meterカウント開始
						break;


					case 3: //左向きの時
						tama[i].y = ch.y;
						tama[i].x = ch.x - 20;
						tama[i].g_angle = PI / 2 + PI;
						tama[i].muki_flag = ch.muki_num;//向きを保存
						tama[i].kyori_meter = 0;  //kyori_meterカウント開始
						break;


					case 6: //右向きの時
						tama[i].y = ch.y;
						tama[i].x = ch.x + 16;
						tama[i].g_angle = PI / 2;
						tama[i].muki_flag = ch.muki_num;//向きを保存
						tama[i].kyori_meter = 0;  //kyori_meterカウント開始
						break;


					case 9: //後ろ向きの時
						tama[i].y = ch.y - 23;
						tama[i].x = ch.x;
						tama[i].g_angle = 0;
						tama[i].muki_flag = ch.muki_num;//向きを保存
						tama[i].kyori_meter = 0;  //kyori_meterカウント開始
						break;

					} //switch
					break;
				} //if tama deth_flag

			} //for
		}//if key Z
	}// if ch.shot_count

	for (int i = 0; i<MAX_SHOT; i++)
	{
		if (tama[i].deth_flag == true)//弾の生存フラグが立っていれば
		{
			tama[i].kyori_meter += 8;//距離制限meterを増やす

			switch (tama[i].muki_flag)//弾の向きを判定して進める方向を決める
			{
			case 0: //正面
				tama[i].y += 8;
				break;

			case 3: //左
				tama[i].x -= 8;
				break;

			case 6: //右
				tama[i].x += 8;
				break;

			case 9: //後ろ
				tama[i].y -= 8;
				break;
			}//switch
		}//if
	}//for

	for (int i = 0; i<MAX_SHOT; i++)
	{
		//距離制限メータと距離制限フラグの値が同じになったら
		if (tama[i].kyori_meter >= ch.tama_kyori_flag)
		{
			tama[i].deth_flag = false;//生存フラグを折る
			tama[i].kyori_meter = 0;//距離制限メーターを初期化
		}//if
	}//for

	 //キャラのショット間隔カウンタが0より大きかったら
	if (ch.shot_count>0)
	{
		ch.shot_count--;//値を引く
	}


}

//キャラクタ斬撃攻撃関数
void c_zangeki()
{

	//画像差し替え間隔カウンタを減少させる０になったら画像をずらす
	if (ken.kaunt > 0 && ken.deth_flag >= false)
	{
		ken.kaunt--;
	}

	if (Key[KEY_INPUT_X] == 1 && ken.deth_flag <= false)
	{
		sound_se(5);
		ken.deth_flag = true;   //剣生存フラグオン
		ken.kaunt = KEN_MAX_COUNT;      //画像差し替え感覚カウンタ設定

		switch (ch.muki_num) ///キャラの向きで判定
		{

		case 0:

			ken.x = ch.x;
			ken.y = ch.y + 20;
			ken.g_angle = PI;

			break;

		case 3:
			ken.x = ch.x - 20;
			ken.y = ch.y;
			ken.g_angle = PI + PI / 2;

			break;

		case 6:

			ken.x = ch.x + 16;
			ken.y = ch.y;
			ken.g_angle = PI / 2;

			break;


		case 9:
			ken.x = ch.x;
			ken.y = ch.y - 23;
			ken.g_angle = 0;

			break;


		}
	}

	if (ken.kaunt <= 0 && ken.deth_flag == true)//間隔が０になったら画像差し替え
	{
		ken.g_num = ++ken.g_num % 4;
	}


}

//キャラクタ特殊攻撃
void c_special()
{
	if (special.deth_flag && Key[KEY_INPUT_C] == 1 && special.coolcount <= 0)
	{
		special.attack_flag = true;

		////////効果時間クールタイム設定/////////]
		//効果時間設定
		special.effectcount = special.effect_maxcount;
		special.coolcount = special.cool_maxcount;		//クールタイム設定
	}

	if (special.coolcount >= 0)
	{
		special.coolcount--;
	}

	if (special.attack_flag)
	{
		switch (special.get_flag)
		{

			////////特殊攻撃1　一定時間遠距離貫通//////////
		case 3:
			if (special.effectcount > 0)
			{
				special.effectcount--;		//特殊攻撃効果時間を減らす
			}
			else
			{
				special.attack_flag = false;		//効果時間が終わったら攻撃フラグを折る
			}
			break;

		case 4:
			if (special.effectcount > 0)
			{
				special.effectcount--;		//特殊攻撃効果時間を減らす
			}
			else
			{
				special.attack_flag = false;		//効果時間が終わったら攻撃フラグを折る
			}
			break;
		}
	}
}

//自機キャラクタ攻撃関数
void c_attack()
{

	c_shot();

	c_zangeki();

	c_special();

}

//画像表示関数
void graph(int preyr[], int shot, int zan[], int enemy[][12], int g_tower, int icon_gui[], int baku[], int eneshot, int item_gra[], int back_g[])//画像表示関数
{
	static int co = GetColor(255, 0, 0);
	DrawGraph(0, 0, back_g[naw_number % 3], false);//背景

	if (tower.deth_flag) {

		DrawGraph(tower.x - 16, tower.y - 16, g_tower, TRUE);//塔画像表示
	}

	DrawGraph(ch.x - 16, ch.y - 16, preyr[image_nam], TRUE);//プレイヤー画像表示

															//弾
	for (int i = 0; i<MAX_SHOT; i++)
	{
		if (tama[i].deth_flag == true)//弾の生存フラグが立っていれば
		{
			DrawRotaGraph2(tama[i].x, tama[i].y, 8, 8, 1.0f, tama[i].g_angle, shot, TRUE, FALSE);
		}
	}

	//剣
	if (ken.deth_flag == true)  //剣の生存フラグが立っていれば
	{
		DrawRotaGraph2(ken.x, ken.y, 8, 8, 1.0f, ken.g_angle, zan[ken.g_num], TRUE, FALSE);
		if (ken.g_num >= 3) {
			ken.deth_flag = false;  //剣の生存フラグを折る
			ken.g_num = 0;    //画像ナンバーも初期化
		}
	}


	//敵
	for (int i = 0; i< MAX_ENE; i++)
	{
		if (ene[i].deth_flag == true)
		{
			double eneper;
			eneper = (ene[i].hp / ene[i].hp_max);
			DrawGraph(ene[i].x - 16, ene[i].y - 16, enemy[ene[i].pattern][ene[i].g_namu], TRUE);//敵表示
			if (ene[i].hp <= 100)
				DrawBox(ene[i].x - 16, ene[i].y + 20, (ene[i].x - 16) + (32 * eneper), ene[i].y + 25, GetColor(255, 0, 0), true);//HP表示
			else
				DrawBox(ene[i].x - 16, ene[i].y + 20, (ene[i].x - 16) + (32 * eneper), ene[i].y + 25, GetColor(255, 255, 0), true);//HP表示
		}
		else if (ene[i].deth_flag == false && ene[i].baku == true)
		{
			DrawGraph(ene[i].x - 16, ene[i].y - 16, baku[ene[i].bakuimage_num], TRUE);//爆発
		}
	}
	//敵弾
	for (int i = 0; i < MAX_ENE_SHOT; i++)
	{
		if (ene_shot[i].deth_flag)
		{
			DrawGraph(ene_shot[i].x - 8, ene_shot[i].y - 8, eneshot, TRUE);
		}
		else if (!ene_shot[i].deth_flag && ene_shot[i].baku)
		{
			DrawGraph(ene_shot[i].x - 8, ene_shot[i].y - 8, baku[ene_shot[i].bakuimage_num], TRUE);//爆発
		}
	}


	//アイテム
	for (int i = 0; i < MAX_ITEM; i++)
	{
		if (item[i].deth_flag)
		{
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, item[i].count / 2);//段々スケさせる
			DrawGraph(item[i].x - 8, item[i].y - 8, item_gra[item[i].item_num - 1], TRUE);//画像表示
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}


	//GUI
	//DrawBox(0,0,SCREEN_X,80,GetColor(0,0,255),true);
	DrawRotaGraph(60, 30, 2, 0, icon_gui[0], true, false);//遠距離アイコン
	DrawRotaGraph(140, 30, 2.0, 0, icon_gui[1], true, false);//近距離アイコン
	if (special.deth_flag)
	{
		DrawRotaGraph(220, 30, 2.0, 0, icon_gui[special.gicon_num - 1], true, false);//特殊アイコン

																					 //////特殊クール時間/////
		double spe_per;//パーセント
		spe_per = (double)special.coolcount / (double)special.cool_maxcount;
		DrawBox(204, 50, 204 + (24 * spe_per), 60, GetColor(0, 0, 255), true);
		DrawBox(204, 50, 234, 60, GetColor(255, 255, 0), false);

		if (special.attack_flag)
		{////////効果時間のゲージ表示
			spe_per = ((double)special.effectcount / (double)special.effect_maxcount);
			DrawBox(ch.x - 16, ch.y - 23, ch.x - 16 + (30 * spe_per), ch.y - 18, GetColor(255, 0, 255), true);
		}
	}

	DrawRotaGraph(100, 30, 2.0, 0, icon_gui[ch.shot_lev + 5], true, false);//遠距離レベル
	DrawBox(45, 50, 45 + (3 * ch.shot_lev_count), 60, GetColor(0, 0, 255), true);//レベルアップメーター
	DrawBox(45, 50, 75, 60, GetColor(255, 255, 0), false);
	DrawRotaGraph(180, 30, 2.0, 0, icon_gui[ch.ken_lev + 5], true, false);//近距離レベル
	DrawBox(125, 50, 125 + (3 * ch.ken_lev_count), 60, GetColor(0, 0, 255), true);//レベルアップメーター
	DrawBox(125, 50, 155, 60, GetColor(255, 255, 0), false);

	double per;
	per = (tower.hp / tower.hp_max);
	if (tower.deth_flag)
	{
		DrawBox(345, 20, 345 + 250 * per, 40, GetColor(124, 252, 0), true);//タワーHP	laengreen
		per = per * 100;
		DrawFormatString(370, 23, GetColor(255, 0, 0), "%.1lf %%", per);
	}
	DrawString(310, 23, "HP", GetColor(165, 42, 42));
	DrawBox(345, 20, 346 + 250, 40, GetColor(255, 165, 0), false);//タワーHP枠	orange

}


//ステージ数、読み込みファイル名の読み込み
void select_faile_open()
{
	static int j;
	FILE *fp_stage;

	if (!all_stage_num)	//全ステージ数と読み込みファイル名が読み込まれていなければ
	{
		///////////キャラ攻撃力/////////////////
		fp_stage = fopen("stage_data/char_data.csv", "r");
		if (fp_stage == NULL)
			DxLib_End();

		fscanf(fp_stage, "%d,%d,%d", &ch.attackpoint_shot, &ch.attackpoint_ken, &ch.stan_meter);

		fclose(fp_stage);

		//ステージデータ読み込み
		fp_stage = fopen("stage_data/AllStageNum.csv", "r");
		if (fp_stage == NULL)//読み込めているか確認失敗してたら終了
			DxLib_End();

		fscanf(fp_stage, "%d\n", &all_number);		//ステージ数読み込み

		do {
			//読み込みファイル名タワー情報読み込み座標体力
			fscanf(fp_stage, "%s[,]\n", &open_stage_name[j]);
			j++;
		} while (j <= all_number);

		for (int i = 0; i<20; i++)
		{
			memcpy(open_stage_name_temp[i], open_stage_name[i], sizeof(char) * 30);
		}
		all_stage_num = true;

		fclose(fp_stage);
	}
}



//file読み込み//
void open()
{
	static int i;		//何個めま敵出現パターンで読んだかのカウンタ,現在ステージ数
	char s[255];
	int gomi;
	FILE *fp;

	///////ステージデータ読み込み
	fp = fopen(open_stage_name_temp[naw_number], "r");
	//コメント飛ばし
	fgets(s, 200, fp);

	//タワー状態読み込みプラスキャラ座標
	fscanf(fp, "\n%lf,%lf,%lf,%lf,%lf,%d,%d\n", &tower.x, &tower.y, &tower.hp_max, &ch.x, &ch.y, &gomi, &ch.lev_up_grade);
	tower.hp = tower.hp_max;
	//コメント飛ばし
	fgets(s, 200, fp);

	//敵行動パターン読み込み設定
	while (1)
	{
		//全部テンプに入れる
		fscanf(fp, "%lf,%lf,%d,%d,%d,%lf,%d\n", &ene_tmp[set_ene_num].x, &ene_tmp[set_ene_num].y, &ene_tmp[set_ene_num].pattern, &ene_tmp[set_ene_num].item_num, &ene_tmp[set_ene_num].spwan_count, &ene_tmp[set_ene_num].hp_max, &ene_tmp[set_ene_num].plus_num);
		//ene[set_ene_num].hp = ene[set_ene_num].hp_max;
		last_game_count = ene_tmp[set_ene_num].spwan_count;
		set_ene_num++;
		if (set_ene_num >= 500 || feof(fp) != 0)///最後まで行くかEOFで
		{

			ene_load_notend = true;
			break;
		}
	}
	fclose(fp);
}

//敵データ代入
void ene_assignment()
{
	static int count;
	for (int i = 0; i<MAX_ENE; i++)
	{
		if (!ene[i].deth_flag && ene[i].spawn_count < gamecount)
		{
			ene[i].x = ene_tmp[count].x;
			ene[i].y = ene_tmp[count].y;
			ene[i].hp = ene[i].hp_max = ene_tmp[count].hp_max;
			ene[i].item_num = ene_tmp[count].item_num;
			ene[i].pattern = ene_tmp[count].pattern;
			ene[i].plus_num = ene_tmp[count].plus_num;
			ene[i].spawn_count = ene_tmp[count].spwan_count;
			count++;
			if (count >= set_ene_num)
			{
				count = 0;
				set_ene_num = 0;
				ene_load_notend = false;
				break;

			}
		}
	}

}


//メイン
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpcmdLine, int nCmdShow)
{
	SetGraphMode(SCREEN_X, SCREEN_Y, 32);
	ChangeWindowMode(TRUE), DxLib_Init(), SetDrawScreen(DX_SCREEN_BACK); //ウィンドウモード変更と初期化と裏画面設定;



	ini(); //初期化

	select_faile_open();


	int chr1[12], ene0[12], ene1[12], ene2[12], tama1, ken[4], tower1, icon1[12], bang[6], ene_tama, item_icon[6], title, gameover, gameclear, back[6], menu;//画像ハンドル
	int time = 0; //画像の番号と表示の時間

	int ene3type_gra[3][12];
	title = LoadGraph("image/Title.png");///タイトル画像
	gameover = LoadGraph("image/gameover.png");//ゲームオーバー
	gameclear = LoadGraph("image/gameclear.png");//ゲームクリア
	menu = LoadGraph("image/menu.png");

	back[0] = LoadGraph("image/back1.jpg");
	back[1] = LoadGraph("image/back2.jpg");
	back[2] = LoadGraph("image/back3.jpg");
	back[3] = LoadGraph("image/back4.jpg");
	back[4] = LoadGraph("image/back5.jpg");
	back[5] = LoadGraph("image/back6.jpg");

	tama1 = LoadGraph("image/alor.png");//矢
	tower1 = LoadGraph("image/tower.png");//塔
	ene_tama = LoadGraph("image/tama_ene.png");//敵弾
	LoadDivGraph("image/chrct1.png", 12, 3, 4, 32, 32, chr1); //自機画像を分割ロード　歩行
	LoadDivGraph("image/teki0.png", 12, 3, 4, 32, 32, ene0); //敵画像を分割ロード　歩行
	LoadDivGraph("image/teki1.png", 12, 3, 4, 32, 32, ene1); //敵画像を分割ロード　歩行
	LoadDivGraph("image/teki2.png", 12, 3, 4, 32, 32, ene2); //敵画像を分割ロード　歩行
	LoadDivGraph("image/ken1.png", 4, 4, 1, 16, 16, ken);   //剣(斬撃)画像ロード
	LoadDivGraph("image/icon1_1.png", 12, 6, 2, 16, 16, icon1);	//アイコン画像分割ロード順番は遠距離　近距離　特殊１２３４の順
	LoadDivGraph("image/bang.png", 6, 6, 1, 32, 32, bang);		//爆発画像
	LoadDivGraph("image/item1.png", 6, 6, 1, 16, 16, item_icon);	//アイテム画像

																	/////SEロード//////
	se[0] = LoadSoundMem("music/sel.mp3");
	se[1] = LoadSoundMem("music/clear.mp3");
	se[2] = LoadSoundMem("music/gameover.mp3");
	se[3] = LoadSoundMem("music/bom.mp3");
	se[4] = LoadSoundMem("music/enn.mp3");
	se[5] = LoadSoundMem("music/ken.mp3");
	se[6] = LoadSoundMem("music/taiatari.mp3");
	se[7] = LoadSoundMem("music/updown.mp3");

	for (int i = 0; i < 12; i++)
	{
		ene3type_gra[0][i] = ene0[i];
		ene3type_gra[1][i] = ene1[i];
		ene3type_gra[2][i] = ene2[i];
	}
	while (ScreenFlip() == 0 && ProcessMessage() == 0
		&& ClearDrawScreen() == 0 && gpUpdateKey() == 0) {
		if (ProcessMessage() == -1) { break; } //メッセージ処理

		ClearDrawScreen();//画像を消す

		switch (naw_mode)
		{
			//タイトルメニュ
		case 0:
			DrawGraph(0, 0, back[3], false);//背景
			title_menu(title);
			break;

			//ステージ選択
		case 1:
			DrawGraph(0, 0, back[3], false);//背景
			stage_select();
			break;

			////////ゲームメイン部
		case 2:
			//敵データ読み込み
			if (gamecount == 0)
			{
				open();
			}
			///データ入れ込み
			if (ene_load_notend)
			{
				ene_assignment();
			}

			move();//動き
			ene_spawn();//敵出現
			ch_move_motion();//プレイヤーの動きモーション処理
			ene_move_motion();//敵の歩行モーション
			ene_move();//敵移動
			c_attack(); //キャラ攻撃関数へ
			enemyattack();//敵攻撃
			chstan();//キャラクタスタン
			damage_check();//ダメージ処理
			graph(chr1, tama1, ken, ene3type_gra, tower1, icon1, bang, ene_tama, item_icon, back);//画像表示
			gamecount++;//カウントを増やす

						////セットした値とゲーム内カウントを比べ超えたらクリア判定処理へ/////
			if (last_game_count < gamecount && tower.hp > 0)
			{
				game_clear_Judg();
			}

			/////ゲームオーバー判定////
			if (tower.hp <= 0)
			{
				static int over_g;
				over_g += 5;
				sound_se(2);
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, over_g);//段々スケさせる
				DrawGraph(150, 100, gameover, TRUE);//画像表示
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
				if (over_g >= 255)
				{

					over_g = 0;
					naw_mode = 3;
				}
			}

			///////////エスケープ押したらポーズ画面へ////////
			if (Key[KEY_INPUT_ESCAPE])
			{
				naw_mode = 5;
			}
			break;

			///ゲームオーバー///
		case 3:
			DrawGraph(0, 0, back[5], false);//背景
			game_over(gameover);
			break;

			//ゲームクリア//
		case 4:
			DrawGraph(0, 0, back[4], false);//背景
			game_clear(gameclear);
			break;

			////ポーズ画面/////
		case 5:
			graph(chr1, tama1, ken, ene3type_gra, tower1, icon1, bang, ene_tama, item_icon, back);//画像表示
			pose(menu);
			break;

			//////10だったら終了
		case 10:
			DxLib_End();
		}
	}

	DxLib_End();
	return 0;
}
