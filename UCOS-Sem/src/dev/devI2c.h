#ifndef    __DEVI2C_H__ 
#define    __DEVI2C_H__ 

#if 0
#include    <hw_types.h> 
#include    <hw_memmap.h>
#include    <pin_map.h>
#include    <i2c.h> 

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long
#define uint unsigned int

//    定义 I2C 主机结构体 
typedef struct 
{ 
  unsigned char ucSLA;      //    从机地址（这是 7 位纯地址，不含读写控制位） 
  unsigned long ulAddr;     //    数据地址 
  unsigned int uiLen;      //    数据地址长度（取值 1、2 或 4） 
  char *pcData;        //    指向收发数据缓冲区的指针 
  unsigned int uiSize;      //    收发数据长度 
} tI2CM_DEVICE; 
 
//    对 tI2CM_DEVICE 结构体变量初始化设置所有数据成员 
extern void I2CM_DeviceInitSet(tI2CM_DEVICE *pDevice, unsigned char ucSLA, 
                          unsigned long ulAddr, 
                          unsigned int uiLen, 
                          char *pcData, 
                          unsigned int uiSize); 
 
//    对 tI2CM_DEVICE 结构体变量设置与数据收发相关的成员（数据地址、数据缓冲区、数据大小） 
extern void I2CM_DeviceDataSet(tI2CM_DEVICE *pDevice, unsigned long ulAddr, 
                      char *pcData, 
                      unsigned int uiSize); 
 
//    I2C 主机初始化 
extern void I2CM_Init(void); 
 
//    I2C 主机发送或接收数据 
extern unsigned long I2CM_SendRecv(tI2CM_DEVICE *pDevice, tBoolean bFlag); 
 
//    定义宏函数：发送数据 
#define    I2CM_DataSend(pDevice)    I2CM_SendRecv(pDevice, false) 
 
//    定义宏函数：接收数据 
#define    I2CM_DataRecv(pDevice)    I2CM_SendRecv(pDevice, true) 
#endif

#include "inc/hw_memmap.h"	  
#include "inc/hw_types.h" 
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "main.h"


#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned long
#define uint unsigned int

/*************************************************************************/ 
/*************************************************************************/ 
#define  SCL_PERIPH     SYSCTL_PERIPH_GPIOB
#define  SCL_PORT       GPIO_PORTB_BASE
#define  SCL_PIN        GPIO_PIN_2//SCL

#define  SDA_PERIPH     SYSCTL_PERIPH_GPIOB
#define  SDA_PORT       GPIO_PORTB_BASE
#define  SDA_PIN        GPIO_PIN_3      //SDA

#define SCL_H           GPIOPinWrite (SCL_PORT, SCL_PIN, SCL_PIN)
#define SCL_L           GPIOPinWrite (SCL_PORT, SCL_PIN, 0)
#define SDA_H           GPIOPinWrite (SDA_PORT, SDA_PIN, SDA_PIN)
#define SDA_L           GPIOPinWrite (SDA_PORT, SDA_PIN, 0)

#define SDA_in          GPIOPinTypeGPIOInput(SDA_PORT, SDA_PIN)     //SDA改成输入模式
#define SDA_out         GPIOPinTypeGPIOOutput(SDA_PORT,SDA_PIN)     //SDA变回输出模式
#define SDA_val         GPIOPinRead(SDA_PORT,SDA_PIN)               //SDA的位值
/*************************************************************************/ 
/*************************************************************************/ 
//#define   FMSLAVE_ADDR       0xAC   // 1010_1100
#define   FMSLAVE_ADDR       0xA8   // 1010_1100


#define   delay_us(x)   SysCtlDelay(x*(TheSysClock/3000000));       //延时uS
#define   delay_ms(x)   SysCtlDelay(x*(TheSysClock/3000));          //延时mS
/*************************************************************************/ 
/*************************************************************************/ 

//以下为I2C驱动程序
/*************************************************************************/ 
/*************************************************************************/ 
/*************************************************************************/  
void I2C_FM24C_Init(void);


//以下为FM24C04应用程序
/*************************************************************************/ 
/*************************************************************************/ 
extern void  Write_FM24C(BYTE sla,WORD suba,BYTE *s,BYTE no);
//多字节数据读出
extern void  Read_FM24C(BYTE sla,WORD suba,BYTE *s,BYTE num);
//
extern void  Clear_FM24C(BYTE sla,WORD suba,BYTE no);

/*************************************************************************************************
**					　　　　　				END FILE
**************************************************************************************************/

 
#endif    //    __DEVI2C_H__ 




