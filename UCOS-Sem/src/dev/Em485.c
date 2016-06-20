/*************************************************************
成都昊普环保技术有限公司   版权所有

文件名:  Em485.c
作  者:  潘国义
描  述:  电表485通讯模块
修订记录:

**************************************************************/
#include "includes.h"
#include <uart.h>
#include <dev.h>
#include <SCom.h>
#include <Em485.h>
#include "DLT645.h"
#include "Tcpip.h"
//BYTE Em_Buf_Loc = 0;  // 当前数据位置
//BYTE Loc;
//BYTE EmDataLen;

//DWORD Power_Int = 0;  // 整数部分
//BYTE Power_Dec = 0;   // 小数部分


void send485Term ( unsigned char *buff , int len )
{
    int sendlen = 0;
    rs485TxSwitch(1);//ENABLE RS485
    for ( sendlen = 0; sendlen < len   ; sendlen++ )
    {
       UARTCharPut ( UART2_BASE , buff[sendlen] );
    }

    while(UARTBusy(UART2_BASE));// Waiting end of send
    
    rs485TxSwitch(0);//Disable 485

}

void taskRS485Rx ( void *param )
{
//    unsigned char ch;
//    unsigned char err;

    OSTimeDly ( OS_TICKS_PER_SEC );
    printf ( "\r\nRS485Rx task begin...\r\n" );

    while(1)
    {
        Recv645Buf[Recv_645Save_num++] = commBaseGetChar ( RS485_UART_IDX ) ;
       
        if ( Recv_645Save_num > 0 )
        {
            if ( Recv645Buf[0] != 0x68 )
            {
                Recv_645Save_num = 0;
            }
        }
        if(Recv_645Save_num > RECV_645_MAX)
        {
           Recv_645Save_num = 0; 
        }
         
        if((Recv645Buf[Recv_645Save_num-1] == 0x16)&&(Recv_645Save_num > 11))
        {
            ClearDLT645SendBuf();
            MeterCmdAnalysis1(Recv645Buf); 
            send485Term(Send645Buf, Send_645Buf_num);
            ClearDLT645RecvBuf();
            
        }
         
 
        if ( Recv_645Save_num >=  MAX_RX_485_PACK_SIZE )
        {
            Recv_645Save_num = 0;
        }
   
    }


}



