//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ADC制御(SPI)プログラム
// Copyright (c) 2016 ヘタレ塾 (https://github.com/hetarejuku/)
// Released under the MIT License.
// https://opensource.org/licenses/mit-license.php
// 概要　: SPIでAD(MCP3202/MCP4822)読込み速度を測定するプログラム
//         製品名(ADC-DAC Pi) https://www.abelectronics.co.uk/
//         製品のサンプルコード(https://github.com/abelectronicsuk/ABElectronics_C_Libraries)
// 履歴　: v1.0 (2016/10/23) ヘタレ塾 (新規作成)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//*****インクルードファイル*****
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <time.h>
#include <getopt.h>

#include "ABE_ADCDACPi.h"  //製品のサンプルライブラリーを使用

//*****マクロ定義*****
#define ADC_NUM       240  //計測回数（適当）
#define ADC_CHANNEL   1    //チャンネル 1～2
#define ADC_MODE      0    //モード 0 = シングルエンド / 1 = 差動

#define VOLTAGE_DISP       //ADCを電圧表示。コメントアウトするとADC生値

//------------------------------------------------------------------------------
// main : メイン関数
//  引数 argc : 引数の総個数
//       argv : 引数のポインター（[0]はファイル名）
//  戻り値 : EXIT_SUCCESS/EXIT_FAILURE
//  処理概要 :
//  ・ADC用SPIのオープン/クローズを要求する
//  ・240回ADCを読む
//  ・AD値の平均値と計測速度を算出する
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    int i = 0;                    //カウンタ
    struct timeval t1;            //計測開始時刻
    struct timeval t2;            //計測終了時刻
    double elapsed = 0;           //計測時間
    double sum = 0;               //ADC総和
    double average = 0;           //ADC平均値
    double rate = 0;              //ADC計測速度

    //ADC用SPIオープン
    if(open_adc() == 0){
        perror("OPEN ERROR");
        exit(EXIT_FAILURE);
    }

    //計測開始時刻取得
    gettimeofday(&t1, NULL);

    //240回計測
    for (i=0 ; i<=ADC_NUM ; i++){
        //12bit ADCリード
#ifdef VOLTAGE_DISP
        sum += read_adc_voltage(ADC_CHANNEL, ADC_MODE); //0～3.3v
#else
        sum += read_adc_raw(ADC_CHANNEL, ADC_MODE);     //0～4095
#endif
    }

    //計測終了時刻取得
    gettimeofday(&t2, NULL);

    //計測時間の総和算出(ms)
    elapsed = (t2.tv_sec - t1.tv_sec) * 1000.0;      //sec to ms
    elapsed += (t2.tv_usec - t1.tv_usec) / 1000.0;   //us to ms

    //計測速度の算出(ksps)
    rate = ADC_NUM / elapsed;
    //ADC平均値の算出
    average = sum / ADC_NUM;

    printf("SAMPLES=%d (%.2f ms)\n", ADC_NUM, elapsed);
    printf("SAMPLING RATE=%.2f ksps\n", rate);
#ifdef VOLTAGE_DISP
    printf("AVERAGE VOLTAGE=%.2f v\n", average);
#else
    printf("AVERAGE VALUE=%.0f\n", average);
#endif

    //ADC用SPIクローズ
    close_adc();

    return EXIT_SUCCESS;
}
