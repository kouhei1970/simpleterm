/***********************************************************************/
/*                                                                     */
/* GSM-MG100受信用プログラム                                           */
/* シリアルポート受信サンプルプログラム                                */
/* Y.Ebihara (SiliconLinux)さんのプログラムを改造して                  */
/* GSM-MG100からのデータを受信する簡易プログラム                       */
/*                                                                     */
/* このプログラムはシリアルポートをopenして、データを16進数表示する    */
/* サンプルプログラムです。                                            */
/*   Ubuntuで動作検証をしています。                                    */
/*                                                                     */
/* 2016.9 Kouhei Ito @ KTC                                             */
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
  memset(&tio,0,sizeof(tio));
  tio.c_cflag = CS8 | CREAD | CRTSCTS | PARODD;
  tio.c_cc[VTIME] = 100;
  // ボーレートの設定
  cfsetispeed(&tio,BAUD_RATE);
  cfsetospeed(&tio,BAUD_RATE);
  // デバイスに設定を行う
  tcsetattr(fd,TCSANOW,&tio);
}


unsigned char getdata(int fd, unsigned char *buf){
    static int len=-1;         //  受信データ数（バイト）
    static int cnt=0;
    unsigned char data;
    static int isfirst=1;

    if (isfirst){
      // ここで受信待ち
      while((len=read(fd,buf,BUFF_SIZE))==0);
      cnt=0;
    
      if(len<0){
        printf("Data read  ERROR\n");
        // read()が負を返したら何らかのI/Oエラー
        perror("");
        exit(2);
      }
      isfirst=0;
    } 

    // read()が正を返したら受信データ
    if(cnt<len){
      data=buf[cnt];
      cnt++;
    }
    else {
      //printf(" END\n");
      while((len=read(fd,buf,BUFF_SIZE))==0);
      cnt=0;
      if(len<0){
        printf("Data read  ERROR\n");
        // read()が負を返したら何らかのI/Oエラー
        perror("");
        exit(2);
      }
      data=buf[cnt];
      cnt++;
    }
    return data;
}


/* --------------------------------------------------------------------- */
/* メイン                                                                */
/* --------------------------------------------------------------------- */

int main(int argc,char *argv[]){
  int fd, len;
  int rcounter=1;

  unsigned char buffer[BUFF_SIZE];    // データ受信バッファ
  
  int i;
 
  unsigned char data2;
  unsigned int datasize;
  unsigned char data;
  
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

    for(int i=0; i<25;i++){
        data = getdata(fd, buffer);
        printf("%02X ",data);
    }
    printf("\n");
    continue;



    if (data!=0x41)continue;
    data = getdata(fd, buffer);
    if (data!=0x53)continue;
    data = getdata(fd, buffer);
    data = getdata(fd, buffer);
    data = getdata(fd, buffer);
    if (data!=0x47)continue;
    data = getdata(fd, buffer);
    if (data==0x4E)break;
  }
  for(int i=0;i<12;i++)
      data=getdata(fd, buffer);
  unsigned char lower = getdata(fd, buffer);
  unsigned char upper = getdata(fd, buffer);
  unsigned int num=(unsigned int)upper*0xff+lower;
  for(int i=0; i<num; i++)
      data=getdata(fd, buffer);
  for(i=0;i<4;i++)
      data=getdata(fd, buffer);





  while(1){

    for(;;){
      data=getdata(fd, buffer);
      // 受信したデータを 16進数形式で表示    
      //printf("%02X ", data);      
      //受信データは意味の塊ごとに先頭に16進数で16 16 06 02ちと言うデータがつくことになっているので
      //以下でそのデータを受信するごとに改行して表示するようになっている．
      switch(rcounter){
        case 1:
          if (data==0x47){
            rcounter++;
            printf("HEADER %02X ", data);
          }
          else rcounter=1;
          break;
        case 2:
          if (data==0x4E){
            rcounter++;
            printf("%02X\n", data);
          }
          else rcounter=1;
          break;
        case 3:
          rcounter++;
          printf("ID %02X", data);
          data=getdata(fd, buffer);
          printf("%02X\n", data);
                      
          break;
        case 4:
          rcounter++;
          printf("TIME %02X", data);
          data=getdata(fd, buffer);
          printf("%02X", data);
          data=getdata(fd, buffer);
          printf("%02X", data);
          data=getdata(fd, buffer);
          printf("%02X\n", data);
          
          break;
        case 5:

          rcounter++;
          printf("MULTI FRAME %02X", data);
          data=getdata(fd, buffer);
          printf("%02X", data);
          data=getdata(fd, buffer);
          printf("%02X", data);
          data=getdata(fd, buffer);
          printf("%02X\n", data);
          break;
        case 6:
          rcounter++;
          printf("PROVISION %02X", data);
          data=getdata(fd, buffer);
          printf("%02X\n", data);
                      
          break;
        case 7:
          rcounter++;
          printf("PAYLOAD SIZE %02X", data);
          data2=data;
          data=getdata(fd, buffer);
          printf("%02X\nDATA ", data);
          datasize=data*0xff + data2;
          break;
        case 8:
          printf("%02X ",data);
          if(--datasize==0){
            printf("\n");
            rcounter++;
          }

          break;
        case 9:
          rcounter++;
          printf("FUTTER %02X ",data);
          data=getdata(fd, buffer);
          printf("%02X\n",data);
    
          break;
        case 10:
          rcounter=1;
          printf("CRC %02X", data);
          data=getdata(fd, buffer);
          printf("%02X\n", data);
      }
    }        
  }
}

