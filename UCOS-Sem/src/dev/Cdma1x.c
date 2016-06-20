/*************************************************************
成都昊普环保技术有限公司   版权所有

文件名:  Cdma1x.c
作  者:  潘国义
描  述:  Cdma模块
修订记录:   

**************************************************************/


#include "includes.h"
#include "driverlib/uart.h"
#include "Hw_uart.h"
#include "Comm.h"
#include "Cdma1x.h"
#include "Sms.h"
#include "Dev.h"
#include "Scom.h"
#include "DLT645.h"
#include "devTcpip.h"
#include "WlModule.h"
extern BYTE SendFrame_Buffer[];


extern BYTE TcpSendDataLen;

extern BYTE Uart0_Buffer[];
extern WORD Uart0_Sav_Num;

extern BYTE SysTime[];
//extern  char *SysParam;

extern BYTE SmsLength;
extern BYTE SmsContent[];
extern BYTE SmsNumber[];
extern WORD Sms_buf_addr;

extern BYTE IpAddrStr[];
extern BYTE PortStr[];

//extern BYTE Sms_Index;
extern BYTE Sms_IndexStr[];
extern BYTE NeedReConnect;
extern BYTE CurTcpLinkId;

extern uchar NetModulState;


BYTE DataModeEnable = 0;

extern BYTE CurTcpLinkId;

BYTE Test_AtCmd(void)
{
    static BYTE Ec;

    Ec = 0;

    while(1)
    {
        if (Send_AT_Cmd("AT") == TRUE)
        //if (Send_AT_Cmd("AT+COPN") == TRUE)
        {
            DebugMsg();
            //if ((Check_string_rxed("CHINA  MOBILE") == 1) || (Check_string_rxed("CHN-CUGSM") == 1))
            {
              
                return TRUE;
            }
        }

        DebugMsg();

        Sleep(2000);
    
 //       ALM_SW();

        Ec ++;
        if (Ec > 100)
        {
            return FALSE;
        }
    }

//    return FALSE;
}



BYTE Init_Cdma1x(void)
{
    static BYTE ErrCnt;

    //Init TCP/UDP 
    ErrCnt = 0;
    while(1)
    {
        if (ATCmd_InitIp() != TRUE)
        {
            DebugMsg();
            
            ErrCnt ++ ;
            if (ErrCnt > RETRY_COUNT) 
            {
                return FALSE;
            }

            //5秒后重试
            Sleep(5000);       
        }
        else
        {
            break;
        }
    }

    
    DebugMsg();
    //Sleep(200);

    // 启动服务器监听客户端的连接
    ErrCnt = 0;
    while(1)
    {
        if (ATCmd_IpListen() != TRUE)
        {
            ErrCnt ++;
            if (ErrCnt > RETRY_COUNT)
            {
                return FALSE;
            }
            Sleep(5000);           
        }
        else
        {
            break;
        }
    }

    DebugMsg();
    Clear_Sms_buf();
    //Sleep(200);
    
    //主动上报数据
    if (Send_AT_Cmd("AT^IPDATMODE=1") != TRUE)
    {
        DebugMsg();
        Clear_Sms_buf();
        return FALSE;
    }
    
    DebugMsg();
    Clear_Sms_buf();
    
    
    
    return TRUE;
}





BYTE Send_AT_Cmd(BYTE *Cmd)
{
    static BYTE i;
    Clear_Tcp_RecBuf();
 
    Send_String(Cmd); 
    
    Send_a_byte(0x0D);
    Send_a_byte(0x0A);
 
    i  = 0;
    while(Check_string_rxed("OK") == 0)
    {
        Sleep(200);
        if(Check_string_rxed("ERROR") == 1) 
        {
            Sleep(200);
            return FALSE;
        }
        //WDog();
        i ++;
        if (i > 100)  // 20s
        {
            
            CmdTimeOutHandle();
            return TIMEOUT;
        }
    }

    //WDog();
    return TRUE;
}


BYTE Hang_Call(void)
{
    if (Sms_BUFFER_LEN >= 4)
    {
        if (Check_string_rxed("RING") == 1)
        {
            //DebugMsg();
            Send_AT_Cmd("AT+CHV");
            Clear_Tcp_RecBuf();
            return TRUE;
        }

        if (Check_string_rxed("^MODE:0") == 1) // 网络故障或者无卡
        {
            Clear_Tcp_RecBuf();
            return FALSE;
        }
    }

    return FALSE;
}



