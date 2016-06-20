/********************************************************************************************************
*                                              任务优先级及堆栈                                         *
********************************************************************************************************/
#include "includes.h"
#include <update.h>
#include <dev.h>
#include <comm.h>
#include <hw_types.h>
#include <sysctl.h>
#include <lwiplib.h>
#include <hw_ints.h>
#include <systick.h>
#include <flash.h>
#include <interrupt.h>
#include <spi.h>
#include <ethernet.h>
#include <Watchdog.h>
#include "rn8209x.h"
#include "devTimer.h"
#include "ip_addr.h"
#include "API.h"
#include "main.h"
#include "WlModule.h"
#include "devI2c.h"


unsigned int  TheSysClock =0;

const char* DefHost = "www.ecgcp.com";

//软件发布时请修改下面的数据  -- 软件版本在Startup.s 的110行
BYTE DevVersion[7]           = {0};        // 格式 001.100        // 设备版本  7字节
const BYTE DevRelTime[]      = {0,48,15,26,02,0xDD,0x07};    // 软件发布时间  2012年  7字节
//BYTE DevRelTime[10] ;
const BYTE *DevIndex         = "CDMA-R01";                    // 设备编号  8字节

//#define SWVERSION   "V1.9T02"   // 设备版本,7字节
//#define DEVICENO    "00000002"  // 设备编号,8字节

BYTE PowerStr[3][30]   = {0};
BYTE IpAddrStr[16]  = {0};
BYTE PortStr[6]     = {0};


BYTE NoCenter = 0;          // 没有回报中心
BYTE NeedSleep = 0;        // 是否睡眠   采集器启动后网络处于休眠状态
BYTE NeedReConnect  = 0;  // 需要重新汇报心跳包
BYTE NeedConnSrv  = 0 ;   // 是否连接到服务器

BYTE DeviceStatus = 0;
//BYTE NeedReboot = 0;




#define SYSTICKHZ               100
#define SYSTICKMS               (1000 / SYSTICKHZ)
#define SYSTICKUS               (1000000 / SYSTICKHZ)
#define SYSTICKNS               (1000000000 / SYSTICKHZ)

#if 0
#define My_Mac_ID 	{0X00,0x14,0x97,0x0F,0x1D,0xE3}  //存储以太网控制器的物理地址,即MAC地址
#define IP_MARK_ID 	{255,255,255,0} 		         //255.255.255.0,子网掩码
#define MY_IP_ID       	{192,168,1,25}                   //以太网通信的IP地址
#define MY_GATEWAY_ID   {192,168,1,254}                  //以太网通信的网关地址

static const unsigned char MACAddress[] = My_Mac_ID;
unsigned char IPAddress[] = MY_IP_ID;
unsigned char NetMaskAddr[] = IP_MARK_ID;
unsigned char GwWayAddr[] = MY_GATEWAY_ID;
#endif

uint debugFlag = 0x0;
tPubVal gPubVal;

extern void setRandMac(void);
extern void tcpServerInit ( void );

/*******************************************************************************
说明 os 任务定义,将优先级定义在main.h中，方便其他地方调用
*******************************************************************************/
/*tcp server 主消息处理函数*/
#define STK_SIZE_MAIN_HANDLE    400
//#define PRIO_MAIN_HANDLE        2
static  OS_STK  stkTaskMainHandle[STK_SIZE_MAIN_HANDLE];


/*SHELL处理任务*/
#define STK_SIZE_SHELL   300
//#define PRIO_SHELL        3 
static  OS_STK  stkTaskShell[STK_SIZE_SHELL];


/*hart report任务*/
#define STK_SIZE_HEART_REP    300
//#define PRIO_HEART_REP        8
static  OS_STK  stkTaskHearReq[STK_SIZE_HEART_REP];


/*定期处理任务*/
#define STK_SIZE_PRID    300
//#define PRIO_PRID        4
static  OS_STK  stkTaskPrid[STK_SIZE_PRID];


/*WATCHDOG 处理任务*/
#define STK_SIZE_WATCH0   200
//#define PRIO_WATCH0        11
static  OS_STK  stkTaskWatch0[STK_SIZE_WATCH0];


/*rs485处理任务*/
#define STK_SIZE_RS485    300
//#define PRIO_RS485        5
static  OS_STK  stkTaskRS485[STK_SIZE_RS485];

/*GSM处理任务*/
#define STK_SIZE_GSMMSG    200
//#define PRIO_GSMMSG       
static  OS_STK  stkTaskGSMMSG[STK_SIZE_GSMMSG];


/*GSM处理任务*/
#define STK_SIZE_GSM    300
//#define PRIO_GSM        7
static  OS_STK  stkTaskGSM[STK_SIZE_GSM];

/*判断RN8209中断任务，周期任务*/
//#define STK_SIZE_RNISR    200
//#define PRIO_RNISR       9
//static  OS_STK  stkTaskRnISR[STK_SIZE_RNISR];

