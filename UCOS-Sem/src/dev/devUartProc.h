/*************************************************************
成都昊普环保技术有限公司   版权所有
无线采集器


文件名:   
作  者:    潘国义
描  述:    无线模块
修订记录:

**************************************************************/

#ifndef __devUartProc_H__
#define __devUartProc_H__
#include "comm.h"
#include "DLT645.h"

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long


#define UART0_BUFLEN  1200
#define RS232_BUFLEN  200
//#define Uart0_Buffer  Recv645Buf
//#define UART0_BUFLEN  RECV_645_MAX

extern BYTE Uart0_Buffer[UART0_BUFLEN];
extern WORD Uart0_Sav_Num ;
extern void Rs232SendByte(unsigned char dat);
extern void taskUartHandle ( void * pdata );
extern void ClearUart0Buf(void);
extern void sendUart0Term ( unsigned char *buff , int len );
extern void sendUart1Term ( unsigned char *buff , int len );
extern void ClearRs232Buf(void);


#endif

