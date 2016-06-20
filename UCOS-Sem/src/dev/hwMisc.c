
#include <includes.h>
#include <sysctl.h>
#include <timer.h>
#include <hw_gpio.h>
#include <hw_nvic.h>
#include <hw_ints.h>
#include <interrupt.h>
#include <hw_types.h>
#include <watchdog.h>
#include <ethernet.h>
#include "comm.h"
uint gVal_SysCtlClockGet;



static void ioDelay ( uint dly )
{
    volatile uint cnt = dly;

    while ( cnt-- )
        ;
}

void watchdogInit ( void )
{
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_WDOG0 );

    gVal_SysCtlClockGet = SysCtlClockGet() * 4;

    WatchdogReloadSet ( WATCHDOG_BASE, gVal_SysCtlClockGet );

    WatchdogResetEnable ( WATCHDOG_BASE );
    //  WatchdogIntEnable(WATCHDOG_BASE);
    //  IntEnable(INT_WATCHDOG);

    WatchdogEnable ( WATCHDOG0_BASE );

}

/*PH3*/
void signalOut ( unsigned long ulPort ,  uchar ucPins , uint bHigh )
{
    GPIOPadConfigSet ( ulPort, ucPins, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD );
    GPIODirModeSet ( ulPort, ucPins, GPIO_DIR_MODE_OUT );

    HWREG ( ulPort + GPIO_O_PUR ) |= ucPins;
    HWREG ( ulPort + GPIO_O_PDR ) &= ~ucPins;

    ioDelay ( 5 );
    GPIOPinWrite ( ulPort,  ucPins , bHigh? ucPins:0);

}