//RN isr信号量
OS_EVENT * RNSemIsr;


void taskHeartRep ( void *pParam );
void taskShellCmd ( void *pParam );
//void taskPriod ( void *pParam );
void taskUartHandle ( void * pdata );
void taskRS485Rx ( void *param );
//
void GetVersion()
{
    static DWORD ver = 0;
    ver = *(DWORD *)ADDR_BIN_VER;
    DevVersion[0] = '0';
    DevVersion[1] = '0';
    DevVersion[2] = (BYTE)(ver>>16) + 0x30;
    DevVersion[3] = '.';
    DevVersion[4] = (BYTE)(ver) + 0x30;
    DevVersion[5] = '0';
    DevVersion[6] = '0';
}

void SetDefParam(void)
{
    static BYTE i,len;

    //定时汇报手机号码
    for (i=0;i<11;i++)   
    {
        SysParam[SP_MOBLENUM+i] = '9';
    }
    SysParam[SP_MOBLENUM+11] = 0;
    //NoCenter = 0;

    //本机号
    for (i=SP_SELFNUM;i<SP_SELFNUM+11;i++)   
    {
        SysParam[i] = '9';
    }
    SysParam[SP_SELFNUM+11] = 0;

    *(WORD *)&SysParam[SP_RECTIMEOUT] = DEF_RECTIMEOUT;       
    SysParam[SP_SW_VERSION]   = 0;
    SysParam[SP_SW_VERSION+1] = 0;
    *(DWORD *)&SysParam[SP_SW_LENGTH] = 0;
    *(WORD *)&SysParam[SP_SW_CURID] = 0;
    SysParam[SP_SW_UPDATE] = FALSE;
   
         
    //是否偷电 
    SysParam[SP_DEVICETAP] = 0;

    //是否循环控制外部设备    
    SysParam[SP_DEVICECTL] = 0;
   
    //是否启用HTTP    
    SysParam[SP_ENABLEHTTP] = DEF_ENABLEHTTP;

    //是否启用TCP   
    SysParam[SP_ENABLESOCKET] = DEF_ENABLESOCKET;
    
    SysParam[SP_COMMMODE] = COMMMODE_CDMA1X;
    SysParam[SP_NETMODE] = NETMODE_TCP;
    SysParam[SP_WORKMODE] = WORKMODE_MIX;
    
    //服务器域名
    len = StrLen(DefHost);
    mcpy(&SysParam[SP_HOSTNAME],(BYTE *)DefHost,len);
    
    //错误码   
    SysParam[SP_LASTERROR] = ERR_NULL;

    //电表类型
    SysParam[SP_EMTYPE]= 0;
    
    // 是否允许自动上报
    SysParam[SP_REPENABLE] = 1;
    
    //抄表时间间隔    
    *(WORD *)&SysParam[SP_READTIME] = 1000;
    
    // 电表数量
    SysParam[SP_METERCOUNT] = 3;
        
    // IP Mode 
    SysParam[SP_DEVIPMODE] = IPMODE_DHCP;

    //串口参数
    SysParam[SP_BAUDRATE] = DEF_BAUDRATE;
    SysParam[SP_DATABIT]  = DEF_DATABIT;
    SysParam[SP_CHECKBIT] = DEF_CHECKBIT;
    SysParam[SP_STOPBIT]  = DEF_STOPBIT;

    SysParam[SP_TIMERCOUNT] = 0;
    SysParam[SP_TIMER1_HOUR] = 0;
    SysParam[SP_TIMER1_MINUTE] = 0;
    SysParam[SP_TIMER2_HOUR] = 0;
    SysParam[SP_TIMER2_MINUTE] = 0;
    SysParam[SP_TIMER3_HOUR] = 0;
    SysParam[SP_TIMER3_MINUTE] = 0;

    /*
    *(WORD *)&SysParam[SP_CMDTIMER] = 0;
    SysParam[SP_CMDDIS] = 10;
    SysParam[SP_RETRYCNT] = 0;

    *(WORD *)&SysParam[SP_DATALENGTH] = 200;

    SysParam[SP_ENABLE_MINUTE] = DISABLE;
    SysParam[SP_MINUTE_MINUTE] = 0;

    SysParam[SP_ENABLE_HOUR] = DISABLE;
    SysParam[SP_HOUR_HOUR]   = 0;
    SysParam[SP_HOUR_MINUTE] = 0;

    SysParam[SP_ENABLE_DAY] = DISABLE;
    SysParam[SP_DAY_DAY]    = 0;
    SysParam[SP_DAY_HOUR]   = 0;
    SysParam[SP_DAY_MINUTE] = 0;

    SysParam[SP_ENABLE_MONTH] = DISABLE;
    SysParam[SP_MONTH_MONTH]    = 0;
    SysParam[SP_MONTH_DAY]   = 0;
    SysParam[SP_MONTH_HOUR] = 0;
    SysParam[SP_MINTH_MINUTE] = 0;
    */
    

    SysParam[SP_MODTYPE] = DEF_MODULE;

    *(WORD *)&SysParam[SP_RECTIMEOUT] = DEF_RECTIMEOUT;
   
    //网络用户名和密码       
    mset((BYTE *)&SysParam[SP_NETUSERNAME],0,32);
    mcpy((BYTE *)&SysParam[SP_NETUSERNAME],"card",4);
       
    mset((BYTE *)&SysParam[SP_NETPASSWORD],0,32);
    mcpy((BYTE *)&SysParam[SP_NETPASSWORD],"card",4);
    
    //打印调试信息   
    SysParam[SP_DEBUGMSG] = DEBUG_DISABLE;

    SysParam[SP_SW_VERSION]   = 0;
    SysParam[SP_SW_VERSION+1] = 0;

    *(DWORD *)&SysParam[SP_SW_LENGTH] = 0;
    *(WORD *)&SysParam[SP_SW_CURID] = 0;
    SysParam[SP_SW_UPDATE] = FALSE;

    *(DWORD *)&SysParam[SP_FLOWRATE] = 0;

}
void GetSysParam(void)
{
    static WORD i,j,len;
    static WORD Day;
    for (i=0;i<SYSPARAM_COUNT;i++)
    {
        SysParam[i] = *(BYTE *)(SYSPARAMSTART+i);
    }

    // 短信汇报手机号码
    for (i=0;i<11;i++)   
    {
        if ((SysParam[SP_MOBLENUM+i] < '0') || (SysParam[SP_MOBLENUM+i] > '9'))
        {
            //NoCenter = 1;
            SysParam[SP_MOBLENUM+i] = '9';
        }
    }
    SysParam[SP_MOBLENUM+11] = 0;

    //本机号
    for (i=SP_SELFNUM;i<SP_SELFNUM+11;i++)   
    {
        if ((SysParam[i] < '0') || (SysParam[i] > '9'))
        {
            SysParam[i] = '9';
        }
    }
    SysParam[SP_SELFNUM+11] = 0;
    
    // 汇报时间间隔
    Day = *(WORD *)&SysParam[SP_DAYDELTA];
    if (Day > 9999)
    {
        Day = 0;
        *(WORD *)&SysParam[SP_DAYDELTA] = 0;
    }

    //下次汇报时间
    Day = *(WORD *)&SysParam[SP_NEXTREPDAY];
    if (Day > 9999)
    {
        Day = 0;
        *(WORD *)&SysParam[SP_NEXTREPDAY] = 0;
    }

    //汇报时间
    if (SysParam[SP_REPTIME] > 23)
    {
        SysParam[SP_REPTIME] = 0;
    }

    //是否偷电 
    if (SysParam[SP_DEVICETAP] > 1)
    {
        SysParam[SP_DEVICETAP] = 0;
    }

    //是否循环控制外部设备
    if (SysParam[SP_DEVICECTL] > 1)
    {
        SysParam[SP_DEVICECTL] = 0;
    }

    //是否启用HTTP
    if (SysParam[SP_ENABLEHTTP] > 1)
    {
        SysParam[SP_ENABLEHTTP] = DEF_ENABLEHTTP;
    }


    //是否启用TCP
    if (SysParam[SP_ENABLESOCKET] > 1)
    {
        SysParam[SP_ENABLESOCKET] = DEF_ENABLESOCKET;
    }

    
    //服务器域名
    for (i=SP_HOSTNAME;i<SP_HOSTNAME+5;i++)
    {
        if ((SysParam[i]==0) || (SysParam[i]==0xFF))
        {
            for (j=SP_HOSTNAME;j<SP_HOSTNAME+32;j++)
            {
                SysParam[j] = 0;
            }
            len = StrLen(DefHost);
            mcpy((BYTE *)&SysParam[SP_HOSTNAME],(BYTE *)DefHost,len);
            break;
        }
    }
    

    //错误码
    if (SysParam[SP_LASTERROR] >= ERR_COUNT)
    {
        SysParam[SP_LASTERROR] = ERR_NULL;
    }

    
    if (SysParam[SP_EMTYPE] >= EM_TYPWCOUNT)
    {
        SysParam[SP_EMTYPE]= 0;
    }

    #if 0
    // 117.89.241.99   高信测试平台   221.237.10.76
    SysParam[SP_SERVERIP]   = 117;
    SysParam[SP_SERVERIP+1] = 89;
    SysParam[SP_SERVERIP+2] = 241;
    SysParam[SP_SERVERIP+3] = 99;
    *(WORD *)&SysParam[SP_SERVERPORT] = 7905;
    #endif

    // 设备地址
    if (mcmp(&SysParam[SP_DEVADDR],"\xFF\xFF\xFF\xFF\xFF",5))
    {
        //Def add
        mset(&SysParam[SP_DEVADDR],0x99,3);
        SysParam[SP_DEVADDR+3] = 0x0F;    // 9999
        SysParam[SP_DEVADDR+4] = 0x27;
    }
    
    #if 0
    mcpy(&SysParam[SP_DEVADDR],"\x12\x31\x11\x11\x11",5);
    #endif
    
    mcpy(DevAddrTmp,&SysParam[SP_DEVADDR],5);
    
    
    // 密码
    if (mcmp(&SysParam[SP_PASSWORD],"\xFF\xFF\xFF\xFF\xFF\xFF",6))
    {
        //Def Password  123456
        mcpy(&SysParam[SP_PASSWORD],"\x40\xE2\x01\x00\x00\x00",6);
    }
    
    

    //通讯模式
    if ((SysParam[SP_COMMMODE] < COMMMODE_EARTHNET) || (SysParam[SP_COMMMODE] > COMMMODE_CDMA1X))
    {
        SysParam[SP_COMMMODE] = COMMMODE_CDMA1X;
    }

    //网络模式
    if ((SysParam[SP_NETMODE] < NETMODE_TCP) || (SysParam[SP_NETMODE] > NETMODE_UDP))
    {
        SysParam[SP_NETMODE] = NETMODE_TCP;
    }

    //工作模式
    /*
    if ((SysParam[SP_WORKMODE] |= WORKMODE_NORMAL) && (SysParam[SP_WORKMODE] |= WORKMODE_TRANC))
    {
        SysParam[SP_WORKMODE] = WORKMODE_NORMAL;
    }
    */

    if ((SysParam[SP_WORKMODE] < WORKMODE_CLIENT) || (SysParam[SP_WORKMODE] > WORKMODE_MIX))
    {
        SysParam[SP_WORKMODE] = WORKMODE_MIX;
    }

    // 是否允许自动上报
    if (SysParam[SP_REPENABLE] > 1)
    {
        SysParam[SP_REPENABLE] = 1;
    }

    //抄表时间间隔
    if (*(WORD *)&SysParam[SP_READTIME] == 0xFFFF)
    {
        *(WORD *)&SysParam[SP_READTIME] = 1000;
    }

    // 电表数量
    if ((SysParam[SP_METERCOUNT] <1 ) || (SysParam[SP_METERCOUNT] > 9))
    {
        SysParam[SP_METERCOUNT] = 3;
    }
    
    //串口参数  -->
    if ((SysParam[SP_BAUDRATE] < BAUDRATE_300) || (SysParam[SP_BAUDRATE] > BAUDRATE_115200))
    {
        SysParam[SP_BAUDRATE] = DEF_BAUDRATE;
    }

    if (SysParam[SP_DATABIT] != 8)
    {
        SysParam[SP_DATABIT]  = DEF_DATABIT;
    }

    if ((SysParam[SP_CHECKBIT] < CHECK_ODD) || (SysParam[SP_CHECKBIT] > CHECK_NONE))
    {
        SysParam[SP_CHECKBIT] = DEF_CHECKBIT;
    }

    if (SysParam[SP_STOPBIT] != 1)
    {
        SysParam[SP_STOPBIT]  = DEF_STOPBIT;
    }
    //<--


    // IP Mode 
    if ((SysParam[SP_DEVIPMODE] < IPMODE_FIXED) || (SysParam[SP_DEVIPMODE] > IPMODE_PPPOE))
    {
        SysParam[SP_DEVIPMODE] = IPMODE_DHCP;
    }


    if (SysParam[SP_TIMERCOUNT] > 3)
    {
        SysParam[SP_TIMERCOUNT] = 0;
    }

    if (SysParam[SP_TIMER1_HOUR] > 23)
    {
        SysParam[SP_TIMER1_HOUR] = 0;
    }

    if (SysParam[SP_TIMER2_HOUR] > 23)
    {
        SysParam[SP_TIMER2_HOUR] = 0;
    }

    if (SysParam[SP_TIMER3_HOUR] > 23)
    {
        SysParam[SP_TIMER3_HOUR] = 0;
    }

    if (SysParam[SP_TIMER1_MINUTE] > 59)
    {
        SysParam[SP_TIMER1_MINUTE] = 0;
    }

    if (SysParam[SP_TIMER2_MINUTE] > 59)
    {
        SysParam[SP_TIMER2_MINUTE] = 0;
    }

    if (SysParam[SP_TIMER3_MINUTE] > 59)
    {
        SysParam[SP_TIMER3_MINUTE] = 0;
    }


    if (SysParam[SP_MODTYPE] > CDMA_MC323)
    {
        SysParam[SP_MODTYPE] = DEF_MODULE;
    }

    if (*(WORD *)&SysParam[SP_RECTIMEOUT] == 0xFFFF)
    {
        *(WORD *)&SysParam[SP_RECTIMEOUT] = DEF_RECTIMEOUT;
    }


    //网络用户名和密码    
    if (!CheckValidUserName())
    {
        mset((BYTE *)&SysParam[SP_NETUSERNAME],0,32);
        mcpy((BYTE *)&SysParam[SP_NETUSERNAME],"card",4);
    }

    if (!CheckValidPassWord())
    {
        mset((BYTE *)&SysParam[SP_NETPASSWORD],0,32);
        mcpy((BYTE *)&SysParam[SP_NETPASSWORD],"card",4);
    }

    //是否打印调试信息
    if (SysParam[SP_DEBUGMSG] > DEBUG_ENABLE)
    {
        SysParam[SP_DEBUGMSG] = DEBUG_DISABLE;
    }
    
    if (*(DWORD *)&SysParam[SP_FLOWRATE] == 0xFFFFFFFF)
    {
        *(DWORD *)&SysParam[SP_FLOWRATE] = 0;
    }
    //校表参数
    for(i=0;i<18;i+=2)
    {
        if((*(WORD *)&SysParam[SP_RNKIA+i] ==0xFFFF)||\
            (*(WORD *)&SysParam[SP_RNKIA+i] ==0))
        {
            *(WORD *)&SysParam[SP_RNKIA+i] = DEF_KI_VALUE;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if((*(WORD *)&SysParam[SP_RNKIB+i] ==0xFFFF) ||\
           (*(WORD *)&SysParam[SP_RNKIB+i] ==0) )
        {
            *(WORD *)&SysParam[SP_RNKIB+i] =DEF_KI_VALUE;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if((*(WORD *)&SysParam[SP_RNKU+i] ==0xFFFF)||\
            (*(WORD *)&SysParam[SP_RNKU+i] ==0))
        {
            *(WORD *)&SysParam[SP_RNKU+i] =DEF_KU_VALUE;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if((*(WORD *)&SysParam[SP_RNKAP+i] ==0xFFFF)||\
           (*(WORD *)&SysParam[SP_RNKAP+i] ==0))
        {
            *(WORD *)&SysParam[SP_RNKAP+i]= DEF_KP_VALUE;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if((*(WORD *)&SysParam[SP_RNKBP+i] ==0xFFFF)||\
            (*(WORD *)&SysParam[SP_RNKBP+i] ==0))
        {
            *(WORD *)&SysParam[SP_RNKBP+i]= DEF_KP_VALUE;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNIARMSOS+i] ==0xFFFF)   
        {
            *(WORD *)&SysParam[SP_RNIARMSOS+i]= 0x0;;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNIBRMSOS+i] ==0xFFFF)  
        {
            *(WORD *)&SysParam[SP_RNIBRMSOS+i]= 0x0;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNIBGAIN+i] ==0xFFFF)  
        {
            *(WORD *)&SysParam[SP_RNIBGAIN+i]= 0;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNGPQA+i] ==0xFFFF)   
        {
            *(WORD *)&SysParam[SP_RNGPQA+i]= 0x0000;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNGPQB+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNGPQB+i]= 0x0000;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNAPOSA+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNAPOSA+i]= 0x00;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNAPOSB+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNAPOSB+i]= 0x00;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNPHSA+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNPHSA+i]= 0x00;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNPHSB+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNPHSB+i]= 0x00;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNRPOSA+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNRPOSA+i]= 0x00;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNRPOSB+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNRPOSB+i]= 0x00;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNQPHSCAL+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNQPHSCAL+i]= 0;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNHFCONST+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNHFCONST+i]= 0x58b;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNSYSCON+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNSYSCON+i]= 0x40;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNPSTART+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNPSTART+i]= 0x60;
        }
    }
    for(i=0;i<18;i+=2)
    {
        if(*(WORD *)&SysParam[SP_RNQSTART+i] ==0xFFFF)    
        {
            *(WORD *)&SysParam[SP_RNQSTART+i]= 0x120;
        }
    }
    //电表电能寄存器溢出次数
    for(i=0;i<9;i++)
    {
        if(SysParam[SP_PEOIF_COUNT+i]==0xFF)
        {
            SysParam[SP_PEOIF_COUNT+i]=0;   
        }
    }
 
    //电表地址 6位*6
    for(i=0;i<6;i++)
    {
        if(SysParam[SP_METERADDR1+i]==0xFF)
        {
            SysParam[SP_METERADDR1+i]=0x01;   
        }
    }
    for(i=0;i<6;i++)
    {
        if(SysParam[SP_METERADDR2+i]==0xFF)
        {
            SysParam[SP_METERADDR2+i]=0x02;   
        }
    }
    for(i=0;i<6;i++)
    {
        if(SysParam[SP_METERADDR3+i]==0xFF)
        {
            SysParam[SP_METERADDR3+i]=0x03;   
        }
    }
    for(i=0;i<6;i++)
    {
        if(SysParam[SP_METERADDR4+i]==0xFF)
        {
            SysParam[SP_METERADDR4+i]=0x04;   
        }
    }
    for(i=0;i<6;i++)
    {
        if(SysParam[SP_METERADDR5+i]==0xFF)
        {
            SysParam[SP_METERADDR5+i]=0x05;   
        }
    }
    for(i=0;i<6;i++)
    {
        if(SysParam[SP_METERADDR6+i]==0xFF)
        {
            SysParam[SP_METERADDR6+i]=0x06;   
        }
    }
    if(*(WORD *)&SysParam[SP_RNIE]==0xFFFF)
    {
        *(WORD *)&SysParam[SP_RNIE]=0x0;//禁止中断脉冲输出        
    }
   
}


