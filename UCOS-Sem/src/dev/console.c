/*************************************************************
成都昊普环保技术有限公司   版权所有


文件名:       Console.c
作  者:       潘国义   
描  述:       串口控制台
修订记录:

**************************************************************/


/*
串口控制台功能设计
1.设置设备ID
2.设置485速率
3.设置485校验
3.设置服务器IP地址和端口
4.IO自检
*/
#include "includes.h"
#include "Comm.h"
#include "console.h"
#include "devTcpip.h"
#include "devTimer.h"
#include "dev.h"
#include "rn8209x.h"
#include "devi2c.h"
extern BYTE Rs232_Buffer[];
extern BYTE Rs232_Sav_Num;

extern BYTE DevVersion[];   // 设备版本
//extern const BYTE DevRelTime[];
//extern BYTE SysParam[];
extern BYTE SysTime[];
extern BYTE IpAddrStr[];
extern BYTE PortStr[];
extern char Tmp0;
extern WORD SysAlarm;

extern BYTE Rs485_Buffer[];
extern BYTE Rs485_Sav_Num;

extern BYTE DevAddrTmp[];


//extern BYTE RecFrame_Buffer[];
extern BYTE SendFrame_Buffer[];
extern BYTE TcpSendDataLen;

// RunTime;
extern BYTE Ac1Rt;   
extern BYTE Ac2Rt;
extern BYTE DevRt;

//StopTime
extern BYTE Ac1St;   
extern BYTE Ac2St;
extern BYTE DevSt;

//PowerOn
extern BYTE Ac1PowerOn;
extern BYTE Ac2PowerOn;
extern BYTE DevPowerOn;

extern BYTE NeedConnSrv;
static BYTE CheckMetering=0;
static BYTE SETADRESSFLAG=0;


BYTE CmdStr[100] = {0};
BYTE CmdType = CMD_NONE;

void PrintStr(const char * Msg)  
{                    
    printf("%s",Msg);        
}

void PrintChar(BYTE ch)
{
    Rs232SendByte(ch);
}

void PrintByte(BYTE by)
{
    if (by > 99)
    {
        Rs232SendByte(by/100+0x30);
        Rs232SendByte(by%100/10+0x30);
        Rs232SendByte(by%10+0x30);
    }
    else if (by > 9)
    {
        Rs232SendByte(by/10+0x30);
        Rs232SendByte(by%10+0x30);
    }
    else
    {
        Rs232SendByte(by+0x30);
    }
}
void PrintMeterAdress(void)
{
    printf("NO1:%02d%02d%02d%02d%02d%02d;",SysParam[SP_METERADDR1+5],SysParam[SP_METERADDR1+4],\
    SysParam[SP_METERADDR1+3],SysParam[SP_METERADDR1+2],SysParam[SP_METERADDR1+1],SysParam[SP_METERADDR1]);

    printf("NO2:%02d%02d%02d%02d%02d%02d;",SysParam[SP_METERADDR2+5],SysParam[SP_METERADDR2+4],\
    SysParam[SP_METERADDR2+3],SysParam[SP_METERADDR2+2],SysParam[SP_METERADDR2+1],SysParam[SP_METERADDR2]);
   
    printf("NO3:%02d%02d%02d%02d%02d%02d",SysParam[SP_METERADDR3],SysParam[SP_METERADDR3+1],\
    SysParam[SP_METERADDR3+3],SysParam[SP_METERADDR3+2],SysParam[SP_METERADDR3+1],SysParam[SP_METERADDR3]);
    printf(" *\r\n");
}

void PrintVersion(void)
{
    static WORD temp;
    
    PrintStr("*****************************************************\r\n");
    PrintStr("*HOPEP Wrieless Multi-loop Meter ");
    
    PrintChar('V');
    PrintChar(DevVersion[2]);
    PrintChar('.');
    PrintChar(DevVersion[4]);
    PrintChar(' ');
	
    PrintChar('2');
    PrintChar('0');

    temp = (WORD)DevRelTime[6];
    temp = (temp << 8) + DevRelTime[5];
    PrintChar((temp%100)/10+0x30);  // 年
    PrintChar((temp%100)%10+0x30);
    PrintChar('-');

    PrintChar(DevRelTime[4]/10+0x30);  // 月
    PrintChar(DevRelTime[4]%10+0x30);
    PrintChar('-');
    PrintChar(DevRelTime[3]/10+0x30);  // 日
    PrintChar(DevRelTime[3]%10+0x30);
    PrintStr("    *\r\n");
    PrintStr("*                                                   *\r\n");
    PrintChar('*');
    PrintMeterAdress();
    PrintStr("*****************************************************\r\n");
}


void PrintDevId(void)
{
    static WORD temp;
    //SysParam[SP_DEVADDR]
    PrintStr("1.Device ID      : ");  
    PrintChar(SysParam[SP_DEVADDR+2]/16+0x30);
    PrintChar(SysParam[SP_DEVADDR+2]%16+0x30);
    PrintChar(SysParam[SP_DEVADDR+1]/16+0x30);
    PrintChar(SysParam[SP_DEVADDR+1]%16+0x30);
    PrintChar(SysParam[SP_DEVADDR]/16+0x30);
    PrintChar(SysParam[SP_DEVADDR]%16+0x30);   
    
    temp = (WORD)SysParam[SP_DEVADDR+4];
    temp = (temp << 8) + SysParam[SP_DEVADDR+3];
    PrintChar(temp/1000+0x30);
    temp = temp%1000;
    PrintChar(temp/100+0x30);
    temp = temp%100;
    PrintChar(temp/10+0x30);
    PrintChar(temp%10+0x30);
    PrintStr("\r\n");

}
void PrintBaudRate(void)
{
    // SP_BAUDRATE  
    // 00:150 01:300;02:600;03:1200;04:2400;05:4800;06:9600;07:19200;08:38400;09:57600,10:115200
    PrintStr("2.Rs485 BaudRate : ");  
    if (SysParam[SP_BAUDRATE] == 0)
    {
        PrintStr("150"); 
    }
    else if (SysParam[SP_BAUDRATE] == 1)
    {
        PrintStr("300"); 
    }
    else if (SysParam[SP_BAUDRATE] == 2)
    {
        PrintStr("600"); 
    }
    else if (SysParam[SP_BAUDRATE] == 3)
    {
        PrintStr("1200"); 
    }
    else if (SysParam[SP_BAUDRATE] == 4)
    {
        PrintStr("2400"); 
    }
    else if (SysParam[SP_BAUDRATE] == 5)
    {
        PrintStr("4800"); 
    }
    else if (SysParam[SP_BAUDRATE] == 6)
    {
        PrintStr("9600"); 
    }
    else if (SysParam[SP_BAUDRATE] == 7)
    {
        PrintStr("19200"); 
    }
    else if (SysParam[SP_BAUDRATE] == 8)
    {
        PrintStr("38400"); 
    }
    else if (SysParam[SP_BAUDRATE] == 9)
    {
        PrintStr("57600"); 
    }
    else if (SysParam[SP_BAUDRATE] == 10)
    {
        PrintStr("115200"); 
    }
    else
    {
        PrintStr("Invalid."); 
    }
    PrintStr("\r\n");
}

void PrintCheckBit(void)
{
    PrintStr("3.Rs485 Check    : ");
    if (SysParam[SP_CHECKBIT] == CHECK_EVEN)
    {
        PrintStr("Even"); 
    }
    else if (SysParam[SP_CHECKBIT] == CHECK_ODD)
    {
        PrintStr("Odd"); 
    }
    else if (SysParam[SP_CHECKBIT] == CHECK_NONE)
    {
        PrintStr("None"); 
    }
    else
    {
        PrintStr("Invalid."); 
    }   
    PrintStr("\r\n");
}


