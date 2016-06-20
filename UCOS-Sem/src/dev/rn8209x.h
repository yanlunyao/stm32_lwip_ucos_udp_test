
#ifndef __RN8209X_H__
#define __RN8209X_H__

#include <cmmtype.h>
#include <includes.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define WIDTH_ONE_B     1
#define WIDTH_TWO_B     2
#define WIDTH_THREE_B   3
#define WIDTH_FOUR_B    4

//Isr Flag
#define RN_RZXIF      (1<<5)    //过零事件
#define RN_RQEOIF     (1<<4)    //无功电能寄存器溢出
#define RN_RPEOIF     (1<<3)    //有功电能寄存器溢出
#define RN_RQFIF      (1<<2)    //QF脉冲输出
#define RN_RPFIF      (1<<1)    //PF脉冲输出
#define RN_RDUPDIF    (1<<0)    //更新事件

//rn8209电能寄存器的值在铁电存储的地址

#define EEPROM_ENERGYADDR        0x0000 // 长度 4 * 9 ---->0x0023
#define EEPROM_ENERGYLASTADDR    0x0024 // 长度 4 * 9 ---->0x0047
#define EEPROM_OVERFLOWADDR      0x0048  //长度 1*9byte--->0x0050


#define EEPROM_ENERGY         0    // 4 * 9
#define EEPROM_ENERGYLAST     36    // 4 * 9
#define EEPROM_OVERFLOW       72   // 1 * 9
#define EEPROM_COUNT           81
//#define FMSLAVE_ADDR      0xA8


//电表常数
#define EC 800

//control register
#define SYSCON      0x00    // 2 bytes width
#define EMUCON      0x01    // 2 bytes width
#define HFCONST     0x02    // 2 bytes width
#define PSTART      0x03    // 2 bytes width
#define QSTART      0x04    // 2 bytes width
#define GPQA        0x05    // 2 bytes width
#define GPQB        0x06    // 2 bytes width
#define PHSA        0x07    // 1 bytes width
#define PHSB        0x08    // 1 bytes width
#define QPHSCAL     0x09    // 2 bytes width
#define APOSA       0x0A    // 2 bytes width
#define APOSB       0x0B    // 2 bytes width
#define RPOSA       0x0C    // 2 bytes width
#define RPOSB       0x0D    // 2 bytes width
#define IARMSOS     0x0E    // 2 bytes width
#define IBRMSOS     0x0F    // 2 bytes width
#define IBGAIN      0x10    // 2 bytes width

//measure register
#define PFCNT       0x20    // 2 bytes width
#define QFCNT       0x21    // 2 bytes width
#define IARMS       0x22    // 3 bytes width
#define IBRMS       0x23    // 3 bytes width
#define URMS        0x24    // 3 bytes width
#define UFREQ       0x25    // 2 bytes width
#define POWERPA     0x26    // 4 bytes width
#define POWERPB     0x27    // 4 bytes width
#define POWERQ      0x28    // 4 bytes width
#define ENERGYP     0x29    // 3 bytes width
#define ENERGYP2    0x2A    // 3 bytes width
#define ENERGYQ     0x2B    // 3 bytes width
#define ENERGYQ2    0x2C    // 3 bytes width
#define EMUSTATUS   0x2D    // 3 bytes width

//interrupt regiter
#define IE          0x40    // 1 bytes width
#define IF          0x41    // 1 bytes width
#define RIF         0x42    // 1 bytes width


//other register
#define SYSSTATUS   0x43    // 1 bytes width
#define RDATA       0x44    // 4 bytes width
#define WDATA       0x45    // 2 bytes width
#define COMMD       0xEA    // 1 bytes width
#define DEVID       0x7F    // 3 bytes width

#if 0
typedef struct
{
    uchar  add;
    uchar  Bwidth;
    ushort value;
} rn8209_fmt_t;
#else
typedef struct
{
   volatile uchar  add;
   volatile uchar  Bwidth;
   volatile ushort *value;
} rn8209_fmt_t;

#endif

extern const linCmdTabSTRUC cmmSetRnParamCmdTab[];
extern rn8209_fmt_t initTable[];
extern BYTE EEPROMPARAM[EEPROM_COUNT];

extern void init8209(void);
extern void getRawMeasure(int channel);
extern void QF_PF_IRQn_Output(int channel);
extern void taskRnIsr(void *pmsg);
extern void ShowMeterAddr(void);
extern void RnCheckSelf(void);
extern void RnCheckSelfCh(BYTE channel);
extern void taskPriod ( void *pParam );
extern void Check_KIA_KIB(BYTE channel,uint dati);
extern void Check_KU(BYTE channel,uint datu);
extern void Check_KPA_KPB(BYTE channel,uint datp);
extern void Check_HFConst1( BYTE channel,uint datE);








#ifdef __cplusplus
}
#endif

#endif