//取模块时间
BYTE Get_Modul_Time(void)
{
    static BYTE loc;
    if (Send_AT_Cmd("AT^TIME"))
    {
        loc = (BYTE)GetSignLoc(':', 1);
        SysTime[0] = (Sms_Buffer[loc+3] - 0x30)*16 + (Sms_Buffer[loc+4] - 0x30);
        SysTime[2] = (Sms_Buffer[loc+6] - 0x30)*16 + (Sms_Buffer[loc+7] - 0x30);
        SysTime[3] = (Sms_Buffer[loc+9] - 0x30)*16 + (Sms_Buffer[loc+10] - 0x30);
        SysTime[4] = (Sms_Buffer[loc+12] - 0x30)*16 + (Sms_Buffer[loc+13] - 0x30);
        SysTime[5] = (Sms_Buffer[loc+15] - 0x30)*16 + (Sms_Buffer[loc+16] - 0x30);
        SysTime[6] = (Sms_Buffer[loc+18] - 0x30)*16 + (Sms_Buffer[loc+19] - 0x30);
        return TRUE;
    }

    return FALSE;
}



BYTE Check_CdmaNet(BYTE FLed)
{
    static BYTE Ec;

    if (FLed)
    {
        //ALM_ON();
    }

    Ec = 0;
    while(1)
    {
        if (Check_string_rxed("^MODE:2") == 1)   // CDMA 模块
        {
            //SysParam[SP_MODTYPE] = CDMA_MC323;
            //DebugStr("Checked Cdma module\r\n");
            //ALM_OFF();
            break;
        }

  
        Sleep(2000);
        WDog();
        
        if (FLed)
        {
          //  ALM_SW();
        }

        DebugMsg();

        Ec ++;
        if (Ec > 60) 
        {
            return FALSE;
        }
    }

    
    Ec = 0;
    while(1)
    {   
        if (Send_AT_Cmd("AT+CREG?") == TRUE)
        {
            //DebugMsg();
            if ((Check_string_rxed("+CREG:0,1") == 1)  // 注册了本地运营商
              || (Check_string_rxed("+CREG:0,5") == 1)  // 注册了漫游网络
            )
            {
                if (FLed)
                {
                    //LED_ALARM = LED_OFF;
//                    ALM_OFF();
                }
                return TRUE;
            }
        }

        Sleep(2000);
      

        if (FLed)
        {
            //LED_ALARM = !LED_ALARM;
           // ALM_SW();
        }

        Ec ++;
        if (Ec > 100)
        {
            return FALSE;
        }
    }

    //return TRUE;
}


// 不上报信号强度
void DisableRep(void)
{
    Send_AT_Cmd("AT^RSSIREP=0");
}


BYTE NewMsg_C(void)
{
    static BYTE l1,l2;

    #if 0
    //不使用自动上报
    if (Sms_BUFFER_LEN >= 5)   // 自动上报的信息。
    {
        if (Check_string_rxed("+CMTI") == 1)
        {
            loc = GetSignLoc(',',1);
            if (loc == 0xFF)
            {
                return FALSE;
            }

            Sms_Index = Sms_Buffer[loc + 1] - 0x30;
            //获得短信存储位置
            //mset(Sms_IndexStr, 0, 3);

            DebugMsg();
            Clear_Tcp_RecBuf();
            
            return TRUE;
        }
    }
    #endif

    //查询已经达到的信息
    if (Send_AT_Cmd("AT^HCMGL=0") != TRUE)
    {
        return FALSE;
    }

    if (Check_string_rxed("^HCMGL:") != 1)
    {
        return FALSE;
    }
    
    
    l1 = (BYTE)GetSignLoc(':',1);
    if (l1 == 0xFF)
    {
        return FALSE;
    }

    l2 = (BYTE)GetSignLoc(',',1);
    if (l2 == 0xFF)
    {
        return FALSE;
    }

    if ((l2 - l1) > 3)
    {
        return FALSE;
    }

    //Sms_Index = Sms_Buffer[loc + 1] - 0x30;
    //获得短信存储位置
    mset(Sms_IndexStr, 0, 3);
    mcpy(Sms_IndexStr,&Sms_Buffer[l1 + 1],l2-l1-1);
    

    DebugMsg();
    Clear_Tcp_RecBuf();

    return TRUE;
}

//设置文本模式
BYTE Set_Txt_SMS(void)
{
    //return Send_AT_Cmd("AT^HSMSSS=0,0,1,0");
    return Send_AT_Cmd("AT^HSMSSS?");
}


