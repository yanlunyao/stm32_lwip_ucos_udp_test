
#include <stdio.h>
#include <spi.h>
#include <ssi.h>
#include <gpio.h>
#include <hw_gpio.h>
#include <hw_memmap.h>
#include <hw_ssi.h>
#include "driverlib/debug.h"

unsigned int SSIBusytest ( unsigned long ulBase )
{
    //
    // Check the arguments.
    //
    ASSERT ( ( ulBase == SSI0_BASE ) || ( ulBase == SSI1_BASE ) );
    //
    // Determine if the SSI is busy.
    //
    return ( ( HWREG ( ulBase + SSI_O_SR ) & SSI_SR_BSY ) ? true : false );
}

void channelSelect(int channel)
{
    if(channel > 8 || channel < 0)
    {
        printf("\nSPI channel over range.");
        return ;
    }
    GPIOPinWrite(GPIO_PORTE_BASE , GPIO_PIN_1, (channel & 0x08)? GPIO_PIN_1:0);
    GPIOPinWrite(GPIO_PORTE_BASE , GPIO_PIN_2, (channel & 0x04)? GPIO_PIN_2:0);
    GPIOPinWrite(GPIO_PORTE_BASE , GPIO_PIN_3, (channel & 0x02)? GPIO_PIN_3:0);        
    GPIOPinWrite(GPIO_PORTB_BASE , GPIO_PIN_4, (channel & 0x01)? GPIO_PIN_4:0);
    
}

void resetSPI()
{
    channelSelect(SPI_UNUSED_CHANNEL);
}


void spiInit(void)
{   
    printf ("\r\nSPI interface init..." );
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
    
    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA3_SSI0FSS);
    GPIOPinConfigure(GPIO_PA4_SSI0RX);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);
    
    GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_3 | GPIO_PIN_2);

    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_1, SSI_MODE_MASTER, SPI_BIT_RATE_1M, SPI_8BIT_WIDTH);
   
    SSIEnable(SSI0_BASE);
   
    //init 4->9 selector
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE , GPIO_PIN_1);
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE , GPIO_PIN_2);
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE , GPIO_PIN_3);
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE , GPIO_PIN_4);
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4,0);
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 , 0);
}

bool spiRead(BYTE channel, uint add, uint *data, BYTE bytes)
{
    int i;
    uint temp = 0;
    uint mask = 0;

    if(bytes > 4)
    {
        printf("\nSPI data lenght over range.");
        return false;
    }
    while(SSIBusytest(SSI0_BASE))
    {
      ;
    }

    channelSelect(channel);
    //OSTimeDlyHMSM(0,0,0,2);

    //clear SPI FIFO first
    while(SSIDataGetNonBlocking(SSI0_BASE, data))
    {
        ;
    }
   
    SSIDataPut(SSI0_BASE, add);
    SSIDataGet(SSI0_BASE, data);

    for(i = 0; i < bytes; i++)
    {
        SSIDataPut(SSI0_BASE, 0);
        SSIDataGet(SSI0_BASE, data);
        temp <<= 8;
        temp |= *data;
        mask <<= 8;
        mask |= 0xFF;
    }
    
    if(temp == mask)
    {
        *data = 0;
        return false;
    }
    *data = temp;
   

    //channelSelect(SPI_UNUSED_CHANNEL);
    
    return true;
}

bool spiWrite(BYTE channel, uint add, uint data, BYTE bytes)
{
    int i;
    char temp = 0;
    
    if(bytes > 4)
    {
        printf("\nSPI data lenght over range.");
        return false;
    }  
      
    while(SSIBusy(SSI0_BASE))
    {
        ;
    }
    channelSelect(channel);
    //enable write action
    SSIDataPut(SSI0_BASE, 0xEA);
    SSIDataPut(SSI0_BASE, 0xE5);
    
    add |= 0x80; 
    SSIDataPut(SSI0_BASE, add);

    for(i = bytes-1; i >= 0; i--)
    {
        //printf("dat=%x\n",data);
        temp = (char)((data >> i*8) & 0xFF);
        //printf("temp=%x\n",temp);
        SSIDataPut(SSI0_BASE, temp);
    }
    
    //channelSelect(SPI_UNUSED_CHANNEL);
    //Disable write
    SSIDataPut(SSI0_BASE, 0xEA);
    SSIDataPut(SSI0_BASE, 0xDC);
    return true;
}

void spiRegShow()
{
    printf("\n  CR0=%x", HWREG ( SSI0_BASE + SSI_O_CR0 ));
    printf("\n  CR1=%x", HWREG ( SSI0_BASE + SSI_O_CR1 ));
    printf("\n CPSR=%x", HWREG ( SSI0_BASE + SSI_O_CPSR ));
}



