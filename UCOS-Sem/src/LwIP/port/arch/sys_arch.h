#ifndef __ARCH_SYS_ARCH_H__
#define __ARCH_SYS_ARCH_H__

#include "arch/cc.h" //包含cc.h头文件
#include "ucos_ii.h"
#include "app_cfg.h" // define LWIP TASK Prio

#ifdef SYS_ARCH_GLOBALS
#define SYS_ARCH_EXT
#else
#define SYS_ARCH_EXT extern
#endif

/*-----------------macros-----------------------------------------------------*/
#define LWIP_STK_SIZE	300

#define LWIP_TASK_MAX	  (LWIP_TASK_END_PRIO - LWIP_TASK_START_PRIO + 1)		
//max number of lwip tasks (TCPIP) note LWIP TASK start with 1

#define LWIP_START_PRIO	  LWIP_TASK_START_PRIO		//first prio of lwip tasks in uC/OS-II

#define MAX_QUEUES        10	// the number of mailboxes
#define MAX_QUEUE_ENTRIES 20	// the max size of each mailbox

#define SYS_MBOX_NULL (void *)0
#define SYS_SEM_NULL  (void *)0

#define sys_arch_mbox_tryfetch(mbox,msg) \
      sys_arch_mbox_fetch(mbox,msg,1)

/*-----------------type define------------------------------------------------*/

/** struct of LwIP mailbox */
typedef struct {
    OS_EVENT*   pQ;
    void*       pvQEntries[MAX_QUEUE_ENTRIES];
} TQ_DESCR, *PQ_DESCR;

typedef OS_EVENT *sys_sem_t; // type of semiphores
typedef PQ_DESCR sys_mbox_t; // type of mailboxes
typedef INT8U sys_thread_t; // type of id of the new thread

typedef INT8U sys_prot_t;

/*-----------------global variables-------------------------------------------*/

SYS_ARCH_EXT OS_STK LWIP_TASK_STK[LWIP_TASK_MAX][LWIP_STK_SIZE];

#endif /* __SYS_RTXC_H__ */
