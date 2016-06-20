/*************************************************************
成都昊普环保技术有限公司   版权所有

文件名:  Tcpip.c
作  者:  潘国义
描  述:  网络处理模块
修订记录:   

**************************************************************/

#include "Comm.h"
#include "DLT645.h"
#include "devTcpip.h"
#include "Gsm.h"
#include "Sms.h"
#include "dev.h"
#include "Cdma1x.h"
BYTE CurTcpLinkId = 0;


extern BYTE Rs232_Buffer[];
extern BYTE Rs232_Sav_Num;

extern BYTE Uart0_Buffer[];
extern WORD Uart0_Sav_Num;
//extern BYTE SysParam[];
extern BYTE TcpSendDataLen;

extern BYTE SendFrame_Buffer[];




BYTE Open_TcpIpLink_G(void)
{
    if (ATCmd_IpOpen_G() != TRUE)
    {
        return FALSE;
    }

    return TRUE;
}


BYTE Open_TcpIpLink_C(void)
{
    static BYTE ErrCnt,i,loc;

    if (Send_AT_Cmd("AT^IPOPEN?") != TRUE)
    {
        DebugMsg();
        Clear_Sms_buf();
        return FALSE;
    }
    
    // 已经打开则返回TRUE
    i = 1;
    loc = GetSignLoc(':', i);
    while (loc != 0xFF)
    {
        if (CurTcpLinkId == (Sms_Buffer[loc+1] - 0x30))
        {
            DebugMsg();
            Clear_Sms_buf();
            return TRUE;
        }

        i++;
        loc = GetSignLoc(':', i);
    }

    DebugMsg();
    Clear_Sms_buf();

    
    // Open TCP/IP 连接到主站
    ErrCnt = 0;
    while(1)
    {
        if (ATCmd_IpOpen() != TRUE)
        {
            ErrCnt ++;
            if (ErrCnt > RETRY_COUNT)
            {
                return FALSE;
            }
            Sleep(2000);
            //WDog();
            Sleep(2000);
            //WDog();
            Sleep(1000);
            //WDog();
        }
        else
        {
            break;
        }
    }

    DebugMsg();
    Clear_Sms_buf();
    //Sleep(200);
    return TRUE;
}


// FLed : 是否闪灯
BYTE Init_TCPIP(void)
{
    if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        return Init_Cdma1x();
    }
    else if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        return Init_Gprs();
    }
    else
    {
        return FALSE;
    }
}


BYTE Open_TcpIpLink(void)
{
    if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        return Open_TcpIpLink_C();
    }
    else if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        return Open_TcpIpLink_G();
    }
    else
    {
        return FALSE;
    }
    
}


//发送TCP数据
BYTE Send_Tcp_Data(void)
{
    #ifdef UART_TESTMODE
    static BYTE i;
    
    for (i=0;i<TcpSendDataLen;i++)
    {
        Send_a_byte(TCP_SendBuf[i]);
    }

    #if 0
    Uart1_SendByte(0x0D);
    Uart1_SendByte(0x0A);
    for (i=0;i<TcpSendDataLen;i++)
    {
        Uart1_SendByte(HexToBcd(TCP_SendBuf[i]/16));
        Uart1_SendByte(HexToBcd(TCP_SendBuf[i]%16));
        Uart1_SendByte(' ');
    }
    Uart1_SendByte(0x0D);
    Uart1_SendByte(0x0A);
    #endif
    
    #else

    *(DWORD *)&SysParam[SP_FLOWRATE] += TcpSendDataLen;
    
    if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        return ATCmd_SendDataEx();
    }
    else if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        return ATCmd_SendData_G();
    }
    else
    {
        return FALSE;
    }
    
    #endif
}


BYTE Query_Tcp_Data(void)  
{
    if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        if (TCP_RecLen >= 7)
        {
            if (Check_string_rxed("^IPDATA") == 1)
            {
                return TRUE;
            }
        }

        return FALSE;
    }
    else if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        return QueryTcpData_G(0);
    }
    else
    {
        return FALSE;
    }
}




//关闭当前连接
BYTE Close_TcpIpLink(void)
{
    //关闭链接命令：AT%IPCLOSE
    if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        return ATCmd_CloseIp();
    }
    else if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        return ATCmd_CloseIp_G();
    }
    else
    {
        return FALSE;
    }
}


void Clear_Tcp_RecBuf(void)
{
    static WORD i;

    for(i=0;i<TCP_RecBufLen;i++) 
    {
        TCP_RecBuf[i] = 0x00;
    }
    TCP_RecLen = 0;
}

void Clear_Tcp_SendBuf(void)
{
    static BYTE i;
    for (i=0;i<TCP_SENDLEN;i++)
    {
        TCP_SendBuf[i] = 0;
    }
    TcpSendDataLen = 0;
}


