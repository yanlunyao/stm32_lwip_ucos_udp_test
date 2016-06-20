#ifndef __SMS_H__
#define __SMS_H__

#include "comm.h"
#include "devUartProc.h"

#define SMSMSGCOUNT 100

#define Sms_BUFFER_LEN  UART0_BUFLEN
#define Sms_Buffer   Uart0_Buffer
#define Sms_sav_num  Uart0_Sav_Num
#define Sms_SendStr   sendUart0Term
#ifdef UART_TESTMODE
#define Send_a_byte(x)    UARTCharPut ( UART1_BASE ,(x) );
#else
#define Send_a_byte(x)    UARTCharPut ( UART0_BASE ,(x) );
#endif

#define Clear_Sms_buf ClearUart0Buf

typedef struct
{
    BYTE Chr;
    WORD Code;
}UNICODE;


void Init_GsmModul(void);
void ReportSysParam(void);
BYTE NewMsgRec(void);

BYTE Init_SmsFun(void);
BYTE Send_Txt_SMS(BYTE *number,BYTE *content);

BYTE ReceiveSMS(void);
//void Clear_Sms_buf(void);
void Send_String(BYTE *Word_ptr);
BYTE Check_string_rxed(BYTE *String_ptr);
void RemoveFrontData(void);
BYTE Get_SMS_index_array(void);
BYTE Check_Valid_Cmd(void);
BYTE Set_Txt_SMS(void);
BYTE Set_Pdu_SMS(void);
void Clear_Sms_Content(void);
WORD GetSignLoc(BYTE Sign,BYTE Index);
BYTE Set_AutoRegNet(void);
BYTE GetSmsCenter(void);
void DealSmsContent(void);
BYTE ReportCurentPower(void);
BYTE ChangeContent(void);
void SmsWakeup(void);
WORD GetRepDay(void);
BYTE GetRepHour(void);
void SetNextPepTime(void);
BYTE GetCurrentPower(void);

BYTE Encode7bit(BYTE *Src, BYTE Len);

void SmsMsgHandle(void);
void Task_1h(void);


#endif