//发送短信
BYTE Send_Txt_SMS(BYTE *number,BYTE *content)
{  
    static BYTE i;

    if (Set_Txt_SMS() == FALSE)
    {
        return FALSE;
    }

    DebugMsg();
    Clear_Tcp_RecBuf();                            //清空串口缓冲区

    Send_String("AT^HCMGS=");             

    Send_a_byte('"');

    //Send_String("+86");
    Send_String(number);

    Send_a_byte('"');

    Send_a_byte(0x0D);
    Send_a_byte(0x0A);

    i = 0;
    while(Check_string_rxed("> ") == 0)               //收到">"则继续发送SMS内容
    {
        Sleep(200);
    	if (Check_string_rxed("ERROR") == 1)     //若收到ERROR，则短信发送失败 
        {
            Sleep(200);
            //WDog();  
            return FALSE;
    	}

        //WDog();  
        i ++;
        if (i > 250)  // 20s
        {
            CmdTimeOutHandle();
            return TIMEOUT;
        }
    }
    //DebugMsg();
    
    Sleep(200);
    Send_String(content); 
    Send_a_byte(0x1A);                      //短信内容结束标志 CTRL-Z

    Sleep(1000);

    i = 0;
    while(Check_string_rxed("OK") == 0)         //收到OK则短信发送成功
    {
        Sleep(200);
    	if(Check_string_rxed("ERROR") == 1)      //收到ERROR则短信发送失败
        {
            Sleep(1000);

            return FALSE;
        }

        //WDog();  
        i ++;
        if (i > 250)  // 20s
        {
            CmdTimeOutHandle();
            return TIMEOUT;
        }
    }

    // ^HCMGSS 发送成功
    // ^HCMGSF 发送失败
    i = 0;
    while((Check_string_rxed("^HCMGSS") == 0) &&
            (Check_string_rxed("^HCMGSF") == 0))
    {
        Sleep(200);
        //WDog();  
        i ++;
        if (i > 250)  // 20s
        {
            CmdTimeOutHandle();
            return TIMEOUT;
        }
    }

    
    if(Check_string_rxed("^HCMGSS") == 1)     // 成功
    {
        return TRUE;
    }

    return FALSE;
}



BYTE Read_SMS(void)
{
    static BYTE i;
    Clear_Tcp_RecBuf();

    Send_String("AT^HCMGR=");
    //Send_a_byte(Sms_Index+0x30);
    for (i=0;i<3;i++)
    {
        if (Sms_IndexStr[i] != 0)
        {
            Send_a_byte(Sms_IndexStr[i]);
        }
    }
    Send_a_byte(0x0D);
    Send_a_byte(0x0A);

    i = 0;
    while(Check_string_rxed("OK") == 0)
    {
        Sleep(200); 
    	if(Check_string_rxed("ERROR") == 1)
        {
            Sleep(100);
            return FALSE;
    	}

        //WDog();

        i ++;
        if (i > 250)  // 20s
        {
            CmdTimeOutHandle();
            return TIMEOUT;
        }
    }

    return TRUE;
}






// 获取本地和DNS IP地址
BYTE GetIpAddress(void)
{
    static BYTE i,j;
    
    i = (BYTE)GetSignLoc(':', 1);

    if (i == 0xFF)
    {
        return FALSE;
    }
        
    if (Sms_Buffer[i+1] == '0')   // 没有初始化
    {
        return FALSE;
    }

    j = (BYTE)GetSignLoc(',', 2);
    if (j == 0xFF)
    {
        return FALSE;
    }

    //IP 地址变了要重新汇报心跳包
    IpToStr((BYTE *)&SysParam[SP_LOCALIP]);
    if (!mcmp(IpAddrStr,&Sms_Buffer[i+3],j-i-3))
    {
 
        DebugStr("IP Changed\r\n"); 

        NeedReConnect = 1;
        mset(IpAddrStr,0,16);
        mcpy(IpAddrStr,&Sms_Buffer[i+3],j-i-3);
        if (!StrToIpAdd((BYTE *)&SysParam[SP_LOCALIP]))
        {
            return FALSE;
        }
    }
    
    
    return TRUE;
}


void WaitModuleNormal(void)
{
    static BYTE t;

    t = 0;
    //需要等待模块AT命令恢复正常
    while(1)
    {
        if (Send_AT_Cmd("AT") == TRUE)
        {
            DebugMsg();
            Clear_Tcp_RecBuf();
            break;
        }
        DebugMsg();
        Clear_Tcp_RecBuf();

        Sleep(2000);
     
        t++;
        if (t>150)   // 5分钟没有恢复则复位
        {
            //SysReset();
            NetModulState =0;//InitState
        }
    }
        
}