BYTE SaveSysParam(void)
{
   return saveParam(&gDevParam);
}
BYTE CheckValidUserName(void)
{
    static BYTE i;

    if (SysParam[SP_NETUSERNAME] == 0)
    {
        return FALSE;
    }
        
    for (i=0; i<32; i++)
    {
        if (SysParam[i+SP_NETUSERNAME] > 127)
        {
            return FALSE;
        }
    }
    return TRUE;
}

BYTE CheckValidPassWord(void)
{
    static BYTE i;

    if (SysParam[SP_NETPASSWORD] == 0)
    {
        return FALSE;
    }
    
    for (i=0; i<32; i++)
    {
        if (SysParam[i+SP_NETPASSWORD] > 127)
        {
            return FALSE;
        }
    }
    return TRUE;
}


void SysReset(void)
{
    //关闭无线模块电源
    MOD_POWEROFF();
    
    while(1)
    {
        ;
    }
}

BYTE SysSelfCheck(void)
{
    /*if (!CheckRealTime())
    {
        DebugStr("RealTimer Err.\r\n");
        SetLastError(ERR_RTCFAIL);
        return FALSE;
    }*/
      //显示当前时间
    DebugChar(SysTime[0]/16+0x30);
    DebugChar(SysTime[0]%16+0x30);
    DebugChar('-');

    DebugChar(SysTime[2]/16+0x30);
    DebugChar(SysTime[2]%16+0x30);
    DebugChar('-');

    DebugChar(SysTime[3]/16+0x30);
    DebugChar(SysTime[3]%16+0x30);
    DebugChar(' ');
    
    DebugChar(SysTime[4]/16+0x30);
    DebugChar(SysTime[4]%16+0x30);
    DebugChar(':');

    DebugChar(SysTime[5]/16+0x30);
    DebugChar(SysTime[5]%16+0x30);
    DebugChar(':');

    DebugChar(SysTime[6]/16+0x30);
    DebugChar(SysTime[6]%16+0x30);

    printf("\r\n");
    

    return TRUE;    
}


