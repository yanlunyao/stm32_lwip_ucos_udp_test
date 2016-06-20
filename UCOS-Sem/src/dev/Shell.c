


#include <includes.h>
#include <stdio.h>
#include <ethernet.h>
#include <dev.h>
#include <Em485.h>
#include <devTimer.h>
#include <hw_flash.h>
#include <flash.h>
#include <hw_gpio.h>
#include <hw_types.h>
#include <ctype.h>
#include <string.h>
#include <SCom.h>
#include <crc.h>
#include <spi.h>
#include <rn8209x.h>
#include "Cdma1x.h"
#include "DLT645.h"
#include "devI2c.h"

#if 1
#define shellGetChar()  commBaseGetChar ( 1 )
#else
#define shellGetChar()  commBaseGetChar ( 0 )
#endif

static char cmdBuf[MAX_CMD_BUF_LEN+2];
static CMD_LIN cmd;

/*******************************************************************************
*函数名:
　　()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/

int fputc ( int ch, FILE *f )
{
    shellPutChar ( ch );

    if ( ch == 0x0a )
    {
        shellPutChar ( 0x0d );
    }

    return ch;
}


int fgetc ( FILE *  f )
{
    f = f;
    return shellGetChar();
}



/*******************************************************************************
*函数名:
　　parseCmd()
*功能:shell命令解析
*输入:
*输出:
*说明:
*******************************************************************************/
static int parseCmd
(
    char * buf, // buffer of command line
    CMD_LIN * pCmd  // command
)
{
    char * pCh;
    char * pArgv;
    char blank_flag = 0;
    uint i, j;


    pCh = buf;
    pCmd->argc = 0;
    pCmd->level = 0;

    for ( i = 0; i < PARA_CNT_MAX; i++ )
        pCmd->argv[i][0] = '\0';

    for ( i = 0; i < PARA_CNT_MAX; i++ )
    {
        j = 0;
        blank_flag = 0;
        pArgv = ( char* ) & pCmd->argv[i][0];

        while ( 1 )
        {
            //skip the space
            if ( *pCh == ' ' || *pCh == '\t' )
            {
                pCh++;
                blank_flag = 1;
            }
            else
                break;
        }

        while ( isalnum( ( uint ) *pCh ) || *pCh == '.' || *pCh == '-')
        {
            if ( j == ( PARA_LEN_MAX - 1 ) )
            {
                pCh++;
                continue;
            }

            j++;
            *pArgv++ = *pCh++;

        }

        if ( *pCh == '?' )
            *pArgv++ = '?';

        *pArgv = '\0';

        if ( ( ! ( *pCh == '\r' || *pCh == '\n' ) ) || blank_flag == 0 || j != 0 )
        {
            pCmd->argc++;
        }

        if ( *pCh == '\r' || *pCh == '\n' || *pCh == '?' )
            break;
    }

    if ( i >= PARA_CNT_MAX )
        return ( -1 );

    return i;
}
/*******************************************************************************
*函数名:
　　matchCmd()
*功能:参数匹配
*输入:
*输出:
*说明:
*******************************************************************************/
int matchCmd ( CMD_LIN * pCmd, const linCmdTabSTRUC * pCmdTab )
{
    linCmdTabSTRUC * pTempTab = ( linCmdTabSTRUC * ) pCmdTab;
    uint level = pCmd->level;
    char *pString = pCmd->argv[level];
    uint helpFlag = FALSE;
    uint i, j, k;

    pCmd->level++;

    if ( pString[0] == '?' )
    {
        while ( pTempTab->routine != NULL )
        {
            printf ( "\n\r      %-20s%s", pTempTab->cmdName, pTempTab->cmdHelp );
            pTempTab++;
        }

        return -1;
    }

    if ( pString[0] == '\0' )
    {
        printf ( "\n\rType \"" );
        i = 0;

        while ( i < level )
        {
            printf ( "%s ", pCmd->argv[i] );
            i++;
        }

        printf ( "?\" for a list of subcommands" );
        return -1;
    }

    if ( pString[ ( strlen ( ( const char * ) pString ) - 1 ) ] == '?' )
    {
        pString[ ( strlen ( ( const char * ) pString ) - 1 ) ] = '\0';
        helpFlag = TRUE;
    }

    i = j = k = 0;

    while ( strlen (pTempTab->cmdName) )
    {
        if ( !strncmp ( pString, pTempTab->cmdName, strlen ( ( const char * ) pString ) ) )
        {
            j++;
            k = i;
        }

        i++;
        pTempTab++;

    }

    pTempTab = ( linCmdTabSTRUC * ) pCmdTab;

    if ( ( j == 1 ) && ( helpFlag == FALSE ) )
    {
        strcpy ( pString, ( pTempTab + k )->cmdName );
        return k;
    }
    else if ( j != 0 )
    {
        if ( helpFlag == TRUE )
        {
            while ( strlen (pTempTab->cmdName) )
            {
                if ( !strncmp ( pString, pTempTab->cmdName, strlen ( ( const char * ) pString ) ) )
                {
                    printf ( "\n\r      %-20s%s", pTempTab->cmdName, pTempTab->cmdHelp );
                }

                pTempTab++;

            }
        }
        else
        {
            printf ( "\n\rAmbiguous command: %s", pString );
            printf ( "\n\rPlease select:" );

            while ( strlen (pTempTab->cmdName) )
            {
                if ( !strncmp ( pString, pTempTab->cmdName, strlen ( ( const char * ) pString ) ) )
                {
                    printf ( "\n\r%-16s%s", "", pTempTab->cmdName );
                }

                pTempTab++;

            }
        }
    }
    else
    {
        printf ( "\n\rno this command <%s>!", pString );
    }

    return -1;
}
/*******************************************************************************
*函数名:
　　matchDigit()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
uint matchDigit ( CMD_LIN * pCmd, uint base, const linCmdTabSTRUC * pCmdTab )
{
    linCmdTabSTRUC * pTempTab = ( linCmdTabSTRUC * ) pCmdTab;
    uint level = pCmd->level;
    char * pString = pCmd->argv[level];
    uint i, j;

    pCmd->level++;

    if ( ( pString[0] == '?' ) || ( pString[ ( strlen ( ( const char * ) pString ) - 1 ) ] == '?' ) )
    {
        printf ( "\n\r      %-20s%s", pTempTab->cmdName, pTempTab->cmdHelp );
        return false;
    }

    if ( pString[0] == '\0' )
    {
        printf ( "\n\rType \"" );
        i = 0;

        while ( i < level )
        {
            printf ( "%s ", pCmd->argv[i] );
            i++;
        }

        printf ( "?\" for a list of subcommands" );
        return false;
    }

    j = strlen ( ( const char * ) pString );

    if ( base == 10 )
    {
        for ( i = 0; i < j; i++ )
        {
            if ( isdigit ( pString[i] ) == FALSE )
            {
                printf ( "\n\r<%s> is not a decade number!", pString );
                return false;
            }
        }
    }

    if ( base == 16 )
    {
        for ( i = 0; i < j; i++ )
        {
            if ( isxdigit ( pString[i] ) == FALSE )
            {
                if ( ( ( pString[i] != 'x' ) && ( pString[i] != 'X' ) ) )
                {
                    printf ( "\n\r<%s> is not a hex number!", pString );
                    return false;
                }
            }
        }
    }

    return true;
}


uint nullRoute ( CMD_LIN *pCmd )
{
    return true;
}


/*******************************************************************************
*函数名:
　　StrToNum()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
uint StrToNum ( char * str )
{
    uint temp = 0;
    uint bHex = 0;
    char * pch = str;

    while ( *pch != 0 )
    {
        if ( ( ( *pch ) == 'x' ) || ( ( *pch ) == 'X' ) )
        {
            bHex = 1;
            break;
        }

        pch++;
    }

    if ( !bHex )
    {
        return ( uint ) ( atoi ( str ) );
    }
    else
    {
        do
        {
            pch++;

            if ( *pch == 0 )
            {
                return temp;
            }
            else
            {
                temp <<= 4;

                if ( ( *pch >= '0' ) && ( *pch <= '9' ) )
                    temp += *pch - '0';
                else if ( ( *pch >= 'a' ) && ( *pch <= 'f' ) )
                    temp += ( *pch - 'a' ) + 0x0a ;
                else if ( ( *pch >= 'A' ) && ( *pch <= 'F' ) )
                    temp += ( *pch - 'A' ) + 0x0a ;
                else
                    return 0;
            }
        }
        while ( 1 );
    }
}

#if 0
/*******************************************************************************
*函数名:
　　cmdMemRead()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
uint cmdMemRead ( CMD_LIN * pCmd )
{  
    char * pint8 = NULL;
    unsigned short * pint16 = NULL;
    uint * pint32 = NULL;
    uint val = 0;
    uint addr = 0;
    uint i = 0;

    uint readtimes = 1;

    if ( pCmd->argc < 3 )
    {
        printf ( "\r\n useage : memrd byte/hfword/word  addr [cnt]" );
        printf ( "\r\n exarm:  memrd word 0x40000000 " );
        return true;
    }

    if ( pCmd->argc == 4 )
    {
        readtimes = StrToNum ( pCmd->argv[3] );
        printf ( "\readtimes:%d ", readtimes );
    }



    printf ( "\r\n" );

    addr = StrToNum ( pCmd->argv[2] );
    pint8 = ( char * ) addr;
    pint16 = ( unsigned short * ) addr;
    pint32 = ( uint * ) addr;

    if ( readtimes == 1 )
    {
        if ( 0 == strcmp ( pCmd->argv[1], "byte" ) )
        {
            val = *pint8;
        }
        else if ( 0 == strcmp ( pCmd->argv[1], "hfword" ) )
        {
            val = *pint16;
        }
        else if ( 0 == strcmp ( pCmd->argv[1], "word" ) )
        {
            val = *pint32;
        }
        else
        {
            printf ( "\r\n arg must be one of byte/hfword/word !" );
            return true;
        }

        printf ( "\r\n read from address 0x%x get val 0x%x(%d) \r\n ", addr,
                 val, val );
    }
    else
    {
        printf ( "\r\nread from address 0x%x get val : \r\n   ", addr );

        if ( 0 == strcmp ( pCmd->argv[1], "byte" ) )
        {
            for ( i = 0; i < readtimes; i++ )
                printf ( "-0x%x", pint8[i] );
        }
        else if ( 0 == strcmp ( pCmd->argv[1], "hfword" ) )
        {
            for ( i = 0; i < readtimes; i++ )
                printf ( "-0x%x", pint16[i] );
        }
        else if ( 0 == strcmp ( pCmd->argv[1], "word" ) )
        {
            for ( i = 0; i < readtimes; i++ )
                printf ( "-0x%x", pint32[i] );
        }
        else
        {
            printf ( "\r\n arg must be one of byte/hfword/word !" );
            return true;
        }
    }

    return true;

}

/*******************************************************************************
*函数名:
　　cmdMemWrite()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
uint cmdMemWrite ( CMD_LIN * pCmd )
{
    char * pint8 = NULL;
    unsigned short * pint16 = NULL;
    uint * pint32 = NULL;
    uint val = 0;
    uint addr = 0;

    if ( pCmd->argc != 4 )
    {
        printf ( "\r\n useage : memwr byte/hfword/word  addr val" );
        printf ( "\r\n exarm:  memwr word 0x40000000  0x05 " );
        return true;
    }

    printf ( "\r\n" );

    addr = StrToNum ( pCmd->argv[2] );
    pint8 = ( char * ) addr;
    pint16 = ( unsigned short * ) addr;
    pint32 = ( uint * ) addr;
    val = StrToNum ( pCmd->argv[3] );

    if ( 0 == strcmp ( pCmd->argv[1], "byte" ) )
    {
        *pint8 = val;
    }
    else if ( 0 == strcmp ( pCmd->argv[1], "hfword" ) )
    {
        *pint16 = val;
    }
    else if ( 0 == strcmp ( pCmd->argv[1], "word" ) )
    {
        *pint32 = val;
    }
    else
    {
        printf ( "\r\n arg must be one of byte/hfword/word !" );
        return true;
    }

    printf ( "\r\n write  to address 0x%x with val 0x%x(%d) \r\n ", addr, val,
             val );
    return true;
}

#endif
uint cmdReset ( CMD_LIN * pCmd )
{
    /******* reset mcu ********/
    //HWREG ( NVIC_APINT ) = NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ;
    SysCtlReset();

    return true;
}

