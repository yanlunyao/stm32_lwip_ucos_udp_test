#ifndef __EM485_H__
#define __EM485_H__


#define MAX_RX_485_PACK_SIZE 200
#define RS485_UART_IDX  2  // 2

BYTE ReceiveReadCmd(void);
void SendReadCmd(BYTE UseId);
BYTE CheckMeter(void);
void HandleMeterFrame(void);
void Clear_Sms_buf(void);
void askBeginWaitRs485 ( int bDbMod );

void rs485SendBuff ( unsigned char * buff , int len );
int checkPackMod(unsigned char * buff ,int len);
BYTE RecValidMeterFrame ( unsigned char * rxBuff , int rxLen );

#endif

