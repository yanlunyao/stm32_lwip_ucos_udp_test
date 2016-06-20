#include "includes.h"
#include <dev.h>
#include <comm.h>
#include <SCom.h>
#include <err.h>
#include <inet.h>
#include <ip_addr.h>
#include <tcp.h>
#include <crc.h>

BYTE RecValidTcpFrameGen ( unsigned char * buff , int bufflen );


static int gbheardAck = false;
static struct tcp_pcb *Clipcb = NULL;


/*******************************************************************************
*函数名:
　　()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
static err_t heartTcp_recv ( void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err )
{

    if ( ( err == ERR_OK ) && ( p != NULL ) )
    {

        tcp_recved ( pcb, p->tot_len );
        _pl_;

        if ( debugFlag & 0x01 )
        {
            printf ( "\r\n p->len %d/%d " , p->len, p->tot_len );
            printf ( "msg:[%s]" , p->payload );
        }

        if(RecValidTcpFrameGen ( p->payload , p->len ) )
        {
            gbheardAck = true;
        }

        tcp_close ( pcb );
        //  gConnectedpcb = NULL;
        err = ERR_OK;
        return err;
    }
    else
    {
        tcp_close ( pcb );
        //  gConnectedpcb = NULL;
        err = ERR_OK;
        return err;

    }
}


/*******************************************************************************
*函数名:
　　()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
err_t heartTcpCliConnected ( void *arg, struct tcp_pcb *pcb, err_t err )
{
    WORD Crc;
    static char buffSend[23];
    //const static uint8 TCP_TestData[]="This is LwIP TCP Client  在 Luminary Cortex-M3 上的测试！\r\n"; 

   // _pl_;
    buffSend[0] = 0x68;
    buffSend[1] = 0x09;
    buffSend[2] = 0x00;
    buffSend[3] = 0x68;
    buffSend[4] = SysParam[SP_DEVADDR];
    buffSend[5] = SysParam[SP_DEVADDR+1];
    buffSend[6] = SysParam[SP_DEVADDR+2];
    buffSend[7] = SysParam[SP_DEVADDR+3];
    buffSend[8] = SysParam[SP_DEVADDR+4];

    buffSend[9]  = 0x1C;
    buffSend[10] = 0x11;

    //数据 9 字节
    buffSend[11] = SysParam[SP_LOCALIP];
    buffSend[12] = SysParam[SP_LOCALIP+1];
    buffSend[13] = SysParam[SP_LOCALIP+2];
    buffSend[14] = SysParam[SP_LOCALIP+3];

    buffSend[15] = SysParam[SP_NETMODE];  // 网络通信方式

    buffSend[16] = SysParam[SP_SERVERPORT+1];   // 远程端口
    buffSend[17] = SysParam[SP_SERVERPORT];

    buffSend[18] = SysParam[SP_LOCALPORT+1];   // 本地端口
    buffSend[19] = SysParam[SP_LOCALPORT];

    //CRC
    Crc = CRC16 ( buffSend, 20 );
    buffSend[20] = ( BYTE ) Crc;
    buffSend[21] = ( BYTE ) ( Crc >> 8 );

    buffSend[22] = 0x16;  // 结束

    //endDataLen = 23;
    tcp_recv ( pcb, heartTcp_recv );        /* 设置TCP段到时的回调函数 */

    tcp_setprio ( pcb, TCP_PRIO_MIN );
    tcp_write ( pcb, buffSend, sizeof(buffSend) , 0 );      //发送数据
    tcp_output ( pcb );

    //tcp_close(pcb);//在接收里关
    /*告警相关*/
    gPubVal.iHeartReportKeepLive  =  *(WORD *)& SysParam[ SP_HEARTTIME];

    gPubVal.bTcpInComm += 50;

    return ERR_OK;

}