#if 0
uint cmmPinOut ( CMD_LIN * pCmd )
{
    const uint bases[9] =
    {
        GPIO_PORTA_BASE,
        GPIO_PORTB_BASE,
        GPIO_PORTC_BASE,
        GPIO_PORTD_BASE,
        GPIO_PORTE_BASE,
        GPIO_PORTF_BASE,
        GPIO_PORTG_BASE,
        GPIO_PORTH_BASE,
        GPIO_PORTJ_BASE

    };

    uint base , pin, high;

    if ( ( pCmd->argv[1][0] >= 'a' ) && ( pCmd->argv[1][0] <= 'j' ) )
        pCmd->argv[1][0] = pCmd->argv[1][0] - 'a' + 'A';


    if ( pCmd->argv[1][0] == 'J' )
    {
        base = 8;
    }
    else
    {
        base  = pCmd->argv[1][0] - 'A' ;
    }

    pin  = StrToNum ( pCmd->argv[2] );

    if ( pCmd->argv[3][0] == 'i' )
    {
        printf ( "\r\n pinIn:0%x" ,  signalIn ( bases[base], 0x01 << pin ) );
    }
    else
    {
        high  = StrToNum ( pCmd->argv[3] );
        signalOut ( bases[base], 0x01 << pin , high );
    }

    return true;
}
#endif

