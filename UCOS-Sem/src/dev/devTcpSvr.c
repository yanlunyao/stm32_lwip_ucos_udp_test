#include "includes.h"
#include <dev.h>
#include <SCom.h>
#include <comm.h>
#include <err.h>
#include <ip_addr.h>
#include <tcp.h>
#include "DLT645.h"
#include "devTcpip.h"

/*当客户端发起一个tcp连接，保存其连接句柄，
在rs485收到数据时，往这个句柄中写入数据*/
struct tcp_pcb * gPcbLastConnectFromClient = NULL;

uchar TCP_DataBuf1[100]={0};
uchar TCP_sav_num1=0;
unsigned int debugFlag = 1<<2;
//void tcpsendend(void *arg, struct tcp_pcb *tpcb, u16_t len);
static err_t tcpSvr_err ( void *arg,  err_t err );
/*******************************************************************************
*函数名:
　　tcpSvr_recv()
*功能: 这是一个回调函数，当一个TCP段到达这个连接时会被调用

*输入:
*输出:
*说明:
*******************************************************************************/
static err_t tcpSvr_recv ( void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err )
{
   // char i;

    if ( ( err == ERR_OK ) && ( p != NULL ))
    {
        gPcbLastConnectFromClient = pcb;

        /* Inform TCP that we have taken the data. */
        tcp_recved ( pcb, p->tot_len );
        
        if ( debugFlag & 0x01 )
        {
            printf ( "\r\n p->len %d/%d " , p->len, p->tot_len );
            printf ( "msg:[%s]" , p->payload );																		
        }

        memcpy ( TCP_DataBuf1 , p->payload , p->len );
        TCP_sav_num1 = p->len;
        
       // gPubVal.bLastRs485CmdFromUart = false;
        
        tcp_write(gPcbLastConnectFromClient, TCP_DataBuf1,TCP_sav_num1,0);
        tcp_output(gPcbLastConnectFromClient);
/*
        //DLT645-2007
        if(RxValidDataFrame(TCP_DataBuf,TCP_sav_num))
        {           
           
            ClearDLT645SendBuf();
            MeterCmdAnalysis1(TCP_DataBuf);//645格式命令解析 
            tcp_write(gPcbLastConnectFromClient, Send645Buf,Send_645Buf_num,0);
            tcp_output(gPcbLastConnectFromClient);
            ClearDLT645RecvBuf();
            //len = tcp_sndbuf(pcb);
            //ClearDLT645SendBuf();
            
        }
        else
        {
           // printf("false");
        }*/
        //喂一次狗，避免客户端多次访问一直占用CPU而重启
       // WDog();
        //gPubVal.iSvrNoCommCnt = 0;

        //gPubVal.bTcpInComm += 10;

        pbuf_free ( p );  //释放该TCP段
       

    }

    if ( ( err == ERR_OK )  &&  ( p == NULL ) )
    {
        DBMTR( "\r\n close socket in recv \r\n" );

        tcp_recv ( pcb, NULL );        /* 设置TCP段到时的回调函数 */

        // tcp_err ( pcb, cli_conn_err );

        tcp_close ( pcb );
        gPcbLastConnectFromClient = NULL;
    }

    if (err != ERR_OK)
    {
        DBMTR("");
      
    }
    
//    tcp_close ( pcb );                                           /* 关闭这个连接 */

    err = ERR_OK; 
    return err;
}

/*******************************************************************************
*函数名:
　　devTcpSvr_accept()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
static err_t devTcpSvr_accept ( void *arg, struct tcp_pcb *pcb, err_t err )
{
    if ( debugFlag & 0x01 )
        printf ( "\r\n tcp connect accepted.. " );

    tcp_setprio ( pcb, TCP_PRIO_MIN );  /* 设置回调函数优先级，当存在几个连接时特别重要，此函数必须调用*/
    tcp_recv ( pcb, tcpSvr_recv );        /* 设置TCP段到时的回调函数 */

   tcp_err ( pcb,   tcpSvr_err );

    //gPcbLastConnectFromClient = pcb;


    //gPubVal.iSvrNoCommCnt = 0;

    //printf("test tcp\n");
    err = ERR_OK;

    return err;
}
/*******************************************************************************
*函数名:
　　taskTcpServer()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
void tcpServerInit ( void )
{
    struct tcp_pcb *pcb;
    err_t ret;
    struct ip_addr ipaddr; 

    printf ( "\r\nstart  tcpServerInit " );

    OSTimeDly ( OS_TICKS_PER_SEC );
  
    pcb = tcp_new();   //建立TCP连接           

#if 0
    ret = tcp_bind ( pcb, IP_ADDR_ANY, 5000 );        /* 绑定本地IP地址和端口号 */
#else

    /* 绑定本地IP地址和端口号 */
    //ret = tcp_bind ( pcb, IP_ADDR_ANY,  *(WORD *)&SysParam[ SP_LOCALPORT] ); 
     ret = tcp_bind ( pcb, IP_ADDR_ANY,  5000 );  

    //printf( "\r\ntcp server bind port %d " , *(WORD *) &SysParam[ SP_LOCALPORT] );
#endif

    if ( ret != ERR_OK )
    {
        DBMTR("");
        printf ( "\r\ntcp_bind ret:0x%x" , ret );
    }
    
    pcb = tcp_listen ( pcb );                                       /* 进入监听状态 */

    if ( pcb == NULL )
    {
        DBMTR("");
        printf ( "\r\ntcp_listen pcb:0x%x" , ( int ) pcb );
    }
    printf ( "\r\ntcp_listen pcb start...." );
    tcp_accept ( pcb, devTcpSvr_accept );                /* 设置有连接请求时的回调函数 */

}


static err_t tcpSvr_err ( void *arg,  err_t err )
{
    printf("\r\n *** tcpSvr_err()**** " );
    
    if (gPcbLastConnectFromClient != NULL)
    {
        tcp_close(gPcbLastConnectFromClient);
        gPcbLastConnectFromClient = NULL;
    }
    return ERR_OK;
}


/*******************************************************************************
*函数名:
　　sendToLastConnectFromClient()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
void sendToLastConnectFromClient ( void * buff , int len )
{
    err_t ret;
    if ( gPcbLastConnectFromClient != NULL )
    {
       // _pl_;
        ret = tcp_write ( gPcbLastConnectFromClient, buff, len , 0 );
        if ( ERR_OK !=  ret )   //发送数据
        {
            DBMTR("");
            printf("ret :0x%x " ,ret);
            //gPcbLastConnectFromClient = NULL;
        }

        if ( tcp_output ( gPcbLastConnectFromClient ) != ERR_OK )
        {
            DBMTR("");
        }
    }


}



