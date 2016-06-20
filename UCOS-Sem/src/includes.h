#ifndef _INCLUDES_H__
#define _INCLUDES_H__

#include <stdio.h>
#include <string.h>

#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#include "ucos_ii.h"

#include "BSP.h"
#include "app.h"
#include "app_cfg.h"
//#include "LwIP.h"

#define uchar unsigned char
#define BYTE  unsigned char
#define WORD  unsigned short
#define uint  unsigned int

#define FALSE     0
#define TRUE      1
typedef struct  
{
	u8 mac[6];      //MAC地址
	u8 remoteip[4];	//远端主机IP地址 
	u8 ip[4];       //本机IP地址
	u8 netmask[4]; 	//子网掩码
	u8 gateway[4]; 	//默认网关的IP地址
	
	vu8 dhcpstatus;	//dhcp状态 
					//0,未获取DHCP地址;
					//1,进入DHCP获取状态
					//2,成功获取DHCP地址
					//0XFF,获取失败.
}__lwip_dev;
extern __lwip_dev lwipdev;	//lwip控制结构体

#endif //_INCLUDES_H__
