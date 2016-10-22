//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// シリアルポート通信制御プログラム
// Copyright (c) 2016 ヘタレ塾 (https://github.com/hetarejuku/)
// Released under the MIT License.
// https://opensource.org/licenses/mit-license.php
// 概要　: 受信したデータを受信元へ送信する、エコーバックプログラム
// 履歴　: v1.0 (2016/10/22) ヘタレ塾 (新規作成)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//*****インクルードファイル*****
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>

//*****マクロ定義*****
//使用するシリアルのデバイスファイルを選択する
//#define DEV_NAME    "/dev/ttyAMA0"  //デバイスファイル名(UART)
#define DEV_NAME    "/dev/ttyUSB0"  //デバイスファイル名(USB FTDI)
//#define DEV_NAME    "/dev/ttyACM0"  //デバイスファイル名(USB CDC)

#define BAUD_RATE    B115200         //RS232C通信ボーレート
#define BUFF_SIZE    256            //適当

//*****サブルーチン宣言*****
void serialInit(int fd);                      //シリアルポートの設定初期化関数

//------------------------------------------------------------------------------
// main : メイン関数
//  引数 argc : 引数の総個数
//       argv : 引数のポインター（[0]はファイル名）
//  戻り値 : EXIT_SUCCESS/EXIT_FAILURE
//  処理概要 :
//  ・シリアルポートのオープン/クローズを要求する
//  ・データの受信を待つ
//  ・データを受信したら、そっくり送り返す（データ送信する）
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    int fd = 0;                  //ファイルディスクリプター
    int i = 0;                   //カウンター
    int len = 0;                 //データ数（バイト）
    uint8_t buf[BUFF_SIZE];      //データバッファ

    //シリアルポートのオープン
    fd = open(DEV_NAME, O_RDWR);
    //オープンエラー？
    if(fd < 0){
        perror("OPEN ERROR");
        exit(EXIT_FAILURE);
    }

    //シリアルポートの設定初期化
    serialInit(fd);

    //無限ループ
    while(1){
        //受信待ち
        len = read(fd, buf, sizeof(buf));
        //受信エラー？
        if(len < 0){
            perror("READ ERROR");
            exit(EXIT_FAILURE);
        }
        //受信なし？
        if(len == 0){
            continue;
        }

        //受信データを16進数で表示
        for(i=0; i<len; i++){
            printf("R%02X ",buf[i]);
        }
        printf("\n");

        //受信したデーターを送信（エコーバック）
        len = write(fd, buf, len);
        //送信エラー？
        if(len < 0){
            perror("WRITE ERROR");
            exit(EXIT_FAILURE);
        }
        //送信データを16進数で表示
        for(i=0; i<len; i++){
            printf("W%02X ",buf[i]);
        }
        printf("\n");
        //一度に送信できない場合もあるので注意
        //（ここでは残（再）送信処理は省略）
    }

    //シリアルポートのクローズ（無限ループなので、ここには来ない）
    close(fd);
    return EXIT_SUCCESS;
}

//------------------------------------------------------------------------------
// serialInit : シリアルポートの設定初期化関数
//  引数 fd : ファイルディスクリプター
//  戻り値 : なし
//  処理概要 :
//  ・main関数からコールされる
//  ・ボーレート、受信タイムアウトなど、各種設定をする
//------------------------------------------------------------------------------
void serialInit(int fd)
{
    struct termios tio;             //シリアル通信設定

    //オール0にして必要なフラグだけ後で立てる
    memset(&tio,0,sizeof(tio));

    //CLOCAL  モデム制御なし
    //CREAD   受信を有効にする
    //CRTSCTS ハードフローを有効にする
    //CSTOPB  ストップビット2
    //CSIZE   データビットのビットマスク
    //CS5〜8  データビット数5〜8まで指定可能
    //PARENB  パリティを有効にする
    //PARODD  パリティビットは奇数とする
    tio.c_cflag = CS8 | CLOCAL | CREAD;

    //VMIN = 0 & VTIME = 0
    //  受信データがあってもなくても、呼び出し元に戻る。
    //VMIN > 0 & VTIME = 0
    //  VMIN 以上のデータを受信しない限り、呼び出し元に戻らない。
    //VMIN > 0 & VTIME > 0
    //  VMIN 以上のデータを受信するか、１バイト目を受信してからVTIME[x100ms]
    //  経過したら、呼び出し元に戻る。何も受信しないと呼び出し元に戻らない。
    //VMIN = 0 & VTIME > 0
    //  受信データがあるか、VTIME[x100ms]経過しても受信データがなかったら、
    //　呼び出し元に戻る。
    tio.c_cc[VTIME] = 1;    //100ms

    //ボーレートの設定
    cfsetispeed(&tio, BAUD_RATE);
    cfsetospeed(&tio, BAUD_RATE);

    //デバイスに設定を行う
    tcsetattr(fd, TCSANOW, &tio);
    
    //シリアルポートのバッファのクリア
    tcflush(fd, TCIFLUSH);
    tcflush(fd, TCOFLUSH);
}
