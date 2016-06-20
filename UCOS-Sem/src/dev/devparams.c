
#include "includes.h"
#include <dev.h>
#include <comm.h>
#include <inet.h>
#include <ip_addr.h>
#include <string.h>
#include <ctype.h>
#include <flash.h>
#include <crc.h>
#include "Lwiplib.h"
#include "rn8209x.h"
#include "devI2c.h"
#include "devTimer.h"

bool checkParam ( tDevParam *pParam )
{
    int i;
    uchar xor = 0;
    uint addval = 0;
    
    uchar *pch = ( uchar * )pParam;
    uchar savexor = pParam->xor;
    uint  saveaddval = pParam->addval;

    pParam->xor = 0;
    pParam->addval = 0;

    for ( i = 0; i < sizeof(tDevParam)-sizeof(int); i++ )
    {
        xor ^= pch[i];
        addval += pch[i];
    }
    printf("\ncheck:%x/%x %x/%x size=%d/%d", savexor, xor, saveaddval, addval, sizeof(tDevParam),i);
  
    if ( ( xor != savexor ) || ( addval != saveaddval ))
    {   
        return false;
    }

    return true;
}

/*******************************************************************************
*函数名:
　　()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
void  loadParamFromFlash ( tDevParam *pParam )
{
    ushort gcrc;
    uchar i;
    memcpy ( ( void * ) pParam , ( void * ) FLASH_ADDR_PRARM  , sizeof ( tDevParam ) );

    gcrc = pParam->scrc;
    pParam->scrc = 0;

    if(pParam->flag != FLASH_PARAM_FLAG )
     //  || (gcrc != CRC16((uchar *)pParam,sizeof(tDevParam)) && !checkParam(pParam)))
    {
        printf("\nload parameter from flash error, set to default.");
        setParamToDefault();
        saveParam ( pParam ); 
        
        //第一次烧写初始时钟和铁电，以后掉电和复位不初始化
        Init_Rtc();
        //clear EEPROM 
        Clear_FM24C(FMSLAVE_ADDR,0x00,EEPROM_COUNT);
        
    }
    else
    {
        pParam->scrc = gcrc;
        printf("\nload parameter from flash ok, CRC=%04x", gcrc);

        Read_FM24C(FMSLAVE_ADDR, EEPROM_ENERGYADDR, EEPROMPARAM, EEPROM_COUNT);
        /*for(i=0;i<EEPROM_COUNT;i++)
        {
            printf("\r\n%x  ",*(DWORD *)&EEPROMPARAM[i]);
        }*/
        //将上次掉电前的值取出,如果是看门狗复位则板子没有掉电，rn8209也没有掉电，
        //电能寄存器的值还在，所以在取出时减去复位前寄存器的值
        if(gPubVal.resetCause&0x08)
        {            
            for(i=0;i<9;i++)
            {
                *(DWORD *)&EEPROMPARAM[EEPROM_ENERGY+4*i] = *(DWORD *)&EEPROMPARAM[EEPROM_ENERGY+4*i] - *(DWORD *)&EEPROMPARAM[EEPROM_ENERGYLAST+4*i] ;
            }
        }
        
    }
    memcpy(DevAddrTmp , &pParam->paramEx[SP_DEVADDR] ,  sizeof(DevAddrTmp));

}

