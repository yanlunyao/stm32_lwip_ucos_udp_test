/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "includes.h"
/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/
/* ----------------- APPLICATION GLOBALS ------------------ */


/* -------- uC/OS-II APPLICATION TASK STACKS -------------- */
static  OS_STK         App_TaskStartStk[APP_CFG_TASK_START_STK_SIZE];
/* ---------- uC/OS-II APPLICATION EVENTS ----------------- */
extern void tcpServerInit ( void );
extern void tcpserv(void* parameter);
/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void            App_TaskStart                (void       *p_arg);
/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This the main standard entry point.
*
* Note(s)     : none.
*********************************************************************************************************
*/

int  main (void)
{
    //BSP_IntDisAll();                                            /* Disable all interrupts until we are ready to accept them */
	
    OSInit();                                         /* Initialize "uC/OS-II, The Real-Time Kernel"              */
	

    OSTaskCreateExt((void (*)(void *)) App_TaskStart,        /* Create the start task                                    */
                    (void           *) 0,
                    (OS_STK         *)&App_TaskStartStk[APP_CFG_TASK_START_STK_SIZE - 1],
                    (INT8U           ) APP_CFG_TASK_START_PRIO,
                    (INT16U          ) APP_CFG_TASK_START_PRIO,
                    (OS_STK         *)&App_TaskStartStk[0],
                    (INT32U          ) APP_CFG_TASK_START_STK_SIZE,
                    (void           *) 0,
                    (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
    OSStart();      /* Start multitasking (i.e. give control to uC/OS-II)       */
}

/*
*********************************************************************************************************
*                                          App_TaskStart()
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Argument(s) : p_arg   is the argument passed to 'App_TaskStart()' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Caller(s)   : This is a task.
*
* Notes       : (1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                   used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  App_TaskStart (void *p_arg)
{   

    (void)p_arg;                                                /* See Note #1                                              */

    BSP_Init();                                                 /* Initialize BSP functions                                 */
    

    SysTick_Init();                                             /* Initialize the SysTick.                              */

// #if (OS_TASK_STAT_EN > 0)
//     OSStatInit();                                               /* Determine CPU capacity                                   */
// #endif
  printf("\r\n uC/OS-II Usart MsgQueue Demo \r\n");
	Init_LwIP();
  App_TaskCreate();                                           /* Create application tasks                                 */

	//	tcpServerInit();
//	tcpserv(0);
//#if LWIP_DHCP
//	lwip_comm_dhcp_creat(); //创建DHCP任务
//#endif
	OSTaskDel(OS_PRIO_SELF);
}