/*******************************************************************************
*函数名:
　　()
*功能: 打开时钟
*输入:
*输出:
*说明:
*******************************************************************************/
int ethernet_Init( void )
{
    unsigned long ulUser0, ulUser1;
    char pucMACArray[8];
    uchar *pex = SysParam;
//    unsigned int  i;
//    unsigned int ulTemp;
#if 1 
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_ETH );
    SysCtlPeripheralReset ( SYSCTL_PERIPH_ETH );

    SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOF );
    GPIOPinConfigure ( GPIO_PF2_LED1 );
    GPIOPinConfigure ( GPIO_PF3_LED0 );
    GPIOPinTypeEthernetLED ( GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3 );
   
    GPIODirModeSet(SYSCTL_PERIPH_GPIOF, GPIO_PIN_2, GPIO_DIR_MODE_OUT);
    GPIOPadConfigSet(SYSCTL_PERIPH_GPIOF, GPIO_PIN_2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
    GPIODirModeSet(SYSCTL_PERIPH_GPIOF, GPIO_PIN_3, GPIO_DIR_MODE_OUT);
    GPIOPadConfigSet(SYSCTL_PERIPH_GPIOF, GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
    
#else

    SysCtlPeripheralReset(SYSCTL_PERIPH_ETH);                  /*复位以太网*/
    
    /*****以下就是对以太网的设置******************/
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ETH);                 /*使能以太网外设*/
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);               /*使能LED外设*/
        
    
    GPIODirModeSet(GPIO_PORTF_BASE, (GPIO_PIN_2 | GPIO_PIN_3),
                   GPIO_DIR_MODE_HW);
    GPIOPadConfigSet(GPIO_PORTF_BASE, (GPIO_PIN_2 | GPIO_PIN_3),
                   GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);             /*配置LED由硬件控制，输出电流为2ma*/
    
    
    EthernetIntDisable(ETH_BASE, (ETH_INT_PHY | ETH_INT_MDIO | ETH_INT_RXER |
                           ETH_INT_RXOF | ETH_INT_TX | ETH_INT_TXER | ETH_INT_RX));
    ulTemp = EthernetIntStatus(ETH_BASE, false);
    EthernetIntClear(ETH_BASE, ulTemp);
    
    EthernetInit(ETH_BASE);                                     /*对于第一次使用以太网的初始化*/
    for(i=0;i<255;i++) ;
    EthernetConfigSet(ETH_BASE, (ETH_CFG_TX_DPLXEN | ETH_CFG_TX_CRCEN |
                                     ETH_CFG_TX_PADEN));            /*使能全双工模式，自动进行CRC校验，自动填充发送数据达到最小值*/
         
    EthernetEnable(ETH_BASE);                                   /*使能以太网*/
    
    //IntEnable(INT_ETH);                                         /*使能以太网外设中断*/
    //IntPrioritySet(INT_ETH, 0xFF);                              /*设置以太网中断优先级*/
    //EthernetIntEnable(ETH_BASE, ETH_INT_RX);                    /*使能以太网接收中断*/
