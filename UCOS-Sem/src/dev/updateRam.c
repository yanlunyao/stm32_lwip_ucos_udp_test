
#include "includes.h"
#include "update.h"
#include <hw_flash.h>
#include <hw_uart.h>
#include <hw_types.h>
#include <hw_nvic.h>
#include <hw_watchdog.h>
#include "dev.h"

//---无线升级
DWORD AppBuf[FLASH_BANK_SIZE/4];


#ifndef ewarm
#define __ramfunc
#endif

#define    SHELL_UART_BASE    UART0_BASE

void uartInitPollMode ( void );
__ramfunc  void ramConsolePutChar ( uchar ch );
__ramfunc void ramDebug ( uchar * buff );


__ramfunc uint ramProgramFlash ( unsigned long ulAddress, unsigned long *  pulData,
                                 unsigned long ulCount );
__ramfunc void ramPollShellUartSend ( unsigned long uartbase , uchar *buff , uint len );

__ramfunc void ramPutByte ( uint val );
__ramfunc void ramPutInt ( uint val );
__ramfunc uchar ramConsoleGetChar ( uint timeout , uint * bTimeout );
__ramfunc void RamMemCpy(DWORD Offset);



__ramfunc void ramPollShellUartSend ( unsigned long uartbase , uchar *buff , uint len )
{
    uint sendlen = 0;

    while ( sendlen < len )
    {
        while ( HWREG ( uartbase + UART_O_FR ) & UART_FR_TXFF )
        {
        }

        HWREG ( uartbase + UART_O_DR ) = buff[sendlen++];
    }
}


__ramfunc uint ramProgramFlash( ulong ulAddress, ulong *pulData, ulong ulCount )
{


#ifdef UPDBG
    int i;
    ramDebug ( "\r\nprogramFlash:" );
    ramPutInt ( ulAddress );
    ramPutInt ( ulCount );
    ramDebug ( "\r\n " );

    for ( i = 0; i < ( 64 / 4 ); i++ )
    {
        ramPutInt ( pulData[i] );
    }

#endif

    /****** erase flash  ******/
    if ( ( ulAddress % FLASH_BANK_SIZE/*1024*/ ) == 0 )
    {
        DBUDT( "\r\nerase flash:" );

        HWREG ( FLASH_FCMISC ) = FLASH_FCMISC_AMISC;
        HWREG ( FLASH_FMA ) = ulAddress;
        HWREG ( FLASH_FMC ) = FLASH_FMC_WRKEY | FLASH_FMC_ERASE;

        while ( HWREG ( FLASH_FMC ) & FLASH_FMC_ERASE )
            ;

        if ( HWREG ( FLASH_FCRIS ) & FLASH_FCRIS_ARIS )
        {
            uint i = 0;
            uint *pint = ( uint * ) ulAddress;

            for ( i = i; i < 0xff; i++ )
            {
                if ( pint[i] != 0xffffffff )
                {
                    ramDebug ( "erase fail!" );
                    return false;
                }
            }
        }

    }


    /*write to flash*/
    do
    {
        //
        // Clear the flash access interrupt.
        //
        HWREG ( FLASH_FCMISC ) = FLASH_FCMISC_AMISC;

            //
            // Loop over the words to be programmed.
            //

            while ( ulCount )
            {
                //
                // Program the next word.
                //
                HWREG ( FLASH_FMA ) = ulAddress;
                HWREG ( FLASH_FMD ) = *pulData;
                HWREG ( FLASH_FMC ) = FLASH_FMC_WRKEY | FLASH_FMC_WRITE;


                //
                // Wait until the word has been programmed.
                //
                while ( HWREG ( FLASH_FMC ) & FLASH_FMC_WRITE )
                {
                }

                //
                // Increment to the next word.
                //
                pulData++;
                ulAddress += 4;
                ulCount -= 4;
            }
        //
        // Return an error if an access violation occurred.
        //
        if ( HWREG ( FLASH_FCRIS ) & FLASH_FCRIS_ARIS )
        {
            ramDebug ( "programe fail!" );
            return ( false );
        }

        return ( true );
        
    }while ( 0 );
    

}

__ramfunc uchar ramConsoleGetChar ( uint timeout , uint * bTimeout )
{
    *bTimeout  = false;

    while ( timeout-- )
    {
        if ( ! ( HWREG ( SHELL_UART_BASE + UART_O_FR ) & UART_FR_RXFE ) )
        {
            return  HWREG ( SHELL_UART_BASE + UART_O_DR ) & 0xff;
        }
    }

    *bTimeout  = true;

    return 0xff ;
}