void PrintSrvIp(void)
{
    static BYTE i,t;
    //SP_SERVERIP
    PrintStr("4.Server IP      : ");
    for (i=0;i<4;i++)
    {
        t = SysParam[SP_SERVERIP+i];
        if(t>99)
        {
            PrintChar(t/100+0x30);
            PrintChar(t%100/10+0x30);
            PrintChar(t%10+0x30);
        }
        else if (t>9)
        {
            PrintChar(t/10+0x30);
            PrintChar(t%10+0x30);
        }
        else
        {
            PrintChar(t+0x30);
        }
        if (i!=3)
        {
            PrintChar('.');
        }
    }
    PrintStr("\r\n");
}

void PrintSrvPort(void)
{
    static WORD Port;
    //SP_SERVERPORT
    PrintStr("5.Server Port    : ");
    
    Port = *(WORD *)&SysParam[SP_SERVERPORT];
    if (Port>9999)
    {
        PrintChar(Port/10000+0x30);
        Port = Port%10000;
        PrintChar(Port/1000+0x30);
        Port = Port%1000;
        PrintChar(Port/100+0x30);
        Port = Port%100;
        PrintChar(Port/10+0x30);
        Port = Port%10;
        PrintChar(Port+0x30);
    }
    else if (Port>999)
    {
        PrintChar(Port/1000+0x30);
        Port = Port%1000;
        PrintChar(Port/100+0x30);
        Port = Port%100;
        PrintChar(Port/10+0x30);
        Port = Port%10;
        PrintChar(Port+0x30);
    }
    else if (Port>99)
    {
        PrintChar(Port/1000+0x30);
        Port = Port%1000;
        PrintChar(Port/100+0x30);
        Port = Port%100;
        PrintChar(Port/10+0x30);
        Port = Port%10;
        PrintChar(Port+0x30);
    }
    else if (Port > 9)
    {
        PrintChar(Port/10+0x30);
        Port = Port%10;
        PrintChar(Port+0x30);
    }
    else
    {
        PrintChar(Port+0x30);
    }
    
    PrintStr("\r\n");
}



void PrintLocIp(void)
{
    static BYTE i,t;
    //
    
    PrintStr("6.Local IP       : ");
    for (i=0;i<4;i++)
    {
        t = SysParam[SP_LOCALIP+i];
        if(t>99)
        {
            PrintChar(t/100+0x30);
            PrintChar(t%100/10+0x30);
            PrintChar(t%10+0x30);
        }
        else if (t>9)
        {
            PrintChar(t/10+0x30);
            PrintChar(t%10+0x30);
        }
        else
        {
            PrintChar(t+0x30);
        }
        if (i!=3)
        {
            PrintChar('.');
        }
    }
    PrintStr("\r\n");
}

void PrintLocPort(void)
{
    static WORD Port;
    //
    PrintStr("7.Local Port     : ");
    Port = *(WORD *)&SysParam[SP_LOCALPORT];
   
    if (Port>9999)
    {
        PrintChar(Port/10000+0x30);
        Port = Port%10000;
        PrintChar(Port/1000+0x30);
        Port = Port%1000;
        PrintChar(Port/100+0x30);
        Port = Port%100;
        PrintChar(Port/10+0x30);
        Port = Port%10;
        PrintChar(Port+0x30);
    }
    else if (Port>999)
    {
        PrintChar(Port/1000+0x30);
        Port = Port%1000;
        PrintChar(Port/100+0x30);
        Port = Port%100;
        PrintChar(Port/10+0x30);
        Port = Port%10;
        PrintChar(Port+0x30);
    }
    else if (Port>99)
    {
        PrintChar(Port/1000+0x30);
        Port = Port%1000;
        PrintChar(Port/100+0x30);
        Port = Port%100;
        PrintChar(Port/10+0x30);
        Port = Port%10;
        PrintChar(Port+0x30);
    }
    else if (Port > 9)
    {
        PrintChar(Port/10+0x30);
        Port = Port%10;
        PrintChar(Port+0x30);
    }
    else
    {
        PrintChar(Port+0x30);
    }
    
    PrintStr("\r\n");
}

#if 0
void PrintOutSt(void)
{
    PrintStr("Output Status : ");
    PrintStr("AC1-");
    if (AC1_SW())
    {
        PrintStr("On;");
    }
    else
    {
        PrintStr("Off;");
    }
    PrintStr("AC2-");
    if (AC2_SW())
    {
        PrintStr("On;");
    }
    else
    {
        PrintStr("Off;");
    }

    PrintStr("DEV-");
    if (DEV_SW())
    {
        PrintStr("On");
    }
    else
    {
        PrintStr("Off");
    }
    
    PrintStr("\r\n");
}

void PrintInSt(void)
{
    PrintStr("Input Status : ");
    PrintStr("AC1-");
    if (AC1_STATUS() == 0)
    {
        PrintStr("On;");
    }
    else
    {
        PrintStr("Off;");
    }

    PrintStr("AC2-");
    if (AC2_STATUS() == 0)
    {
        PrintStr("On;");
    }
    else
    {
        PrintStr("Off;");
    }

    PrintStr("DEV-");
    if (DEV_STATUS() == 0)
    {
        PrintStr("On");
    }
    else
    {
        PrintStr("Off");
    }

    
    PrintStr("\r\n");
}
#endif
void PrintDate(void)
{
    
    PrintStr("8.System Date    : 20");
    PrintChar(SysTime[0]/16+0x30);
    PrintChar(SysTime[0]%16+0x30);
    PrintChar('-');
    PrintChar(SysTime[2]/16+0x30);
    PrintChar(SysTime[2]%16+0x30);
    PrintChar('-');
    PrintChar(SysTime[3]/16+0x30);
    PrintChar(SysTime[3]%16+0x30);
    
    PrintStr("\r\n");
}

void PrintTime(void)
{
    
    PrintStr("9.System Time    : ");
    PrintChar(SysTime[4]/16+0x30);
    PrintChar(SysTime[4]%16+0x30);
    PrintChar(':');
    PrintChar(SysTime[5]/16+0x30);
    PrintChar(SysTime[5]%16+0x30);
    PrintChar(':');
    PrintChar(SysTime[6]/16+0x30);
    PrintChar(SysTime[6]%16+0x30);
    
    PrintStr("\r\n");
}


#if 0
void PrintTmp(void)
{
    static BYTE tmp;
    PrintStr("A.Temperature    : ");
    if (GetTmp())
    {
        if (Tmp0>99)
        {
            PrintChar(Tmp0/100+0x30);
            PrintChar(Tmp0%100/10+0x30);
            PrintChar(Tmp0%10+0x30);
        }
        else if (Tmp0>9)
        {
            PrintChar(Tmp0/10+0x30);
            PrintChar(Tmp0%10+0x30);
        }
        else if (Tmp0>(char)-1)
        {
            PrintChar(Tmp0+0x30);
        }
        else if (Tmp0>(char)-10)
        {
            tmp = 255 - Tmp0 + 1;
            PrintChar('-');
            PrintChar(tmp+0x30);
        }
        else
        {
            tmp = 255 - Tmp0 + 1;
            PrintChar('-');
            PrintChar(tmp/10+0x30);
            PrintChar(tmp%10+0x30);
        }
    }
    else
    {
        PrintStr("None.");
    }
    PrintStr("\r\n");
}