uint cmdReadFhy ( CMD_LIN * pCmd )
{
    unsigned char addr;
    unsigned int val;
 
    addr = StrToNum ( pCmd->argv[1] );

    val = EthernetPHYRead ( ETH_BASE , addr );

    printf ( "\r\n phy addr 0x%x = 0x%x " , addr, val );
    return true;
}

uint cmdWriteFhy ( CMD_LIN * pCmd )
{

    unsigned char addr;
    unsigned int val;

    addr = StrToNum ( pCmd->argv[1] );
    val = StrToNum ( pCmd->argv[2] );

    printf ( "\r\nwrite fhy 0x%x with 0x%x " , addr, val );
    EthernetPHYWrite ( ETH_BASE , addr, val );
    return true;
}


uint cmdShow ( CMD_LIN * pCmd )
{
    if ( strncmp ( pCmd->argv[1] , "param" , 4 ) == 0 )
    {
        showParam();
    }
    else if ( strncmp ( pCmd->argv[1] , "debug" , 4 ) == 0 )
    {
        showDebug();
    }
    else if(strncmp ( pCmd->argv[1] , "meteraddr" , 9 ) == 0)
    {
        ShowMeterAddr();
    }
    else
    {
        printf("\r\nshow param  or  show debug ");
    }
    return 0;
}

