

#include <includes.h>
#include <update.h>
#include <hw_flash.h>
#include <err.h>
#include <inet.h>
#include <ip_addr.h>
#include <tcp.h>
#include <sysctl.h>
#include <crc.h>
#include "uart.h"
#include "Dev.h"
#include <hw_uart.h>
#include "SwUpdate.h"




void cmmHandleUpdateMcu ( int * pRetDataLen , uchar * recvbuff )
{
    static int scrc;
    static int expectId = 0;
    uint ret = true;
    uint *pnt = NULL;
    uint ekey;
    uint imglen;
    uint cipher;
    int packId;
    int maxPackId;
    int i;
    tUpdatePack  *pReq = ( tUpdatePack* ) recvbuff;
    tUpdatePackRet  *pRet = ( tUpdatePackRet* ) recvbuff;

    //printf ( "\r\n UpdateMcu:exp:%d " , expectId );
    *pRetDataLen = sizeof ( tUpdatePackRet );

    packId = ( pReq->packId[0] ) | ( pReq->packId[1] << 8 ) | ( pReq->packId[2] << 16 );
    maxPackId = ( pReq->maxPackId[0] ) | ( pReq->maxPackId[1] << 8 ) | ( pReq->maxPackId[2] << 16 );

    if(debugFlag & CMM_DEBUG_UPDTE)
        printf ( "\r\nrev,id:%d/%d ", packId , maxPackId );

    if ( ( packId != 0 ) && ( packId != expectId ) && ( packId != ( expectId - 1 ) ) )
    {
        pRet->result = CMM_UPDATE_REUSLT_IDFAIL;
        printf ( "\r\nunexpectId %d / %d " , packId , expectId );
        return ;
    }

    if(packId == 0)
    {
        pnt = ( uint * )( ADDR_BIN_CIPHER + pReq->packdata ); 
        if(*pnt)
        {
            ekey = *( uint * )( ADDR_BIN_CRC + pReq->packdata ); 

            //decrypt cipher
            pnt = ( uint * )( ADDR_BIN_CIPHER + pReq->packdata ); 
            cipher = ekey ^ *pnt;

            //decrypt file lenght
            pnt = ( uint * )( ADDR_BIN_LENGTH + pReq->packdata ); 
            imglen = ekey ^ *pnt;

            //decrypt data
            pnt = ( uint * )( pReq->packdata ); 
            for(i = 0; i < CMM_UPDATE_PKT_SIZE/4; i++)
            {
                *pnt ^= cipher;
                pnt++;
                    
             }

            //write back cipher and file lenght
            *( uint * )( ADDR_BIN_CIPHER + pReq->packdata ) = cipher; 
            *( uint * )( ADDR_BIN_LENGTH + pReq->packdata ) = imglen; 
        }
        
        pnt = ( uint * )( ADDR_BIN_CRC + pReq->packdata ); 
        scrc = *pnt;
        *pnt = 0;

        //cipher = *( uint * )( ADDR_BIN_CIPHER + pReq->packdata ); 
        imglen = *( uint * )( ADDR_BIN_LENGTH + pReq->packdata ); 
        printf("\npackId=%d file len=%x CRC=%04x \n", packId, imglen, scrc);
    
    }

    //decrypt data
    cipher = *( uint * )( ADDR_BIN_CIPHER + TEMP_MCU_IMAGE_ADDR ); 
    if(cipher && packId)
    {
        if(packId % 100 == 0)
        {
            printf("\n");
        }
        printf("*");
        pnt = ( uint * )( pReq->packdata ); 
        for(i = 0; cipher && (i < CMM_UPDATE_PKT_SIZE/4); i++)
        {
            *pnt ^= cipher;
            pnt++;
            
        }
    }

    ret = ramProgramFlash ( ( ( packId * CMM_UPDATE_PKT_SIZE ) + TEMP_MCU_IMAGE_ADDR ),
                            ( unsigned long * ) pReq->packdata, CMM_UPDATE_PKT_SIZE ) ;

    if ( ret )
    {
        if ( packId >= ( maxPackId - 1 ) )
        {
            imglen = *( uint * )( ADDR_BIN_LENGTH + TEMP_MCU_IMAGE_ADDR ); 
            if ( scrc == CRC16((uchar *)TEMP_MCU_IMAGE_ADDR, imglen) )
            {
                pRet->result = CMM_UPDATE_REUSLT_OVER;
                gPubVal.mcuNeedReboot = SYS_UPDATE_AND_RESET;

                printf( "\r\npkt receive finish " );
            }
            else
            {
                printf ( "\r\nimage check fail. len=%x CRC: %04x/%04x ", imglen, scrc, CRC16((uchar *)TEMP_MCU_IMAGE_ADDR, imglen));
                pRet->result = CMM_UPDATE_REUSLT_CHKFAIL;
                
            }
            expectId = 0;
        }
        else
        {
            pRet->result = CMM_UPDATE_REUSLT_OK;
            expectId = packId + 1;
            pRet->nextPackId[0] = ( expectId >> 0 ) & 0xff;
            pRet->nextPackId[1] = ( expectId >> 8 ) & 0xff;
            pRet->nextPackId[2] = ( expectId >> 16 )& 0xff;
            //printf("  OK ");
        }

        return  ;
    }
    else
    {
        pRet->result = CMM_UPDATE_REUSLT_PROGRAM_FAIL;
        return ;
    }
    
}