#endif
    //
    // Configure SysTick for a periodic interrupt.
    //
    //SysTickPeriodSet ( SysCtlClockGet() / SYSTICKHZ );
    //SysTickEnable();
    //SysTickIntEnable();

    //
    // Enable processor interrupts.
    //
    IntMasterEnable();


    //
    // Configure the hardware MAC address for Ethernet Controller filtering of
    // incoming packets.
    //
    // For the LM3S6965 Evaluation Kit, the MAC address will be stored in the
    // non-volatile USER0 and USER1 registers.  These registers can be read
    // using the FlashUserGet function, as illustrated below.
    //

    /*mac 地址从芯片内部单次烧写位置读取*/  /*写MAC地址*/
    FlashUserGet ( &ulUser0, &ulUser1 );

    if(ulUser0 == 0xffffffff  ||  ulUser1 == 0xffffffff) 
    {
        uint *add = (uint *)(&pex[SP_MACADDR]);
        if(*add == 0xFFFFFFFF || *add == 0)
        {
            setRandMac();
            saveParam( &gDevParam );
        }
        pucMACArray[0] = pex[SP_MACADDR];
        pucMACArray[1] = pex[SP_MACADDR+1];
        pucMACArray[2] = pex[SP_MACADDR+2];
        pucMACArray[3] = pex[SP_MACADDR+3];
        pucMACArray[4] = pex[SP_MACADDR+4];
        pucMACArray[5] = pex[SP_MACADDR+5];
    }
    else
    {
        pucMACArray[0] = ( ( ulUser0 >>  0 ) & 0xff );
        pucMACArray[1] = ( ( ulUser0 >>  8 ) & 0xff );
        pucMACArray[2] = ( ( ulUser0 >> 16 ) & 0xff );
        pucMACArray[3] = ( ( ulUser1 >>  0 ) & 0xff );
        pucMACArray[4] = ( ( ulUser1 >>  8 ) & 0xff );
        pucMACArray[5] = ( ( ulUser1 >> 16 ) & 0xff );
    }

   printf("\r\neth mac addr:%02X-%02X-%02X-%02X-%02X-%02X",pucMACArray[0],pucMACArray[1],
                            pucMACArray[2],pucMACArray[3],pucMACArray[4],pucMACArray[5]);

    if (SysParam[SP_DEVIPMODE] == IPMODE_DHCP)
    {
           printf("\r\nuse dynamic IP address.");
           lwIPInit((uchar *)pucMACArray, 0, 0, 0, IPADDR_USE_DHCP);
    }
    else
    {
        unsigned int ipaddr;

        printf ( "\r\nstatic IP addr:%s" ,getStrParamIpAddr ( (char *)&SysParam[ SP_LOCALIP] ) );
        
        memcpy ( &ipaddr , &SysParam[SP_LOCALIP] , 4 );
        ipaddr =  htonl ( ipaddr );

         lwIPInit ( (uchar *)pucMACArray, ipaddr,
                   htonl ( inet_addr ( gDevParam.DevIpAddrMask ) ),
                   htonl ( inet_addr ( gDevParam.DevIpGatway ) ),
                   IPADDR_USE_STATIC );
    }
   
    EthernetEnable(ETH_BASE); 

    return true;

}


