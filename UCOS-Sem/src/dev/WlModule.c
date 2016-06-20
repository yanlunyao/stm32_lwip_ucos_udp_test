/*************************************************************
成都昊普环保技术有限公司   版权所有
无线采集器


文件名:   
作  者:    潘国义
描  述:    无线模块
修订记录:

**************************************************************/
#include "main.h"
#include "Comm.h"
#include "Hw_memmap.h"
#include "gpio.h"
#include "HW_gpio.h"
#include "WlModule.h"
#include "main.h"
#include "Cdma1x.h"
#include "Sms.h"
#include "Gsm.h"
#include "devTimer.h"
#include "dev.h"
#include "devTcpip.h"
#include "Scom.h"
#include "DLT645.h"
//extern BYTE SysParam[];
BYTE SignPower = 0;
extern BYTE SwUpdating ;
extern BYTE CurTcpLinkId;
extern BYTE NeedConnSrv;
extern BYTE StartLoc  ;
extern BYTE HeartPackArrived;
extern BYTE NoCenter ;          // 没有回报中心
extern BYTE NeedSleep ;        // 是否睡眠   采集器启动后网络处于休眠状态
extern BYTE NeedReConnect  ;  // 需要重新汇报心跳包
extern BYTE NeedConnSrv  ;   // 是否连接到服务器



//无线模块工作状态变量
uchar NetModulState=InitState;


void Init_NetModul()
{
    static BYTE time;
    static BYTE RetryCnt;

    RetryCnt = 0;

Retry:
    DebugStr("\nInit Module\r\n");

    PowerOn_Modul();   
    if (!Check_Modul())
    {
        MOD_POWEROFF();
            
        DebugStr("Check_Modul Error\r\n");
        SetLastError(ERR_MODULEFAIL);

        if (SysParam[SP_MODTYPE] == CDMA_MC323)
        {
            SysParam[SP_MODTYPE] = GSM_MG323;
        }
        else
        {
            SysParam[SP_MODTYPE] = CDMA_MC323;
        }

        Sleep(2000);

        RetryCnt ++;

        if (RetryCnt < 2)
        {
            goto Retry;
        }

        #ifdef CDMA_SRVMODE
        //SysReset();
        //这里不能复位，直接休眠等待下一个周期唤醒
        NeedSleep = 1;
        return;
        #else
        SysReset();
        //NetModulState=InitState
        #endif
    }

    DebugMsg();

    DebugStr("Check_Net\r\n");
    if (!Check_Net(1))
    {
        DebugStr("Check_Net Error\r\n");
        SetLastError(ERR_GSMFAIL);
        SysReset();
    }

    DebugMsg();

    //wait 30s for module ok
    time = 0;
    while(time<60)
    {
        time++;
        Sleep(500);   
    }

    // 从CDMA模块取时间 并设置 实时钟
    if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        if (Get_Modul_Time())
        {
            SetSysTime();
        }
        else
        {
            SetLastError(ERR_MODULEFAIL);
            SysReset();
        }
    }

    ATCmd_GetVersion();

    //删除所有短信，防止垃圾短信
    Del_all_SMS();
    DebugMsg();

    if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        // 不上报信号强度
        DisableRep();
        DebugMsg();
    }
   NetModulState=InitEndState; 
}

void PowerOn_Modul(void)
{
    
    if(SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        Sleep(3000);
        MOD_POWERON();  // Open Power Ctl
        Sleep(2000);
        // Module Reset
        DebugStr("Mod Reset\r\n");

        MOD_PDLOW();
        Sleep(1000);
        MOD_RESETLOW();
        Sleep(100);
        MOD_RESETHIGH();
        Sleep(200);

        // Module Power On
        MOD_PDHIGH();
    }
    else
    {
        Sleep(3000);
     
        MOD_POWERON();  // Open Power Ctl
        Sleep(2000);

        // Module Reset
        DebugStr("Mod Reset\r\n");
        MOD_RESETLOW();
        Sleep(1000);
        MOD_RESETHIGH();
        Sleep(2000);

        // Module Power On
        MOD_PDLOW();
        Sleep(2000);
        MOD_PDHIGH();
    }
    
}

void PowerOff_Modul()
{
    static BYTE i;
    // Module Power Off
    MOD_PDLOW();
    OSTimeDlyHMSM(0,0,2,100);
    MOD_PDHIGH();

    i = 0;
    while(1)
    {
        if (Check_string_rxed("^SHUTDOWN") == 1)   // CDMA 模式
        {
            MOD_POWEROFF();
            return;
        }

        Sleep(1000);

        i++;
        if (i > 20)
        {
            MOD_POWEROFF();
            return;
        }
        
    }
}

