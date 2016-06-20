/*************************************************************
成都昊普环保技术有限公司   版权所有


文件名:       Console.h
作  者:       潘国义   
描  述:       串口控制台
修订记录:

**************************************************************/


#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#define CMD_NONE              0
#define CMD_MAIN_MENU         1
#define CMD_INPUT_DEVID       2
#define CMD_INPUT_BAUDRATE    3
#define CMD_INPUT_CHECK       4
#define CMD_INPUT_SRVIP       5
#define CMD_INPUT_SRVPORT     6
#define CMD_INPUT_LOCIP       7
#define CMD_INPUT_LOCPORT     8
#define CMD_INPUT_DATE        9
#define CMD_INPUT_TIME        10
#define CMD_INPUT_SETPARAM    11
#define CMD_INPUT_READPARAM   12
#define CMD_INPUT_YESNO       13
#define CMD_INPUT_METERID     14


//校表命令
#define CMD_CHECK_METER       15
#define CMD_CHECK_SELF        16
#define CMD_CHECK_CURT        17
#define CMD_CHECK_VLT         18
#define CMD_CHECK_PWR         19
#define CMD_CHECK_ENERGR      20
#define CMD_CHECK_QIUT        21
#define CMD_CHECK_ADRESS      22
#define CMD_CHECK_GRAW        23




void CmdLine(void*Msg);
void PrintStr(const char * Msg);
void TestConsole(void);
void PrintVersion(void);

#endif