void PrintHum(void)
{
    static BYTE hum;
    
    PrintStr("B.Humidity       : ");
    hum = GetHum();
    if (hum > 100)
    {
        PrintStr("None.");
    }
    else if (hum>99)
    {
        PrintChar(hum/100+0x30);
        PrintChar(hum%100/10+0x30);
        PrintChar(hum%10+0x30);
        PrintChar('%');
    }
    else if (hum>9)
    {
        PrintChar(hum/10+0x30);
        PrintChar(hum%10+0x30);
        PrintChar('%');
    }
    else
    {
        PrintChar(hum+0x30);
        PrintChar('%');
    }
    PrintStr("\r\n");
}
#endif

void PrintCheckHF(void)
{
    
}
void PrintDebug(void)
{
    PrintStr("Debug Info:");
    if(SysParam[SP_DEBUGMSG] == DEBUG_DISABLE)
    {
        SysParam[SP_DEBUGMSG] = DEBUG_ENABLE;
        PrintStr("Enable.");
    }
    else
    {
        SysParam[SP_DEBUGMSG] = DEBUG_DISABLE;
        PrintStr("Disable.");
    }
                    
   
    PrintStr("\r\n");
}
#if 0
void PrintAlarm(void)
{
    static BYTE i;
    PrintStr("Sys Alarm:");
    for (i=0;i<ALM_COUNT;i++)
    {
        if ((SysAlarm&(1<<(ALM_COUNT-1-i))) == 0)
        {
            PrintChar('0');
        }
        else
        {
            PrintChar('1');
        }
    }
    PrintStr("\r\n");
}

void PrintBeep(void)
{
    PrintStr("Beep Info:");
    if(SysParam[SP_BEEP_ENABLE] == ENABLE)
    {
        SysParam[SP_BEEP_ENABLE] = DISABLE;
        PrintStr("Disable.");
    }
    else
    {
        SysParam[SP_BEEP_ENABLE] = ENABLE;
        PrintStr("Enable.");
    }
                    
   
    PrintStr("\r\n");
}


void PrintUnionStatus(void)
{
    PrintByte(Ac1Rt);
    PrintStr(" ");
    PrintByte(Ac1St);
    PrintStr(" ");
    PrintByte(Ac2Rt);
    PrintStr(" ");
    PrintByte(Ac2St);
    PrintStr(" ");
    PrintByte(DevRt);
    PrintStr(" ");
    PrintByte(DevSt);
    PrintStr(" ");
    PrintStr("\r\n");
    PrintChar(Ac1PowerOn+0x30);
    PrintStr(" ");
    PrintChar(Ac2PowerOn+0x30);
    PrintStr(" ");
    PrintChar(DevPowerOn+0x30);
    PrintStr(" ");
    PrintStr("\r\n");
    
}
#endif
void PrintMenu(void)
{
    GetSysTime();
    
    PrintStr("\r\n");
    PrintVersion();
    PrintDevId();   
    PrintBaudRate();
    PrintCheckBit();
    PrintSrvIp();
    PrintSrvPort();
    PrintLocIp();
    PrintLocPort();
    //PrintOutSt();
    //PrintInSt();
    PrintDate();
    PrintTime();
//    PrintTmp();
//    PrintHum();
    PrintDebug();

    //软件调试用，不展示给用户
    //PrintStr("P.Set Parameter\r\n");
    //PrintStr("Q.Read Parameter\r\n");
    //PrintStr("H.Input Meter Check Value");
    PrintStr("S.Save Parameter\r\n");
    PrintStr("R.Reset\r\n");
    
    
    PrintStr("\r\n");
    CmdType = CMD_MAIN_MENU;
    
    PrintStr("Please Select:");
    
}

void PrintMeterMenu(void)
{
    PrintStr("\r\n");
    PrintVersion();
    PrintStr("A.Set Meter Adress\r\n");
    PrintStr("S.Check Self   \r\n");
    PrintStr("I.Check Current\r\n");
    PrintStr("V.Check Voltage\r\n");
    PrintStr("P.Check Power  \r\n");
    PrintStr("E.Check Energy \r\n");
    PrintStr("C.Clear Eeprom Energy \r\n");
    PrintStr("R.Read  Eeprom Energy \r\n");
    PrintStr("G.graw channel \r\n");
    PrintStr("Q.Quit \r\n");
    PrintStr("\r\n");
    PrintStr("Please Select:");
    CmdType = CMD_CHECK_METER;
}


void SetDevID(void)
{
    static BYTE i;
    static WORD id;
    if (Rs232_Sav_Num != 11)
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    for (i=0;i<10;i++)
    {
        if ((Rs232_Buffer[i] > '9') || (Rs232_Buffer[i] < '0'))
        {
            PrintStr("\r\nInput Invalid.\r\n");
            return;
        }
    }

    SysParam[SP_DEVADDR+2] = (Rs232_Buffer[0]-0x30)*16 + (Rs232_Buffer[1]-0x30);
    SysParam[SP_DEVADDR+1] = (Rs232_Buffer[2]-0x30)*16 + (Rs232_Buffer[3]-0x30);
    SysParam[SP_DEVADDR]   = (Rs232_Buffer[4]-0x30)*16 + (Rs232_Buffer[5]-0x30);

    id  = (WORD)(Rs232_Buffer[6]-0x30)*1000; 
    id += (WORD)(Rs232_Buffer[7]-0x30)*100;
    id += (WORD)(Rs232_Buffer[8]-0x30)*10;
    id += (WORD)(Rs232_Buffer[9]-0x30);
    
    SysParam[SP_DEVADDR+3] =  (BYTE)id;
    SysParam[SP_DEVADDR+4] =  (BYTE)(id>>8);

    mcpy(DevAddrTmp,&SysParam[SP_DEVADDR],5);

    PrintStr("\r\nSet Success.\r\n");
}

void SetBaudRate(void)
{
    if (Rs232_Sav_Num > 3)
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    if (Rs232_Sav_Num == 3)
    {
        if ((Rs232_Buffer[0] != '1') || (Rs232_Buffer[1] != '0'))
        {
            PrintStr("\r\nInput Invalid.\r\n");
            return;
        }
        else
        {
            SysParam[SP_BAUDRATE] = BAUDRATE_115200;
            PrintStr("\r\nSet Success.\r\n");
        }
    }

    if (Rs232_Sav_Num == 2)
    {
        if ((Rs232_Buffer[0] < '0') || (Rs232_Buffer[0] > '9'))
        {
            PrintStr("\r\nInput Invalid.\r\n");
            return;
        }
        else
        {
            SysParam[SP_BAUDRATE] = Rs232_Buffer[0] - 0x30;
            PrintStr("\r\nSet Success.\r\n");
        }
    }

    
}


void SetCheckBit(void)
{
    //01:奇校验;02:偶校验;03无校验
    if (Rs232_Sav_Num != 2)
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    if ((Rs232_Buffer[0] < '1') || (Rs232_Buffer[0] > '3'))
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }
    else
    {
        SysParam[SP_CHECKBIT] = Rs232_Buffer[0] - 0x30;
        PrintStr("\r\nSet Success.\r\n");
    }
}

#if 0
void ChangeOutput(void)
{
    static BYTE Out = 0;
    Out ++;
    if (Out > 3)
    {
        Out = 0;
    }

    switch(Out)
    {
        case 0:
            AC1_OFF();
            AC2_OFF();
            DEV_OFF();
        break;

        case 1:
            AC1_ON();
            AC2_OFF();
            DEV_OFF();
        break;

        case 2:
            AC1_OFF();
            AC2_ON();
            DEV_OFF();
        break;

        case 3:
            AC1_OFF();
            AC2_OFF();
            DEV_ON();
        break;
    }
    
        
}
#endif
void SetSrvIp(void)
{
    if ((Rs232_Sav_Num < 8) || (Rs232_Sav_Num > 16))
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    mset(IpAddrStr,0,16);
    mcpy(IpAddrStr,Rs232_Buffer,Rs232_Sav_Num-1);
    if (!CheckValidIpAddr())
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    if (!StrToIpAdd(&SysParam[SP_SERVERIP]))
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    PrintStr("\r\nSet Success.\r\n");
}