#if 0
uint cmdBeep ( CMD_LIN * pCmd )
{
    if (pCmd->argc < 2)
    {
        printf("\r\n beep on/off");
        return 0;
    }

    if ( strncmp ( pCmd->argv[1] , "on" , 2 ) == 0 )
    {
        BEEP(1);
    }

    if ( strncmp ( pCmd->argv[1] , "off" , 3 ) == 0 )
    {
        BEEP(0);
    }
    return 0;	    
}

#endif
uint cmdDebug ( CMD_LIN * pCmd )
{
    if (pCmd->argc < 2)
    {
        printf("\r\n debug yes/no/img/modtest/dbtest");
        return 0;
    }
    
    if ( strncmp ( pCmd->argv[1] , "yes" , 2 ) == 0 )
    {
        debugFlag |= 0x01;
    }
    else  if ( strncmp ( pCmd->argv[1] , "no" , 2 ) == 0 )
    {
        debugFlag &= ~ ( 0x01 ) ; 
    }
    else  if ( strncmp ( pCmd->argv[1] , "dbtest" , 3 ) == 0 )
    {
        printf("\r\n测试电表");
        sendTestStrToRs485();
    }
    else  if ( strncmp ( pCmd->argv[1] , "modtest" , 3 ) == 0 )
    {
        printf("\r\n测试MOD电表");

        sendModTestStrToRs485();
    }
    else
    {
        printf ( "\r\ndebug yes/no" );
    }

    return 0;
}

uint cmdParamDef ( CMD_LIN * pCmd )
{
    setParamToDefault ();
    return 0;
}

BYTE dataT[]=
{ 
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 
    0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 
    0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 
    0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4, 
    0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 
    0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 
    0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 
    0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 
    0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 
    0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 
    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 
    0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 
    0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 
    0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 
    0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 
    0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 
    0x43, 0x83, 0x41, 0x81, 0x80, 0x40 
};

