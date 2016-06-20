/*************************************************************
成都昊普环保技术有限公司   版权所有

文件名:  SwUpdate.c
作  者:  潘国义
描  述:  软件升级模块
修订记录:   

**************************************************************/
#include "includes.h"
#include "Comm.h"
#include "devTcpip.h"
#include "DLT645.h"
#include "Swupdate.h"
#include <crc.h>
#include "Dev.h"
#include "update.h"
extern BYTE ServerLogin;
extern BYTE Uart0_Buffer[];
extern WORD Uart0_Sav_Num;

//extern BYTE SysParam[];
extern BYTE SysTime[];
extern BYTE SendFrame_Buffer[];

extern WORD DataLength;   
extern WORD StartLoc;   // 透传起始位置
extern BYTE NeedSaveParam;
extern WORD Crc;
extern BYTE TcpSendDataLen;
extern BYTE DevVersion[];
extern DWORD AppBuf[];
extern DWORD NeedReboot;

BYTE SwUpdating = 0;

BYTE NeedDownlod()
{
    //static WORD ver;
    
    //ver = (WORD)TCP_RecBuf[StartLoc+11];
    //ver <<= 8;
    //ver += TCP_RecBuf[StartLoc+12];

    if ((DevVersion[2] == TCP_RecBuf[StartLoc+11] + 0x30) && 
        (DevVersion[4] == TCP_RecBuf[StartLoc+12] + 0x30) )
    {   //运行软件的版本同传输的版本相同
        DebugStr("Same Version\r\n");
        return FALSE;
    }
    else
    {
        if ((TCP_RecBuf[StartLoc+11] == SysParam[SP_SW_VERSION]) &&
            (TCP_RecBuf[StartLoc+12] == SysParam[SP_SW_VERSION+1]))
        {   
            if (SysParam[SP_SW_UPDATE] == 1)
            {
                DebugStr("Already Downlod\r\n");
                return FALSE;
            }
            else
            {
                if ((SysParam[SP_SW_LENGTH]    != TCP_RecBuf[StartLoc+13]) ||
                    (SysParam[SP_SW_LENGTH+1]  != TCP_RecBuf[StartLoc+14]) ||
                    (SysParam[SP_SW_LENGTH+2]  != TCP_RecBuf[StartLoc+15]) ||
                    (SysParam[SP_SW_LENGTH+3]  != TCP_RecBuf[StartLoc+16])
                    )
                {   // 数据长度异常
                    SysParam[SP_SW_LENGTH]    = TCP_RecBuf[StartLoc+13];
                    SysParam[SP_SW_LENGTH+1]  = TCP_RecBuf[StartLoc+14];
                    SysParam[SP_SW_LENGTH+2]  = TCP_RecBuf[StartLoc+15];
                    SysParam[SP_SW_LENGTH+3]  = TCP_RecBuf[StartLoc+16];

                    SysParam[SP_SW_CURID]     = 0;
                    SysParam[SP_SW_CURID+1]   = 0;  
                    DebugStr("Length Err,Redownlod\r\n");
                    return TRUE;  //  重新下载
                }
                else
                {
                    DebugStr("Continue Download\r\n");
                    return TRUE;  // 断点续传输
                }
            }
        }
        else   // 全新下载
        {
            SysParam[SP_SW_VERSION]   = TCP_RecBuf[StartLoc+11];
            SysParam[SP_SW_VERSION+1] = TCP_RecBuf[StartLoc+12];
            
            SysParam[SP_SW_LENGTH]    = TCP_RecBuf[StartLoc+13];
            SysParam[SP_SW_LENGTH+1]  = TCP_RecBuf[StartLoc+14];
            SysParam[SP_SW_LENGTH+2]  = TCP_RecBuf[StartLoc+15];
            SysParam[SP_SW_LENGTH+3]  = TCP_RecBuf[StartLoc+16];

            SysParam[SP_SW_CURID]     = 0;
            SysParam[SP_SW_CURID+1]   = 0;

            DebugStr("Download New\r\n");
            //printf("SysParam[SP_SW_LENGTH]=%02x ",TCP_RecBuf[StartLoc+13]);
            //printf("SysParam[SP_SW_LENGTH]=%02x ",TCP_RecBuf[StartLoc+14]);
            //printf("SysParam[SP_SW_LENGTH]=%02x ",TCP_RecBuf[StartLoc+15]);
            //printf("SysParam[SP_SW_LENGTH]=%02x \r\n",TCP_RecBuf[StartLoc+16]);

            //printf("SP_SW_LENGTH =%d \r\n",*(DWORD*)&TCP_RecBuf[StartLoc+13]);
            //printf("SP_sys =%d \r\n",*(DWORD*)&SysParam[SP_SW_LENGTH]);
            return TRUE;
        }
    }
}


