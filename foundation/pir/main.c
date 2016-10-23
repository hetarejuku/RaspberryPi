//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// GrovePi+の人感センサー制御プログラム
// Copyright (c) 2016 ヘタレ塾 (https://github.com/hetarejuku/)
// Released under the MIT License.
// https://opensource.org/licenses/mit-license.php
// 概要　: D2につないだ人感センサー（移動物検知）の制御プログラム
//         製品名(GrovePi+/PIR Motion Sensor) http://www.dexterindustries.com/GrovePi/
//         http://wiki.seeedstudio.com/wiki/Category:Grove
//         製品のサンプルコード(https://github.com/DexterInd/GrovePi)
// 履歴　: 2016/10/24 ヘタレ塾
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//*****インクルードファイル*****
#include "grovepi.h"

//------------------------------------------------------------------------------
// main : メイン関数
//  引数 argc : 引数の総個数
//       argv : 引数のポインター（[0]はファイル名）
//  戻り値 : EXIT_SUCCESS/EXIT_FAILURE
//  処理概要 :
//  ・GrovePi+とつながるI2Cのオープンを要求する
//  ・GrovePi+のFWバージョンを確認する
//  ・人感センサーの接続GPIOを設定する
//  ・人感センサーの検知を確認する
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    int dval;

    //GrovePi+とつながるI2Cのオープン
    if(init() == -1){
        exit(EXIT_FAILURE);
    }

    //GrovePi+のFWバージョンの確認
    versionRead();

    //人感センサーの接続GPIOの設定
    pinMode(2,0);    //D2,INPUT

    //無限ループ
    while(1)
    {
        //人感センサーリード(0=未検知、1=人検知)
        dval = digitalRead(2);	//D2
        printf("Digital read %d\n", dval);

        //500ms待ち（特に待たなくても良い）
        pi_sleep(500);
    }

    return EXIT_SUCCESS;
}
