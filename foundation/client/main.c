//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// TCP/IP（クライアント用）通信制御プログラム
// Copyright (c) 2016 ヘタレ塾 (https://github.com/hetarejuku/)
// Released under the MIT License.
// https://opensource.org/licenses/mit-license.php
// 概要　: サーバーにデータを送信した後、受信待ちするプログラム
// 履歴　: v1.0 (2016/11/01) ヘタレ塾 (新規作成)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//*****インクルードファイル*****
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>         //IPアドレス取得で使用
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

//*****マクロ定義*****
//使用するLANのデバイスファイルを選択する
//#define DEV_NAME    "eth0"  //デバイスファイル名(有線LAN)
#define DEV_NAME    "wlan0"  //デバイスファイル名(無線LAN)

#define PORT_NUM    57975    //自由に設定できるポート番号(49152 - 65535)
#define BUFF_SIZE   1024     //適当

//*****サブルーチン宣言*****
void clientInit(int sockfd);                      //クライアント用の設定初期化関数

//------------------------------------------------------------------------------
// main : メイン関数
//  引数 argc : 引数の総個数
//       argv : 引数のポインター（[0]はファイル名、[1]はサーバーのIPアドレス）
//  戻り値 : EXIT_SUCCESS/EXIT_FAILURE
//  処理概要 :
//  ・ソケットのオープン/クローズを要求する
//  ・サーバーに接続する（サーバーのウィルスチェッカーが他からの接続を禁止していると失敗するので注意）
//  ・サーバーにデータを送信し、サーバーからの受信を待つ。これを2回繰り返す
//  ・UNIX以外のSOCKETは、read/writeに対応していないので、移植性を考えるとrecv/sendを使うほうが無難
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    int sockfd = 0;                  //ソケットのファイルディスクリプター
    struct sockaddr_in serv_addr;    //サーバー情報
    int j = 2;                       //送信回数
    int i = 0;                       //カウンター
    int len = 0;                     //データ数（バイト）
    int req = 0;                     //送信データ数（バイト）
    uint8_t buf[BUFF_SIZE];          //データバッファ

    //引数が2個無い？
    if (argc < 2) {
        perror("SERVER IP ADDRESS ERROR\n");
        exit(EXIT_FAILURE);
    }

    //ソケットのオープン
    sockfd = socket(AF_INET, SOCK_STREAM, 0);    //IPv4,byte stream,1 protocol
    //オープンエラー？
    if (sockfd < 0) {
        perror("OPEN ERROR\n");
        exit(EXIT_FAILURE);
    }

    //設定初期化
    clientInit(sockfd);

    printf("WAITING\n");

    //サーバアドレスの構造体を初期化しておく
    memset((char *)&serv_addr, 0 , sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;                  //IPv4
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);  //接続するサーバーのIPアドレス
    serv_addr.sin_port = htons(PORT_NUM);            //バイトオーダを変更して設定
    //接続処理
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 ){
        perror("CONNECT ERROR\n");
        exit(EXIT_FAILURE);
    }

    printf("OPEND\n");

    //2パケット送信
    for( ; j>0 ; j--){
        //送信データの作成
        len = 7;
        memcpy(buf, "Hello\r\n",len);

        //データーを送信
        req = len;
        len = send(sockfd, buf, len, 0);
        //送信エラー？
        if(len < 0 ){
            //サーバーが切断した？
            if(errno == EPIPE || errno == ECONNRESET){
                printf("DISCONNECTED\n");
                break;
            }
            //送信バッファオーバーフロー
            if(errno == EAGAIN || errno == EWOULDBLOCK){
                //一度に送信できない場合もあるので本来は対策必要
                //（サンプルにつき信頼性向上の再送信処理は省略）
                printf("OVERFLOW\n");
                continue;
            }
            perror("WRITE ERROR\n");
            exit(EXIT_FAILURE);
        }
        //送信データを16進数で表示
        printf("W(%dByte) ", len);
        for(i=0; i<len; i++){
            printf("%02Xh ",buf[i]);
        }
        printf("\n");
        //一度に送信できない場合もあるので注意
        //（ここでは残（再）送信処理は省略）

        //受信待ち
        len = recv(sockfd, buf, sizeof(buf), 0);
        //受信エラー？
        if(len < 0) {
            perror("READ ERROR\n");
            exit(EXIT_FAILURE);
        }
        //サーバーが切断した？
        if(len == 0){
            printf("DISCONNECTED\n");
            break;
        }
        //受信データを16進数で表示
        printf("R(%dByte) ", len);
        for(i=0; i<len; i++){
            printf("%02Xh ",buf[i]);
        }
        printf("\n");
    }

    //ソケットのクローズ
    close(sockfd);
    return EXIT_SUCCESS;
}

//------------------------------------------------------------------------------
// clientInit : クライアント用設定初期化関数
//  引数 sockfd : ソケットのファイルディスクリプター
//  戻り値 : なし
//  処理概要 :
//  ・main関数からコールされる
//  ・クライアントの場合は、特段の必要な設定は無い
//------------------------------------------------------------------------------
void clientInit(int sockfd)
{
    struct ifreq ifr;                //IPアドレス
    int on = 1;                      //アドレスの再利用を許可

    //割り振られた自分のIPアドレスと設定したポート番号を表示（確認用）
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, DEV_NAME, IFNAMSIZ-1);
    ioctl(sockfd, SIOCGIFADDR, &ifr);
    printf("CLIENT IP = %s, PORT = %d\n",
          inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr), PORT_NUM);
}