void  Tmr_TickInit ( void )
{
    SysTickPeriodSet ( ( unsigned int ) ( SysCtlClockGet() / OS_TICKS_PER_SEC ) - 1 );
    SysTickEnable();
    SysTickIntEnable();
}


/*******************************************************************************
*函数名:
　　()
*功能: 打开时钟
*输入:
*输出:
*说明:
*******************************************************************************/
void taskWatchDog ( void *pParam )
{
    printf ( "\r\nWatchDog task begin..." );

    SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);
    
    //
    // 检查寄存器是否被锁定，如果锁定了寄存器，将它们释放。
    //
    if(WatchdogLockState(WATCHDOG0_BASE) == true)
    {
        WatchdogUnlock(WATCHDOG0_BASE);
    }
   
    //
    // 初始化看门狗定时器。
    //
    WatchdogReloadSet(WATCHDOG0_BASE,0xFEEFEE);
    //
    // 使能复位。
    //   
    WatchdogResetEnable(WATCHDOG0_BASE);
    //
    // 使能看门狗定时器。
    //   
    WatchdogEnable(WATCHDOG0_BASE);
  
    //IntEnable(INT_WATCHDOG);
    //
    // 喂狗
    //  

    while(1)
    {
       
        WatchdogReloadSet(WATCHDOG0_BASE, TheSysClock*7);//5s
        OSTimeDly(OS_TICKS_PER_SEC*2);
    }
  
}

