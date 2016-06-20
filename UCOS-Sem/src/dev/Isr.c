
#include "includes.h"

/*******************************************************************************
*函数名:
　　**isr
*功能:各中断跳转，方便代码部分升级
*输入:
*输出:
*说明:
*******************************************************************************/
void sysTickIsr ( void )
{
    #if OS_CRITICAL_METHOD ==3
    OS_CPU_SR  cpu_sr;
    #endif

    OS_ENTER_CRITICAL();
   // OSIntNesting++;
    OSIntEnter();
    OS_EXIT_CRITICAL();

    OSTimeTick();                        // Call uC/OS-II's OSTimeTick()  调用uC/OS-II的OSTimeTick()函数

    OSIntExit();
}

void uart0Isr ( void )
{
#if OS_CRITICAL_METHOD ==3
    OS_CPU_SR  cpu_sr;
#endif

    OS_ENTER_CRITICAL();
    //OSIntNesting++;
    OSIntEnter();
    OS_EXIT_CRITICAL();
    
    commBaseIsr ( 0 );

    OSIntExit();
}

void uart1Isr ( void )
{

#if OS_CRITICAL_METHOD ==3
    OS_CPU_SR  cpu_sr;
#endif

    OS_ENTER_CRITICAL();
    //OSIntNesting++;
    OSIntEnter();
    OS_EXIT_CRITICAL();

    commBaseIsr ( 1 );

    OSIntExit();
}

void uart2Isr ( void )
{
#if OS_CRITICAL_METHOD ==3
    OS_CPU_SR  cpu_sr;
#endif

    OS_ENTER_CRITICAL();
    //OSIntNesting++;
    OSIntEnter();
    OS_EXIT_CRITICAL();

    commBaseIsr ( 2 );

    OSIntExit();
 }


void i2cSlaveIsr ( void )
{
    //i2cSlaveHandler();
    //I2C_Slave_ISR();
}


/*
void Reset_Handler(void)
{
     __main();
}
*/

void IntDefaultHandler ( void )
{
    while ( 1 );
}
#if 0
/*********************************************************/
/*RN8209的中断引脚(PF脉冲,IRQ)                           */
/*********************************************************/
void  GPIO_Port_B_ISR (void)
{
    ulong ulStatus;
#if OS_CRITICAL_METHOD ==3
    OS_CPU_SR  cpu_sr;
#endif
    
    OS_ENTER_CRITICAL();
    OSIntNesting++;
    OS_EXIT_CRITICAL();
    
    ulStatus = GPIOPinIntStatus(GPIO_PORTB_BASE, true);        //    读取中断状态 
    GPIOPinIntClear(GPIO_PORTB_BASE, ulStatus);                //    清除中断状态，重要 
    
    if(ulStatus & GPIO_PIN_5)
    {
         OSSemPost(RNSemIsr);
    }
     OSIntExit();
}
#endif