__ramfunc  void ramConsolePutChar ( uchar ch )
{
    while ( HWREG ( SHELL_UART_BASE + UART_O_FR ) & UART_FR_TXFF )
        ;

    HWREG ( SHELL_UART_BASE  + UART_O_DR ) = ch;
}



__ramfunc  void ramDebug ( uchar * buff )
{
    while ( *buff != 0 )
    {
        ramConsolePutChar ( *buff );
        buff++;
    }
}


__ramfunc void ramPutInt ( uint val )
{
    uint i , tmp;
    uchar ch;
    ramConsolePutChar ( '0' );
    ramConsolePutChar ( 'x' );

    for ( i = 0; i < 8; i++ )
    {
        tmp = val >> 28;
        ch = tmp & 0x0f;

        if ( ch < 0x0a )
            ramConsolePutChar ( ch + '0' );
        else
            ramConsolePutChar ( ch - 0x0a + 'A' );

        val = val << 4;
    }
}

__ramfunc void ramPutByte ( uint val )
{
    uint i , tmp;
    uchar ch;
    ramConsolePutChar ( '0' );
    ramConsolePutChar ( 'x' );

    for ( i = 0; i < 2; i++ )
    {
        tmp = val >> 4;
        ch = tmp & 0x0f;

        if ( ch < 0x0a )
            ramConsolePutChar ( ch + '0' );
        else
            ramConsolePutChar ( ch - 0x0a + 'A' );

        val = val << 4;
    }

    ramConsolePutChar ( ' ' );
}



extern uint gVal_SysCtlClockGet ;

__ramfunc void copyMcuImage ( uint dst , uint src)
{
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0u;
#endif

    int blkaddr;
    int i;
    int bSame = true;
    uint imgSize = *( ( uint * ) ( ADDR_BIN_LENGTH + src ) );

    printf("\r\nimage size:0x%x  dst:0x%x, src:0x%x\n" , imgSize , dst ,src);

    OS_ENTER_CRITICAL();

    for ( blkaddr = 0 ; blkaddr < imgSize; blkaddr += FLASH_BANK_SIZE )
    {
        bSame = true;

        for ( i = 0; i < FLASH_BANK_SIZE;  i += 4 )
        {
            if ( *( uint * )( i + src + blkaddr ) != *( uint * )( i + dst + blkaddr ) )
            {
                bSame = false;
                break;
            }
        }

        if ( !bSame )
        {
            ramDebug ( "!" );

            if (true != ramProgramFlash(( blkaddr + dst ), (unsigned long *)(blkaddr + src), FLASH_BANK_SIZE))
            {
                ramDebug ( "\r\ncopyMcuImage program flash fail" );
                ramPutInt ( blkaddr );
            }
        }

        //watchdogFeed
        HWREG ( WATCHDOG_BASE + WDT_O_LOAD ) = TheSysClock*4;// 4s

    }

    OS_EXIT_CRITICAL();

}

//无线升级程序
/*---------------------------------------------------------------/
/---------------------------------------------------------------*/
void RamWDog()
    {
    #ifdef INNER_WDOG
        HWREG(WATCHDOG0_BASE + WDT_O_LOAD) = TheSysClock*4;
    #else
        static BYTE d = 0;
        if (d == 0)
        {
            d = 1;
            HWREG(GPIO_PORTE_BASE + (GPIO_O_DATA + (GPIO_PIN_2 << 2))) = GPIO_PIN_2;
        }
        else
        {
            d = 0;
            HWREG(GPIO_PORTE_BASE + (GPIO_O_DATA + (GPIO_PIN_2 << 2))) = 0;
        }
    #endif
    }

void RamDebugChar ( BYTE ch )
{
    if (SysParam[SP_DEBUGMSG] == DEBUG_DISABLE)
    {
        return;
    }
    
    while ( HWREG ( UART1_BASE + UART_O_FR ) & UART_FR_TXFF )
        ;

    HWREG ( UART1_BASE  + UART_O_DR ) = ch;
}


void RamDebugStr(const char * Msg)  
{                       
    if (SysParam[SP_DEBUGMSG] == DEBUG_DISABLE)
    {
        return;
    }
    
    while ( *Msg != 0 )
    {
        RamDebugChar ( *Msg );
        Msg++;
    }
        
}
void RamDebugWord(unsigned short int Num)  
{   
    static unsigned char m,t;                    
    if (SysParam[SP_DEBUGMSG] == DEBUG_DISABLE)
    {
        return;
    }
    m = (BYTE)(Num / 10000);
    RamDebugChar(m+0x30);
    t = Num % 10000;
    m = (BYTE)(t/1000);
    RamDebugChar(m+0x30);
    t = t%1000;
    m = (BYTE)(t/100);
    RamDebugChar(m+0x30);
    t = t%100;
    m = (BYTE)(t/10);
    RamDebugChar(m+0x30);
    m = (BYTE)(t%10);
    RamDebugChar(m+0x30);        
}