void taskMainPackHandle ( void *p_arg )
{

    gPubVal.resetCause  = SysCtlResetCauseGet();
    SysCtlResetCauseClear(gPubVal.resetCause);
    
    printf("\r\n\nresetCause:%x " ,gPubVal.resetCause  );
    printf("\nCPU speed %u",SysCtlClockGet());    
    printf ( "\r\nMainPackHandle task begin..." );
    
    loadParamFromFlash( &gDevParam );//读取参数
    uartInitRs485();

    commBaseCtrlInit ( UART0_BASE , 0 );
    commBaseCtrlInit ( UART1_BASE , 1 );
    commBaseCtrlInit ( UART2_BASE , 2 );
   
    IntEnable ( INT_UART0 );
    IntEnable ( INT_UART1 );
    IntEnable ( INT_UART2 );
    
    
    ethernet_Init();
    spiInit();
    OSTimeDly(2 * OS_TICKS_PER_SEC);
    init8209();
      
    if ( gPubVal.bCfgShell )
    {
        OSTaskCreate ( CmdLine,         // Initialize the start task  初始化shell任务
                       ( void * ) 0 ,
                       &stkTaskShell[STK_SIZE_SHELL-1],
                       PRIO_SHELL );
    }
    else
    {
       OSTaskCreate ( taskUartHandle ,         // Initialize the start task  初始化shell任务
                       ( void * ) 0 ,
                       &stkTaskShell[STK_SIZE_SHELL-1],
                       PRIO_SHELL );
    } 
              
    OSTaskCreate ( taskPriod,         //  初始化定期任务
                   ( void * ) 0 ,
                   &stkTaskPrid[STK_SIZE_PRID-1],
                   PRIO_PRID );
 

    OSTaskCreate ( taskWatchDog,         //   初始化taskWatchDog任务
                   ( void * ) 0 ,
                   &stkTaskWatch0[STK_SIZE_WATCH0-1],
                   PRIO_WATCH0 );
#if 1
    OSTaskCreate ( taskHeartRep,         // Initialize the start task  初始化定期任务
                   ( void * ) 0 ,
                   &stkTaskHearReq[STK_SIZE_HEART_REP-1],
                   PRIO_HEART_REP );

    OSTaskCreate ( taskRS485Rx,         // Initialize the start task  初始化485任务
                   ( void * ) 0 ,
                   &stkTaskRS485[STK_SIZE_RS485-1],
                   PRIO_RS485 );
 
   
    OSTaskCreate ( taskGSMGetMsg,         // 
                   ( void * ) 0 ,
                   &stkTaskGSMMSG[STK_SIZE_GSMMSG-1],
                   PRIO_GSMMSG);
                   
     OSTaskCreate ( taskGSM,         // 
                   ( void * ) 0 ,
                   &stkTaskGSM[STK_SIZE_GSM-1],
                   PRIO_GSM);
#endif                   
    tcpServerInit();
    upDateServerInit();

    //printf("\r\n\r\nUpdatae test!!!!!!!!!!!!!\r\n"); 
    while (1) 
    {                             
        OSTaskSuspend(OS_PRIO_SELF);  /*  The start task can be pended here */
    } 
      
}