BYTE Check_Modul(void)
{
    static BYTE Ec;

    Ec = 0;
    
    while(1)
    {
        if (Check_string_rxed("^MODE:2") == 1)   // CDMA 模式
        {
            SysParam[SP_MODTYPE] = CDMA_MC323;
            DebugStr("Checked Cdma module\r\n");
//            armLed(0);
            return TRUE;
        }

        if (Check_string_rxed("^SYSSTART") == 1)   // GSM
        {
            
            SysParam[SP_MODTYPE] = GSM_MG323;
            DebugStr("Checked Gsm module\r\n");

            return TRUE;
        }
      
        Sleep(2000);
       
        Ec ++;
        if (Ec > 30) 
        {
            return FALSE;
        }
    }
   
}


BYTE Check_Net(BYTE FLed)
{
    if (SysParam[SP_MODTYPE] == GSM_MG323)
    {
        return Check_GsmNet(FLed);
    }
    else if (SysParam[SP_MODTYPE] == CDMA_MC323)
    {
        return Check_CdmaNet(FLed);
    }
    return TRUE;
}


BYTE ATCmd_GetVersion(void)
{
    static BYTE ret;
    
    ret = Send_AT_Cmd("AT+GMR");

    DebugMsg();
    Clear_Sms_buf();
        
    return ret;
}
BYTE GetSignPower(void)
{
    static BYTE ret,l1,l2,len;
    
    ret = Send_AT_Cmd("AT+CSQ");
    if (ret != TRUE)
    {
        return FALSE;
    }

    DebugMsg();

    if (Check_string_rxed("+CSQ:") != 1) 
    {
        return FALSE;
    }

    l1 = (BYTE)GetSignLoc(':', 1);
    if (l1 == 0xFF)
    {
        return FALSE;
    }

    //去掉空格- G网模块有空格
    if (TCP_RecBuf[l1+1] == ' ')
    {
        l1++;
    }

    l2 = (BYTE)GetSignLoc(',', 1);
    if (l2 == 0xFF)
    {
        return FALSE;
    }

    len = l2-l1-1;
    if (len > 2)
    {
        return FALSE;
    }

    if (!StrToByte(&TCP_RecBuf[l1+1], len, &SignPower))
    {
        SignPower = 0;
        return FALSE;
    }

    if (SignPower == 99)
    {
        SignPower = 0;
    }
    else
    {
        SignPower *= 3;
    }

//    DebugByte(SignPower);
    
    //DebugMsg();
    //Clear_Sms_buf();
    
    return TRUE;
}

void Del_all_SMS(void)
{
    Send_AT_Cmd("AT+CMGD=1,4");

}
void CmdTimeOutHandle(void)
{
    static BYTE Cnt = 0;
    DebugStr("AtCmd timeout\r\n");
    
    Cnt ++;
    if (Cnt > 3)
    {
        DebugStr("Cmd timeout>3, resetting ... \r\n");
        SysReset();
        //NetModulState =InitState ;
    }
}

#ifndef CDMA_SRVMODE
BYTE CommHandle(void)
{
    static BYTE ret; 

    SwUpdating = 0;
    
    ret = RepHeartPack();
    if (ret == FALSE)
    {
        Close_TcpIpLink();
        return FALSE;
    }
    
    ret = HandleTcpFrame(ret);
    if (ret == FALSE)
    {
        Close_TcpIpLink();
        return FALSE;
    }

    Sleep(2000);
    Close_TcpIpLink();
    return TRUE;
}
#endif


/***********************************************************************/
//获取模块数据
void taskGSMGetMsg(void* msg)
{
    printf ( "\r\nGSMUART0 Start..." );
    while(1)
    {

        Uart0_Buffer[Uart0_Sav_Num++] = commBaseGetChar( 0 );            
        //printf("%c",Uart0_Buffer[Uart0_Sav_Num-1]);

        if ( Uart0_Sav_Num >=  UART0_BUFLEN )
        {
            ClearUart0Buf();
        } 
    }
}