int tcpUpdataHandle(char * buff , int packlen , int * backlen)
{
    t_cmmCommPack *pPack = (t_cmmCommPack*)buff;
    
    if (pPack->packHead != COMM_PACK_HEAD_FLAG)
    {
        return false;
    }

    if (pPack->packType == PACKTYPE_UP_MCU)
    {
        cmmHandleUpdateMcu(backlen ,pPack->data);
    }
    else  if (pPack->packType == PACKTYPE_GET_INFO)
    {
        memcpy(pPack->data, &gPubVal, sizeof(tPubVal));
        *backlen = sizeof(tPubVal) ;
    }
    else  if (pPack->packType == PACKTYPE_RESET_MCU)
    {
        memcpy(pPack->data, &gPubVal, sizeof(tPubVal));
        *backlen = sizeof(tPubVal) ;
        gPubVal.mcuNeedReboot = SYS_JUST_ONLY_RESET;
    }

    *backlen = *backlen + sizeof(t_cmmCommPack);

	return 0;
}

/*******************************************************************************
*函数名:
　　tcpSvr_recv()
*功能: 这是一个回调函数，当一个TCP段到达这个连接时会被调用

*输入:
*输出:
*说明:
*******************************************************************************/
static err_t updateSvr_recv( void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err )
{
    int backlen ;

    if ( ( err == ERR_OK ) && ( p != NULL ))
    {
        /* Inform TCP that we have taken the data. */
        tcp_recved ( pcb, p->tot_len );

        if ( debugFlag & CMM_DEBUG_UPDTE )
        {
            printf ( "\r\n p->len %d/%d " , p->len, p->tot_len );
            printf ( "msg:[%s]" , p->payload );
        }
        
        backlen = p->len;
        tcpUpdataHandle(p->payload , p->len , &backlen);
        
        if ( ERR_OK !=  tcp_write ( pcb, p->payload, backlen , 0 ) )   //发送数据
        {
            DBUDT("");
        }

        if ( tcp_output ( pcb ) != ERR_OK )
        {
            DBUDT("");
        }

        pbuf_free ( p );

    }

     if ( ( err == ERR_OK ) && ( p == NULL ))
     {
        if ( debugFlag & CMM_DEBUG_UPDTE )
            printf ( "\r\n sock error close socket in recv " );

        tcp_recv ( pcb, NULL );        /* 设置TCP段到时的回调函数 */
        tcp_close ( pcb );

     }
     
     return ERR_OK;
}


/*******************************************************************************
*函数名:
　　devTcpSvr_accept()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
static err_t updateSvr_accept( void *arg, struct tcp_pcb *pcb, err_t err )
{    
    if ( debugFlag & CMM_DEBUG_UPDTE )
        printf ( "\r\n tcp connect accepted.. " );

    tcp_setprio ( pcb, TCP_PRIO_MIN );      /* 设置回调函数优先级，当存在几个连接时特别重要，此函数必须调用*/
    tcp_recv ( pcb, updateSvr_recv );       /* 设置TCP段到时的回调函数 */

    return ERR_OK;
}

/*******************************************************************************
*函数名:
　　taskTcpServer()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
void upDateServerInit( void )
{
    struct tcp_pcb *pcb;
    err_t ret;

    printf ( "\r\nstart  updateServerInit " );
    
    OSTimeDly ( OS_TICKS_PER_SEC );

    pcb = tcp_new();                                        /* 建立通信的TCP控制块(pcb) */

    ret = tcp_bind ( pcb, IP_ADDR_ANY, UPDATE_SERVER_PORT);        /* 绑定本地IP地址和端口号 */
    if ( ret != ERR_OK )
    {
        DBUDT("");
        printf ( "\r\n tcp_bind error: ret=0x%x" , ret );
    }    
    printf( "\r\nupdate server bind port %d", UPDATE_SERVER_PORT);
    
    pcb = tcp_listen ( pcb );                                      /* 进入监听状态 */
    if ( pcb == NULL )
    {
        DBUDT("");
        printf ( "\r\n tcp_listen pcb:0x%x" , ( int ) pcb );
    }

    tcp_accept ( pcb, updateSvr_accept );                /* 设置有连接请求时的回调函数 */

}



void updatePeriodChk(void)
{
    if ( gPubVal.mcuNeedReboot == SYS_UPDATE_AND_RESET )
    {
        printf ( "\r\n\r\nprogram will update and reboot" );                
        copyMcuImage ( 0, TEMP_MCU_IMAGE_ADDR );
        
        printf ( "\nprogram update finish now!\n" );
        OSTimeDly (OS_TICKS_PER_SEC/2);
        SysCtlReset();

        gPubVal.mcuNeedReboot = 0;
    }
    if ( gPubVal.mcuNeedReboot == SYS_JUST_ONLY_RESET )
    {
        gPubVal.mcuNeedReboot = 0;
        SysCtlReset();
    }
    if (SysParam[SP_SW_UPDATE] == 1)
    {
        SoftWareUpdate();
    }
}