void SetSrvPort(void)
{
    if ((Rs232_Sav_Num < 2) || (Rs232_Sav_Num > 6))
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    mset(PortStr,0,6);
    mcpy(PortStr,Rs232_Buffer,Rs232_Sav_Num-1);
    if (!StrToPort(&SysParam[SP_SERVERPORT]))
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    PrintStr("\r\nSet Success.\r\n");
}


void SetLocIp(void)
{
    if ((Rs232_Sav_Num < 8) || (Rs232_Sav_Num > 16))
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    mset(IpAddrStr,0,16);
    mcpy(IpAddrStr,Rs232_Buffer,Rs232_Sav_Num-1);
    if (!CheckValidIpAddr())
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    if (!StrToIpAdd(&SysParam[SP_LOCALIP]))
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    PrintStr("\r\nSet Success.\r\n");
}

void SetLocPort(void)
{
    if ((Rs232_Sav_Num < 2) || (Rs232_Sav_Num > 6))
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    mset(PortStr,0,6);
    mcpy(PortStr,Rs232_Buffer,Rs232_Sav_Num-1);
    if (!StrToPort(&SysParam[SP_LOCALPORT]))
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    PrintStr("\r\nSet Success.\r\n");
}

void SetDate(void)
{
    static BYTE i,tmp;
    
    if (Rs232_Sav_Num != 11)
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    //Check valid Input
    for (i=0;i<Rs232_Sav_Num-1;i++)
    {
        if (Rs232_Buffer[i] != '-')
        {
            if ((Rs232_Buffer[i] > '9') || (Rs232_Buffer[i] < '0'))
            {
                PrintStr("\r\nInput Invalid.\r\n");
                return;
            }
        }
    }

    GetSysTime();

    tmp = (Rs232_Buffer[2]-0x30)*10 + (Rs232_Buffer[3]-0x30);
    SysTime[0] = tmp/10*16+tmp%10;

    tmp = (Rs232_Buffer[5]-0x30)*10 + (Rs232_Buffer[6]-0x30);
    if ((tmp > 12) || (tmp == 0))
    {
        PrintStr("\r\nInput Invalid.");
        return;
    }
    SysTime[2] = tmp/10*16+tmp%10;

    tmp = (Rs232_Buffer[8]-0x30)*10 + (Rs232_Buffer[9]-0x30);
    if ((tmp > 31) || (tmp == 0))
    {
        PrintStr("\r\nInput Invalid.");
        return;
    }
    SysTime[3] = tmp/10*16+tmp%10;

    SetSysTime();

    PrintStr("\r\nSet Success.\r\n");
}

void SetTime(void)
{
    static BYTE i,tmp;
    
    if (Rs232_Sav_Num != 9)
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    //Check valid Input
    for (i=0;i<Rs232_Sav_Num-1;i++)
    {
        if (Rs232_Buffer[i] != ':')
        {
            if ((Rs232_Buffer[i] > '9') || (Rs232_Buffer[i] < '0'))
            {
                PrintStr("\r\nInput Invalid.\r\n");
                return;
            }
        }
    }

    GetSysTime();

    tmp = (Rs232_Buffer[0]-0x30)*10 + (Rs232_Buffer[1]-0x30);
    if (tmp > 23)
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }
    SysTime[4] = tmp/10*16+tmp%10;

    tmp = (Rs232_Buffer[3]-0x30)*10 + (Rs232_Buffer[4]-0x30);
    if (tmp > 59)
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }
    SysTime[5] = tmp/10*16+tmp%10;

    tmp = (Rs232_Buffer[6]-0x30)*10 + (Rs232_Buffer[7]-0x30);
    if (tmp > 59)
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }
    SysTime[6] = tmp/10*16+tmp%10;

    SetSysTime();

    PrintStr("\r\nSet Success.\r\n");
}


void SetParam(void)
{
    static BYTE i,p,v,loc;
    
    if ((Rs232_Sav_Num < 4) || (Rs232_Sav_Num > 8))
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    loc = 0;
    for (i=0;i<Rs232_Sav_Num-1;i++)
    {
        if (Rs232_Buffer[i] != '=')
        {
            if ((Rs232_Buffer[i] > '9') || (Rs232_Buffer[i] < '0'))
            {
                PrintStr("\r\nInput Invalid.\r\n");
                return;
            }
        }
        else
        {
            loc = i;
        }
    }

    if (Rs232_Buffer[loc] != '=')
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }
        

    if ((loc < 1) || (loc > 3))
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    if (Rs232_Sav_Num - loc > 5)
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    if (loc == 1)
    {
        p = Rs232_Buffer[0] - 0x30;
    }
    else if (loc == 2)
    {
        p = (Rs232_Buffer[0] - 0x30) * 10 + 
            (Rs232_Buffer[1] - 0x30);
    }
    else if (loc == 3)
    {
        p = (Rs232_Buffer[0] - 0x30) * 100 + 
            (Rs232_Buffer[1] - 0x30) * 10 + 
            (Rs232_Buffer[2] - 0x30);
    }
    
    if (p > SYSPARAM_COUNT)
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }
    //PrintByte(loc);
    //PrintByte(Rs232_Sav_Num);
    if ((Rs232_Sav_Num - loc -1) == 2)
    {
        v = Rs232_Buffer[loc+1] - 0x30;
    }
    else if ((Rs232_Sav_Num - loc -1) == 3)
    {
        v = (Rs232_Buffer[loc+1] - 0x30)*10 +
            (Rs232_Buffer[loc+2] - 0x30);
    }
    else if ((Rs232_Sav_Num - loc -1) == 4)
    {
        v = (Rs232_Buffer[loc+1] - 0x30)*100 +
            (Rs232_Buffer[loc+2] - 0x30)*10 +
            (Rs232_Buffer[loc+3] - 0x30);
    }

    //PrintByte(v);
    //PrintStr("\r\n");

    SysParam[p] = v;
    PrintStr("\r\nSet Success.\r\n");
    
}

void ReadParam(void)
{
    static BYTE i,p;
    
    if ((Rs232_Sav_Num < 2) || (Rs232_Sav_Num > 4))
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    for (i=0;i<Rs232_Sav_Num-1;i++)
    {
        
        if ((Rs232_Buffer[i] > '9') || (Rs232_Buffer[i] < '0'))
        {
            PrintStr("\r\nInput Invalid.\r\n");
            return;
        }
        
    }

    if (Rs232_Sav_Num == 2)
    {
        p = Rs232_Buffer[0] - 0x30;
    }
    else if (Rs232_Sav_Num == 3)
    {
        p = (Rs232_Buffer[0] - 0x30) * 10 + 
            (Rs232_Buffer[1] - 0x30);
    }
    else if (Rs232_Sav_Num == 4)
    {
        p = (Rs232_Buffer[0] - 0x30) * 100 + 
            (Rs232_Buffer[1] - 0x30) * 10 + 
            (Rs232_Buffer[2] - 0x30);
    }

    if (p > SYSPARAM_COUNT)
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    PrintStr("\r\n");
    PrintStr("SysParam[");
    PrintByte(p);
    PrintStr("]=");
    PrintByte(SysParam[p]);
    PrintStr("\r\n");
}

