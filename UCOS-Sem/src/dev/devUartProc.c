
#include "includes.h"
#include "dev.h"
#include <netBuf.h>
#include <uart.h>
#include <hw_uart.h>
#include <hw_gpio.h>
#include <hw_types.h>
#include <hw_ints.h>

#include <SCom.h>
#include <Em485.h>
#include "DLT645.h"
#include "rn8209x.h"
#include "Sms.h"
#include "DLT645.h"
#include "devUartProc.h"

BYTE Uart0_Buffer[UART0_BUFLEN]={0};
WORD Uart0_Sav_Num = 0;

BYTE Rs232_Buffer[RS232_BUFLEN]={0};
BYTE Rs232_Sav_Num = 0;

/*******************************************************************************
*函数名:
　　()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/

int fputc ( int ch, FILE *f )
{
   
    UARTCharPut(UART1_BASE, ch);

    if ( ch == 0x0a )
    {
        UARTCharPut(UART1_BASE, 0x0d);
    }

    return ch;
}


int fgetc ( FILE *  f )
{
    f = f;
    return commBaseGetChar(1);
}


void sendUart1Term ( unsigned char *buff , int len )
{
    int sendlen = 0;
    for(sendlen=0;sendlen<len;sendlen++)
    {
        UARTCharPut ( UART1_BASE , buff[sendlen] );
        
    }       
}

void sendUart0Term ( unsigned char *buff , int len )
{
    int sendlen = 0;

    for ( sendlen = 0; sendlen < len   ; sendlen++ )
    {
        UARTCharPut ( UART0_BASE , buff[sendlen] );
    }

}

void ClearUart0Buf(void)
{
    static WORD i;
    for (i=0;i<UART0_BUFLEN;i++)
    {
        Uart0_Buffer[i] = 0;
    }
    Uart0_Sav_Num = 0;
}

void Rs232SendByte(BYTE dat)
{
    UARTCharPut(UART1_BASE, dat);
}

void ClearRs232Buf(void)
{
    BYTE i;
    for (i=0;i<RS232_BUFLEN;i++)
    {
        Rs232_Buffer[i] = 0;
    }
    Rs232_Sav_Num = 0;
}

/*******************************************************************************
*函数名:
　　()
*功能: 打开时钟
*输入:
*输出:
*说明:
*******************************************************************************/

void taskUartHandle ( void * pdata )
{

    printf ( "\r\nUartHandle task begin..." );
    
    while ( 1 )
    {
       

        Recv645Buf[Recv_645Save_num++] = commBaseGetChar ( 1 ) ;
       
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
            
            if(RxValidDataFrame(Recv645Buf,Recv_645Save_num))
            {   
                ClearDLT645SendBuf();
                MeterCmdAnalysis1(Recv645Buf);
            
                sendUart1Term(Send645Buf, Send_645Buf_num);
               
                ClearDLT645RecvBuf();
                ClearDLT645SendBuf();
            }
        }
         
 
        if ( Recv_645Save_num >=  MAX_RX_485_PACK_SIZE )
        {
            Recv_645Save_num = 0;
        }

    }

}
