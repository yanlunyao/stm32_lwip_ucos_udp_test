
#include "includes.h"
#include <dev.h>
#include <hw_nvic.h>
#include <devAdc.h>
#include <update.h>
#include <crc.h>
#include "devUartProc.h"
#include "comm.h"
#include "rn8209x.h"
#include "DLT645.h"
#include "spi.h"
extern struct tcp_pcb * gPcbLastConnectFromClient;


BYTE Uart2_Buffer[UART2_BUFLEN ];
BYTE Uart2_sav_num;


BYTE SysTime[7];
BYTE SendFrame_Buffer[UART2_BUFLEN];

BYTE LastError;

BYTE DevAddrTmp[5];   // 设备地址缓存

uint bDevAddModify = false;

extern BYTE NeedSaveParam ;
extern BYTE NeedReboot ;
extern BYTE NeedRebootChk ;

tDevParam gDevParam;

#if 0
void showDebug ( void )
{

    printf("\r\n bEthPhyLink() :%d " ,  bEthPhyLink());
    printf ( "\r\n NeedSaveParam:%d " , NeedSaveParam );
    printf ( "\r\n NeedReboot:%d " , NeedReboot );
    printf ( "\r\n NeedRebootChk:%d " , NeedRebootChk );
    printf ( "\r\n ServerLogin:%d " , ServerLogin );

    printf ( "\r\n bDevAddModify:%d " , bDevAddModify );
    printf ( "\r\n LastError:%d " , LastError );
    printf ( "\r\n  bootCode:%d " ,  gPubVal.resetCause );

        

    printf ( "\r\ngPubVal.iSvrNoCommCnt :%d" , gPubVal.iSvrNoCommCnt );

    showIpParam();

}


void doAlarm ( int cnt )
{
    int i;

    for ( i = 0; i < cnt; i++ )
    {
        BEEP(1);
        OSTimeDly ( OS_TICKS_PER_SEC / 6 );
        BEEP(0);
        OSTimeDly ( OS_TICKS_PER_SEC / 6 );
    }
}


void Clear_Sms_buf(void)
{
    ClearUart0Buf();
}


/*******************************************************************************
*函数名:
　　()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/

unsigned short crc_16(unsigned char * buff ,int len)
{
  int i;  
  int bit; 
  unsigned short crc = 0xffff;
  unsigned char *pch = (unsigned char *)&crc;

  for( i=0;i<len ;i++)
  {
    unsigned char ch = *pch ^ buff[i];

    *pch = ch;
    //crc = ch;

     for( bit = 0;bit < 8; bit++)
     {
           unsigned short tmpsh = crc;
           crc >>= 1;

           if (tmpsh & 0x01)
           {
              crc ^= 0xa001;
           }
     }
  }
  return crc;

}
#endif