/*******************************************************************************
*函数名:
　　()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
char * getStrParamIpAddr ( char *ipAddr )
{
    struct in_addr   ip;
    
    memcpy ( &ip , ipAddr , 4 );

    return  inet_ntoa ( ip );

}

void showIpParam(void)
{
   unsigned long ulIPAddress;
   ulIPAddress =  lwIPLocalIPAddrGet();
   printf("\r\n lwIPLocalIPAddrGet()[0x%x]" , ulIPAddress);

}


/*******************************************************************************
*函数名:
　　()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
uint saveParam ( tDevParam *pParam )
{
    if (bDevAddModify)
    {
        _pl_;
        printf(" bDevAddModify is %d  " , bDevAddModify);
        bDevAddModify = false;
       memcpy(&pParam->paramEx[SP_DEVADDR] ,  DevAddrTmp , sizeof(DevAddrTmp));
    }
     
    pParam->flag = FLASH_PARAM_FLAG;
    pParam->scrc = 0;
    pParam->scrc = CRC16((uchar *)pParam,sizeof(tDevParam));

    mcpy(&SysParam[SP_DEVADDR],DevAddrTmp,5);
    //printf("\r\nsaveParam is called, CRC=0x%04x ", pParam->scrc);
    if ( 0 == FlashErase ( ( unsigned long ) FLASH_ADDR_PRARM ))
    {
        return ( 0 == FlashProgram ( ( unsigned long *) pParam , FLASH_ADDR_PRARM , sizeof ( tDevParam ) ) );
    }

    return false;
}

/*******************************************************************************
*函数名:
　　()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
static char getRand()
{
    int i;
    int tmp = 0;

    for(i=0; i<6; i++)
    {
        tmp += SysTime[i];
    }
    srand(0x10000);
    tmp += rand();
    
    return (tmp % 0xFF);
}

void setRandMac(void)
{
    char *mac = (char *)gDevParam.paramEx;
    
    //mac地址的最后两个字节用随机数
    mac[SP_MACADDR]   = 0x00;
    mac[SP_MACADDR+1] = 0x22;
    mac[SP_MACADDR+2] = 0x44;
    mac[SP_MACADDR+3] = 0xC2;
    mac[SP_MACADDR+4] = getRand();
    mac[SP_MACADDR+5] = (getRand()*0xF)%0xFF;

    printf("\nnew Mac:%02X-%02X-%02X-%02X-%02X-%02X",mac[SP_MACADDR],mac[SP_MACADDR+1],mac[SP_MACADDR+2],mac[SP_MACADDR+3],mac[SP_MACADDR+4],mac[SP_MACADDR+5]);
}
/*******************************************************************************
*函数名:
　　()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
void setParamExDef ( char * pex )
{
    struct ip_addr ipaddr;

    GetSysParam();
      
    ipaddr.addr  = /*htonl*/ ( inet_addr ( "118.112.231.22" ) );
    memcpy ( &pex[ SP_SERVERIP]  , &ipaddr , 4 );

    *(WORD *)&pex[ SP_SERVERPORT] = 7080 ;
    
    ipaddr.addr  = /*htonl*/ ( inet_addr ( "192.168.0.200" ) );
    memcpy ( &pex[ SP_LOCALIP]  , &ipaddr , 4 );

    *(WORD *)&pex[ SP_LOCALPORT] = 9000; 
        
    //0x01 e2 40,密码
    pex[SP_PASSWORD] = 0x40;
    pex[SP_PASSWORD+1] = 0xe2;
    pex[SP_PASSWORD+2] = 0x01;
    pex[SP_PASSWORD+3] = 0x00;
    pex[SP_PASSWORD+4] = 0x00;
    pex[SP_PASSWORD+5] = 0x00;

    
    *(WORD *)&pex[ SP_HEARTTIME] = DEF_HEARTTIME;//心跳包周期
    *(DWORD *)&pex[ SP_SLEEPTIME] = 600;
    

    //更新标志
    pex[SP_SW_UPDATE]  = 0;

    setRandMac();   

}
#if 0 
static bool macFormatError(char *mac)
{
    int i;
    int num = 0;
    char *prt = mac;
    char *pnext = NULL;

    for(i=0; i<strlen(mac); i++)
    {
        if(isxdigit(mac[i]) || mac[i] == '-')
        {
            ;
        } else 
        {
            return true;
        }
    }
    if(mac[2]!='-' || mac[5]!='-' || mac[8]!='-' || mac[11]!='-' || mac[14]!='-')
    {
        return true;
    }
    
    do{
        pnext = strchr(mac, '-');        
        if(pnext)
        {
            if(pnext-prt != 2)
            {   
                return true;
            }
            num++;
            strcpy(pnext, pnext+1);
            prt = pnext;
        }
    }while(pnext);
    
    if(num != 5)
    {   
        return true;
    }

    return false;
    
}

static void chtonum(char *mac)
{
    int i;
    int ms;
    int ls;
    int index = 0;

    for(i=0; i<=10; i+=2)
    {
        if(isdigit(mac[i]))
        {
            ms = mac[i] - '0';
        } else 
        {
            ms = tolower(mac[i]) - 'a' + 10;
        }
        if(isdigit(mac[i+1]))
        {
            ls = mac[i+1] - '0';
        } else 
        {
            ls = tolower(mac[i+1]) - 'a' + 10;
        }
        mac[index++] = ms*16 + ls;
    }
}

ushort getParamBpsIdx(unsigned short bps)
{
    int i; 
     unsigned int bpsary[] = {300 , 600,1200,2400,4800,9600,19200,38400,57600,115200};

    for(i=0;i<10;i++)
    {
        if (bps == bpsary[i])
            return i+1;
    }
    
    return 0;     
    
}

/*******************************************************************************
*函数名:
　　()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
uint cmdSaveParam ( CMD_LIN * pCmd )
{
    saveParam ( &gDevParam );
    SysReset();

    return 0;
}


ushort getParamBps(void)
{
     
     unsigned int bpsary[] = {300 , 600,1200,2400,4800,9600,19200,38400,57600,115200};

     if ((SysParam[SP_BAUDRATE] == 0) || (SysParam[SP_BAUDRATE] > 9))
     {
        return 1200;
     }

     return bpsary[SysParam[SP_BAUDRATE] -1] ;
    
}
#endif

/*******************************************************************************
*函数名:
　　()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
void setParamToDefault(void)
{
        setParamExDef ( (char *)gDevParam.paramEx );       
        strcpy ( gDevParam.DevIpAddrMask , "255.255.255.0" );
        strcpy ( gDevParam.DevIpGatway, "0.0.0.0" );
}


