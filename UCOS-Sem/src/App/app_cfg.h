#ifndef  __APP_CFG_H__
#define  __APP_CFG_H__

/*
*********************************************************************************************************
*                                            设置任务优先级
*********************************************************************************************************
*/

#define  APP_CFG_TASK_START_PRIO               3
#define  APP_CFG_TASK_ToggleLED_PRIO           8
#define  APP_CFG_TASK_ToggleLED1_PRIO           9
#define  LWIP_TASK_START_PRIO                 10
#define  LWIP_TASK_END_PRIO                   12
////add xpz
//lwip DHCP任务
//设置任务优先级
#define LWIP_DHCP_TASK_PRIO       		15
#define UDPRECV_PRIO                  LWIP_DHCP_TASK_PRIO+1
//设置任务堆栈大小
#define LWIP_DHCP_STK_SIZE  		    128
#define UDPRECV_STK_SIZE						300
////
#define  OS_TASK_TMR_PRIO              (OS_LOWEST_PRIO - 2)

/*
*********************************************************************************************************
*                                            设置栈大小(单位：OS_STK)
*********************************************************************************************************
*/

#define  APP_CFG_TASK_START_STK_SIZE         128
#define  APP_CFG_TASK_ToggleLED_STK_SIZE     128
////add xpz
#define DISPLAY_STK_SIZE	128
////
#endif