/*******************************************************************************
*函数名:
　　()
*功能: 打开时钟
*输入:
*输出:
*说明:
*******************************************************************************/
int main ( void )
{
    memset ( &gPubVal , 0 , sizeof ( gPubVal ) );
    //sprintf(DevRelTime, "%s %s " , __DATE__ , __TIME__ );
    gPubVal.bCfgShell = true;
    
    //strcpy(gPubVal.version, SWVERSION);
    //strcpy(gPubVal.devNo, DEVICENO);
    //sprintf(gPubVal.buildTime, "%s %s " , __DATE__ , __TIME__ );

    IntDisAll();                       // Disable all the interrupts 关闭所有中断
    
//    SysCtlClockSet( SYSCTL_SYSDIV_12 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ ); //for 9B96
//    SysCtlClockSet( SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ );  //for 6938
    SysCtlClockSet( SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ );  //for 6938

    TheSysClock = SysCtlClockGet();
    FlashUsecSet(TheSysClock/1000000);
    
    GetVersion();
    gpioCommInit();
    commUartInit();
    I2C_FM24C_Init();
    IntMasterEnable();//

    OSInit();                          // Initialize the kernel of uC/OS-II 初始化uC/OS-II的内核

    OSTaskCreate ( taskMainPackHandle,         // Initialize the start task  初始化启动任务
                   ( void * ) 0,
                   &stkTaskMainHandle[STK_SIZE_MAIN_HANDLE-1],
                   PRIO_MAIN_HANDLE ); 
    
    Tmr_TickInit();//为OS 提供时钟
   
    OSStart();                        // Start uC/OS-II  启动uC/OS-II
  
    return ( 0 ) ;

}



