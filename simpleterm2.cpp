/***********************************************************************/
/*                                                                     */
/* GSM-MG200受信用プログラム                                           */
/* シリアルポート受信サンプルプログラム                                */
/*                                                                     */
/* このプログラムはシリアルポートをopenして、データを16進数表示する    */
/* サンプルプログラムです。                                            */
/*                                                                     */
/* 2018.12 Kouhei Ito @ ITC                                            */
/*                                                                     */
/***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <time.h>

#define DEV_NAME    "/dev/ttyUSB0"        // デバイスファイル名
#define BAUD_RATE    B3000000             // RS232C通信ボーレート
#define BUFF_SIZE    4096                 // 適当

// シリアルポートの初期化
void serial_init(int fd)
{
  struct termios tio;
#if 0
  memset(&tio,0,sizeof(tio));
  tio.c_cflag = CS8 | CREAD | CRTSCTS | PARODD ;
  tio.c_cc[VTIME] = 255;//0
  // ボーレートの設定
  cfsetispeed(&tio,BAUD_RATE);
  cfsetospeed(&tio,BAUD_RATE);
  // デバイスに設定を行う
  tcsetattr(fd,TCSANOW,&tio);
#endif
  memset(&tio,0,sizeof(tio));
  tio.c_iflag=0;
  tio.c_oflag=0;
  tio.c_cflag=CS8|CREAD|CRTSCTS|PARODD;           // 8n1, see termios.h for more information
  tio.c_lflag=0;
  //tio.c_cc[VMIN]=1;
  tio.c_cc[VTIME]=100;

  cfsetospeed(&tio,B3000000);            
  cfsetispeed(&tio,B3000000);            

}



/* --------------------------------------------------------------------- */
/* メイン                                                                */
/* --------------------------------------------------------------------- */

int main(int argc,char *argv[]){
  int fd;
  unsigned char buffer[BUFF_SIZE];    // データ受信バッファ
  char c;
  
  // デバイスファイル（シリアルポート）オープン
  fd = open(DEV_NAME,O_RDWR);
  if(fd<0){
    // デバイスの open() に失敗したら
    perror(argv[1]);
    exit(1);
  }

  // シリアルポートの初期化
  serial_init(fd);

  // メインの無限ループ
  for(;;){
    read(fd, &c, 1);
    printf("%02X ", c);
  }
}