/**************************************************************************/
/**************************************************************************/
void taskGSM(void* msg)
{
    static BYTE ret=0,h=0;
    static BYTE ConnErrCnt;
    static WORD time=0;
    static BYTE HeartPackErr;
    static BYTE Sleeping = 0;
    static DWORD SleepTimer = 0;
    static BYTE Frm = 0;
    OSTimeDlyHMSM(0, 0, 3, 0);
    while(1)
    {
       
        OSTimeDlyHMSM(0, 0, 0, 950);
        time++;
    
        switch(NetModulState)
        {
            case InitState:
                Init_NetModul();
                if(NetModulState ==InitEndState)
                {
                    DebugStr("GSM Init End\r\n");
                    NetModulState = CheckIpModState;
                }
                else
                {
                    OSTimeDlyHMSM(0,0,30,0);
                }
            break;

            case CheckIpModState:
               if ((SysParam[SP_DEVIPMODE] == IPMODE_DHCP) || \
                    (SysParam[SP_DEVIPMODE] == IPMODE_PPPOE))
                {
                    NetModulState = LoginSvr;
                }
                else
                {
                    NetModulState = NomState;
                }
            break;
            case LoginSvr:
                //上电连接服务器
                // 使用Socket通信 南京高信
                if (SysParam[SP_ENABLESOCKET] == 1)
                {
                    Frm = RepHeartPack();
                    
                    if (Frm != FALSE)
                    {
                        HandleTcpFrame(Frm);
                        NeedSleep = 0;
                        HeartPackErr = 0;
                        NeedReConnect = 0;
                    }
                    else
                    {
                        NeedReConnect = 1;
                        SetLastError(ERR_HEARTFAIL);
                        HeartPackErr ++;
                        // 10分钟后重试
                        // 3次失败后休眠
                         if (HeartPackErr > 2)
                         {
                              HeartPackErr = 0;
                              SysReset();
                              NeedSleep = 1;
                          }
                    }
                    DebugMsg();
            
                }             
                NetModulState = NomState;
            break;

            default:
            break;
            
        }

     
        if((time%5==0)&&(NetModulState != InitState))
        {
           
            Hang_Call();  // 挂断来电
            SmsMsgHandle();//短信处理
            //GetSignPower();//得到信号强度
            
        }
        
        //一个小时轮检
        if ((time % 3600) == 0)  
        {
            Task_1h();
            
            h++;
            if (h>=24)  
            {
                time = 0;  // time 记录周期为1天
                h = 0;
            }
        }
        //1s
        if ((SysParam[SP_ENABLESOCKET] == 1)&&(NetModulState != InitState))
        {
            if (1 == NeedSleep)
            {
                SysSleep();
                #ifdef CDMA_SRVMODE
                Sleeping = 1;
                SleepTimer = 0;
                #endif
                NeedSleep = 0;
            }

            #ifdef CDMA_SRVMODE
            if (1 == Sleeping)  // 正在休眠
            {
                SleepTimer ++;
                if (SleepTimer > *(DWORD *)&SysParam[SP_SLEEPTIME])   // 唤醒
                {                    
                    Sleeping = 0;
                    //NetModulState = InitState;
                    SysReset();
                }
                Sleep(90);
                continue;
            }
            //
            if (Query_Tcp_Data())
            {
                ret = (BYTE)GetSignLoc(':',1);
                if (ret != 0xFF)
                {
                    CurTcpLinkId = TCP_RecBuf[ret+1] - 0x30;
                    if ((CurTcpLinkId > 0) && (CurTcpLinkId < 6))
                    {
                        //ServerHandle();
                        //HandleTcpFrame();
                    }
                }
                else
                {
                    DebugMsg();
                    //Clear_Tcp_RecBuf();
                }
            }
            else
            {
                if (TCP_RecBufLen > 0)
                {
                    
                    Clear_Tcp_RecBuf();
                }
            }
                
            // 两分钟检测一下网络状态
            if ((time % 120) == 0)
            {
                if (!Init_TCPIP())
                {
                    SetLastError(ERR_TCPIPFAIL);
                    SysReset();
                    //NetModulState = InitState;
                }
            }
            
            #endif
         
            if (SysParam[SP_DEVIPMODE] == IPMODE_DHCP)
            {
                //上报心跳包
                if (*(WORD *)&SysParam[SP_HEARTTIME] > 0) 
                {
                    if (((time % *(WORD *)&SysParam[SP_HEARTTIME]) == 0)  ||   // 定时汇报心跳包
                        ((NeedReConnect == 1) & ((time % 600) == 0)) ||          // 10分钟后重新连接
                        (NeedConnSrv == 1)      // 短信唤醒
                        )
                    {
                        NeedConnSrv = 0;
                        
                        NetModulState = LoginSvr;
                        printf("heartpack...\r\n");
                        if (GetSignPower())//得到信号强度
                        {
                            DebugMsg();
                        }
                       
                    }
                }
            }
        }                      
   }
}

/**************************************************************************/
/**************************************************************************/