void RamClearBuf()
{
    static WORD i;
    for (i=0;i<FLASH_BANK_SIZE/4;i++)
    {
        AppBuf[i] = 0;
    }
}

void RamMemCpy(DWORD Offset)
{
    static WORD i;
    for (i=0;i<FLASH_BANK_SIZE/4;i++)
    {
        AppBuf[i]  = *(DWORD *)(BAKAPPSTART + Offset + (i*4));
    }
}


//删除下载的软件
void RamEraseBakApp()
{
    static DWORD i;
    static DWORD ulAddress;
    
    for(i=0;i<APPMAXLENGTH/FLASH_BANK_SIZE;i++)
    {
        ulAddress = BAKAPPSTART + i*FLASH_BANK_SIZE;
        if ( ( ulAddress % FLASH_BANK_SIZE) == 0 )
        {
            HWREG ( FLASH_FCMISC ) = FLASH_FCMISC_AMISC;
            HWREG ( FLASH_FMA ) = ulAddress;
            HWREG ( FLASH_FMC ) = FLASH_FMC_WRKEY | FLASH_FMC_ERASE;

            while ( HWREG ( FLASH_FMC ) & FLASH_FMC_ERASE )
                ;

            if ( HWREG ( FLASH_FCRIS ) & FLASH_FCRIS_ARIS )
            {
                BYTE i = 0;
                DWORD *pint = ( DWORD * ) ulAddress;

                for ( i = i; i < 0xff; i++ )
                {
                    if ( pint[i] != 0xffffffff )
                    {
                        RamDebugStr("Erase fail!\r\n" );
                        return;
                    }
                }
            }

        }

        RamWDog();
    }
}

void RamResetSys()
{
    
    while(1)
    {
        ;
    }
}


// 1次解密1k数据
void RamDecodeApp(DWORD Offset)
{
    static DWORD PassWord = 0;
    static DWORD CRC = 0;
    static DWORD Version = 0;
    static WORD i;
    static DWORD Length = 0;

    RamMemCpy(Offset);

    if (Offset == 0)
    {
        CRC      = AppBuf[ADDR_BIN_CRC/4];
        Version  = AppBuf[ADDR_BIN_VER/4];
        
        Length   = AppBuf[ADDR_BIN_LENGTH/4];       
        Length  ^= CRC;

        PassWord = AppBuf[ADDR_BIN_PASSWORD/4];
        PassWord ^= CRC;

    }
    //解密数据
    for (i=0;i<FLASH_BANK_SIZE/4;i++)
    {
        AppBuf[i] ^= PassWord;
    }

    if (Offset == 0)
    {
        AppBuf[ADDR_BIN_VER/4]    = Version; // 软件版本要写回去
        AppBuf[ADDR_BIN_LENGTH/4] = Length; // 软件长度还原
  
    }
    
}

void RamUpdateApp()
{
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0u;
#endif

    static DWORD AppLen,i,Offset;

    RamWDog();
    OS_ENTER_CRITICAL();
    
    AppLen = *(DWORD *)&SysParam[SP_SW_LENGTH];
   
    if (AppLen > APPMAXLENGTH - 1024)  // 预留 1k 安全内存
    {
        RamDebugStr("Length Error!\r\n");
        RamEraseBakApp();
        RamResetSys();
    }

    Offset = 0;
    i = 0;
    while(Offset < AppLen)
    {
        RamClearBuf();
        RamDecodeApp(Offset);

        // 1次写1k数据
        ramProgramFlash(i*FLASH_BANK_SIZE,AppBuf,FLASH_BANK_SIZE);
        RamDebugChar('.');
        RamWDog();

        Offset += FLASH_BANK_SIZE;
        i++;
    }
    RamDebugStr("\r\n");
    
    RamEraseBakApp();
    
    //更新参数
    SysParam[SP_SW_UPDATE]    = 0;
    SysParam[SP_SW_VERSION]   = 0;
    SysParam[SP_SW_VERSION+1] = 0;
    *(DWORD *)&SysParam[SP_SW_LENGTH] = 0;
    *(WORD *)&SysParam[SP_SW_CURID] = 0;

    gDevParam.flag = FLASH_PARAM_FLAG;
    gDevParam.scrc = 0;
    gDevParam.scrc = CRC16((uchar *)&gDevParam,sizeof(tDevParam));
    
    ramProgramFlash(SYSPARAMSTART,(DWORD *)&gDevParam ,sizeof ( tDevParam ));
   
    RamDebugStr("Update Success.\r\n");
    RamResetSys();
    OS_EXIT_CRITICAL();
}