void SwUpdateRequest(void)
{
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x02;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x95;
    TCP_SendBuf[10] = 0x10;
  
    if (DataLength == 6)
    {
        if (NeedDownlod())
        {
            DebugStr("SwUpdateStart\r\n");
            NeedSaveParam = 1;
            SwUpdating = 1;
            TCP_SendBuf[11] = 1;  // 0：不需升级  1 : 需要升级
            TCP_SendBuf[12] = SysParam[SP_SW_CURID];  // 起始包
            TCP_SendBuf[13] = SysParam[SP_SW_CURID+1];  
        }
        else
        {
            TCP_SendBuf[11] = 0;  // 0：不需升级  1 : 需要升级
            TCP_SendBuf[12] = 0;  // 起始包
            TCP_SendBuf[13] = 0;  // 
        }
    }
    else
    {
        TCP_SendBuf[11] = 0xFF;  // 失败
        TCP_SendBuf[12] = 0x00;  // 成功
        TCP_SendBuf[13] = 0x00;  // 成功
        //ServerLogin = 0;
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,14);
    TCP_SendBuf[14] = (BYTE)Crc;
    TCP_SendBuf[15] = (BYTE)(Crc>>8);

    TCP_SendBuf[16] = 0x16;  // 结束

    TcpSendDataLen = 17;
    Send_Tcp_Data();
}