bool flashWrite(uint addr, const void *str, uint len)
{
    if(len > 1024)
    {   
        return false;
    }
    if ( 0 == FlashErase( ( unsigned long )addr ))
    {   
        return ( 0 == FlashProgram( ( unsigned long * )str, addr, len ) );
    }
  
    return false;
}

#if 0
uint cmdFlasht ( CMD_LIN * pCmd )
{
    int i;
    uint total = 0;
    uint rerr = 0, werr = 0;
    ushort scrc, ncrc;
    uchar buff[256];
    uint taddr = 1024 * 127;

    while(1)
    {
        for(i = 0; i < 256; i++)
        {
            dataT[i] ^= dataT[total%256];
        }
        scrc = CRC16(dataT, 256);
        
        if(!flashWrite(taddr, dataT, 256))
        {
            werr++;
        }
        OSTimeDly (2 * OS_TICKS_PER_SEC);

        //read
        memset(buff, 0, 256);
        memcpy ( ( void * )buff, ( void * )taddr, 256);

        ncrc = CRC16(buff, 256);
        if(scrc != ncrc)
        {
            rerr++;
        }
        total++;

        printf("\n flash test(w/r/t): %d/%d/%d  CRC:%04x/%04x", werr, rerr, total, scrc, ncrc);
    }
}

uint multiGet( CMD_LIN * pCmd )
{
    printf("\nGPIO_O_AFSEL:%x", HWREG(GPIO_PORTC_BASE + GPIO_O_AFSEL));
    printf("\nGPIO_O_DEN  :%x", HWREG(GPIO_PORTC_BASE + GPIO_O_DEN));
    printf("\nGPIO_O_PDR  :%x", HWREG(GPIO_PORTC_BASE + GPIO_O_PDR));
    printf("\nGPIO_O_PUR  :%x", HWREG(GPIO_PORTC_BASE + GPIO_O_PUR));
    printf("\nGPIO_O_PCTL :%x", HWREG(GPIO_PORTC_BASE + GPIO_O_PCTL));
    printf("\nGPIO_O_CR   :%x", HWREG(GPIO_PORTC_BASE + GPIO_O_CR));
    printf("\nGPIO_O_LOCK :%x", HWREG(GPIO_PORTC_BASE + GPIO_O_LOCK)); 
    return 0;
}
#endif