void SetMeterHF(void)
{
    BYTE i;
    uint val=0;
    if ((Rs232_Sav_Num < 2) || (Rs232_Sav_Num > 6))
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }
    for (i=0;i<Rs232_Sav_Num-1;i++)
    {
        
        if ((Rs232_Buffer[i] > '9') || (Rs232_Buffer[i] < '0'))
        {
            PrintStr("\r\nInput Invalid.\r\n");
            return;
        }
        
    }
    
    if (Rs232_Sav_Num == 2)
        {
            val = Rs232_Buffer[0] - 0x30;
        }
        else if (Rs232_Sav_Num == 3)
        {
            val = (Rs232_Buffer[0] - 0x30) * 10 + 
                (Rs232_Buffer[1] - 0x30);
        }
        else if (Rs232_Sav_Num == 4)
        {
            val = (Rs232_Buffer[0] - 0x30) * 100 + 
                (Rs232_Buffer[1] - 0x30) * 10 + 
                (Rs232_Buffer[2] - 0x30);
        }
        else if (Rs232_Sav_Num == 5)
        {
            val = (Rs232_Buffer[0] - 0x30) * 1000 + 
                (Rs232_Buffer[1] - 0x30) * 100 + 
                (Rs232_Buffer[2] - 0x30)*10+
                (Rs232_Buffer[2] - 0x30);
        }
        else if (Rs232_Sav_Num == 6)
        {
            if((Rs232_Buffer[0] - 0x30) > 6)
            {
                PrintStr("\r\nInput Invalid.\r\n");
                return;   
            }
            val = (Rs232_Buffer[0] - 0x30) * 10000 + 
                (Rs232_Buffer[1] - 0x30) * 1000 +
                (Rs232_Buffer[2] - 0x30) * 100 +
                (Rs232_Buffer[3] - 0x30) * 10 +
                (Rs232_Buffer[4] - 0x30);
        }
        printf("val=%d ",val);
        for(i=0;i<9;i++)
        {
            Check_HFConst1(i,val);
        }
        
        SaveSysParam();
        PrintStr("\r\n");
}
void SetMeterKi(void)
{
    BYTE i;
    uint val=0;
    
    if ((Rs232_Sav_Num < 2) || (Rs232_Sav_Num > 6))
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }
    for (i=0;i<Rs232_Sav_Num-1;i++)
    {
        
        if ((Rs232_Buffer[i] > '9') || (Rs232_Buffer[i] < '0'))
        {
            PrintStr("\r\nInput Invalid.\r\n");
            return;
        }
        
    }
    
    if (Rs232_Sav_Num == 2)
        {
            val = Rs232_Buffer[0] - 0x30;
        }
        else if (Rs232_Sav_Num == 3)
        {
            val = (Rs232_Buffer[0] - 0x30) * 10 + 
                (Rs232_Buffer[1] - 0x30);
        }
        else if (Rs232_Sav_Num == 4)
        {
            val = (Rs232_Buffer[0] - 0x30) * 100 + 
                (Rs232_Buffer[1] - 0x30) * 10 + 
                (Rs232_Buffer[2] - 0x30);
        }
        else if (Rs232_Sav_Num == 5)
        {
            val = (Rs232_Buffer[0] - 0x30) * 1000 + 
                (Rs232_Buffer[1] - 0x30) * 100 + 
                (Rs232_Buffer[2] - 0x30)*10+
                (Rs232_Buffer[2] - 0x30);
        }
        else if (Rs232_Sav_Num == 6)
        {
            if((Rs232_Buffer[0] - 0x30) > 6)
            {
                PrintStr("\r\nInput Invalid.\r\n");
                return;   
            }
            val = (Rs232_Buffer[0] - 0x30) * 10000 + 
                (Rs232_Buffer[1] - 0x30) * 1000 +
                (Rs232_Buffer[2] - 0x30) * 100 +
                (Rs232_Buffer[3] - 0x30) * 10 +
                (Rs232_Buffer[4] - 0x30);
        }
        for(i=0;i<9;i++)
        {
            Check_KIA_KIB(i,val);
        }
        SaveSysParam();
        PrintStr("\r\nSet Success.\r\n");
}

void SetMeterKu(void)
{
    BYTE i;
    uint val=0;
    if ((Rs232_Sav_Num < 2) || (Rs232_Sav_Num > 6))
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }
    for (i=0;i<Rs232_Sav_Num-1;i++)
    {
        
        if ((Rs232_Buffer[i] > '9') || (Rs232_Buffer[i] < '0'))
        {
            PrintStr("\r\nInput Invalid.\r\n");
            return;
        }
        
    }
    
    if (Rs232_Sav_Num == 2)
        {
            val = Rs232_Buffer[0] - 0x30;
        }
        else if (Rs232_Sav_Num == 3)
        {
            val = (Rs232_Buffer[0] - 0x30) * 10 + 
                (Rs232_Buffer[1] - 0x30);
        }
        else if (Rs232_Sav_Num == 4)
        {
            val = (Rs232_Buffer[0] - 0x30) * 100 + 
                (Rs232_Buffer[1] - 0x30) * 10 + 
                (Rs232_Buffer[2] - 0x30);
        }
        else if (Rs232_Sav_Num == 5)
        {
            val = (Rs232_Buffer[0] - 0x30) * 1000 + 
                (Rs232_Buffer[1] - 0x30) * 100 + 
                (Rs232_Buffer[2] - 0x30)*10+
                (Rs232_Buffer[2] - 0x30);
        }
        else if (Rs232_Sav_Num == 6)
        {
            if((Rs232_Buffer[0] - 0x30) > 6)
            {
                PrintStr("\r\nInput Invalid.\r\n");
                return;   
            }
            val = (Rs232_Buffer[0] - 0x30) * 10000 + 
                (Rs232_Buffer[1] - 0x30) * 1000 +
                (Rs232_Buffer[2] - 0x30) * 100 +
                (Rs232_Buffer[3] - 0x30) * 10 +
                (Rs232_Buffer[4] - 0x30);
        }
        for(i=0;i<9;i++)
        {
            Check_KU(i,val);
        }
        SaveSysParam();
        PrintStr("\r\nSet Success.\r\n");
}

