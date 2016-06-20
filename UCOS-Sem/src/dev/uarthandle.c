
#include "includes.h"
#include <dev.h>
#include <netBuf.h>
#include <uart.h>
#include <hw_uart.h>
#include <hw_gpio.h>
#include <hw_types.h>
#include <hw_ints.h>


#define MAXCOMMCTRL  3

t_commCtrl gCommCtrls[MAXCOMMCTRL];

/*******************************************************************************
*函数名:
　　commUartInit()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/

void commUartInit ( void )
{
    /*********** UART 0 ***************/
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_UART0 );
    //
    // Set GPIO A0 and A1 as UART pins.
    //
    GPIOPinConfigure ( GPIO_PA0_U0RX );
    GPIOPinConfigure ( GPIO_PA1_U0TX );
    GPIOPinTypeUART ( GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1 );

    UARTConfigSetExpClk ( UART0_BASE, SysCtlClockGet(), SPEED_UART_STD,
                          ( UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                            UART_CONFIG_PAR_NONE ) );
    UARTFIFOEnable ( UART0_BASE );
    UARTFIFOLevelSet ( UART0_BASE , UART_FIFO_TX4_8 , UART_FIFO_RX4_8 );
    
    UARTIntEnable ( UART0_BASE, UART_INT_RX | UART_INT_RT );


    /*********** UART 1 ***************/
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_UART1 );
    //
    // Set GPIO D2 and D3 as UART pins.
    //
    GPIOPinConfigure ( GPIO_PD2_U1RX );
    GPIOPinConfigure ( GPIO_PD3_U1TX );
    GPIOPinTypeUART ( GPIO_PORTD_BASE, GPIO_PIN_2 | GPIO_PIN_3 );

    UARTConfigSetExpClk ( UART1_BASE, SysCtlClockGet(), SPEED_UART_STD,
                          ( UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                            UART_CONFIG_PAR_NONE) );
    UARTFIFOEnable ( UART1_BASE );
    UARTFIFOLevelSet ( UART1_BASE , UART_FIFO_TX4_8 , UART_FIFO_RX4_8 );
    
    UARTIntEnable ( UART1_BASE, UART_INT_RX | UART_INT_RT );
    
    /*********** UART 2 ***************/
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_UART2 );
    //
    // Set GPIO G0 and G1 as UART pins.
    //
    GPIOPinConfigure ( GPIO_PG0_U2RX );
    GPIOPinConfigure ( GPIO_PG1_U2TX );
    GPIOPinTypeUART ( GPIO_PORTG_BASE, GPIO_PIN_0 | GPIO_PIN_1 );

    UARTConfigSetExpClk ( UART2_BASE, SysCtlClockGet(), SPEED_UART_RS485,
                          ( UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                            UART_CONFIG_PAR_NONE ) );
    UARTFIFOEnable ( UART2_BASE );
    UARTFIFOLevelSet ( UART2_BASE , UART_FIFO_TX4_8 , UART_FIFO_RX4_8 );
    
    UARTIntEnable ( UART2_BASE, UART_INT_RX | UART_INT_RT );

    

}


void commBaseCtrlInit ( unsigned long uartBase , unsigned int channel )
{
    gCommCtrls[channel].uartBase = uartBase;
    gCommCtrls[channel].pSendNoteBack = OSSemCreate ( 0 ); //同步

    if ( gCommCtrls[channel].pSendNoteBack == ( void * ) 0 )
    {
        while ( 1 )
            ;
    }

    gCommCtrls[channel].pRecvNoteBack = OSSemCreate ( 0 ); //同步

    if ( gCommCtrls[channel].pRecvNoteBack == ( void * ) 0 )
    {
        while ( 1 )
            ;
    }

    UARTIntDisable ( gCommCtrls[channel].uartBase, UART_INT_TX );
    UARTIntEnable ( gCommCtrls[channel].uartBase, UART_INT_RX | UART_INT_RT );
}



void commBaseIsr ( unsigned int channel )
{
    unsigned long ulStatus;

    unsigned long UART_BASE =   gCommCtrls[channel].uartBase;

    ulStatus = UARTIntStatus ( UART_BASE, true );

    UARTIntClear ( UART_BASE, ulStatus );

    if ( ulStatus & UART_INT_TX )
    {
        UARTIntDisable ( UART_BASE, UART_INT_TX );
        OSSemPost ( gCommCtrls[channel].pSendNoteBack );
    }

    if ( ulStatus & ( UART_INT_RX | UART_INT_RT ) )
    {
        UARTIntDisable ( UART_BASE, UART_INT_RX | UART_INT_RT );
        
        OSSemPost ( gCommCtrls[channel].pRecvNoteBack );
    }
}


char commBaseGetChar ( unsigned int channel )
{
    uchar ch;
    unsigned long UART_BASE =   gCommCtrls[channel].uartBase;

    while ( ( HWREG ( UART_BASE + UART_O_FR ) & UART_FR_RXFE ) )
    {
        uchar err;
        UARTIntEnable ( UART_BASE, UART_INT_RX | UART_INT_RT );
        
        OSSemPend ( gCommCtrls[channel].pRecvNoteBack , 0 , &err );
    }
    ch = HWREG ( UART_BASE + UART_O_DR ) & 0xff;
    return ch;
}


void commBasePutChar ( unsigned int channel , char ch )
{
    uchar err;
    unsigned long UART_BASE =   gCommCtrls[channel].uartBase;

    if ( ( HWREG ( UART_BASE + UART_O_FR ) & UART_FR_TXFF ) )
    {
        UARTIntEnable ( UART_BASE, UART_INT_TX );
        OSSemPend ( gCommCtrls[channel].pSendNoteBack , 10 , &err );
    }

    HWREG ( UART_BASE + UART_O_DR ) = ch;
}


void rs485TxSwitch ( unsigned int bTx )
{
    signalOut ( GPIO_PORTC_BASE , GPIO_PIN_4, bTx );
    
}


void uartInitRs485(void)
{
    unsigned long cfg = 0;

    SysCtlPeripheralEnable ( SYSCTL_PERIPH_UART2 );
    //
    // Set GPIO A0 and A1 as UART pins.
    //
    GPIOPinConfigure ( GPIO_PG0_U2RX );
    GPIOPinConfigure ( GPIO_PG1_U2TX );
    GPIOPinTypeUART ( GPIO_PORTG_BASE, GPIO_PIN_0 | GPIO_PIN_1 );
    //GPIOPadConfigSet ( GPIO_PORTG_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU );

    cfg = UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE ;

    if (SysParam[SP_CHECKBIT] == 0x02)
    {
        cfg &= ~ UART_CONFIG_PAR_MASK;
        cfg |= UART_CONFIG_PAR_EVEN;
    }    

    UARTConfigSetExpClk ( UART2_BASE, SysCtlClockGet(), BaudRateTable[SysParam[SP_BAUDRATE]], cfg);


    UARTFIFOEnable ( UART2_BASE );
    UARTFIFOLevelSet ( UART2_BASE , UART_FIFO_TX4_8 , UART_FIFO_RX1_8 );

    UARTIntEnable ( UART2_BASE, UART_INT_RX | UART_INT_RT );
}






