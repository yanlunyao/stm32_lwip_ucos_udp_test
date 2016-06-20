/*************************************************************
³É¶¼ê»ÆÕ»·±£¼¼ÊõÓÐÏÞ¹«Ë¾   °æÈ¨ËùÓÐ


ÎÄ¼þÃû:   
×÷  Õß:         
Ãè  Êö:
ÐÞ¶©¼ÇÂ¼:

**************************************************************/


#ifndef __COMM_H__
#define __COMM_H__
#include "devUartProc.h"


#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long
#define uint  unsigned int

#define INNER_WDOG    // ÄÚ²¿¿´ÃÅ¹·
#define CDMA_SRVMODE // CDMA ·þÎñÆ÷Ä£Ê½

//Sys Address Define
#define MAINAPPSTART      0x0
#define BAKAPPSTART       0x0001FC00
#define SYSPARAMSTART     0x0003FC00
#define APPMAXLENGTH      0x1FC00   //127*1024

#define DEBUG_DISABLE   0
#define DEBUG_ENABLE    1

//²¨ÌØÂÊ
#define BAUDRATE_150        0
#define BAUDRATE_300        1
#define BAUDRATE_600        2
#define BAUDRATE_1200       3
#define BAUDRATE_2400       4
#define BAUDRATE_4800       5
#define BAUDRATE_9600       6
#define BAUDRATE_19200      7
#define BAUDRATE_38400      8
#define BAUDRATE_57600      9
#define BAUDRATE_115200     10

//Ð£Ñé
#define CHECK_ODD      1     // ÆæÐ£Ñé
#define CHECK_EVEN     2     // Å¼Ð£Ñé
#define CHECK_NONE     3     // ÎÞÐ£Ñé

#define GSM_MG323    0
#define CDMA_MC323   1

//#define MODULE_MG323
//#define MODULE_MC323
//#define MODULE_GTM900 

//#define RT_DS1302   // HT1381
#undef  RT_DS1302 

#define BYTE  unsigned char
#define WORD  unsigned short
#define DWORD unsigned long


#define FALSE     0
#define TRUE      1
#define OK        2
#define TIMEOUT   3
#define LINKDOWN  4  // Á´Â·¶Ï¿ª
#define NOMETER   5  // Ã»ÓÐ¼ì²âµ½µç±í

//IPµØÖ·»ñÈ¡·½Ê½
#define IPMODE_FIXED   1
#define IPMODE_DHCP    2
#define IPMODE_PPPOE   3

#define DEVFRAME    1   // Éè±¸Ö¡
#define METERFRAME  2  // µç±íÖ¡
#define MODBUSFRAME 3  //mod µç±í


#define SENDBUFLEN  100

//¶ÌÐÅµç±íÅäÖÃ
#define HTTP_ENABLE        1   
#define SOCKET_ENABLE      1

#define RETRY_COUNT       3


typedef void (*tFun)();
typedef struct 
{
    WORD Time;   //MS
    tFun Fun;
}LOGICTIMER;



// ÏµÍ³ÔËÐÐÄ£Ê½
//#define SYSMODE_CONTRON    0
//#define SYSMODE_CHECK      1
//#define SYSMODE_TEST       2

#define COMMMODE_EARTHNET  1
#define COMMMODE_CDMA1X    2

#define NETMODE_TCP     1
#define NETMODE_UDP     2

#define WORKMODE_CLIENT  1
#define WORKMODE_SERVER  2
#define WORKMODE_MIX     3


#define SP_MOBLENUM       0  // ³¤¶È 12    »ã±¨ÊÖ»úºÅÂë
#define SP_DAYDELTA       12 // ³¤¶È 2    »ã±¨Ê±¼ä¼ä¸ô
#define SP_NEXTREPDAY     14 // ³¤¶È 2   ÏÂ´Î»ã±¨Ê±¼ä
#define SP_REPTIME        16 // ³¤¶È 1    »ã±¨Ê±¼ä
#define SP_DEVICETAP      17 // ³¤¶È 1    ÊÇ·ñÍµµç