void SetMeterKp(void)
{
    BYTE i;
    uint val=0;
    if ((Rs232_Sav_Num < 2) || (Rs232_Sav_Num > 6))
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }
    for (i=0;i<Rs232_Sav_Num-1;i++)
    {
        
        if ((Rs232_Buffer[i] > '9') || (Rs232_Buffer[i] < '0'))
        {
            PrintStr("\r\nInput Invalid.\r\n");
            return;
        }
        
    }
    
    if (Rs232_Sav_Num == 2)
        {
            val = Rs232_Buffer[0] - 0x30;
        }
        else if (Rs232_Sav_Num == 3)
        {
            val = (Rs232_Buffer[0] - 0x30) * 10 + 
                (Rs232_Buffer[1] - 0x30);
        }
        else if (Rs232_Sav_Num == 4)
        {
            val = (Rs232_Buffer[0] - 0x30) * 100 + 
                (Rs232_Buffer[1] - 0x30) * 10 + 
                (Rs232_Buffer[2] - 0x30);
        }
        else if (Rs232_Sav_Num == 5)
        {
            val = (Rs232_Buffer[0] - 0x30) * 1000 + 
                (Rs232_Buffer[1] - 0x30) * 100 + 
                (Rs232_Buffer[2] - 0x30)*10+
                (Rs232_Buffer[2] - 0x30);
        }
        else if (Rs232_Sav_Num == 6)
        {
            if((Rs232_Buffer[0] - 0x30) > 6)
            {
                PrintStr("\r\nInput Invalid.\r\n");
                return;   
            }
            val = (Rs232_Buffer[0] - 0x30) * 10000 + 
                (Rs232_Buffer[1] - 0x30) * 1000 +
                (Rs232_Buffer[2] - 0x30) * 100 +
                (Rs232_Buffer[3] - 0x30) * 10 +
                (Rs232_Buffer[4] - 0x30);
        }
        for(i=0;i<9;i++)
        {
            Check_KPA_KPB(i,val);
        }
        SaveSysParam();
        PrintStr("\r\nSet Success.\r\n");
}
void SetMeterAdress(void)
{
    static BYTE i,Index,v,loc;
    
    if ((Rs232_Sav_Num < 4) || (Rs232_Sav_Num > 6))
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    loc = 0;
    for (i=0;i<Rs232_Sav_Num-1;i++)
    {
        if (Rs232_Buffer[i] != '=')
        {
            if ((Rs232_Buffer[i] > '9') || (Rs232_Buffer[i] < '0'))
            {
                PrintStr("\r\nInput Invalid.\r\n");
                return;
            }
        }
        else
        {
            loc = i;
        }
    }

    if (Rs232_Buffer[loc] != '=')
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    if ((loc < 1) || (loc > 2))
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    if (Rs232_Sav_Num - loc > 4)
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }

    if (loc == 1)
    {
        Index = Rs232_Buffer[0] - 0x30;
    }
    else if (loc == 2)
    {
        Index = (Rs232_Buffer[0] - 0x30) * 10 + 
            (Rs232_Buffer[1] - 0x30);
    }
        
    if (Index > 17)
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }
    
    if ((Rs232_Sav_Num - loc -1) == 2)
    {
        v = Rs232_Buffer[loc+1] - 0x30;
    }
    else if ((Rs232_Sav_Num - loc -1) == 3)
    {
        v = (Rs232_Buffer[loc+1] - 0x30)*16 +
            (Rs232_Buffer[loc+2] - 0x30);
    }
    SysParam[SP_METERADDR1+Index] = v;
    PrintStr("\r\nSet Success.\r\n");
}

void GetRawValue(void)
{
    BYTE channel;
    if ((Rs232_Sav_Num != 2) )
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }
      
    if ((Rs232_Buffer[0] > '9') || (Rs232_Buffer[0] < '0'))
    {
        PrintStr("\r\nInput Invalid.\r\n");
        return;
    }
    channel = Rs232_Buffer[0]-0x30;
    getRawMeasure(channel);
}
#if 0
BYTE Meter645_1997(void)
{
    static BYTE c,i;

    //while(1)
    {
    
        //FE FE 68 84 31 80 08 00 00 68 01 02 43 C3 16 16
        mset(TCP_SendBuf,0,TCP_SENDLEN);

        //645-1997
        mcpy(TCP_SendBuf,"\xFE\xFE\x68\x99\x99\x99\x99\x99\x99\x68\x01\x02\x43\xC3\x6F\x16",16);
        
        //645-2007
        //FE FE 68 83 01 33 02 00 34 68 11 04 32 34 33 37 A2 16
        //FE FE 68 99 99 99 99 99 99 68 11 04 32 34 33 37 4B 16
        //FE FE 68 AA AA AA AA AA AA 68 11 04 32 34 33 37 B1 16 
        //mcpy(TCP_SendBuf,"\xFE\xFE\x68\xAA\xAA\xAA\xAA\xAA\xAA\x68\x11\x04\x32\x34\x33\x37\xB1\x16",18);

//        Em_SendStr(TCP_SendBuf,16); 

//        Clear_Em_buf();
        c = 0;
        while(1)
        {
            Sleep(100);
            WDog();
            if (RecValidMeterFrame())
            {
                DebugStr("MeterRet ok\r\n");
                //PrintStr("Meter Test Success.\r\n");
                
                //mcpy(TCP_SendBuf,Em_Buffer,Em_sav_num);
                for (i=0;i<Em_sav_num;i++)
                {
                    DebugChar(HexToBcd(Em_Buffer[i]/16));
                    DebugChar(HexToBcd(Em_Buffer[i]%16));
                    DebugChar(' ');
                }
                DebugChar('\r');
                DebugChar('\n');

                return TRUE;
                //break;
            }

            c++;
            if (c>20)  // 2秒超时 电表没有返回数据
            {
                //PrintStr("Meter Test Fail.\r\n");
                for (i=0;i<Em_sav_num;i++)
                {
                    DebugChar(HexToBcd(Em_Buffer[i]/16));
                    DebugChar(HexToBcd(Em_Buffer[i]%16));
                    DebugChar(' ');
                }
                DebugChar('\r');
                DebugChar('\n');

                return FALSE;
                //break;
            }
        }
    }

}


BYTE Meter645_2007(void)
{
    static BYTE c,i;

    //while(1)
    {
    
        //FE FE 68 84 31 80 08 00 00 68 01 02 43 C3 16 16
        mset(TCP_SendBuf,0,TCP_SENDLEN);

        //645-1997
        //mcpy(TCP_SendBuf,"\xFE\xFE\x68\x99\x99\x99\x99\x99\x99\x68\x01\x02\x43\xC3\x6F\x16",16);
        
        //645-2007
        //FE FE 68 83 01 33 02 00 34 68 11 04 32 34 33 37 A2 16
        //FE FE 68 99 99 99 99 99 99 68 11 04 32 34 33 37 4B 16
        //FE FE 68 AA AA AA AA AA AA 68 11 04 32 34 33 37 B1 16 
        mcpy(TCP_SendBuf,"\xFE\xFE\x68\xAA\xAA\xAA\xAA\xAA\xAA\x68\x11\x04\x34\x34\x33\x37\xB3\x16",18);

        Em_SendStr(TCP_SendBuf,18); 

        Clear_Em_buf();
        c = 0;
        while(1)
        {
            Sleep(100);
            WDog();
            if (RecValidMeterFrame())
            {
                DebugStr("MeterRet ok\r\n");
                //PrintStr("Meter Test Success.\r\n");
                
                //mcpy(TCP_SendBuf,Em_Buffer,Em_sav_num);
                for (i=0;i<Em_sav_num;i++)
                {
                    DebugChar(HexToBcd(Em_Buffer[i]/16));
                    DebugChar(HexToBcd(Em_Buffer[i]%16));
                    DebugChar(' ');
                }
                DebugChar('\r');
                DebugChar('\n');

                return TRUE;
                //break;
            }

            c++;
            if (c>20)  // 2秒超时 电表没有返回数据
            {
                //PrintStr("Meter Test Fail.\r\n");
                for (i=0;i<Em_sav_num;i++)
                {
                    DebugChar(HexToBcd(Em_Buffer[i]/16));
                    DebugChar(HexToBcd(Em_Buffer[i]%16));
                    DebugChar(' ');
                }
                DebugChar('\r');
                DebugChar('\n');

                return FALSE;
                //break;
            }
        }
    }

}



