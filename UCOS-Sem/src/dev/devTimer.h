#ifndef __DEVTIMER_H__
#define __DEVTIMER_H__

#include "includes.h"

#ifdef RT_DS1302
#error "aaa"
#define  SetSysTime  RTCWirte
#define  GetSysTime  RTCRead
#define  Init_Rtc    _nop_

void EnableRTCWrite();
void DisableRTCWrite();
void RTCWirte();
void RTCRead();

#else

#define  SetSysTime  WriteRealtime
#define  GetSysTime  ReadRealtime
void Init_Rtc(void);
void WriteRealtime ( void );
void ReadRealtime ( void );

uint cmdRtc ( CMD_LIN * pCmd );
uint cmdSetDataTime ( CMD_LIN * pCmd );
#endif

#endif
