/*************************************************************
成都昊普环保技术有限公司   版权所有

文件名:  GSM.H
作  者:  潘国义
描  述:  GSM模块
修订记录:   

**************************************************************/

#ifndef __GSM_H__
#define __GSM_H__

#define TCP_RecBuf        Uart0_Buffer
#define TCP_RecBufLen     UART0_BUFLEN
#define TCP_RecLen        Uart0_Sav_Num

BYTE Init_Gprs(void);

BYTE Check_GsmNet(BYTE FLed);
BYTE ATCmd_IpOpen_G(void);
BYTE ATCmd_SendData_G(void);
BYTE ATCmd_CloseIp_G(void);
BYTE QueryTcpData_G(WORD DataLen);

BYTE NewMsg_G(void);
BYTE Read_SMS_G(void);
BYTE Send_Pdu_SMS(void);

#endif

