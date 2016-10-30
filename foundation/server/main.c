//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// TCP/IP（サーバー用）通信制御プログラム
// Copyright (c) 2016 ヘタレ塾 (https://github.com/hetarejuku/)
// Released under the MIT License.
// https://opensource.org/licenses/mit-license.php
// 概要　: 受信したデータを送信元へ送信する、エコーバックプログラム
// 履歴　: v1.0 (2016/10/30) ヘタレ塾 (新規作成)
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
void serverInit(int sockfd);                      //サーバー用の設定初期化関数

//------------------------------------------------------------------------------
// main : メイン関数
//  引数 argc : 引数の総個数
//       argv : 引数のポインター（[0]はファイル名）
//  戻り値 : EXIT_SUCCESS/EXIT_FAILURE
//  処理概要 :
//  ・ソケットのオープン/クローズを要求する
//  ・クライアントからの接続を待つ
//  ・クライアントからのデータ受信を待つ
//  ・データを受信したら、そっくりクライアントへ送り返す（データ送信する）
//  ・UNIX以外のSOCKETは、read/writeに対応していないので、移植性を考えるとrecv/sendを使うほうが無難
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    int sockfd = 0;                  //ソケットのファイルディスクリプター
    int tcpfd = 0;                   //TCPセッションのファイルディスクリプター
    struct sockaddr_in serv_addr;    //サーバー情報
    struct sockaddr_in cli_addr;     //クライアント情報
    int cli_len = sizeof(cli_addr);  //クライアント情報のサイズ
    int i = 0;                       //カウンター
    int len = 0;                     //データ数（バイト）
    int req = 0;                     //送信データ数（バイト）
    uint8_t buf[BUFF_SIZE];          //データバッファ

    //ソケットのオープン
    sockfd = socket(AF_INET, SOCK_STREAM, 0);    //IPv4,byte stream,1 protocol
    //オープンエラー？
    if (sockfd < 0) {
        perror("OPEN ERROR");
        exit(EXIT_FAILURE);
    }

    //設定初期化
    serverInit(sockfd);

    //ソケットの設定
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT_NUM);
    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("BIND ERROR");
        exit(EXIT_FAILURE);
    }

    //接続許可
    listen(sockfd, 1);	//1コネクションのみ許可

    //無限ループ
    while (1) {
        printf("WAITING\n");
        
        //クライアントからのTCP接続待ち
        tcpfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t*)&cli_len);

        //接続エラー？
        if (tcpfd < 0 ){
            perror("ACCEPT ERROR");
            exit(EXIT_FAILURE);
        }
 
        printf("OPEND (CLIENT IP = %s)\n", inet_ntoa(cli_addr.sin_addr));

        //無限ループ
        while (1) {
            //受信待ち
            len = recv(tcpfd, buf, sizeof(buf), 0);
            //受信エラー？
            if(len < 0) {
                perror("RECV ERROR");
                exit(EXIT_FAILURE);
            }
            //クライアントが切断した？
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

            //受信したデーターを送信（エコーバック）
            req = len;
            len = send(tcpfd, buf, len, 0);
            //送信エラー？
            if(len < 0 ){
                //クライアントが切断した？
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
                perror("SEND ERROR");
                exit(EXIT_FAILURE);
            }
            //送信データを16進数で表示
            printf("W(%dByte) ", len);
            for(i=0; i<len; i++){
                printf("%02Xh ",buf[i]);
            }
            printf("\n");
	    //全部送信できなかった？
            if(len < req){
                //一度に送信できない場合もあるので本来は対策必要
                //（サンプルにつき信頼性向上の残（再）送信処理は省略）
                printf("NOT ENOUGH\n");
                continue;
            }
        }
        //TCPセッションのクローズ
        close(tcpfd);
    }

    //ソケットのクローズ（無限ループなので、ここには来ない）
    close(sockfd);
    return EXIT_SUCCESS;
}

//------------------------------------------------------------------------------
// serverInit : サーバー用の設定初期化関数
//  引数 sockfd : ソケットのファイルディスクリプター
//  戻り値 : なし
//  処理概要 :
//  ・main関数からコールされる
//  ・bindとlistenなど、各種設定をする
//------------------------------------------------------------------------------
void serverInit(int sockfd)
{
    struct ifreq ifr;                //IPアドレス
    int on = 1;                      //アドレスの再利用を許可

    //割り振られた自分のIPアドレスと設定したポート番号を表示（確認用）
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, DEV_NAME, IFNAMSIZ-1);
    ioctl(sockfd, SIOCGIFADDR, &ifr);
    printf("SERVER IP = %s, PORT = %d\n", inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr), PORT_NUM);

    //アドレスの再利用を許可
    //（クライアントより先にサーバーがクローズした後に、即再オープンするとBINDエラーになるのを防止する）
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        perror("SET ERROR");
        exit(EXIT_FAILURE);
    }
}