#if 0
uint jtagSet( CMD_LIN * pCmd )
{
    if(pCmd->argc < 2)
    {
        printf("\nplease input setjtag open/closse");
        return 0;
    }

    if(strcmp(pCmd->argv[1], "close") == 0)
    {
        printf("\nclose the jtag interface.");

        //unlock first
        HWREG(GPIO_PORTC_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY_DD;
        while(HWREG(GPIO_PORTC_BASE + GPIO_O_LOCK))
            ;
        HWREG(GPIO_PORTC_BASE + GPIO_O_CR) = 0xC;

        HWREG(GPIO_PORTC_BASE + GPIO_O_AFSEL) &= 0xF3;
        HWREG(GPIO_PORTC_BASE + GPIO_O_DEN) &= 0xF3;
        HWREG(GPIO_PORTC_BASE + GPIO_O_PUR) &= 0xF3;
        HWREG(GPIO_PORTC_BASE + GPIO_O_PCTL) &= 0xFFFF0033;
    }

    if(strcmp(pCmd->argv[1], "open") == 0)
    {
        printf("\nopen the jtag interface.");

        //unlock first
        HWREG(GPIO_PORTC_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY_DD;
        while(HWREG(GPIO_PORTC_BASE + GPIO_O_LOCK))
            ;
        HWREG(GPIO_PORTC_BASE + GPIO_O_CR) = 0xC;

        HWREG(GPIO_PORTC_BASE + GPIO_O_AFSEL) |= 0xC;
        HWREG(GPIO_PORTC_BASE + GPIO_O_DEN) |= 0xC;
        HWREG(GPIO_PORTC_BASE + GPIO_O_PUR) |= 0xC;
        HWREG(GPIO_PORTC_BASE + GPIO_O_PCTL) |= 0x3300;
    }

    HWREG(GPIO_PORTC_BASE + GPIO_O_CR) = 0;
    HWREG(GPIO_PORTC_BASE + GPIO_O_LOCK) = 0;
    multiGet(NULL);

	return 0;
}


uint spiRW( CMD_LIN * pCmd )
{
 
    int bytes = 0;
    uint data = 0;
    uint add = 0;
    uint channel = 0;
    
    if (pCmd->argc < 2)
    {
        printf("\n spi read/write");
        return 0;
    }
    if ( strcmp ( pCmd->argv[1] , "init" ) == 0 )
    {
        spiInit();
        return 0;
    }

    channel = StrToNum(pCmd->argv[2]);
    add = StrToNum(pCmd->argv[3]);
    bytes = StrToNum(pCmd->argv[4]);
    if ( strcmp ( pCmd->argv[1] , "read" ) == 0 )
    {   
        if(spiRead(channel, add, &data, bytes))
        {
            printf("\n channel=%d add=0x%x: bytes=%d data=0x%x", channel, add, bytes, data);
        }
        else
        {
            printf("\n channel=%d add 0x%x read fail.", channel, add);
        }
        
    }

    data = StrToNum(pCmd->argv[5]);
    if ( strcmp ( pCmd->argv[1] , "write" ) == 0 )
    {
        if(spiWrite(channel,add, data, bytes))
        {
            printf("\n channel=%d  add=0x%x: bytes=%d data=0x%x write ok", channel, add, bytes, data);
        }
        else
        {
            printf("\n channel=%d add=0x%x: write fail", channel, add);
        }
    }
    spiRegShow();
	return 1;

}



uint cmdAverage( CMD_LIN * pCmd )
{
    int i;
    int times;
    int bytes = 0;
    uint sum = 0;
    uint data = 0;
    uint add = 0;
    uint channel = 0;

    channel = StrToNum(pCmd->argv[1]);
    add = StrToNum(pCmd->argv[2]);
    bytes = StrToNum(pCmd->argv[3]);
    times = StrToNum(pCmd->argv[4]);
    printf("\n");
    for(i = 0; i < times; i++)
    {
         if(spiRead(channel, add, &data, bytes))
        {
            if(add == 0x24)
                data -= 0x232;
            sum += data;
            printf(" %d-0x%x", i, data);
        }
        else
        {
            i--;
            //printf("\n channel=%d add 0x%x read fail.", channel, add);
        }
        OSTimeDly (OS_TICKS_PER_SEC);
    }
    data = sum/times;
    data = ~(data * data) + 1;
    printf("\n %d times read, sum:0x%x average:0x%x/%d CC:0x%x",times,sum,sum/times,sum/times,data);
    return 1;
}


uint cmdResetSPI( CMD_LIN * pCmd )
{
    resetSPI();
    return 1;

}
#endif

uint cmd8209Show( CMD_LIN * pCmd )
{    
    if (pCmd->argc < 2)
    {
        printf("\n input channel please.");
        return 0;
    }
    getRawMeasure(StrToNum(pCmd->argv[1]));
    
    return 1;
}

uint cmdSetRnParam(CMD_LIN * pCmd)
{
    int short ret = 0;
    BYTE channel=0;
    uint data =0;
    uint temp;

    if(strcmp ( pCmd->argv[1] , "emucon" ) == 0)//校表时用于设置脉冲加速模块，此值不会被保存，重新上电后恢复默认值
    {  
        spiRead(StrToNum ( pCmd->argv[2]), EMUCON, &temp, 2);
        
        spiWrite(StrToNum ( pCmd->argv[2]), EMUCON, temp|StrToNum ( pCmd->argv[3]), 2);
        return true;
    }
    
    ret = matchCmd ( pCmd, cmmSetRnParamCmdTab );
    channel = StrToNum ( pCmd->argv[2]);
    data = StrToNum ( pCmd->argv[3]);
    
    if ( ret == ERROR)
    {
        return 0;
    }
    
    if(ret <= 21)//小于21时，对输入参数数量进行核实
    {
        if(pCmd->argc < 2)
        {
            printf("\r\n input ParamName please.");
            return 0;
        }
        else if(pCmd->argc < 3)
        {
            printf("\r\n input channel/number please.");
            return 0;
        }
        else if(pCmd->argc < 4)
        {
            printf("\r\n input Parameter please.");
            return 0;
        }
        else if(pCmd->argc > 4)
        {
            printf("\r\n Command format error.");
            return 0;
        }
    }

    switch(ret)
    {
        case 21://set address
            if(channel > 17)
            {
                printf("\r\n Address number out of range\r\n");
                return 0;
            }
            if(data > 0xFF)
            {
                printf("\r\n Value out of range\r\n");
                return 0;
            }
            SysParam[SP_METERADDR1 + channel] = data;
            ShowMeterAddr();
            if(SaveSysParam())
            {
                printf("\n Set Succease.\n");
                return true;
            }
        break;
        
        case 22://checkself
            if(( strcmp ( pCmd->argv[2] , "all" )==0)&&(pCmd->argc == 3))
            {
                printf("\n Check began...");
                RnCheckSelf();
                if(SaveSysParam())
                {
                    printf("\n CheckSelf end.\r\n");
                }
                return true;   
            }
            else if(pCmd->argc == 3)
            {
                if(StrToNum ( pCmd->argv[2])>= 9)
                {
                    printf("\r\n Channel out of range\r\n");
                    return 0;   
                }
                printf("\n Check began...");
                RnCheckSelfCh(StrToNum ( pCmd->argv[2]));
                if(SaveSysParam())
                {
                    printf("\n CheckSelf end.\r\n");
                }
                return true;   
            }
        break;
        
        case 23: //enable interrupt
            *(WORD *)&SysParam[SP_RNIE]=0x0A;//开启有功电能溢出中断和PF中断
            for(channel=0;channel<9;channel++)
            {
                spiWrite(channel, IE,*(WORD *)&SysParam[SP_RNIE] ,WIDTH_ONE_B);
            }
            if(SaveSysParam())
            {
                printf("\n Set Succease.\n");
            }
        break;
        
        case 24://disable interrput
            *(WORD *)&SysParam[SP_RNIE]=0x00;
            for(channel=0;channel<9;channel++)
            {
                spiWrite(channel, IE,*(WORD *)&SysParam[SP_RNIE] ,WIDTH_ONE_B);
            }
            if(SaveSysParam())
            {
                printf("\n Set Succease.\n");
            }
        
        break;

        case 25:
            printf("\n Check Ki");
            if(strcmp ( pCmd->argv[2] , "all" )==0)
            {
                for(channel =0;channel<9;channel++)
                {
                    Check_KIA_KIB(channel,data);    
                }
            }
            else
            {
                Check_KIA_KIB(channel,data);
            }
            if(SaveSysParam())
            {
                printf("\n Check end.\n");
                return true;
            }
        break;

        case 26:
            printf("\n Check Ku");
            if(strcmp ( pCmd->argv[2] , "all" )==0)
            {
                for(channel =0;channel<9;channel++)
                {
                    Check_KU(channel,data);    
                }
            }
            else
            {
                Check_KU(channel,data);
            }
       
            if(SaveSysParam())
            {
                printf("\n Check end.\n");
                return true;
            }
        break;

        case 27:
            printf("\n Check Kp");

            if(strcmp ( pCmd->argv[2] , "all" )==0)
            {
                for(channel =0;channel<9;channel++)
                {
                    Check_KPA_KPB(channel,data);    
                }
            }
            else
            {
                Check_KPA_KPB(channel,data);
            }
        
            if(SaveSysParam())
            {
                printf("\n Check end.\n");
                return true;
            }
        break;
        
        case 28:
            printf("\n Check HFconst");
            if(strcmp ( pCmd->argv[2] , "all" )==0)
            {
                for(channel=0;channel<9;channel++)
                {                   
                    Check_HFConst1(channel,data);
                    temp=0;
                }
                if(SaveSysParam())
                printf("\n Set Succease.\n");
            }
            else
            {
                Check_HFConst1(data,channel);
                if(SaveSysParam())
                printf("\n input enable!\n");
            }           
        break;

        case 29://clear
            printf("\n Clear the electric energy");
            
            Clear_FM24C(FMSLAVE_ADDR, A_EEAddr,81);
            for(channel=0;channel<9;channel++)
            {
                SysParam[SP_PEOIF_COUNT+channel]=0;
            }
            SaveSysParam();
        break;
        case 30://read eeprom
            for(channel=0;channel<9;channel++)
            {
                Read_FM24C(FMSLAVE_ADDR, EEPROM_ENERGYADDR+channel*4,(uchar *)&data,4);
                printf("\nEE%d=0x%x ",channel,data);
            }
            
        break;
        default:
            if(channel >=9)//小于21时，
            {
                printf("\r\n Channel out of range\r\n");
                return 0;
            }
            if(data > 0xFFFF)
            {
                printf("\r\n Value out of range\r\n");
                return 0;
            }
        
            *(WORD*)&SysParam[SP_RNKIA+(ret*18)+(channel*2)] = data;
            printf("SetValue=%d",*(WORD*)&SysParam[SP_RNKIA+(ret*18)+(channel*2)]);
            
            if(SaveSysParam())
                {
                    if(ret > 4)//如果设置寄存器的值则需要将值写入RN8209
                    {    
                        
                        spiWrite(channel, initTable[ret-5].add,*(initTable[ret-5].value+channel), initTable[ret-5].Bwidth);
                    }
                    printf("\n Set Succease.\n");
                }
        break;
    }
  
    return true;

}

/*******************************************************************************
*
*功能: shell命令-- 执行函数对照表
*输入:
*输出:
*说明:
*******************************************************************************/
const linCmdTabSTRUC    linCmdTab[] =
{  
//    {"memrd",               " ",           cmdMemRead},
//    {"memwr",               " ",           cmdMemWrite},
    {"debug",               " ",           cmdDebug},
    {"rphy",                " ",           cmdReadFhy},
    {"wphy",                " ",           cmdWriteFhy},
    {"rtc",                 " ",           cmdRtc},
    {"reset",               " ",           cmdReset},
    {"paramdef",            " ",           cmdParamDef},
    {"timeset",             " ",           cmdSetDataTime},
    {"setparam",            " ",           cmdSetParam},
    {"show",                " ",           cmdShow},
    {"save",                " ",           cmdSaveParam},
    {"graw",                " ",           cmd8209Show},
    {"setrn",               " ",           cmdSetRnParam},
    {"end",                 "",            NULL},
    {"",    "",     NULL}
//    {"flasht",              " ",           cmdFlasht},
//    {"spi",                 " ",           spiRW},    
//    {"rtspi",               " ",           cmdResetSPI},

//    {"aver",                " ",           cmdAverage},    

    //below is protect cmd, don't show in help.    
//    {"gjtag",               " ",           multiGet},
//    {"sjtag",               "close/open",  jtagSet},
   
};
/*******************************************************************************
*函数名:
　　taskShellCmd()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
void taskShellCmd ( void *pParam )
{
    uint c;
    int  i;
//    INT8U err;
    OSTimeDly (3 * OS_TICKS_PER_SEC); /*wait other task print */

    sprintf(gPubVal.buildTime, "%s %s " , __DATE__ , __TIME__ );
    
    printf ( "\r\nShellCmd task begin..." );

    printf ( "\r\n\n\n++++++++ hop collector ++++++++++++"); 
    printf ( "\r\n+      ver:%s                +" ,gPubVal.version);    
    printf ( "\r\n+      cmp:%s  +" ,gPubVal.buildTime);
    printf ( "\r\n+++++++++++++++++++++++++++++++++++\n" );

   // MeterParamInit();
    while ( 1 )
    {
        printf ( "\n\rshell:>" );

        for ( i = 0; i < MAX_CMD_BUF_LEN; i++ )
        {
            if ( ( c = shellGetChar() ) == 0xff )    //by zdq
            {
                goto endProc;
            }


            cmdBuf[i] = ( char ) c;

            if ( ( cmdBuf[i] == 0x0d ) || ( cmdBuf[i] == '?' ) )//0x0d 表示回车键
                break;

            if ( c == '\b' )//退格
            {
                if ( i != 0 )
                {
                    shellPutChar ( '\b' );
                    shellPutChar ( ' ' );
                    shellPutChar ( '\b' );
                    i -= 2;
                }
                else
                {
                    i--;
                }
            }
            else
                shellPutChar ( c );
        }


        if ( i >= MAX_CMD_BUF_LEN )
        {
            printf ( "\n\rcmd is too long" );
            goto endProc;
        }

        if ( ( i == 0 ) && ( cmdBuf[0] != '?' ) )
            goto endProc;


        if ( parseCmd ( cmdBuf, &cmd ) < 0 )
        {
            printf ( "\n\rcommand and parameters error! " );
            goto endProc;
        }

        i = matchCmd ( &cmd, linCmdTab );

        if ( i == -1 )
            goto endProc;

        ( *linCmdTab[i].routine ) ( &cmd );


endProc:
        i++;
    }
    
}


