#ifndef __DEV_H__
#define __DEV_H__

#include "includes.h"
#include "comm.h"


#define DEV_WORK_MODE_FIX_IP  1
#define DEV_WORK_MODE_DHCP    2
#define DEV_WORK_MODE_PPPOE   3

#define EleMeterCnt   3  //电表数量


typedef enum
{
    PARAM_SERVER_IP = 0,
    PARAM_SERVER_PORT,
    PARAM_DEVICE_IP,
    PARAM_DEVICE_PORT,
    PARAM_DEVICE_AREA,
    PARAM_DEVICE_ADDR,
    PARAM_DEVICE_PSWD,
    PARAM_DEVICE_UTCP,
    PARAM_DEVICE_CMMMODE,
    PARAM_DEVICE_NETMODE,
    PARAM_DEVICE_WRKMODE,
    PARAM_DEVICE_HRTDLY,
    PARAM_DEVICE_UDHCP,
    PARAM_DEVICE_MASK,
    PARAM_DEVICE_GTWAY,
    PARAM_DEVICE_RS485BPS,
    PARAM_DEVICE_RS485PRT,
    PARAM_DEVICE_MAC,    
} tSetParam;


typedef struct
{
    uint flag;                  //force to 0x12345678,
    uint xor;
    uint addval;
    ushort scrc;
    char DevIpAddrMask[20];
    char DevIpGatway[20];
    uchar paramEx[SYSPARAM_COUNT];
   
} tDevParam;

extern tDevParam gDevParam;

#define SysParam  gDevParam.paramEx
#define UART2_BUFLEN  100
extern BYTE Uart2_Buffer[UART2_BUFLEN];

extern BYTE Uart2_sav_num;
//extern BYTE gDevParam.paramEx[];
//extern  char *SysParam;
extern BYTE SysTime[7];
extern BYTE SendFrame_Buffer[UART2_BUFLEN];

extern BYTE LastError;

extern BYTE DevAddrTmp[5];   // 设备地址缓存
//extern BYTE DevPassTmp[];   // 设备密码缓存
extern BYTE ServerLogin ;
extern unsigned int bDevAddModify;

char * getStrParamIpAddr ( char * ipAddr );
ushort getParamword ( char * buff );
void  loadParamFromFlash ( tDevParam * pParam );

void showParam ( void );
void showDebug ( void );
void setParamToDefault ( void );
uint cmdSetParam ( CMD_LIN * pCmd );
uint cmdSaveParam ( CMD_LIN * pCmd );
uint checkParam ( tDevParam * pParam );
uint saveParam ( tDevParam * pParam );
void showIpParam(void);
void Clear_Sms_buf(void);
unsigned short crc_16(unsigned char * buff ,int len);

#endif

