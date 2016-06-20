/**
* @file tcpserver.c
* @brief A brief file description.
* @details
*     A more elaborated file description.
* @author Wang Mengyin
* @date 2010Jul31 21:00:58
* @note
*               Copyright 2010 Wang Mengyin.ALL RIGHTS RESERVED.
*                            http://tigerwang202.blogbus.com
*    This software is provided under license and contains proprietary and
* confidential material which is the property of Company Name tech.
*/


#define __TCPSERVER_C


/* Includes ------------------------------------------------------------------*/
#include "tcpserver.h"
#include "lwip/mem.h" /* 为了分配内存 */
#include <lwip/sockets.h> /* 使用BSD Socket接口必须包含sockets.h这个头文件 */
#include "includes.h"

#include "app_cfg.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#ifndef TCPSERVER_STK_SIZE          /* tcpserver 使用堆栈大小 */
#define TCPSERVER_STK_SIZE  300
#endif

#ifndef LWIP_TCPSERVER_PRIO         /* tcpserver 任务优先级 */
#define LWIP_TCPSERVER_PRIO 15
#endif

#ifndef RECV_BUFFER_SIZE            /* 接收缓存区大小 */
#define RECV_BUFFER_SIZE    512
#endif

#ifndef TCPSERVER_PORT_NO
#define TCPSERVER_PORT_NO   5002    /* tcpserver 使用端口号 */
#endif
/* Private macro -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
static const char send_data[] = "This is TCP Server from uC/OS-II."; /* 发送用到的数据 */
static OS_STK   TcpServerStack[TCPSERVER_STK_SIZE];

/* Private function prototypes -----------------------------------------------*/
//static void tcpserv(void* parameter);
static void tcpserver_thread(void* arg);
/* Private functions ---------------------------------------------------------*/
/**
* @brief tcpserver accept tcp connect receive data and output on debug thermal.
* @param void* parameter :unused.
*/
//static void tcpserv(void* parameter)
void tcpserv(void* parameter)
{
   char *recv_data; /* 用于接收的指针，后面会做一次动态分配以请求可用内存 */
   u32_t sin_size;
   int sock, connected, bytes_received;
   struct sockaddr_in server_addr, client_addr;
   uchar stop = FALSE; /* 停止标志 */

   recv_data = mem_malloc(RECV_BUFFER_SIZE); /* 分配接收用的数据缓冲 */
   if (recv_data == NULL)
   {
       printf("No memory\n");
       return;
   }

   /* 一个socket在使用前，需要预先创建出来，指定SOCK_STREAM为TCP的socket */
   if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
       /* 创建失败的错误处理 */
       printf("Socket error\n");

       /* 释放已分配的接收缓冲 */
       mem_free(recv_data);
       return;
   }

   /* 初始化服务端地址 */
   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(TCPSERVER_PORT_NO); /* 服务端工作的端口 */
   server_addr.sin_addr.s_addr = INADDR_ANY ;
   memset(&(server_addr.sin_zero),8, sizeof(server_addr.sin_zero));

   /* 绑定socket到服务端地址 */
   if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
   {
       /* 绑定失败 */
       printf("Unable to bind\n");

       /* 释放已分配的接收缓冲 */
       mem_free(recv_data);
       return;
   }

   /* 在socket上进行监听 */
   if (listen(sock, 5) == -1)
   {
       printf("Listen error\n");

       /* release recv buffer */
       mem_free(recv_data);
       return;
   }

   printf("\nTCPServer Waiting for client on port %d...\n", TCPSERVER_PORT_NO);
   while(stop != TRUE)
   {
       sin_size = sizeof(struct sockaddr_in);

       /* 接受一个客户端连接socket的请求，这个函数调用是阻塞式的 */
       connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
       /* 返回的是连接成功的socket */

       /* 接受返回的client_addr指向了客户端的地址信息 */
       printf("I got a connection from (%s , %d)\n",
                  inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

       /* 客户端连接的处理 */
       while (1)
       {
           /* 发送数据到connected socket */
           //send(connected, send_data, strlen(send_data), 0);

           /* 从connected socket中接收数据，接收buffer是1024大小，但并不一定能够收到1024大小的数据 */
           bytes_received = recv(connected,recv_data, RECV_BUFFER_SIZE, 0);
           if (bytes_received <= 0)
           {
               if(bytes_received == 0)
                   printf("client close connection.\n");
               else
                   printf("received failed, server close connection.\n");
               
               /* 接收失败，关闭这个connected socket */
               lwip_close(connected);
               break;
           }
            //讲收到的数据回发
            send(connected,recv_data,bytes_received,0);

            
           /* 有接收到数据，把末端清零 */
           recv_data[bytes_received] = '\0';
           if (strcmp(recv_data , "q") == 0 || strcmp(recv_data , "Q") == 0)
           {
               /* 如果是首字母是q或Q，关闭这个连接 */
               printf("receive \"q\" command, close connection.\n");
               lwip_close(connected); // close socket
               break;
           }
           else if (strcmp(recv_data, "exit") == 0)
           {
               /* 如果接收的是exit，则关闭整个服务端 */
               printf("receive \"exit\" command, exit tcpserver task.\n");
               lwip_close(connected); // close socket
               stop = TRUE;
               break;
           }
           else
           {
               /* 在控制终端显示收到的数据 */
              // printf("RECIEVED DATA = %s \n" , recv_data);
           }
       } // end of while(1)
   } // end of while(stop != TRUE)

   /* 释放接收缓冲 */
   mem_free(recv_data);

   return ;
}

/**
* @brief tcpserver thread
* @param void *arg :unused.
*/
static void tcpserver_thread(void *arg)
{
    tcpserv(arg);
    
    printf("Delete Tcpserver thread.\n");
    OSTaskDel(OS_PRIO_SELF);
}

/**
* @brief start tcpserver
*/
void tcpserver_init()
{
    uchar  os_err;
    
    OSTaskCreate(tcpserver_thread, 
                 NULL, 
                 &TcpServerStack[TCPSERVER_STK_SIZE - 1], 
                 LWIP_TCPSERVER_PRIO
                     );
#if (OS_TASK_NAME_SIZE >= 11)
//    OSTaskNameSet(LWIP_TCPSERVER_PRIO, (uchar *)"tcpserver", &os_err);
#endif
}

/*-- File end --*/

