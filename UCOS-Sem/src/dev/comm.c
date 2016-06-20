/*************************************************************
成都昊普环保技术有限公司   版权所有
CDMA采集器


文件名:   
作  者:    潘国义
描  述:    公共模块
修订记录:

**************************************************************/

#include "includes.h"
#include "comm.h"
#include "main.h"
#include "devUartProc.h"
#include "dev.h"
#include <sysctl.h>
//用于通讯发送数据缓存
//BYTE SendFrame_Buffer[SENDBUFLEN];

const BYTE MonthTable[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const DWORD BaudRateTable[] = 
{
    150,
    300,
    600,
    1200,
    2400,
    4800,
    9600,
    19200,
    38400,
    57600,
    115200,
};

//extern BYTE SysParam[];
extern BYTE SysTime[];
extern BYTE IpAddrStr[];
extern BYTE PortStr[];


extern BYTE Uart0_Buffer[];
extern WORD Uart0_Sav_Num;


void Sleep (WORD ms)
{
    if(ms>999)
    {
        ms=ms/1000;
        OSTimeDlyHMSM(0, 0,ms,0);
    }
    else
    {
        OSTimeDlyHMSM(0, 0,0,ms);
    }
    
}

void TestSleep()
{
    while(1)
    {
//        SPK_ON();
        Sleep(100);
//        SPK_OFF();
        Sleep(900);
    }
}

void Delay(unsigned long ulSeconds)
{
    //
    // Loop while there are more seconds to wait.
    //
    while(ulSeconds--)
    {
        //
        // Wait until the SysTick value is less than 1000.
        //
        while(SysTickValueGet() > 1000)
        {
            //WDog();
        }

        //
        // Wait until the SysTick value is greater than 1000.
        //
        while(SysTickValueGet() < 1000)
        {
            //WDog();
        }
    }
}

#ifdef INNER_WDOG
void Init_WDog(void)
{
    //IntEnable(INT_WATCHDOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);
    WatchdogReloadSet(WATCHDOG0_BASE, TheSysClock*2); // 约 2 秒
    WatchdogResetEnable(WATCHDOG0_BASE);
    WatchdogEnable(WATCHDOG0_BASE);
}

void WDog(void)
{
    WatchdogReloadSet(WATCHDOG0_BASE,TheSysClock*5);
}

#else
void WDog(void)
{
    static BYTE Dog = 0;
    if (Dog==0)
    {
        Dog = 1;
        //GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_2,GPIO_PIN_2);
    }
    else
    {
        Dog = 0;
        //GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_2,0);
    }
}
#endif


BYTE HexToBcd(BYTE Hex)
{
    if (Hex > 9)
    {
        return Hex + 0x37;
    }
    else
    {
        return Hex + 0x30;
    }
}

BYTE BcdToHex(BYTE Bh, BYTE Bl)
{
    static BYTE temp;
    if (Bh > 0x39)
    {
        temp = Bh - 0x37;
    }
    else
    {
        temp = Bh - 0x30;
    }

    temp <<= 4;
    
    if (Bl > 0x39)
    {
        temp |= Bl - 0x37;
    }
    else
    {
        temp |= Bl - 0x30;
    }

    return temp;
}

BYTE  StrLen(const char *Str)
{
    static BYTE i;
    i = 0;
    while(Str[i] != 0)
    {
        i++;
    }
    return i;
}

void mcpy(BYTE *Des,BYTE *Src, WORD Len)
{
    static WORD i;
    for (i=0;i<Len;i++)
    {
        Des[i] = Src[i];
    }
}

void mset(BYTE *Des, BYTE Data, BYTE Len)
{
    static BYTE i;
    for (i=0;i<Len;i++)
    {
        Des[i] = Data;
    }
}

BYTE mcmp(BYTE *Des,BYTE *Src,BYTE Len)
{
    static BYTE i;
    for (i=0;i<Len;i++)
    {
        if (Des[i] != Src[i])
        {
            return FALSE;
        }
    }
    return TRUE;
}

BYTE mcmp2(BYTE *Des,BYTE Src,BYTE Len)
{
    static BYTE i;
    for (i=0;i<Len;i++)
    {
        if (Des[i] != Src)
        {
            return FALSE;
        }
    }
    return TRUE;
}

//跳过bit位不比较
BYTE mcmp3(BYTE *Des,BYTE *Src,BYTE Len,BYTE bit)
{
    static BYTE i;
    for (i=0;i<Len;i++)
    {
        if(i==bit)
        {
            continue;
        }
        else if (Des[i] != Src[i])
        {
            return FALSE;
        }
    }
    return TRUE;
}


//将年月日转换为天
WORD DateToDay(void)
{
    static WORD day;
    static BYTE y,m,d,i;
    y = SysTime[0]/0x10 * 10 + SysTime[0]%0x10;
    m = SysTime[2]/0x10 * 10 + SysTime[1]%0x10;
    d = SysTime[3]/0x10 * 10 + SysTime[2]%0x10;
    day = y * 365;
    day += y/4 + 1;  // 闰年

    if (y%4 == 0)
    {
        if (m < 3)
        {
            day -= 1;
        }
    }

    for (i=1;i<m;i++)
    {
        day += MonthTable[i-1];
    }
    day += d;

    //day -= 1; // 2000年是平年

    return day;
}

//将天装换为年月日
void DayToDate(WORD day)
{
    static BYTE y,m,d,i;
    static WORD t,p;

    t = day;
    y = t / 365;

    if (t >= 60)
    {
        t -= (y/4+1);
    }

    y = t/365;

    if (t%365 == 0)
    {
        y-=1;
        m = 12;
        d = 31;
    }
    else
    {
        p = t % 365;
        t = 0;
        for (i=1;i<=12;i++)
        {
            t += MonthTable[i-1];
            if (t >= p)
            {
                m = i;
                break;
            }
        }

        if (t == p)
        {
            d = MonthTable[m-1];
        	if (m==2)
        	{
                if (y%4 == 0)
                {
                    d+=1;
                }
        	}
        }
        else
        {
        	d = MonthTable[m-1] - (t-p);
        }
    }

    
    //NextRepTime[0] = y/10*0x10 + y%10;
    //NextRepTime[1] = m/10*0x10 + m%10;
    //NextRepTime[2] = d/10*0x10 + d%10;
    
}



// 将IP地址转化为字符串
void IpToStr(BYTE *IpAddr)
{
    static BYTE i,loc;
    loc = 0;
    mset(IpAddrStr, 0 ,16);
    for (i=0;i<4;i++)
    {
        if (IpAddr[i] == 0)
        {
            IpAddrStr[loc++] = '0';
            if (i != 3)
            {
                IpAddrStr[loc++] = '.';
            }
        }
        else
        {
            if ((IpAddr[i] / 100) != 0)
            {
                IpAddrStr[loc++] = (IpAddr[i] / 100) + 0x30;
                IpAddrStr[loc++] = (IpAddr[i] % 100 / 10) + 0x30;
                IpAddrStr[loc++] = (IpAddr[i] % 100 % 10) + 0x30;
                if (i != 3)
                {
                    IpAddrStr[loc++] = '.';
                }
            }
            else if ((IpAddr[i] / 10) != 0)
            {
                IpAddrStr[loc++] = (IpAddr[i] / 10) + 0x30;
                IpAddrStr[loc++] = (IpAddr[i] % 10) + 0x30;
                if (i != 3)
                {
                    IpAddrStr[loc++] = '.';
                }
            }
            else
            {
                IpAddrStr[loc++] = IpAddr[i] + 0x30;
                if (i != 3)
                {
                    IpAddrStr[loc++] = '.';
                }
            }
                
        }
    }
}

// 服务器端口转化成字符串
void PortToStr(BYTE *Port)
{
    static BYTE loc;
    static WORD p,tmp;
    p = *(WORD *)Port;
    
    //p=getParamword((char *)Port);

    loc = 0;
    mset(PortStr, 0, 6);
    if ((p / 10000) != 0)
    {
        PortStr[loc++] = p / 10000 + 0x30;
        tmp = p % 10000;

        PortStr[loc++] = tmp / 1000 + 0x30;
        tmp = tmp % 1000;

        PortStr[loc++] = tmp / 100 + 0x30;
        tmp = tmp % 100;

        PortStr[loc++] = tmp / 10 + 0x30;
        tmp = tmp % 10;

        PortStr[loc++] = tmp + 0x30;
    }
    else if ((p / 1000) != 0)
    {
        PortStr[loc++] = p / 1000 + 0x30;
        tmp = p % 1000;

        PortStr[loc++] = tmp / 100 + 0x30;
        tmp = tmp % 100;

        PortStr[loc++] = tmp / 10 + 0x30;
        tmp = tmp % 10;

        PortStr[loc++] = tmp + 0x30;
    }
    else if ((p / 100) != 0)
    {
        PortStr[loc++] = p / 100 + 0x30;
        tmp = p % 100;

        PortStr[loc++] = tmp / 10 + 0x30;
        tmp = tmp % 10;

        PortStr[loc++] = tmp + 0x30;
    }
    else if ((p / 10) != 0)
    {
        PortStr[loc++] = p / 10 + 0x30;
        tmp = p % 10;

        PortStr[loc++] = tmp + 0x30;
    }
    else 
    {
        PortStr[loc++] = p + 0x30;
    }
}


BYTE StrToByte(BYTE *Str, BYTE Len, BYTE *Dat)
{
    if (Len == 3)
    {
        if (Str[0] > '2')
        {
            return FALSE;
        }
        else if (Str[0] == '2')
        {
            if (Str[1] > '5')
            {
                return FALSE;
            }
            else if (Str[1] == '5')
            {
                if (Str[2] > '5')
                {
                    return FALSE;
                }
            }
        }

        *Dat = (Str[0] - 0x30) * 100 + (Str[1] - 0x30) * 10 + (Str[2] - 0x30);
        
    }
    else if (Len == 2)
    {
        *Dat = (Str[0] - 0x30) * 10 + (Str[1] - 0x30);
    }
    else if (Len == 1)
    {
        *Dat = Str[0] - 0x30;
    }
    else 
    {
        return FALSE;
    }

    return TRUE;
}



BYTE StrToWord(BYTE *Str, BYTE Len, WORD *Dat)
{
    static BYTE i;
    for (i=0; i<Len; i++)
    {
        if ((Str[i] > '9') || (Str[i] < '0'))
        {
            return FALSE;
        }
    }
    
    if (Len == 4)
    {
        *Dat = (Str[0] - 0x30) * 1000 + (Str[1] - 0x30) * 100 + 
                (Str[2] - 0x30) * 10 + (Str[2] - 0x30);   
    }
    else if (Len == 3)
    {
        *Dat = (Str[0] - 0x30) * 100 + (Str[1] - 0x30) * 10 + (Str[2] - 0x30);   
    }
    else if (Len == 2)
    {
        *Dat = (Str[0] - 0x30) * 10 + (Str[1] - 0x30);
    }
    else if (Len == 1)
    {
        *Dat = Str[0] - 0x30;
    }
    else 
    {
        return FALSE;
    }

    return TRUE;
}


// 将IP地址串(IpAddrStr)转化成4字节
BYTE StrToIpAdd(BYTE *IpAddr)
{
    static BYTE i,len;
    
    i = 0;
    len = 0;
    while(IpAddrStr[i] != 0)
    {
        i ++;
        if (IpAddrStr[i] == '.')
        {
            break;
        }
    }

    if (!(StrToByte(&IpAddrStr[0], i, &IpAddr[0])))
    {
        return FALSE;
    }

    len = i;

    while(IpAddrStr[i] != 0)
    {
        i ++;
        if (IpAddrStr[i] == '.')
        {
            break;
        }
    }

    if (!(StrToByte(&IpAddrStr[len+1], i-len-1 , &IpAddr[1])))
    {
        return FALSE;
    }

    len = i;

    while(IpAddrStr[i] != 0)
    {
        i ++;
        if (IpAddrStr[i] == '.')
        {
            break;
        }
    }

    if (!(StrToByte(&IpAddrStr[len+1], i-len-1 , &IpAddr[2])))
    {
        return FALSE;
    }

    len = i;
    
    while(IpAddrStr[i] != 0)
    {
        i ++;
        if (IpAddrStr[i] == '.')
        {
            break;
        }
    }

    if (!(StrToByte(&IpAddrStr[len+1], i-len-1 , &IpAddr[3])))
    {
        return FALSE;
    }
    
    return TRUE;
}

// 将Port(PortStr)转化成Word
BYTE StrToPort(BYTE *Port)
{
    static BYTE i;
    static WORD tmp;
    
    i = 0;
    while(PortStr[i] != 0)
    {
        i ++;
    }

    if ((i > 5) || (i == 0))
    {
        return FALSE;
    }

    if (i == 5)
    {
        tmp = (PortStr[0]-0x30)*10000 +
              (PortStr[1]-0x30)*1000 +
              (PortStr[2]-0x30)*100 +
              (PortStr[3]-0x30)*10 +
              (PortStr[4]-0x30);
    }
    else if (i == 4)
    {
        tmp = (PortStr[0]-0x30)*1000 +
              (PortStr[1]-0x30)*100 +
              (PortStr[2]-0x30)*10 +
              (PortStr[3]-0x30);
    }
    else if (i == 3)
    {
        tmp = (PortStr[0]-0x30)*100 +
              (PortStr[1]-0x30)*10 +
              (PortStr[2]-0x30);
    }
    else if (i == 2)
    {
        tmp = (PortStr[0]-0x30)*10 +
              (PortStr[1]-0x30);
    }
    else 
    {
        tmp = (PortStr[0]-0x30);
    }

    *(WORD *)Port = tmp;
    
    return TRUE;
}


// 判断有效的IP地址串
BYTE CheckValidIpAddr(void)
{
    static BYTE i,j;
    static BYTE cnt[4];
    i = 0;
    j = 0;
    memset(cnt,0,4);
    while(IpAddrStr[i] != 0)
    {
        
        if(IpAddrStr[i] == '.')
    	{
    		if(cnt[j]>3) return FALSE;
    		j++;
    	}
        if (IpAddrStr[i] != '.')
        {
        	cnt[j]++;
            if ((IpAddrStr[i] > '9') || (IpAddrStr[i] < '0'))
            {
                return FALSE;
            }
        }
        
        i++; 
    }
    return TRUE;
}

// 判断有效的Port串
BYTE CheckValidPort(void)
{
    static BYTE i;
    i = 0;
    while(PortStr[i] != 0)
    {
        if ((PortStr[i] > '9') || (PortStr[i] < '0'))
        {
            return FALSE;
        }

        i++;
    }

    if (i > 5)
    {
        return FALSE;
    }

    if (i == 5)
    {
        if (PortStr[0] > '6')
        {
            return FALSE;
        }
        else if (PortStr[0] == '6')
        {
            if (PortStr[1] > '5')
            {
                return FALSE;
            }
            else if (PortStr[1] == '5')
            {
                if (PortStr[2] > '5')
                {
                    return FALSE;
                }
                else if (PortStr[2] == '5')
                {
                    if (PortStr[3] > '3')
                    {
                        return FALSE;
                    }
                    else if (PortStr[3] == '3')
                    {
                        if (PortStr[4] > '5')
                        {
                            return FALSE;
                        }
                    }
                }
            }
        }
    }
    
    return TRUE;
}

void DebugStr(const char * Msg)  
{                       
    if (SysParam[SP_DEBUGMSG] == DEBUG_DISABLE)
    {
        return;
    }
    printf("%s\r\n",Msg);        
}

void DebugMsg(void)  
{   

    if (SysParam[SP_DEBUGMSG] == DEBUG_DISABLE)
    {
        return;
    }

    printf("%s\r\n",Uart0_Buffer);
    
         
}

void DebugChar(BYTE ch)
{
    if (SysParam[SP_DEBUGMSG] == DEBUG_DISABLE)
    {
        return;
    }
    
    printf("%c",ch);
}

void DebugByte(BYTE Num)
{
    
    static BYTE t,m;

    if (SysParam[SP_DEBUGMSG] == DEBUG_DISABLE)
    {
        return;
    }
    
    m = Num / 100;
    Rs232SendByte(m+0x30);
    t = Num % 100;
    m = t/10;
    Rs232SendByte(m+0x30);
    m = t%10;
    Rs232SendByte(m+0x30);
   
    Rs232SendByte('\r');
    Rs232SendByte('\n');

}

void DebugWord(WORD Num)
{
    
    static WORD t;
    static BYTE m;

    if (SysParam[SP_DEBUGMSG] == DEBUG_DISABLE)
    {
        return;
    }
    
    m = (BYTE)(Num / 10000);
    Rs232SendByte(m+0x30);
    t = Num % 10000;
    m = (BYTE)(t/1000);
    Rs232SendByte(m+0x30);
    t = t%1000;
    m = (BYTE)(t/100);
    Rs232SendByte(m+0x30);
    t = t%100;
    m = (BYTE)(t/10);
    Rs232SendByte(m+0x30);
    m = (BYTE)(t%10);
    Rs232SendByte(m+0x30);
   
    Rs232SendByte('\r');
    Rs232SendByte('\n');

}


void SetLastError(BYTE Err)
{
    SysParam[SP_LASTERROR] = Err;
    SaveSysParam();
}


void ALM_SW(void)
{
    static BYTE alm = 0;
    if (alm == 0)
    {
        alm = 1;
//        armLed(alm);
    }
    else
    {
        alm = 0;
//        armLed(alm);
    }
}

void SPK_SW(void)
{
    static BYTE alm = 0;
    if (alm == 0)
    {
        alm = 1;
//        SPK_ON();
    }
    else
    {
        alm = 0;
//        SPK_OFF();
    }
}
/******************************************************************/
/*                                                                */
/*求平均值                                                        */
/*                                                                */
/******************************************************************/

uint  MeanValue(uint *pBuf ,BYTE len)
{
    uint min;
    uint max;
    uint temp=0;
    BYTE i;
    max = pBuf[0];
    min = pBuf[0];
    for(i=1;i<len;i++)
    {
        if(pBuf[i]>max)
        {
            max = pBuf[i];
        }
        if(pBuf[i]<min)
        {
            min = pBuf[i];
        }
    }
    
    for(i=0;i<len;i++)
    {
        temp += pBuf[i];
    }
    temp =(temp-min-max)/(len-2);

    return temp;
    
}