#define SP_DEVICECTL      18 // ³¤¶È 1    ÊÇ·ñ¶¨Ê±¿ª¹ØÍâÉè
#define SP_ENABLEHTTP     19 // ³¤¶È1     ÊÇ·ñÊ¹ÓÃHTTP
#define SP_HOSTNAME       20 // ³¤¶È32    ·þÎñÆ÷ÓòÃû
#define SP_SELFNUM        52 // ³¤¶È12    ±¾»úºÅÂë  
#define SP_LASTERROR      64  //³¤¶È1     ´íÎóÂë
#define SP_EMTYPE         65  //³¤¶È1     µç±íÀàÐÍ

/*heart server*/
#define SP_SERVERIP       66  // ³¤¶È4    ·þÎñÆ÷IPµØÖ·
/*def to 7080*/
#define SP_SERVERPORT     70  // ³¤¶È2    ·þÎñÆ÷¶Ë¿Ú

/*local ip*/
#define SP_LOCALIP        72  // ³¤¶È4     ±¾µØIP
#define SP_LOCALPORT      76  // ³¤¶È2     ±¾µØ¶Ë¿Ú

/*no need*/
#define SP_SLEEPTIME      78  // ³¤¶È4     ÐÝÃßÊ±¼ä : Ãë

/*need to */
#define SP_ENABLESOCKET   88  // ³¤¶È1    ÊÇ·ñÊ¹ÓÃSOCKET-TCP
#define SP_COMMMODE       89  // 1     Í¨Ñ¶Ä£Ê½      1: ÒÔÌ«Íø  2: ÎÞÏß(CDMA1X)
/*devId*/
#define SP_DEVADDR        90  // ³¤¶È5  Éè±¸µØÖ· 
#define SP_NETMODE        95  // 1     ÍøÂçÍ¨Ñ¶Ä£Ê½   1: TCP  2: UDP 3:TCP-DHCP
/*pwd*/
#define SP_PASSWORD       96  // ³¤¶È6  ÃÜÂë
/*fix to misc*/
#define SP_WORKMODE       102 // 1  ¹¤×÷Ä£Ê

/*no need*/
#define SP_REPENABLE      103 // 1  ÊÇ·ñÔÊÐí×Ô¶¯ÉÏ±¨

/*Ãë def to 3600*/
#define SP_HEARTTIME      104 // 2  ÐÄÌø°üÖÜÆÚ
/*no need*/
#define SP_READTIME       106 // 2   ³­±íÊ±¼ä¼ä¸ô
#define SP_METERCOUNT     108 // 1  µç±íÊýÁ¿
//#define SP_USEDHCP        109 // 1  ipµØÖ·¹Ì¶¨»òÊ¹ÓÃDHCP
#define SP_MACADDR        110 //6  MACµØÖ·
#define SP_MODTYPE        116 // 1 Í¨ÐÅÄ£¿éÀàÐÍ

//485´®¿Ú²ÎÊý
#define SP_BAUDRATE       117  // ³¤¶È1    µç±í´®¿Ú²¨ÌØÂÊ  
#define SP_DATABIT        118  // ³¤¶È1    Êý¾ÝÎ»      
#define SP_CHECKBIT       119  // ³¤¶È1    Ð£ÑéÎ»      //01:ÆæÐ£Ñé;02:Å¼Ð£Ñé;03ÎÞÐ£Ñé
#define SP_STOPBIT        120  // ³¤¶È1    Í£Ö¹Î»   

#define SP_DEVIPMODE      121 // ³¤¶È1     ²É¼¯Æ÷IPµØÖ·Ä£Ê½  1: ¾²Ì¬£¬ 2: DHCP  3: PPPOE

//CDMA ÍøÂçÓÃ»§ÃûºÍÃÜÂë
#define SP_NETUSERNAME    122 // ³¤¶È32    ÍøÂçÓÃ»§Ãû
#define SP_NETPASSWORD    154 // ³¤¶È32    ÍøÂçÃÜÂë   -- 224
#define SP_DEBUGMSG       186// 1  ÊÇ·ñ´òÓ¡µ÷ÊÔÐÅÏ¢


//¶¨µã²É¼¯Ê±¿Ì
#define SP_TIMERCOUNT      187
#define SP_TIMER1_HOUR     188
#define SP_TIMER1_MINUTE   189
#define SP_TIMER2_HOUR     190
#define SP_TIMER2_MINUTE   191
#define SP_TIMER3_HOUR     192
#define SP_TIMER3_MINUTE   193

#define SP_RECTIMEOUT      194  // ³¤¶È2 ½ÓÊÕ³¬Ê±

