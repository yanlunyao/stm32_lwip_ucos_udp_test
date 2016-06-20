#ifndef __TCPIP_H__
#define __TCPIP_H__


#define TCPDDATA_NULL  0
#define TCPDDATA_QUEST  1
#define TCPDDATA_REPORT 2

#ifdef UART_TESTMODE
#define TCP_RecBuf       Rs232_Buffer
#define TCP_RecBufLen    RS232_BUFLEN
#define TCP_RecLen       Rs232_Sav_Num
#else
#define TCP_RecBuf       Uart0_Buffer
#define TCP_RecBufLen    UART0_BUFLEN
#define TCP_RecLen       Uart0_Sav_Num
#endif

#define TCP_SENDLEN   SENDBUFLEN
//#define TCP_SendBuf    SendFrame_Buffer
#define TCP_SendBuf   Send645Buf
#define TCP_Send_Len  Send_645Buf_num



BYTE Init_TCPIP(void);
BYTE Send_Tcp_Data(void);
BYTE Open_TcpIpLink(void);
BYTE Close_TcpIpLink(void);
BYTE Query_Tcp_Data(void);

void Clear_Tcp_RecBuf(void);
void Clear_Tcp_SendBuf(void);
#endif