void SwUpdateData(void)
{
    static BYTE chk;
    static WORD Id,i;
    static DWORD Addr;
    
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x95;
    TCP_SendBuf[10] = 0x11;
    
    if (DataLength == 0x403)
    {
        DebugStr("SwUpdateData:");
        // 地位在前
        Id = (WORD)TCP_RecBuf[StartLoc+12];   
        Id <<= 8;
        Id += TCP_RecBuf[StartLoc+11];
        DebugWord(Id);

        chk = 0;
        for (i=0;i<1024;i++)
        {
            chk += TCP_RecBuf[StartLoc+13 + i];
        }

        if (chk == TCP_RecBuf[StartLoc+13+1024])
        {
            Addr = Id * FLASH_BANK_SIZE;
            if (Addr < APPMAXLENGTH - 1024)
            {
                mcpy((BYTE *)AppBuf,(BYTE *)&TCP_RecBuf[StartLoc+13],FLASH_BANK_SIZE);
                
                //if (ramProgramFlash(BAKAPPSTART + Addr, AppBuf, FLASH_BANK_SIZE))
                if (ramProgramFlash(BAKAPPSTART + Addr, AppBuf, FLASH_BANK_SIZE))
                {
                    *(WORD *)&SysParam[SP_SW_CURID] = Id;
                    NeedSaveParam = 1;
                    TCP_SendBuf[11] = 0x00;  // 成功
                }
                else
                {
                    TCP_SendBuf[11] = 0x01;  
                }
            }
            else
            {
                TCP_SendBuf[11] = 0x01;  
            }
        }
        else
        {
            TCP_SendBuf[11] = 0x01;  
        }
    }
    else
    {
        TCP_SendBuf[11] = 0x01;  // 失败
        
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // 结束

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}

void SwUpdateFinish(void)
{
    static WORD crc,c1;
    static DWORD AppLen;
    
    if (ServerLogin == 0)
    {
        return;
    }

    Clear_Tcp_SendBuf();

    TCP_SendBuf[0] = 0x68;
    TCP_SendBuf[1] = 0x01;
    TCP_SendBuf[2] = 0x00;
    TCP_SendBuf[3] = 0x68;
    TCP_SendBuf[4] = SysParam[SP_DEVADDR];
    TCP_SendBuf[5] = SysParam[SP_DEVADDR+1];
    TCP_SendBuf[6] = SysParam[SP_DEVADDR+2];
    TCP_SendBuf[7] = SysParam[SP_DEVADDR+3];
    TCP_SendBuf[8] = SysParam[SP_DEVADDR+4];

    TCP_SendBuf[9]  = 0x95;
    TCP_SendBuf[10] = 0x12;
    
    if (DataLength == 2)
    {
        DebugStr("SwUpdateEnd\r\n");

        SwUpdating = 0;
        
        crc = (WORD)TCP_RecBuf[StartLoc+11];
        crc <<= 8;
        crc += (WORD)TCP_RecBuf[StartLoc+12];

        AppLen = *(DWORD *)&SysParam[SP_SW_LENGTH];
        c1 = CRC16((BYTE *)BAKAPPSTART, AppLen); 
        
        if (crc == c1)
        {
            SysParam[SP_SW_UPDATE] = 1;
            NeedSaveParam = 1;
            NeedReboot = 1;
            TCP_SendBuf[11] = 0x00;  // 成功
        }
        else
        {
            DebugWord(crc);
            DebugWord(c1);
            DebugStr("Crc Err!\r\n");
            ClearSwDownloadInfo();
            
            TCP_SendBuf[11] = 0x01;  // 失败
        }
    }
    else
    {
        DebugStr("Length err!\r\n");
        TCP_SendBuf[11] = 0x01;  // 失败
    }

    //CRC
    Crc = CRC16(TCP_SendBuf,12);
    TCP_SendBuf[12] = (BYTE)Crc;
    TCP_SendBuf[13] = (BYTE)(Crc>>8);

    TCP_SendBuf[14] = 0x16;  // 结束

    TcpSendDataLen = 15;
    Send_Tcp_Data();
}


BYTE SwNeedUpdate()
{
    static DWORD AppLen = 0;
    //static WORD crc;
    
    if (SysParam[SP_SW_UPDATE] == 0)
    {
        DebugStr("No DownLoad app.\r\n");
        return FALSE;
    }

    if (*(DWORD *)BAKAPPSTART == 0xFFFFFFFF)
    {
        DebugStr("No new app.\r\n");
        ClearSwDownloadInfo();
        return FALSE;
    }

    if ((DevVersion[2] == *(BYTE *)(BAKAPPSTART+ADDR_BIN_VER+2) + 0x30) && 
        (DevVersion[4] == *(BYTE *)(BAKAPPSTART+ADDR_BIN_VER) + 0x30 ))
    {
        DebugStr("Same Version.\r\n");
        ClearSwDownloadInfo();
        return FALSE;
    }

    AppLen = *(DWORD *)&SysParam[SP_SW_LENGTH];
    if ((AppLen == 0) || (AppLen > 127*1024))
    {
        DebugStr("Param Error.\r\n");
        ClearSwDownloadInfo();
        return FALSE;
    }

    RamDecodeApp(0);
    if (AppLen != AppBuf[ADDR_BIN_LENGTH/4])
    {
        ClearSwDownloadInfo();
        DebugStr("File Lenght Error.\r\n");
        DebugWord((WORD)(AppLen>>16));
        DebugWord((WORD)AppLen);
        DebugWord((WORD)(AppBuf[ADDR_BIN_LENGTH/4]>>16));
        DebugWord((WORD)AppBuf[ADDR_BIN_LENGTH/4]);
        return FALSE;
    }

   
    return TRUE;
}

void ClearSwDownloadInfo()
{
    SysParam[SP_SW_UPDATE]    = 0;
    SysParam[SP_SW_VERSION]   = 0;
    SysParam[SP_SW_VERSION+1] = 0;
    *(DWORD *)&SysParam[SP_SW_LENGTH] = 0;
    *(WORD *)&SysParam[SP_SW_CURID] = 0;
    NeedSaveParam = 1;
}

void SoftWareUpdate()
{
    DebugStr("Updating Software\r\n");
    if (SwNeedUpdate())
    {   
    
        printf("Updating Software, Please wait ... \r\n");
        
        //更新软件  -- 运行Ram程序
        RamUpdateApp();
    }
    
}

