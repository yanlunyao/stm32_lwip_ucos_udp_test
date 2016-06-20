//*****************************************************************************
//
// io.h - Prototypes for I/O routines for the enet_io example.
//
// Copyright (c) 2007-2010 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 6075 of the EK-LM3S8962 Firmware Package.
//
//*****************************************************************************

#ifndef __IO_H__
#define __IO_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define CTRL_OUT_0(bon)  signalOut(GPIO_PORTD_BASE , GPIO_PIN_5 ,(!bon))
#define CTRL_OUT_1(bon)  signalOut(GPIO_PORTD_BASE , GPIO_PIN_5 ,(!bon))
#define CTRL_OUT_2(bon)  signalOut(GPIO_PORTD_BASE , GPIO_PIN_5 ,(!bon))
#define CTRL_OUT_3(bon)  signalOut(GPIO_PORTD_BASE , GPIO_PIN_5 ,(!bon))
#define CTRL_OUT_4(bon)  signalOut(GPIO_PORTD_BASE , GPIO_PIN_5 ,(!bon))
#define CTRL_OUT_5(bon)  signalOut(GPIO_PORTD_BASE , GPIO_PIN_5 ,(!bon))
#define CTRL_OUT_6(bon)  signalOut(GPIO_PORTD_BASE , GPIO_PIN_5 ,(!bon))
#define CTRL_OUT_7(bon)  signalOut(GPIO_PORTD_BASE , GPIO_PIN_5 ,(!bon))


#define STATE_IN_0()  signalIn(GPIO_PORTD_BASE , GPIO_PIN_5)
#define STATE_IN_1()  signalIn(GPIO_PORTD_BASE , GPIO_PIN_5)
#define STATE_IN_2()  signalIn(GPIO_PORTD_BASE , GPIO_PIN_5)
#define STATE_IN_3()  signalIn(GPIO_PORTD_BASE , GPIO_PIN_5)
#define STATE_IN_4()  signalIn(GPIO_PORTD_BASE , GPIO_PIN_5)
#define STATE_IN_5()  signalIn(GPIO_PORTD_BASE , GPIO_PIN_5)
#define STATE_IN_6()  signalIn(GPIO_PORTD_BASE , GPIO_PIN_5)
#define STATE_IN_7()  signalIn(GPIO_PORTD_BASE , GPIO_PIN_5)
#define STATE_IN_8()  signalIn(GPIO_PORTD_BASE , GPIO_PIN_5)
#define STATE_IN_9()  signalIn(GPIO_PORTD_BASE , GPIO_PIN_5)
#define STATE_IN_10()  signalIn(GPIO_PORTD_BASE , GPIO_PIN_5)
#define STATE_IN_11()  signalIn(GPIO_PORTD_BASE , GPIO_PIN_5)
#define STATE_IN_12()  signalIn(GPIO_PORTD_BASE , GPIO_PIN_5)
#define STATE_IN_13()  signalIn(GPIO_PORTD_BASE , GPIO_PIN_5)


void io_set_led(tBoolean bOn);
void io_set_pwm(tBoolean bOn);
void io_pwm_freq(unsigned long ulFreq);
void io_pwm_dutycycle(unsigned long ulDutyCycle);
unsigned long io_get_pwmfreq(void);
unsigned long io_get_pwmdutycycle(void);
void io_get_ledstate(char * pcBuf, int iBufLen);
int io_is_led_on(void);
void io_get_pwmstate(char * pcBuf, int iBufLen);
int io_is_pwm_on(void);

#ifdef __cplusplus
}
#endif

#endif // __IO_H__