BYTE ATCmd_InitIp(void)
{
    if (Send_AT_Cmd("AT^IPINIT?") != TRUE)
    {
        DebugMsg();
        Clear_Tcp_RecBuf();
        return FALSE;
    }

    if (GetIpAddress())
    {
        DebugMsg();
        Clear_Tcp_RecBuf();
        return TRUE;
    }

    DebugMsg();
    Clear_Tcp_RecBuf();
    
    if (Send_AT_Cmd("AT^IPINIT=,\"card\",\"card\"") != TRUE)
    {
        DebugMsg();
        Clear_Tcp_RecBuf();
        return FALSE;
    }
        
    DebugMsg();
    Clear_Tcp_RecBuf();

    if (Send_AT_Cmd("AT^IPINIT?") != TRUE)
    {
        return FALSE;
    }
    
    if (!GetIpAddress())
    {
        return FALSE;
    }

    DebugMsg();
    Clear_Tcp_RecBuf();
    
    return TRUE;
}

BYTE ATCmd_CloseIp(void)
{
    static BYTE i;
    
    //先退出数据模式
    if (DataModeEnable == 1)
    {
        ATCmd_EndDataMode();
    }

    Send_String("AT^IPCLOSE=");
    Send_a_byte(CurTcpLinkId+0x30);

    Send_a_byte(0x0D);
    Send_a_byte(0x0A);

    
    i  = 0;
    while(Check_string_rxed("OK") == 0)
    {
        Sleep(200); 
        if(Check_string_rxed("ERROR") == 1) 
        {
            DebugMsg();
            Clear_Tcp_RecBuf();
            Sleep(200);
            return FALSE;
        }
        //WDog();
        i ++;
        if (i > 50)  // 10s
        {
            CmdTimeOutHandle();
            return TIMEOUT;
        }
    }
    DebugMsg();
    Clear_Tcp_RecBuf();

    return TRUE;
}

BYTE ATCmd_SendData(BYTE *Data, BYTE Len)
{
    static BYTE i;
        
    for (i=0;i<Len;i++)
    {
        Send_a_byte(Data[i]);
    }

    return TRUE;
}


//启动数据透传, 只能启动一条连接的透传
BYTE ATCmd_StartDataMode(void)
{
    static BYTE i;
    
    Clear_Tcp_RecBuf();

    Send_String("AT^IPENTRANS=");

    Send_a_byte(CurTcpLinkId + 0x30);

    Send_a_byte(0x0D);
    Send_a_byte(0x0A);
    
    i  = 0;
    while(Check_string_rxed("OK") == 0)
    {
        Sleep(200); 
        if(Check_string_rxed("ERROR") == 1) 
        {
            DebugMsg();
            Clear_Tcp_RecBuf();
            Sleep(200);
            DataModeEnable = 0;
            return FALSE;
        }
        //WDog();
        i ++;
        if (i > 50)  // 10s
        {
            DataModeEnable = 0;
            return FALSE;
        }
    }
    
    
    DataModeEnable = 1;
    return TRUE;
}


BYTE ATCmd_EndDataMode(void)
{
    static BYTE i;

    DataModeEnable = 0;
    
    i  = 0;
    while(i<RETRY_COUNT)
    {
        Sleep(1000);    // 必须延时，否则退不出来
        Send_String("+++");  // 结束透明传输 
        Sleep(1000);

        //WDog();

        if (Send_AT_Cmd("AT") == TRUE)
        {
            DebugMsg();
            Clear_Tcp_RecBuf();
            return TRUE;
        }
        
        i ++;
    }
    
    DebugMsg();
    Clear_Tcp_RecBuf();
    return FALSE;
}

BYTE ATCmd_IpOpen(void)
{
    static BYTE i;
    
    //AT^IPOPEN=1,"TCP","129.11.18.8",10000,9000
    //if (Send_AT_Cmd("AT^IPOPEN=1,\"TCP\",\"222.212.14.20\",5000,9000") != TRUE)
    Clear_Tcp_RecBuf();

    Send_String("AT^IPOPEN=1,\"TCP\",\"");
        
    //服务器IP
    IpToStr((BYTE *)&SysParam[SP_SERVERIP]);
    Send_String(IpAddrStr);
    Send_String("\",");
        
    //服务器端口
    PortToStr((BYTE *)&SysParam[SP_SERVERPORT]);
    Send_String(PortStr);
    Send_a_byte(',');
    
    //本地端口 同 服务器端口
    Send_String(PortStr);
    
    Send_a_byte(0x0D);
    Send_a_byte(0x0A);
    
    i  = 0;
    while(Check_string_rxed("OK") == 0)
    {
        Sleep(200); 
        if(Check_string_rxed("ERROR") == 1) 
        {
            DebugMsg();
            Clear_Tcp_RecBuf();
            Sleep(200);
            return FALSE;
        }
        //WDog();
        i ++;
        if (i > 50)  // 10s
        {
            CmdTimeOutHandle();
            return TIMEOUT;
        }
    }
    DebugMsg();
    Clear_Tcp_RecBuf();

    
    return TRUE;
}