//Ð£±í²ÎÊý
#define SP_RNKIA           196  //³¤¶È9*2
#define SP_RNKIB           214 //³¤¶È9*2
#define SP_RNKU            232  //³¤¶È9*2
#define SP_RNKAP           250 //³¤¶È9*2
#define SP_RNKBP           268 //³¤¶È9*2
        
#define SP_RNSYSCON        286 // 2*9  
#define SP_RNIARMSOS       304 // 2*9
#define SP_RNIBRMSOS       322 // 2*9
#define SP_RNIBGAIN        340 // 2*9 
#define SP_RNGPQA          358 // 2*9
#define SP_RNGPQB          376 // 2*9 
#define SP_RNAPOSA         394 // 2*9  
#define SP_RNAPOSB         412 // 2*9 
#define SP_RNPHSA          430 // 2*9
#define SP_RNPHSB          448 // 2*9 
#define SP_RNRPOSA         466 // 2*9
#define SP_RNRPOSB         484 // 2*9 
#define SP_RNQPHSCAL       502 //2*9 
#define SP_RNHFCONST       520 //2*9 
#define SP_RNPSTART        538 //2*9
#define SP_RNQSTART        556 //2*9

#define SP_PEOIF_COUNT     574 // 1*9,ÓÐ¹¦µçÄÜ¼Ä´æÆ÷Òç³ö´ÎÊý 

#define SP_METERADDR1       583//1*6 µç±íµØÖ·1,
#define SP_METERADDR2       589//1*6 µç±íµØÖ·2,
#define SP_METERADDR3       595//1*6 µç±íµØÖ·3,
#define SP_METERADDR4       601//1*6 µç±íµØÖ·4,
#define SP_METERADDR5       607//1*6 µç±íµØÖ·5,
#define SP_METERADDR6       613//1*6 µç±íµØÖ·6,
#define SP_RNIE             619 // 2*1 Ê¹ÄÜÖÐ¶Ï,
#define SP_FLOWRATE         621 // 4 Êý¾ÝÁ÷Á¿

// Èí¼þÉý¼¶²ÎÊý
#define SP_SW_VERSION       625  // 2 ÐÂÈí¼þ°æ±¾  
#define SP_SW_LENGTH        627  // 4 ÐÂÈí¼þ³¤¶È
#define SP_SW_CURID         631  // 2 µ±Ç°Êý¾Ý°üID
#define SP_SW_UPDATE        633  // 1 Èí¼þÉý¼¶±êÖ¾  0: ²»Éý¼¶  1 : ³É¹¦ÏÂÔØÍê³É£¬ÐèÒªÉý¼¶

#define SYSPARAM_COUNT      636  // 4µÄ±¶Êý

#if 0
#define DELTA_DOWN_LIMIT  1
#define DELTA_UP_LIMIT    10

#define TMP_DOWN_LIMIT   20
#define TMP_UP_LIMIT     45

#define HUM_DOWN_LIMIT   50
#define HUM_UP_LIMIT     95

#define BAT_DOWN_LIMIT   40
#define BAT_UP_LIMIT     60

#define DIS_DOWN_LIMIT   2
#define DIS_UP_LIMIT     60

#define LEN_DOWN_LIMIT   1
#define LEN_UP_LIMIT     60

#define AIR_DOWN_LIMIT   20
#define AIR_UP_LIMIT     40

#define TC_DOWN_LIMIT   0
#define TC_UP_LIMIT     30

#define HOT_DOWN_LIMIT  5
#define HOT_UP_LIMIT    20



//²ÎÊýÄ¬ÈÏÖµ
#define DEF_COOLSTART    25
#define DEF_COOLDELTA    3
#define DEF_HOTSTART     10
#define DEF_HOTDELTA     5
#define DEF_AIRCONSTART   29
#define DEF_AIRCONDELTA   3
#endif

#define DEF_ENABLEHTTP        HTTP_ENABLE
#define DEF_ENABLESOCKET      SOCKET_ENABLE 
#define DEF_HEARTTIME         3600

#define DEF_MODULE       CDMA_MC323
//#define DEF_SRVPORT       7080       // ·þÎñÆ÷Ä¬ÈÏ¶Ë¿Ú
//#define DEF_LOCPORT       9000       // ±¾µØ¼àÌýÄ¬ÈÏ¶Ë¿Ú
#define WORKMODE_NORMAL  0
#define WORKMODE_TRANC   0xFF