/* GPIO_PORTG_BASE GPIO_PIN_5 */
uint signalIn ( unsigned long ulPort ,  uchar ucPins )
{
    GPIOPadConfigSet ( ulPort,  ucPins ,
                       GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU );
    GPIODirModeSet ( ulPort,  ucPins ,  GPIO_DIR_MODE_IN );

    ioDelay ( 5 );
//    return  GPIOPinRead( ulPort, ucPins )?1:0;
    
    if ( GPIOPinRead ( ulPort,  ucPins ) )
        return 1;
    else
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



#define  FAN_PWM_LOAD_VAL    2000        /*25k*/   /* 500 : 100K*/
void pwmInit ( void )
{
    unsigned long tmp;

    SysCtlPeripheralEnable ( SYSCTL_PERIPH_TIMER0 );
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_TIMER1 );
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_TIMER2 );
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_PWM );
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOD );
    //SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOC );
    //SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOB );
    //SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOD );
    //SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOE );

    /*
        GPIOPinConfigure ( GPIO_PC7_CCP0 );
        GPIOPinConfigure ( GPIO_PC5_CCP1 );
        GPIOPinConfigure ( GPIO_PC4_CCP2 );
        GPIOPinConfigure ( GPIO_PD4_CCP3 );
        GPIOPinConfigure ( GPIO_PE2_CCP4 );
        GPIOPinConfigure ( GPIO_PB5_CCP5 );


        GPIOPinConfigure ( GPIO_PD0_PWM0);
        GPIOPinConfigure ( GPIO_PD1_PWM1);

        GPIOPinTypePWM ( GPIO_PORTC_BASE,    GPIO_PIN_4  | GPIO_PIN_5 | GPIO_PIN_7 );
        GPIOPinTypePWM ( GPIO_PORTD_BASE,    GPIO_PIN_4 );
        GPIOPinTypePWM ( GPIO_PORTB_BASE,    GPIO_PIN_5 );
        GPIOPinTypePWM ( GPIO_PORTE_BASE,    GPIO_PIN_2 );
        GPIOPinTypePWM ( GPIO_PORTD_BASE,    GPIO_PIN_0 |GPIO_PIN_1);

    */

    //GPIOPinTypePWM(GPIO_PORTD_BASE,GPIO_PIN_0);
    //GPIOPinTypePWM(GPIO_PORTD_BASE,GPIO_PIN_1);
    //GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE,GPIO_PIN_1);
    //GPIOPinTypeGPIOOutput(GPIO_PORTJ_BASE,GPIO_PIN_6);

    GPIOPinConfigure ( GPIO_PH7_PWM5 );
    GPIOPinConfigure ( GPIO_PG1_PWM1 );
    GPIOPinConfigure ( GPIO_PD0_PWM0 );
    GPIOPinConfigure ( GPIO_PD1_PWM1 );
    GPIOPinTypePWM ( GPIO_PORTD_BASE, GPIO_PIN_0 );
    GPIOPinTypePWM ( GPIO_PORTD_BASE, GPIO_PIN_1 );



    /*TIMER 0*/
    TimerConfigure ( TIMER0_BASE, TIMER_CFG_16_BIT_PAIR |
                     TIMER_CFG_A_PWM | TIMER_CFG_B_PWM );
    TimerLoadSet ( TIMER0_BASE, TIMER_A, FAN_PWM_LOAD_VAL );
    TimerLoadSet ( TIMER0_BASE, TIMER_B, FAN_PWM_LOAD_VAL );
    tmp = TimerLoadGet ( TIMER0_BASE, TIMER_A ) ;
    tmp = tmp / 2;
    TimerMatchSet ( TIMER0_BASE, TIMER_A, tmp );
    TimerMatchSet ( TIMER0_BASE, TIMER_B, tmp );
    TimerEnable ( TIMER0_BASE, TIMER_BOTH );

    /*TIMER 1*/
    TimerConfigure ( TIMER1_BASE, TIMER_CFG_16_BIT_PAIR |
                     TIMER_CFG_A_PWM | TIMER_CFG_B_PWM );
    TimerLoadSet ( TIMER1_BASE, TIMER_A, FAN_PWM_LOAD_VAL );
    TimerLoadSet ( TIMER1_BASE, TIMER_B, FAN_PWM_LOAD_VAL );
    tmp = TimerLoadGet ( TIMER1_BASE, TIMER_A );
    tmp = tmp / 2;
    TimerMatchSet ( TIMER1_BASE, TIMER_A, tmp );
    TimerMatchSet ( TIMER1_BASE, TIMER_B, tmp );
    TimerEnable ( TIMER1_BASE, TIMER_BOTH );

    /*TIMER 2*/
    TimerConfigure ( TIMER2_BASE, TIMER_CFG_16_BIT_PAIR |
                     TIMER_CFG_A_PWM | TIMER_CFG_B_PWM );
    TimerLoadSet ( TIMER2_BASE, TIMER_A, FAN_PWM_LOAD_VAL );
    TimerLoadSet ( TIMER2_BASE, TIMER_B, FAN_PWM_LOAD_VAL );
    tmp = TimerLoadGet ( TIMER2_BASE, TIMER_A );
    tmp = tmp / 2;
    TimerMatchSet ( TIMER2_BASE, TIMER_A, tmp );
    TimerMatchSet ( TIMER2_BASE, TIMER_B, tmp );
    TimerEnable ( TIMER2_BASE, TIMER_BOTH );


}





/*******************************************************************************
*函数名:
　　()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
void pwmSetWid ( uchar channel , uint percent )
{
    unsigned long tmp;

    if ( percent > 99 )
        percent = 99;

    switch ( channel )
    {
        case 0:
            tmp = TimerLoadGet ( TIMER0_BASE, TIMER_A ) ;
            tmp = ( tmp * percent ) / 100;
            TimerMatchSet ( TIMER0_BASE, TIMER_A, tmp );

            break;
        case 1:
            tmp = TimerLoadGet ( TIMER0_BASE, TIMER_B ) ;
            tmp = ( tmp * percent ) / 100;
            TimerMatchSet ( TIMER0_BASE, TIMER_B, tmp );
            break;
#if 1
        case 2:
            tmp = TimerLoadGet ( TIMER1_BASE, TIMER_A ) ;
            tmp = ( tmp * percent ) / 100;
            TimerMatchSet ( TIMER1_BASE, TIMER_A, tmp );
            break;

        case 3:
            tmp = TimerLoadGet ( TIMER1_BASE, TIMER_B ) ;
            tmp = ( tmp * percent ) / 100;
            TimerMatchSet ( TIMER1_BASE, TIMER_B, tmp );
            break;

            //case 4:
        case 5:
            tmp = TimerLoadGet ( TIMER2_BASE, TIMER_A ) ;
            tmp = ( tmp * percent ) / 100;
            TimerMatchSet ( TIMER2_BASE, TIMER_A, tmp );
            break;
            // case 5:
        case 4:
            tmp = TimerLoadGet ( TIMER2_BASE, TIMER_B ) ;
            tmp = ( tmp * percent ) / 100;
            TimerMatchSet ( TIMER2_BASE, TIMER_B, tmp );
            break;
#endif
        default:
            DBCMM("");
    }

}

void hardMiscFlash ( void )
{

    printf ( "aaabbbcc" );
    printf ( "  hardMiscFlash 1111111111" );
    printf ( "2222222222                                             " );


    HWREG ( NVIC_APINT ) = NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ;

}


void armLed(int bOn)
{
     GPIOPinWrite( GPIO_PORTE_BASE , GPIO_PIN_0, bOn ? 0 : GPIO_PIN_0);
}


int bEthPhyLink(void)
{
    if(  (0x01 <<2) &  EthernetPHYRead ( ETH_BASE , 0x01 /*状态*/ ))
    {
        return true;
    }
    else
    {
        return false;
    }

}

