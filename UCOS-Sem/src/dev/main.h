/*************************************************************
成都昊普环保技术有限公司   版权所有

文件名:  main.h
作  者:  潘国义
描  述:  Cdma模块
修订记录:   

**************************************************************/


#ifndef __main_H__
#define __mian_H__
#include "comm.h"
#include "includes.h"
/*******************************************************************************
说明 os 任务定义
*******************************************************************************/
/*tcp server 主消息处理函数*/
#define PRIO_MAIN_HANDLE        2

/*SHELL处理任务*/
#define PRIO_SHELL              3 

//外中断PE0任务
#define PRIO_RNISR              4

/*rs485处理任务*/
#define PRIO_RS485              5

//无线模块接收数据
#define PRIO_GSMMSG             6


/*GSM处理任务*/
#define PRIO_GSM                7

/*GSM初始化任务*/
#define PRIO_GSMINIT            8

/*hart report任务*/
#define PRIO_HEART_REP          9

/*定期处理任务*/
#define PRIO_PRID               10

/*WATCHDOG 处理任务*/
#define PRIO_WATCH0             11

extern BYTE DevVersion[7]        ;       // 格式 001.100        // 设备版本  7字节
extern const BYTE DevRelTime[]   ;       // 软件发布时间  2012年  7字节 
extern const BYTE *DevIndex      ;       // 设备编号  8字节   

extern unsigned int  TheSysClock;


void GetSysParam(void);
void SetDefParam(void);

BYTE SaveSysParam(void);
BYTE CheckValidUserName(void);
BYTE CheckValidPassWord(void);
void SysReset(void);
BYTE SysSelfCheck(void);

#endif