/*******************************************************************************
*函数名:
　　()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
void heartRepConnect ()
{
    struct ip_addr ipaddr;
    err_t ret;
    
    memcpy ( ( void * ) &ipaddr.addr , &SysParam[SP_SERVERIP] , 4 );
    ipaddr.addr = htonl ( ipaddr.addr );
    //IP4_ADDR(&ipaddr,192,168,0,123); 

    Clipcb = tcp_new();            // 建立通信的TCP控制块(Clipcb)

    if ( Clipcb == NULL )
    {
        DBCMM("");
        printf( "\r\nheart report tcp_new error: clipcl:0x%x" , ( int ) Clipcb );
    }

    ret = tcp_bind ( Clipcb, IP_ADDR_ANY, 7807 ); // 绑定本地IP地址和端口号

    if ( ret  != ERR_OK )
    {
        DBCMM("");
        printf( "\r\n tcp_bind ret:0x%x" , ret );

        if ( ret == ERR_USE )
        {
            printf ( " addr aready used " );
        }
    }


    tcp_setprio ( Clipcb, TCP_PRIO_MIN );

    //连接服务器
    ret =  tcp_connect ( Clipcb, &ipaddr, *(WORD *)&SysParam[ SP_SERVERPORT],
                        heartTcpCliConnected );
  
    if ( ret  != ERR_OK )
    {
        DBCMM("");
        printf( "\r\n tcp_connect ret:0x%x" , ret );
    }


}


/*******************************************************************************
*函数名:
　　()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
//收到有效的TCP帧, 返回第一个起始符的位置
BYTE RecValidTcpFrameGen ( unsigned char * buff , int bufflen )
{
    static BYTE i, len;
    static WORD RecCrc;
    int iDataLength = 0;
    WORD Crc;

    int pos = 0;

    if ( bufflen == 0 )
    {
        DBCMM("");

        return FALSE;
    }

    for ( i = 0; i < bufflen; i++ )
    {
        if ( buff[i] == 0x16 ) // 结束符
        {
            // 查找起始符
            for ( pos = 0; pos < i; pos++ )
            {
                if ( ( buff[pos] == 0x68 ) && ( buff[pos+3] == 0x68 ) )
                {
                    iDataLength = buff[pos+1];

                    //CRC
                    Crc = CRC16 ( &buff[pos], iDataLength + 11 );
                    RecCrc = 0;
                    RecCrc = ( WORD ) buff[pos+iDataLength+12];
                    RecCrc = ( RecCrc << 8 ) + buff[pos+iDataLength+11];

                    if ( Crc != RecCrc )
                    {
                        DBCMM("");
                        return FALSE;
                    }

                    // 非本机数据返回
                    if ( !mcmp ( (BYTE*)&buff[pos+4], (BYTE*)&SysParam[SP_DEVADDR], 5 ) )
                    {
                        DBCMM("");
                        return FALSE;
                    }


                    return DEVFRAME;
                }

                if ( ( buff[pos] == 0xFE ) && ( buff[pos+1] == 0xFE ) )
                {
                    if ( ( buff[pos+2] == 0x68 ) && ( buff[pos+9] == 0x68 ) )
                    {
                        len = buff[pos+11];

                        if ( i == ( len + 13 ) )
                        {

                            return METERFRAME;
                        }
                    }
                }
            }
        }
    }

    return FALSE;
}


/*******************************************************************************
*函数名:
　　()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
void taskHeartRep ( void *pParam )
{
    uint sendReportTry = 0;
    uint i;

    OSTimeDly ( OS_TICKS_PER_SEC * 2 );

    printf ( "\r\nHeartRep task begin..." );

    printf ( "\r\n \r\nPlease press Enter key to the console." );
	
    while ( 1 )
    {
        while ( SysParam[SP_DEVIPMODE]  != IPMODE_FIXED)//如果用动态IP这是无线模块发送心跳包
        {
            OSTimeDly ( 100*10 );
        }

        gbheardAck =  false;

        heartRepConnect();

        //等待10s
        OSTimeDly ( OS_TICKS_PER_SEC * 10 );

        sendReportTry++;

        tcp_close ( Clipcb );
        OSTimeDly ( OS_TICKS_PER_SEC * 1 );


        if ( ( gbheardAck == true ) || ( sendReportTry >= 3 ) )
        {
            uint second =  *(WORD *)&SysParam[ SP_HEARTTIME];

            if ( gbheardAck == false )
            {
                second = 30;
            }


            if ( debugFlag & 0x01 )
            {
                printf ( "heart report sleep for %d second " , second );
            }

            for ( i = 0; i < second; i++ )
            {
                OSTimeDly ( OS_TICKS_PER_SEC );
            }

            sendReportTry = 0;
        }
        else
        {
            if ( gbheardAck == false )
            {
                tcp_close ( Clipcb ) ;
                OSTimeDly ( OS_TICKS_PER_SEC * 30 );
            }
        }

    }

}