//485´®¿ÚÄ¬ÈÏ²ÎÊý
#define DEF_BAUDRATE     BAUDRATE_1200  
#define DEF_DATABIT      8
#define DEF_STOPBIT      1
#define DEF_CHECKBIT     CHECK_EVEN   // Å¼Ð£Ñé

#define DEF_RECTIMEOUT   120   // 2·ÖÖÓ

//RN8209±ÈÀýÏµÊýÄ¬ÈÏÖµ
#define DEF_KI_VALUE 472  //µçÁ÷Í¨µÀ±ÈÀýÖµ
#define DEF_KU_VALUE 4655  //µçÑ¹Í¨µÀ±ÈÀýÖµ
#define DEF_KP_VALUE 720  //¹¦ÂÊÍ¨µÀ±ÈÀýÖµ

#if 1
#define LED_ON   0
#define LED_OFF  1
#define EM_TYPWCOUNT 3

//´íÎóÂë¶¨Òå
#define ERR_COUNT      16
#define ERR_NULL       0
#define ERR_EMFAIL     1
#define ERR_MODULEFAIL 2
#define ERR_GSMFAIL    3
#define ERR_GPRSFAIL   4
#define ERR_TCPIPFAIL  5
#define ERR_RTCFAIL    6    // ÊµÊ±ÖÓ
#define ERR_HEARTFAIL  7
#define ERR_CONNSRVFAIL 8
#define ERR_REGSRVFAIL  9
#define ERR_COMMFAIL    10
#define ERR_NOMETER    11    // Ã»ÓÐµç±í£¬»òµç±íÎÞÊý¾Ý·µ»Ø
#define ERR_CHECK      12    // Ð£Ñé´íÎó
#define ERR_NOTMYDATA  13   // ·Ç±¾»úÊý¾Ý
#define ERR_INVALIDDATA 14  // ·Ç·¨»ò²»ÍêÕûµÄÊý¾Ý
#define ERR_LINKDOWN    15   // Á´Â·Òì³£¶Ï¿ª

#define SET_ALARM(Alm)     SysAlarm |= Alm
#define CLEAR_ALARM(Alm)   SysAlarm &= ~Alm
#define GET_ALARM(Alm)     (SysAlarm & Alm)
#endif

//ÎÞÏßÄ£¿é
#define MOD_POWERON()    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_7,0)
#define MOD_POWEROFF()   GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_7,GPIO_PIN_7)
#define MOD_RESETHIGH()  GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6,0)
#define MOD_RESETLOW()   GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6,GPIO_PIN_6)
#define MOD_PDHIGH()     GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_5,0) 
#define MOD_PDLOW()      GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_5,GPIO_PIN_5) 


extern const DWORD BaudRateTable[];

BYTE HexToBcd(BYTE Hex);
BYTE BcdToHex(BYTE Bh, BYTE Bl);
BYTE mcmp(BYTE *Des,BYTE *Src,BYTE Len);
void mcpy(BYTE *Des,BYTE *Src, WORD Len);

BYTE mcmp2(BYTE *Des,BYTE Src,BYTE Len);
BYTE mcmp3(BYTE *Des,BYTE *Src,BYTE Len,BYTE bit);
void mset(BYTE *Des, BYTE Data, BYTE Len);
BYTE StrLen(const char *Str);
void DebugMsg(void);
void DebugWord(WORD Num);

void Init_WDog(void);
void WDog(void);
void Sleep (WORD ms);
void DebugStr(const char * Msg) ;
void SetLastError(BYTE Err);
void ALM_SW(void);
void DebugChar(BYTE ch);
void IpToStr(BYTE *IpAddr);
BYTE StrToIpAdd(BYTE *IpAddr);

WORD DateToDay(void);
void PortToStr(BYTE *Port);
WORD DateToDay(void);
void DayToDate(WORD day);

BYTE StrToWord(BYTE *Str, BYTE Len, WORD *Dat);

BYTE StrToByte(BYTE *Str, BYTE Len, BYTE *Dat);
BYTE CheckValidIpAddr(void);
BYTE StrToPort(BYTE *Port);
BYTE CheckValidPort(void);
uint MeanValue(uint *pBuf ,BYTE len);


#endif