#if 0
void SysReset ( void )
{
    /*reboot*/

    printf ( "\r\nSysReset                         " );
    HWREG ( NVIC_APINT ) = NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ;
}
#endif

/*******************************************************************************
*函数名:
　　gpioCommInit()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
void gpioCommInit ( void )
{
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOA );
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOB );
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOC );
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOD );
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOE );
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOF );
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOG );
//    SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOH );
//    SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOJ );

    //Real Timer
    //I2C_SCL -- PB2
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE,GPIO_PIN_2);
    GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_2,0);
        
    //I2C_SDA -- PB3
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE,GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_3,GPIO_PIN_3);
        
    //I2C_WE -- PF0
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,GPIO_PIN_0);
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_0,0);

    //EN_PWR  -- PA7 无线模块电源控制
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE,GPIO_PIN_7);
    MOD_POWEROFF();
    
    
    //RF_RST   -- PA6  无线模块复位控制
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE,GPIO_PIN_6);
    MOD_RESETHIGH();
        
    //RF_PWRON -- PC5  无线模块开关控制
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE,GPIO_PIN_5);
    MOD_PDHIGH();
    
    //set rs485en GPIO
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE,GPIO_PIN_4);
    GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_4,0); 

    //150通道选择GPIO
    //ADDR_D
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE,GPIO_PIN_0);
    GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_0,0); 
    //ADDR_C
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE,GPIO_PIN_1);
    GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_1,0); 
    //ADDR_B
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE,GPIO_PIN_7);
    GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_7,0); 
    //ADDR_A
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE,GPIO_PIN_6);
    GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_6,0); 

   //配置PB5 为8209的irq
//    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE,GPIO_PIN_5);//输入
//    GPIOIntTypeSet(GPIO_PORTB_BASE,GPIO_PIN_1, GPIO_RISING_EDGE);//设置为上升沿触发
//    GPIOIntTypeSet(GPIO_PORTB_BASE,GPIO_PIN_5, GPIO_FALLING_EDGE);//设置为下降沿触发
//    GPIOIntTypeSet(GPIO_PORTB_BASE,GPIO_PIN_5, GPIO_LOW_LEVEL);
//    GPIOPinIntEnable(GPIO_PORTB_BASE,GPIO_PIN_5);//使能所在管脚的中断
//    IntPrioritySet(INT_GPIOB,4<<5 ); 
//    IntEnable ( INT_GPIOB );                     //使能端口中断
//    IntMasterEnable();                           //使能处理器中断，在其它地方已使能

    //ALM LED 
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE,GPIO_PIN_0);
    GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_0,GPIO_PIN_0); 
    
#if 0  
    //set the jtag as GPIO to protect firmware stored in flash
    HWREG(GPIO_PORTC_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY_DD;
    while(HWREG(GPIO_PORTC_BASE + GPIO_O_LOCK))
        ;
    HWREG(GPIO_PORTC_BASE + GPIO_O_CR) = 0xC;

    HWREG(GPIO_PORTC_BASE + GPIO_O_AFSEL) &= 0xF3;
    HWREG(GPIO_PORTC_BASE + GPIO_O_DEN) &= 0xF3;
    HWREG(GPIO_PORTC_BASE + GPIO_O_PUR) &= 0xF3;
    HWREG(GPIO_PORTC_BASE + GPIO_O_PCTL) &= 0xFFFF0033;
#endif
}