void TestMeter(void)
{
    if (Meter645_1997())
    {
        PrintStr("Meter 645-1997 Test Success.\r\n");
    }
    else if (Meter645_2007())
    {
        PrintStr("Meter 645-2007 Test Success.\r\n");
    }
    else
    {
        PrintStr("Meter Test Fail.\r\n");
    }
}
#endif
void HandleCmdMain(char key)
{
    BYTE channel;
    uint data;
    if(CheckMetering == 0)
    {
        if (key == '1')
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintStr("Input Device ID:");
            CmdType = CMD_INPUT_DEVID;
        }
        else if (key == '2')
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintStr("0:150;  1:300;   2:600;   3:1200;  4:2400;  5:4800\r\n");
            PrintStr("6:9600; 7:19200; 8:38400; 9:57600; 10:115200\r\n");
            PrintStr("Please Select:");
            CmdType = CMD_INPUT_BAUDRATE;
        }
        else if (key == '3')
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintStr("1:Odd;   2:Even;   3:None\r\n");
            PrintStr("Please Select:");
            CmdType = CMD_INPUT_CHECK;
        }
        else if (key == '4')
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintStr("Input Server IP Address:");
            CmdType = CMD_INPUT_SRVIP;
        }
        else if (key == '5')
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintStr("Input Server Port:");
            CmdType = CMD_INPUT_SRVPORT;
        }
        else if (key == '6')
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintStr("Input Local IP Address:");
            CmdType = CMD_INPUT_LOCIP;
        }
        else if (key == '7')
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintStr("Input Local Port:");
            CmdType = CMD_INPUT_LOCPORT;
        }
        else if (key == '8')  // Date
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintStr("Input Data[yyyy-mm-dd]:");
            CmdType = CMD_INPUT_DATE;
        }
        else if (key == '9')  // Time
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintStr("Input Time[hh:mm:ss]:");
            CmdType = CMD_INPUT_TIME;
        }
        else if ((key == 'a') || (key == 'A'))
        {
            
        }
        else if ((key == 'i') || (key == 'I'))  // Input
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            //PrintInSt();
        
            PrintMenu();
        }
        else if ((key == 'o') || (key == 'O'))  // Output
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            //ChangeOutput();
            //PrintOutSt();
            PrintMenu();
        }
        else if ((key == 'c') || (key == 'C'))  // Test Output
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            //ChangeOutput();
            //PrintOutSt();
            PrintMenu();
        }
        else if ((key == 'e') || (key == 'E'))  // Debug Info
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintDebug();
        
            PrintMenu();
        }
        else if ((key == 's') || (key == 'S'))
        {
            if (SaveSysParam())
            {
                PrintStr("\r\nSave Parameter Success.\r\n");
            }
            else
            {
                PrintStr("\r\nSave Parameter Fail.\r\n");
            }
            ClearRs232Buf();
            PrintMenu();
        }
        else if ((key == 'r') || (key == 'R'))
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintStr("Resetting,Please wait...\r\n");
            SysReset();
        }
        else if ((key == 'p') || (key == 'P'))  // Set Param
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintStr("Input Param and Value[Param=Value]:");
            CmdType = CMD_INPUT_SETPARAM;
        }
        else if ((key == 'q') || (key == 'Q'))  // Read Param
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintStr("Input Param ID:");
            CmdType = CMD_INPUT_READPARAM;
        }
        else if ((key == 'm') || (key == 'M'))  // Print Alarm
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            //PrintAlarm();
            PrintMenu();
        }
        else if ((key == 'u') || (key == 'U'))  // Print run counter
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            //PrintUnionStatus();
            PrintMenu();
        }
        else if ((key == 't') || (key == 'T'))  // 测试电表
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            //TestMeter();
            PrintMenu();
        }
        else if ((key == 'z') || (key == 'Z'))
        {
            ClearRs232Buf();
            NeedConnSrv = 1;
            PrintStr("\r\n");
            PrintStr("Reconnest Server.\r\n");
            PrintMenu();
        }
        else if ((key == 'd') || (key == 'D'))
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintStr("All Parameter will be set to Default value, Continue(Y/N?)");
            CmdType = CMD_INPUT_YESNO;
        }
        else if ((key == 'w') || (key == 'W'))  // beep ctrl
        {
            ClearRs232Buf();
            PrintStr("\r\n");        
            PrintMenu();
        }
        else if(key == '@')//进入校表菜单
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintMeterMenu();
            CheckMetering = 0x0F;
            CmdType = CMD_CHECK_METER;
            //不打印调试信息
            SysParam[SP_DEBUGMSG] = DEBUG_DISABLE;
        }   
        else if (key == 0x0D)
        {
            ClearRs232Buf();
            PrintMenu();
        }
        else
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintStr("Invalid Select!\r\n");
            PrintMenu();
        }
    }
    else if(CheckMetering == 0x0F)//校表
    {
        if ((key == 's') || (key == 'S'))
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintStr("Self-Checking,Continue(Y/N?)");
            CmdType = CMD_CHECK_SELF;
        }
        else if ((key == 'i') || (key == 'I'))
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintStr("Input Current Check Value[MAX=65535(0.1mA)]:");
            CmdType = CMD_CHECK_CURT;
        }
        else if ((key == 'v') || (key == 'V'))
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintStr("Input Voltage Check Value[MAX=65535(0.1V)]:");
            CmdType = CMD_CHECK_VLT;
        }
        else if ((key == 'p') || (key == 'P'))
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintStr("Input Power Check Value[MAX=65535(0.1W)]:");
            CmdType = CMD_CHECK_PWR;
        }
        else if ((key == 'e') || (key == 'E'))
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintStr("Input Energy Check Value[MAX=65535(0.01KWh)]:");
            CmdType = CMD_CHECK_ENERGR;
        }
        else if((key == 'a') || (key == 'A'))
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            ShowMeterAddr();
            PrintStr("Input Number and Value[Number=Value]:");
            CmdType = CMD_CHECK_ADRESS;
            SETADRESSFLAG = 1;//进入地址设置
        }
        else if((key == 'g') || (key == 'G'))
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintStr("Input Channel Vlaue[0~8]:");
            CmdType = CMD_CHECK_GRAW;
        }
        else if((key == 'c') || (key == 'C'))
        {   
            
            
            Clear_FM24C(FMSLAVE_ADDR, 0x00,EEPROM_COUNT);           
            PrintStr("\r\nClear Energy Success\r\n");           
            PrintMeterMenu();
        }
        else if ((key == 'r') || (key == 'R'))
        {
            //读
            for(channel=0;channel<9;channel++)
            {
                data = 0;
                Read_FM24C(FMSLAVE_ADDR, EEPROM_ENERGYADDR+4*channel,(BYTE *)&data,4);
                printf("\nEE%d=0x%x ",channel,data);
                
            }
            for(channel=0;channel<9;channel++)
            {
                data = 0;
                Read_FM24C(FMSLAVE_ADDR, EEPROM_ENERGYLASTADDR+4*channel,(BYTE *)&data,4);
                printf("\nEELAST%d=0x%x ",channel,data);
                
            }
            for(channel=0;channel<9;channel++)
            {
                data = 0;
                Read_FM24C(FMSLAVE_ADDR, EEPROM_OVERFLOWADDR+4*channel,(BYTE *)&data,1);
                printf("\nOVERFLOW%d=0x%x ",channel,data);
                
            }
            PrintMeterMenu();
        }
        else if ((key == 'q') || (key == 'Q'))
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintStr("Qiut Check Meter,Continue(Y/N?)");
            CmdType = CMD_CHECK_QIUT;
        }
        else if (key == 0x0D)
        {
            ClearRs232Buf();
            PrintMeterMenu();
        }
        else
        {
            ClearRs232Buf();
            PrintStr("\r\n");
            PrintStr("Invalid Select!\r\n");
            PrintMeterMenu();
        }
    }
}


