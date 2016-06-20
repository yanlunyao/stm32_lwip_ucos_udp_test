/*************************************************************
成都昊普环保技术有限公司   版权所有
无线采集器


文件名:   
作  者:    潘国义
描  述:    无线模块
修订记录:

**************************************************************/

#ifndef __WLMODULE_H__
#define __WLMODULE_H__

//模块状态定义
#define InitState          0
#define InitEndState       1
#define CheckIpModState    2
#define TCPLinkState       3
#define LoginSvr           4
#define NomState           5


extern unsigned char NetModulState;


void Init_NetModul(void);

void PowerOn_Modul(void);
void PowerOff_Modul(void);
BYTE Check_Modul(void);

BYTE Check_Net(BYTE FLed);

BYTE ATCmd_GetVersion(void);
extern void Del_all_SMS(void);
extern void CmdTimeOutHandle(void);

extern void taskGSM(void* msg);
extern void taskGSMInit(void* msg);
extern void taskGSMGetMsg(void* msg);


#endif