BYTE ATCmd_IpListen(void)
{
    static BYTE i;
    
    Clear_Tcp_RecBuf();

    if (Send_AT_Cmd("AT^IPLISTEN?") != TRUE)
    {
        return FALSE;
    }
  
    if (Check_string_rxed("NULL") == 0)
    {
        DebugMsg();
        Clear_Tcp_RecBuf();
        return TRUE;
    }

    DebugMsg();
    Clear_Tcp_RecBuf();

    Send_String("AT^IPLISTEN=\"TCP\",");
    PortToStr((BYTE *)&SysParam[SP_LOCALPORT]);
    Send_String(PortStr);

    Send_a_byte(0x0D);
    Send_a_byte(0x0A);

    i  = 0;
    while(Check_string_rxed("OK") == 0)
    {
        Sleep(200); 
        if(Check_string_rxed("ERROR") == 1) 
        {
            DebugMsg();
            Clear_Tcp_RecBuf();
            Sleep(200);
            return FALSE;
        }
        //WDog();
        i ++;
        if (i > 50)  // 10s
        {
            CmdTimeOutHandle();
            return TIMEOUT;
        }
    }
    
    DebugMsg();
    Clear_Tcp_RecBuf();

    
    return TRUE;
}


BYTE Get_Sms_Content(void)
{
    static BYTE i;
    i = (BYTE)GetSignLoc(0x0A,2);
    if (i == 0xFF)
    {
        return FALSE;
    }

    if (Check_string_rxed("OK") == 1)
    {
        SmsLength = (BYTE)Sms_buf_addr - i - 8;
        if (SmsLength >= SMSMSGCOUNT)
        {
            return FALSE;
        }
        mcpy(SmsContent,&Sms_Buffer[i+1],SmsLength);

        #if 0
        Uart1_SendByte(SmsLength/100+0x30);
        Uart1_SendByte(SmsLength%100/10+0x30);
        Uart1_SendByte(SmsLength%10+0x30);
        Uart1_SendStr(SmsContent,SmsLength); 
        #endif
    }

    return TRUE;
}

BYTE Get_Phone_Number(void)
{
    static BYTE i,j;
    i = (BYTE)GetSignLoc(':',1);
    if (i == 0xFF)
    {
        return FALSE;
    }

    //check phone number
    for (j=0;j<11;j++)   
    {
        if ((Sms_Buffer[i+j+1] < '0') || (Sms_Buffer[i+j+1] > '9'))
        {
            return FALSE;
        }
    }

    //if (Sms_Buffer[i+1] == '"')
    {
        mcpy(SmsNumber,&Sms_Buffer[i+1],11);
        SmsNumber[11] = 0;

        
        DebugStr((char *)SmsNumber); 
        
    }

    return TRUE;
}


BYTE ATCmd_SendDataEx(void)
{
    static BYTE i,h,l;
    static WORD t;
    
    Clear_Tcp_RecBuf();
                   
    Send_String("AT^IPSENDEX=");
    Send_a_byte(CurTcpLinkId+0x30);
    Send_String(",1,\"");
    
    for (i=0;i<TcpSendDataLen;i++)
    {
        h = HexToBcd(TCP_SendBuf[i]/0x10);
        Send_a_byte(h);
        l = HexToBcd(TCP_SendBuf[i]%0x10);
        Send_a_byte(l);
    }
    
    Send_a_byte('"');
    Send_a_byte(0x0D);
    Send_a_byte(0x0A);

    t  = 0;
    while(Check_string_rxed("OK") == 0)
    {
        Sleep(10); 
        //WDog();
        
        if(Check_string_rxed("ERROR") == 1) 
        {
            Sleep(200);
            DebugMsg();
            Clear_Tcp_RecBuf();
    
            return FALSE;
        }
        
        t ++;
        if (t > 3000)  // 30s
        {
            DebugMsg();
            Clear_Tcp_RecBuf();
            CmdTimeOutHandle();
            return TIMEOUT;
        }
    }

    //DebugMsg();
    Clear_Tcp_RecBuf();

    return TRUE;
}