void CmdLine(void*Msg)
{
    static BYTE i=0;
    static char c;

    while ( 1 )
    {
        Rs232_Buffer[Rs232_Sav_Num++]=commBaseGetChar (1);

        if (i != Rs232_Sav_Num)   // 有按键发生
        {
            c = Rs232_Buffer[Rs232_Sav_Num-1];
            if((c == '\b')&&(Rs232_Sav_Num >= 2))//回格键
            {
                Rs232_Buffer[Rs232_Sav_Num-1]=0;
                Rs232_Buffer[Rs232_Sav_Num-2]=0;
                Rs232_Sav_Num -= 2;
                PrintChar ( '\b' );
                PrintChar ( ' ' );
                PrintChar ( '\b' );
            }
            else
            {
                PrintChar(c);  // 回显
            }
            
            switch(CmdType)
            {
                case CMD_NONE:
                    if (c == 0x0D)  // 回车
                    {
                        ClearRs232Buf();
                        PrintMenu();
                    }
                break;
            
                case CMD_MAIN_MENU:
                    HandleCmdMain(c);
                break;


                case CMD_INPUT_DEVID:
                    if (c == 0x0D)  // 回车
                    {
                        SetDevID();
                        ClearRs232Buf();
                        PrintMenu();
                    }
                break;

                case CMD_INPUT_BAUDRATE:
                    if (c == 0x0D)  // 回车
                    {
                        SetBaudRate();
                        ClearRs232Buf();
                        PrintMenu();
                    }
                break;

                case CMD_INPUT_CHECK:
                    if (c == 0x0D)  // 回车
                    {
                        SetCheckBit();
                        ClearRs232Buf();
                        PrintMenu();
                    }
                break;

                case CMD_INPUT_SRVIP:
                    if (c == 0x0D)  // 回车
                    {
                        SetSrvIp();
                        ClearRs232Buf();
                        PrintMenu();
                    }
                break;

                case CMD_INPUT_SRVPORT:
                    if (c == 0x0D)  // 回车
                    {
                        SetSrvPort();
                        ClearRs232Buf();
                        PrintMenu();
                    }
                break;

                case CMD_INPUT_LOCIP:
                    if (c == 0x0D)  // 回车
                    {
                        SetLocIp();
                        ClearRs232Buf();
                        PrintMenu();
                    }
                break;

                case CMD_INPUT_LOCPORT:
                    if (c == 0x0D)  // 回车
                    {
                        SetLocPort();
                        ClearRs232Buf();
                        PrintMenu();
                    }
                break;

                case CMD_INPUT_DATE:
                    if (c == 0x0D)  // 回车
                    {
                        SetDate();
                        ClearRs232Buf();
                        PrintMenu();
                    }
                break;

                case CMD_INPUT_TIME:
                    if (c == 0x0D)  // 回车
                    {
                        SetTime();
                        ClearRs232Buf();
                        PrintMenu();
                    }
                break;

                case CMD_INPUT_SETPARAM:
                    if (c == 0x0D)  // 回车
                    {
                        SetParam();
                        ClearRs232Buf();
                        PrintMenu();
                    }
                break;

                case CMD_INPUT_READPARAM:
                    if (c == 0x0D)  // 回车
                    {
                        ReadParam();
                        ClearRs232Buf();
                        PrintMenu();
                    }
                break;

                case CMD_INPUT_YESNO:
                    if ((c == 'Y') || (c == 'y'))
                    {
                        setParamToDefault();
                        PrintStr("\r\nSet Default Paramater Success.\r\n");
                        ClearRs232Buf();
                        PrintMenu();
                    }
                    else if ((c == 'N') || (c == 'n'))
                    {
                        ClearRs232Buf();
                        PrintMenu();
                    }
                    else if (c == 0x0D) 
                    {
                        ClearRs232Buf();
                        PrintMenu();
                    }
                    
                break;
                //check meter
                case CMD_CHECK_METER:
                    HandleCmdMain(c);
                break;
                case CMD_CHECK_SELF:
                    if ((c == 'Y') || (c == 'y'))
                    {
                        PrintStr("\r\nPleas wait...\r\n");
                        RnCheckSelf();
                        SaveSysParam();
                        PrintStr("\r\nSelfCheck end.\r\n");
                        ClearRs232Buf();
                        PrintMeterMenu();
                    }
                    else if ((c == 'N') || (c == 'n'))
                    {
                        ClearRs232Buf();
                        PrintMeterMenu();
                    }
                    else if (c == 0x0D) 
                    {
                        ClearRs232Buf();
                        PrintMeterMenu();
                    }
                break;
                case CMD_CHECK_CURT://电流
                    if (c == 0x0D)  // 回车
                    {
                        PrintStr("\r\nPleas wait...\r\n");
                        SetMeterKi();
                        ClearRs232Buf();
                        PrintMeterMenu();
                    }
                break;
                case CMD_CHECK_VLT:
                    if (c == 0x0D)  // 回车
                    {    
                        PrintStr("\r\nPleas wait...\r\n");
                        SetMeterKu();
                        ClearRs232Buf();
                        PrintMeterMenu();
                    }
                break;
                case CMD_CHECK_PWR:
                    if (c == 0x0D)  // 回车
                    {    
                        PrintStr("\r\nPleas wait...\r\n");
                        SetMeterKp();
                        ClearRs232Buf();
                        PrintMeterMenu();
                    }
                break;
                case CMD_CHECK_ENERGR:
                    if (c == 0x0D)  // 回车
                    {
                        PrintStr("\r\n");
                        SetMeterHF();
                        ClearRs232Buf();
                        PrintMeterMenu();
                    }
                break;
                case CMD_CHECK_ADRESS:
                    
                    if (c == 0x0D)  // 回车
                    {
                        PrintStr("\r\n");
                        SetMeterAdress();
                        ClearRs232Buf();
                        ShowMeterAddr();
                        PrintStr("Input Number and Value[Number=Value]:");
                        //CmdType = CMD_CHECK_METER;
                        
                    }
                    else if( (c == 'q') ||(c == 'Q'))
                    {
                    	SaveSysParam();//退出时保存
                        HandleCmdMain(c);
                    }
                break;
                case CMD_CHECK_GRAW:
                    if (c == 0x0D)  // 回车
                    {
                        PrintStr("\r\n");
                        GetRawValue();
                        ClearRs232Buf();
                        PrintMeterMenu();
                        CmdType = CMD_CHECK_METER;
                        
                    }
                break;
                case CMD_CHECK_QIUT:
                    if ((c == 'Y') || (c == 'y'))
                    {
                        ClearRs232Buf();
                        //if (SaveSysParam())
                        //{
                        //    PrintStr("\r\nParameter have been saved.\r\n");
                        //}
                        //else
                        //{
                        //    PrintStr("\r\nSave Parameter Fail.\r\n");
                        //}
                        
                        if(SETADRESSFLAG == 1)
                        {
                            CmdType = CMD_CHECK_METER;
                            SETADRESSFLAG = 0;
                            PrintMeterMenu();
                        }
                        else
                        {
                            CheckMetering = 0;
                            CmdType = CMD_NONE;
                            PrintMenu();
                        }                       
                        
                    }
                    else if ((c == 'N') || (c == 'n'))
                    {
                        if(SETADRESSFLAG == 1)
                        {
                            CmdType = CMD_CHECK_ADRESS;
                            ShowMeterAddr();
                            PrintStr("Input Number and Value[Number=Value]:");
                        }
                        else
                        {
                            ClearRs232Buf();
                            PrintMeterMenu();
                        }    
                    }
                    else if (c == 0x0D) 
                    {
                        if(SETADRESSFLAG == 1)
                        {
                            CmdType = CMD_CHECK_ADRESS;
                            ShowMeterAddr();
                            PrintStr("Input Number and Value[Number=Value]:");
                        }
                        else
                        {
                            ClearRs232Buf();
                            PrintMeterMenu();
                        }
                    }
                break;
                default:
                break;
        }
                
        i = Rs232_Sav_Num-1;
    }
    }
    
}

void TestConsole(void)
{
    while(1)
    {
        Sleep(1000);
    }
}

