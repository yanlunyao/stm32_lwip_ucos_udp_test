/*************************************************************
成都昊普环保技术有限公司   版权所有

文件名:  DLT645.c
作  者:  张泽坤
描  述:  645协议处理
修订记录:   

**************************************************************/
#include "includes.h"
#include "driverlib/uart.h"
#include <tcp.h>
#include "DLT645.h"
#include "includes.h"
#include "comm.h"
#include "Dev.h"
#include "devTimer.h"
extern struct tcp_pcb * gPcbLastConnectFromClient ;

#define  DI_EE     0x01
#define  DI_Au     0x02
#define  DI_Bu     0x03
#define  DI_Cu     0x04
#define  DI_Ai     0x05
#define  DI_Bi     0x06
#define  DI_Ci     0x07
#define  DI_PQ     0x08
#define  DI_AE     0x09
#define  DI_BE     0x0A
#define  DI_CE     0x0B
#define  DI_PWR    0x0C
#define  DI_PWRA   0x0D
#define  DI_PWRB   0x0E
#define  DI_PWRC   0x0F
#define  DI_TIME   0x10

#define  DI_COUNT  16 //上面DI_XX的个数



static BYTE MeterID=0;//表示第几个表

                        // DI0   DI1  DI2   DI3
static BYTE DIArr[][4] ={ 0x00,0x00,0x01,0x00,      //正向有功总电能 xxxxxx.xx KWh
                          0x00,0x01,0x01,0x02,      //Au  XXX.X (V)
                          0x00,0x02,0x01,0x02,      //Bu
                          0x00,0x03,0x01,0x02,      //Cu
                          //0x00,0xFF,0x01,0x02,      //电压数据块
                          0x00,0x01,0x02,0x02,      //Ai  XXX.XXX  (A)
                          0x00,0x02,0x02,0x02,      //Bi
                          0x00,0x03,0x02,0x02,      //Ci 
                          //0x00,0xFF,0x02,0x02,      //电流数据块
                          0x00,0x00,0x06,0x02,      //总功率因数 X.XXX
                          0x00,0x00,0x15,0x00,      //当前A相有功电能  xxxxxx.xx KWh
                          0x00,0x00,0x29,0x00,      //B
                          0x00,0x00,0x3D,0x00,      //C
                          0x00,0x00,0x03,0x02,      //total power  XX.XXXX kW
                          0x00,0x01,0x03,0x02,      //A power
                          0x00,0x02,0x03,0x02,      //B
                          0x00,0x03,0x03,0x02,      //C
                          0xff,0x01,0x00,0x04};     // Time


EMeterData_t EMeterDat[6];

BYTE Send645Buf[SEND_645_MAX];
BYTE Recv645Buf[RECV_645_MAX];
WORD Recv_645Save_num = 0;
BYTE Send_645Buf_num = 0;

static BYTE Begin=0;
static BYTE End = 0;
#if 0
void MeterParamInit()//测试用
{
   
    EMeterDat[0].EE=45005;//450.05KWh
    EMeterDat[0].AE=10005;//450.05KWh
    EMeterDat[0].BE=15005;//450.05KWh
    EMeterDat[0].CE=20005;//450.05KWh
    EMeterDat[0].Au=2302;//230.2V
    EMeterDat[0].Bu=2312;//231.2V
    EMeterDat[0].Cu=2322;//232.2V
    EMeterDat[0].Ai=10500;//10.5A
    EMeterDat[0].Bi=35004;//35.004A
    EMeterDat[0].Ci=23220;//23.22A

    EMeterDat[1].EE=55005;//550.05KWh
    EMeterDat[1].AE=1005;//450.05KWh
    EMeterDat[1].BE=1505;//450.05KWh
    EMeterDat[1].CE=2005;//450.05KWh
    EMeterDat[1].Au=3302;//230.2V
    EMeterDat[1].Bu=3312;//231.2V
    EMeterDat[1].Cu=3322;//232.2V
    EMeterDat[1].Ai=20500;//20.5A
    EMeterDat[1].Bi=45004;//45.004A
    EMeterDat[1].Ci=33220;//33.22A

    EMeterDat[2].EE=45005;//450.05KWh
    EMeterDat[2].Au=2302;//230.2V
    EMeterDat[2].Bu=2312;//231.2V
    EMeterDat[2].Cu=2322;//232.2V
    EMeterDat[2].Ai=10500;//10.5A
    EMeterDat[2].Bi=35004;//35.004A
    EMeterDat[2].Ci=23220;//23.22A

    EMeterDat[4].EE=45005;//450.05KWh
    EMeterDat[4].Au=2302;//230.2V
    EMeterDat[4].Bu=2312;//231.2V
    EMeterDat[4].Cu=2322;//232.2V
    EMeterDat[4].Ai=10500;//10.5A
    EMeterDat[4].Bi=35004;//35.004A
    EMeterDat[4].Ci=23220;//23.22A

    EMeterDat[5].EE=45005;//450.05KWh
    EMeterDat[5].Au=2302;//230.2V
    EMeterDat[5].Bu=2312;//231.2V
    EMeterDat[5].Cu=2322;//232.2V
    EMeterDat[5].Ai=10500;//10.5A
    EMeterDat[5].Bi=35004;//35.004A
    EMeterDat[5].Ci=23220;//23.22A
}

static void SendDLT645Str(BYTE *PBuf, BYTE len )
{
    sendUart1Term(PBuf,len); //用串口
    //send485Term(PBuf,len); //用485口
}
#endif
void ClearDLT645SendBuf()
{
    BYTE i;
    for(i=0;i<SEND_645_MAX;i++)
    {
        Send645Buf[i]=0;
    }
    Send_645Buf_num =0;
}

void ClearDLT645RecvBuf()
{
    BYTE i;
    for(i=0;i<Recv_645Save_num;i++)
    {
        Recv645Buf[i]=0;
    }

    Recv_645Save_num = 0;
}


static BYTE IntToHex(BYTE hhex,BYTE lhex)
{
    BYTE ret=0;
    
    ret =(hhex<<4)|(lhex&0x0F);
    return ret;
}

static BYTE HexToInt(BYTE dat)
{
    static BYTE ret,ch,cl;
    ret = 0;
    ch = 0;
    cl = 0;
    
    ch = dat>>4;
    cl = dat&0x0F;

    ret =ch*10+cl;

    return ret;
    
}
static BYTE CSCheck(uint8 *pBuf,uint8 len)
{
    uint8 i=0;
    static WORD CS;
    CS = 0;
    if(len==0)
    {
        return FALSE;
    }
    for(i=0;i<len;i++)
    {
        CS += (*pBuf++)%256;
       
    }
    return ((BYTE)CS);
}

static void HexToAdd33(BYTE *pBuf, BYTE len)
{
    uint8 i;
    if(len==0)
    {
        return ;
    }
    for(i=0;i<len;i++)
    {
        pBuf[i] += 0x33;
    }   
}


static void HexToDec33(BYTE *pBuf, BYTE len)
{
    uint8 i;
    if(len==0)
    {
        return ;
    }
    for(i=0;i<len;i++)
    {
        pBuf[i] -= 0x33;
    }

} 

//将一个整形数据转换成16进制的数组,注意:len 位奇数时加1-->len=len+1;
static BYTE IntChangeByteArr(int dat,BYTE len ,BYTE *pBuf)
{
    static BYTE buf[8];//数据最多为8位十进制数
    BYTE i =0;
    BYTE j =0;
    
    for(i=0;i<len;i++)
    {   
        buf[i] =(BYTE)(dat%10);
        dat = dat/10;
    }
    for(i=0;i<len;i=i+2)
    {
        pBuf[j] = IntToHex(buf[len-1-i],buf[len-2-i]);   
        j++;
    } 
    return (j);
}

static void TxDataHandle645(unsigned int dat, BYTE datlen, BYTE *pBuf)
{
    BYTE length;
    if(datlen%2)
    {
        length = datlen+1;
    }
    else
    {
        length =datlen;
    }
    IntChangeByteArr(dat,length,pBuf);
    HexToAdd33(pBuf,length/2);
   
}

void RxDataHandle645(BYTE *pBuf, BYTE len)
{
    HexToDec33( pBuf, len ); 
    
}
static BYTE MeterAddrCheck(BYTE *RecvAddr, BYTE *LocalAddr)
{
    BYTE i=0;
    BYTE count=0;
    for(i=0;i<6;i++)//判断高位连续有几个地址为0xAA ,通配符 
    {
        if(RecvAddr[i] == 0xAA)
        {
            count++;
        }
        else
        {
            count=0;    
        }
    }
    for(i=0;i<(6-count);i++)
    {
  
        if(RecvAddr[i] != LocalAddr[i])
        {
            return FALSE;
        }
    }
    return TRUE;
}

//判断帧格式是否正确
BYTE RxValidDataFrame(BYTE *pBuf, BYTE len)
{
    BYTE i,j;
    
    i=0;
    j=0;
    for(i=0;i<len;i++)
    {
        if(pBuf[i] == 0x16)
        {
            if(i<11)
            {
                continue;
            }
            End=i;
            if(i != len-1)
            {
                if(pBuf[i+1] == 0x16)//跳过校验和为 0x16
                {
                    End=i+1;    
                }
            }
            j=0;
            while(pBuf[j] != 0x68)
            {
                j++;
                if(j>=End)
                {
                    return FALSE;
                }
            }
            Begin = j;
            j=0; 
            if( pBuf[Begin+7]!=0x68)
            {
                return FALSE;    
            }

            if(pBuf[Begin+9]!=End-2-(Begin+9))//length
            {
                return FALSE;  
            }
            if(pBuf[End-1] !=CSCheck(&pBuf[Begin],End-Begin-1))
            {
                return FALSE;    
            }
    
            if((mcmp2(&pBuf[Begin+1],0x99, 6))&&((pBuf[Begin+8]==0x08)||(pBuf[Begin+8]==0x16)))
            {
                return TRUE;
            }
            if((pBuf[Begin+8] != 0x13)&&(pBuf[Begin+8] != 0x15))//读通信地址,写通信地址
            {
                for(i=0;i<3;i++)
                {
                   //地址比较
                    if(MeterAddrCheck(&pBuf[Begin+1], (uchar *)&SysParam[SP_METERADDR1+6*i]))
                    {
                        //MeterID =0;
                        MeterID =i;//哪个电表有效
                        
                        break;
                    }
                    
                    if(i==2)
                    {
                        return false;
                    }
            
                }
                       
            }
            else
            {               
                 return (mcmp2(&pBuf[Begin+1],0xAA, 6)||mcmp2(&pBuf[Begin+1],0x99, 6));                
            }
    
            return TRUE;    
        }
        
    }
    return FALSE;
    
}


static void GetEMeterDataEE(unsigned int EEdat, BYTE CtlCmd,BYTE *recvbuff)
{
    BYTE DataBuf[4];
    TxDataHandle645(EEdat,8,DataBuf);//将数据EMeterData转换成压缩BCD码
    
    Send645Buf[0]=0x68;
    Send645Buf[1]=SysParam[SP_METERADDR1+MeterID*6+0];
    Send645Buf[2]=SysParam[SP_METERADDR1+MeterID*6+1];
    Send645Buf[3]=SysParam[SP_METERADDR1+MeterID*6+2];
    Send645Buf[4]=SysParam[SP_METERADDR1+MeterID*6+3];
    Send645Buf[5]=SysParam[SP_METERADDR1+MeterID*6+4];
    Send645Buf[6]=SysParam[SP_METERADDR1+MeterID*6+5];
    Send645Buf[7]=0x68;   
    
    Send645Buf[8]=CtlCmd;

    Send645Buf[9]=4+4;
   
    Send645Buf[10]=recvbuff[Begin+10]+0x33;
    Send645Buf[11]=recvbuff[Begin+11]+0x33;
    Send645Buf[12]=recvbuff[Begin+12]+0x33;
    Send645Buf[13]=recvbuff[Begin+13]+0x33;

    Send645Buf[14]=DataBuf[3];
    Send645Buf[15]=DataBuf[2];
    Send645Buf[16]=DataBuf[1];
    Send645Buf[17]=DataBuf[0];

    Send645Buf[18]=CSCheck(Send645Buf, 18); 
    Send645Buf[19]=0x16;
    Send_645Buf_num =20; 
    //SendDLT645Str(Send645Buf,Send_645Buf_num);
    //tcp_write(gPcbLastConnectFromClient, Send645Buf,Send_645Buf_num,1);
    //tcp_output(gPcbLastConnectFromClient);
    //ClearDLT645SendBuf();
}

static void GetEMeterDataVolt(unsigned int Udat, BYTE CtlCmd,BYTE *recvbuff)
{
    BYTE DataBuf[2];
    TxDataHandle645(Udat,4,DataBuf);//将数据EMeterData转换成压缩BCD码
    
    Send645Buf[0]=0x68;
    Send645Buf[1]=SysParam[SP_METERADDR1+MeterID*6+0];
    Send645Buf[2]=SysParam[SP_METERADDR1+MeterID*6+1];
    Send645Buf[3]=SysParam[SP_METERADDR1+MeterID*6+2];
    Send645Buf[4]=SysParam[SP_METERADDR1+MeterID*6+3];
    Send645Buf[5]=SysParam[SP_METERADDR1+MeterID*6+4];
    Send645Buf[6]=SysParam[SP_METERADDR1+MeterID*6+5];
    Send645Buf[7]=0x68;   
    
    Send645Buf[8]=CtlCmd;

    Send645Buf[9]=4+2;
   
    Send645Buf[10]=recvbuff[Begin+10]+0x33;
    Send645Buf[11]=recvbuff[Begin+11]+0x33;
    Send645Buf[12]=recvbuff[Begin+12]+0x33;
    Send645Buf[13]=recvbuff[Begin+13]+0x33;

    Send645Buf[14]=DataBuf[1];
    Send645Buf[15]=DataBuf[0];
    
    Send645Buf[16]=(BYTE)CSCheck(Send645Buf, 16); 
    Send645Buf[17]=0x16;
    Send_645Buf_num = 18;
    //SendDLT645Str(Send645Buf,Send_645Buf_num);
    //tcp_write(gPcbLastConnectFromClient, Send645Buf,Send_645Buf_num, 1);
    //tcp_output(gPcbLastConnectFromClient);
    //ClearDLT645SendBuf();
}


static void GetEMeterDataCurrent(unsigned int Adat, BYTE CtlCmd,BYTE *recvbuff)
{
    BYTE DataBuf[3];
    TxDataHandle645(Adat,6,DataBuf);//将数据EMeterData转换成压缩BCD码
    
    
    Send645Buf[0]=0x68;
    Send645Buf[1]=SysParam[SP_METERADDR1+MeterID*6+0];
    Send645Buf[2]=SysParam[SP_METERADDR1+MeterID*6+1];
    Send645Buf[3]=SysParam[SP_METERADDR1+MeterID*6+2];
    Send645Buf[4]=SysParam[SP_METERADDR1+MeterID*6+3];
    Send645Buf[5]=SysParam[SP_METERADDR1+MeterID*6+4];
    Send645Buf[6]=SysParam[SP_METERADDR1+MeterID*6+5];
    Send645Buf[7]=0x68;   
    
    Send645Buf[8]=CtlCmd;

    Send645Buf[9]=4+3;
   
    Send645Buf[10]=recvbuff[Begin+10]+0x33;
    Send645Buf[11]=recvbuff[Begin+11]+0x33;
    Send645Buf[12]=recvbuff[Begin+12]+0x33;
    Send645Buf[13]=recvbuff[Begin+13]+0x33;

    Send645Buf[14]=DataBuf[2];
    Send645Buf[15]=DataBuf[1];
    Send645Buf[16]=DataBuf[0];
    
    Send645Buf[17]=(BYTE)CSCheck(Send645Buf, 17); 
    Send645Buf[18]=0x16;
    Send_645Buf_num = 19;
    //SendDLT645Str(Send645Buf,Send_645Buf_num);
    //tcp_write(gPcbLastConnectFromClient, Send645Buf,Send_645Buf_num, 1);
    //tcp_output(gPcbLastConnectFromClient);
    //ClearDLT645SendBuf();
}

static void GetEMeterDataPQ(unsigned int Adat, BYTE CtlCmd,BYTE *recvbuff)
{
    BYTE DataBuf[2];
    TxDataHandle645(Adat,4,DataBuf);//将数据EMeterData转换成压缩BCD码
    
    
    Send645Buf[0]=0x68;
    Send645Buf[1]=SysParam[SP_METERADDR1+MeterID*6+0];
    Send645Buf[2]=SysParam[SP_METERADDR1+MeterID*6+1];
    Send645Buf[3]=SysParam[SP_METERADDR1+MeterID*6+2];
    Send645Buf[4]=SysParam[SP_METERADDR1+MeterID*6+3];
    Send645Buf[5]=SysParam[SP_METERADDR1+MeterID*6+4];
    Send645Buf[6]=SysParam[SP_METERADDR1+MeterID*6+5];
    Send645Buf[7]=0x68;   
    
    Send645Buf[8]=CtlCmd;

    Send645Buf[9]=4+2;
   
    Send645Buf[10]=recvbuff[Begin+10]+0x33;
    Send645Buf[11]=recvbuff[Begin+11]+0x33;
    Send645Buf[12]=recvbuff[Begin+12]+0x33;
    Send645Buf[13]=recvbuff[Begin+13]+0x33;

    Send645Buf[14]=DataBuf[1];
    Send645Buf[15]=DataBuf[0];
    
    Send645Buf[16]=(BYTE)CSCheck(Send645Buf, 16); 
    Send645Buf[17]=0x16;
    Send_645Buf_num = 18;
    //SendDLT645Str(Send645Buf,Send_645Buf_num);
    //tcp_write(gPcbLastConnectFromClient, Send645Buf,Send_645Buf_num, 1);
    //tcp_output(gPcbLastConnectFromClient);
    //ClearDLT645SendBuf();
}

static void GetEMeterTime(BYTE CtlCmd,BYTE *recvbuff)
{
    //BYTE DataBuf[5];

    ReadRealtime();
    //TxDataHandle645((unsigned int *)SysTime,8,DataBuf);//
    //TxDataHandle645((unsigned int *)SysTime[4],2,&DataBuf[5]);
    
    Send645Buf[0]=0x68;
    Send645Buf[1]=SysParam[SP_METERADDR1+MeterID*6+0];
    Send645Buf[2]=SysParam[SP_METERADDR1+MeterID*6+1];
    Send645Buf[3]=SysParam[SP_METERADDR1+MeterID*6+2];
    Send645Buf[4]=SysParam[SP_METERADDR1+MeterID*6+3];
    Send645Buf[5]=SysParam[SP_METERADDR1+MeterID*6+4];
    Send645Buf[6]=SysParam[SP_METERADDR1+MeterID*6+5];
    Send645Buf[7]=0x68;  
    Send645Buf[8]=CtlCmd;

    Send645Buf[9]=4+7;
   
    Send645Buf[10]=recvbuff[Begin+10]+0x33;
    Send645Buf[11]=recvbuff[Begin+11]+0x33;
    Send645Buf[12]=recvbuff[Begin+12]+0x33;
    Send645Buf[13]=recvbuff[Begin+13]+0x33;
    Send645Buf[14]=SysTime[1]+0x33; //星期
    Send645Buf[15]=SysTime[3]+0x33; //日
    Send645Buf[16]=SysTime[2]+0x33; //月
    Send645Buf[17]=SysTime[0]+0x33; //年
    Send645Buf[18]=SysTime[6]+0x33; //秒
    Send645Buf[19]=SysTime[5]+0x33; //分
    Send645Buf[20]=SysTime[4]+0x33; //时
    
    Send645Buf[21]=CSCheck(Send645Buf, 21); 
    Send645Buf[22]=0x16;
    Send_645Buf_num =23; 
    //tcp_write(gPcbLastConnectFromClient, Send645Buf,Send_645Buf_num, 1);
    //tcp_output(gPcbLastConnectFromClient);
    //ClearDLT645SendBuf();
}

static void ErrResponseHandle(BYTE CtlCom, BYTE Err )
{
    Send645Buf[0]=0x68;
    Send645Buf[1]=SysParam[SP_METERADDR1+MeterID*6+0];
    Send645Buf[2]=SysParam[SP_METERADDR1+MeterID*6+1];
    Send645Buf[3]=SysParam[SP_METERADDR1+MeterID*6+2];
    Send645Buf[4]=SysParam[SP_METERADDR1+MeterID*6+3];
    Send645Buf[5]=SysParam[SP_METERADDR1+MeterID*6+4];
    Send645Buf[6]=SysParam[SP_METERADDR1+MeterID*6+5];
    Send645Buf[7]=0x68;  
    
    Send645Buf[8]=CtlCom;

    Send645Buf[9]=1;  
    Send645Buf[10]=Err+0x33;
    
    Send645Buf[11]=CSCheck(Send645Buf, 11); 
    Send645Buf[12]=0x16;
    Send_645Buf_num = 13;
    //SendDLT645Str(Send645Buf,Send_645Buf_num);

    //tcp_write(gPcbLastConnectFromClient, Send645Buf,Send_645Buf_num,1);
    //tcp_output(gPcbLastConnectFromClient);
    //ClearDLT645SendBuf();
}
#if 0
void HandleCmdReadData()
{
   
    BYTE ch =0;
    BYTE i =0;
    BYTE id=3;

    
    if(id==3)
    {
        for(i=0;i<DI_COUNT;i++)
        {
            if(mcmp(&Recv645Buf[Begin+10],&DIArr[i][0],4))
            {
                ch = i+1;
                break;
            }
        }
    }
    printf("ch=%d ",ch);
    switch(ch)
    {
        case DI_EE:
             GetEMeterDataEE(EMeterDat[MeterID].EE,0x91);
        break;

        case DI_Au:
             GetEMeterDataVolt(EMeterDat[MeterID].Au, 0x91);
        break;

        case DI_Bu:
             GetEMeterDataVolt(EMeterDat[MeterID].Bu, 0x91);
        break;

        case DI_Cu:
             GetEMeterDataVolt(EMeterDat[MeterID].Cu, 0x91);
        break;

        case DI_Ai:
             GetEMeterDataCurrent(EMeterDat[MeterID].Ai, 0x91);
        break;

        case DI_Bi:
             GetEMeterDataCurrent(EMeterDat[MeterID].Bi, 0x91);
        break;

        case DI_Ci:
             GetEMeterDataCurrent(EMeterDat[MeterID].Ci, 0x91);
        break;

        case DI_PQ:
             GetEMeterDataPQ(EMeterDat[MeterID].PQ, 0x91);
        break;

        case DI_AE:
             GetEMeterDataEE(EMeterDat[MeterID].AE,0x91);
        break;

        case DI_BE:
             GetEMeterDataEE(EMeterDat[MeterID].BE,0x91);
        break;

        case DI_CE:
             GetEMeterDataEE(EMeterDat[MeterID].CE,0x91);
        break;

        case DI_PWR:
             GetEMeterDataCurrent(EMeterDat[MeterID].PWR,0x91);//因为功率和电流的数据都是6位
        break;

        case DI_PWRA:
             GetEMeterDataCurrent(EMeterDat[MeterID].PWRA,0x91);
        break;

        case DI_PWRB:
             GetEMeterDataCurrent(EMeterDat[MeterID].PWRB,0x91);
        break;

        case DI_PWRC:
             GetEMeterDataEE(EMeterDat[MeterID].PWRC,0x91);
        break;
        
        case DI_TIME:
             GetEMeterTime(0x91);
        break;
        
        default:
            Recv645Buf[Begin+8]|=0xC0;
            ErrResponseHandle(Recv645Buf[Begin+8],UNREQSDAT_ERR);//无请求数据
            //ClearDLT645RecvBuf();
          
        break;
        
    }
       
}
#endif
void HandleCmdReadData1(BYTE *pchar)
{
   
    BYTE ch =0;
    BYTE i =0;
    BYTE id=3;

    
    if(id==3)
    {
        for(i=0;i<DI_COUNT;i++)
        {
            if(mcmp(&pchar[Begin+10],&DIArr[i][0],4))
            {
                ch = i+1;
                break;
            }
        }
    }
    printf("ch=%d \r\n",ch);
    switch(ch)
    {
        case DI_EE:
             GetEMeterDataEE(EMeterDat[MeterID].EE,0x91,pchar);
        break;

        case DI_Au:
             GetEMeterDataVolt(EMeterDat[MeterID].Au, 0x91,pchar);
        break;

        case DI_Bu:
             GetEMeterDataVolt(EMeterDat[MeterID].Bu, 0x91,pchar);
        break;

        case DI_Cu:
             GetEMeterDataVolt(EMeterDat[MeterID].Cu, 0x91,pchar);
        break;

        case DI_Ai:
             GetEMeterDataCurrent(EMeterDat[MeterID].Ai, 0x91,pchar);
        break;

        case DI_Bi:
             GetEMeterDataCurrent(EMeterDat[MeterID].Bi, 0x91,pchar);
        break;

        case DI_Ci:
             GetEMeterDataCurrent(EMeterDat[MeterID].Ci, 0x91,pchar);
        break;

        case DI_PQ:
             GetEMeterDataPQ(EMeterDat[MeterID].PQ, 0x91,pchar);
        break;

        case DI_AE:
             GetEMeterDataEE(EMeterDat[MeterID].AE,0x91,pchar);
        break;

        case DI_BE:
             GetEMeterDataEE(EMeterDat[MeterID].BE,0x91,pchar);
        break;

        case DI_CE:
             GetEMeterDataEE(EMeterDat[MeterID].CE,0x91,pchar);
        break;

        case DI_PWR:
             GetEMeterDataCurrent(EMeterDat[MeterID].PWR,0x91,pchar);//因为功率和电流的数据都是6位
        break;

        case DI_PWRA:
             GetEMeterDataCurrent(EMeterDat[MeterID].PWRA,0x91,pchar);
        break;

        case DI_PWRB:
             GetEMeterDataCurrent(EMeterDat[MeterID].PWRB,0x91,pchar);
        break;

        case DI_PWRC:
             GetEMeterDataEE(EMeterDat[MeterID].PWRC,0x91,pchar);
        break;
        
        case DI_TIME:
             GetEMeterTime(0x91,pchar);
        break;
        
        default:
            Recv645Buf[Begin+8]|=0xC0;
            ErrResponseHandle(Recv645Buf[Begin+8],UNREQSDAT_ERR);//无请求数据
            //ClearDLT645RecvBuf();
          
        break;
        
    }
       
}

static void HandleCmdReadAddr()
{
    
    Send645Buf[0]=0x68;
    Send645Buf[1]=SysParam[SP_METERADDR1+MeterID*6+0];
    Send645Buf[2]=SysParam[SP_METERADDR1+MeterID*6+1];
    Send645Buf[3]=SysParam[SP_METERADDR1+MeterID*6+2];
    Send645Buf[4]=SysParam[SP_METERADDR1+MeterID*6+3];
    Send645Buf[5]=SysParam[SP_METERADDR1+MeterID*6+4];
    Send645Buf[6]=SysParam[SP_METERADDR1+MeterID*6+5];
    Send645Buf[7]=0x68;
    Send645Buf[8]=0x93;

    Send645Buf[9]=6;  
    Send645Buf[1]=SysParam[SP_METERADDR1+MeterID*6+0]+0x33;
    Send645Buf[2]=SysParam[SP_METERADDR1+MeterID*6+1]+0x33;
    Send645Buf[3]=SysParam[SP_METERADDR1+MeterID*6+2]+0x33;
    Send645Buf[4]=SysParam[SP_METERADDR1+MeterID*6+3]+0x33;
    Send645Buf[5]=SysParam[SP_METERADDR1+MeterID*6+4]+0x33;
    Send645Buf[6]=SysParam[SP_METERADDR1+MeterID*6+5]+0x33;
    
    Send645Buf[16]=CSCheck(Send645Buf, 16); 
    Send645Buf[17]=0x16;
   
    Send_645Buf_num = 18;
    //SendDLT645Str(Send645Buf,Send_645Buf_num);
    //tcp_write(gPcbLastConnectFromClient, Send645Buf,Send_645Buf_num, 1);
    //tcp_output(gPcbLastConnectFromClient);
    //ClearDLT645SendBuf();//clear buffer
}

#if 0
static void HandleCmdRadioTime()
{

    BYTE i;
    for(i=0;i<5;i++)
    {
        SysTime[6-i] =Recv645Buf[Begin+10+i];//就是16进制数 不用再转换
    }
    SysTime[0] =Recv645Buf[Begin+15];
    WriteRealtime();   
}
#endif
static void HandleCmdRadioTime(BYTE *pchar)
{

    BYTE i;
    for(i=0;i<5;i++)
    {
        SysTime[6-i] =pchar[Begin+10+i];//就是16进制数 不用再转换
    }
    SysTime[0] =pchar[Begin+15];
    WriteRealtime();   
}

#if 0
static void CmdWriteMeterAddr()
{
    SysParam[SP_METERADDR1+MeterID*6+0]=Recv645Buf[Begin+10];
    SysParam[SP_METERADDR1+MeterID*6+1]=Recv645Buf[Begin+11];
    SysParam[SP_METERADDR1+MeterID*6+2]=Recv645Buf[Begin+12];
    SysParam[SP_METERADDR1+MeterID*6+3]=Recv645Buf[Begin+13];
    SysParam[SP_METERADDR1+MeterID*6+4]=Recv645Buf[Begin+14];
    SysParam[SP_METERADDR1+MeterID*6+5]=Recv645Buf[Begin+15];
    SaveSysParam();
    
    Send645Buf[0]=0x68;
    Send645Buf[1]=SysParam[SP_METERADDR1+MeterID*6+0];
    Send645Buf[2]=SysParam[SP_METERADDR1+MeterID*6+1];
    Send645Buf[3]=SysParam[SP_METERADDR1+MeterID*6+2];
    Send645Buf[4]=SysParam[SP_METERADDR1+MeterID*6+3];
    Send645Buf[5]=SysParam[SP_METERADDR1+MeterID*6+4];
    Send645Buf[6]=SysParam[SP_METERADDR1+MeterID*6+5];
    Send645Buf[7]=0x68;
    Send645Buf[8]=0x93;

    Send645Buf[9]=0;  
    Send645Buf[10]=CSCheck(Send645Buf, 10); 
    Send645Buf[11]=0x16;
   
    Send_645Buf_num = 12;
    //tcp_write(gPcbLastConnectFromClient, Send645Buf,Send_645Buf_num, 1);
    //tcp_output(gPcbLastConnectFromClient);
    //ClearDLT645SendBuf();//clear buffer
}
#endif
static void CmdWriteMeterAddr(BYTE *pchar)
{
    SysParam[SP_METERADDR1+MeterID*6+0]=pchar[Begin+10];
    SysParam[SP_METERADDR1+MeterID*6+1]=pchar[Begin+11];
    SysParam[SP_METERADDR1+MeterID*6+2]=pchar[Begin+12];
    SysParam[SP_METERADDR1+MeterID*6+3]=pchar[Begin+13];
    SysParam[SP_METERADDR1+MeterID*6+4]=pchar[Begin+14];
    SysParam[SP_METERADDR1+MeterID*6+5]=pchar[Begin+15];
    SaveSysParam();
    
    Send645Buf[0]=0x68;
    Send645Buf[1]=SysParam[SP_METERADDR1+MeterID*6+0];
    Send645Buf[2]=SysParam[SP_METERADDR1+MeterID*6+1];
    Send645Buf[3]=SysParam[SP_METERADDR1+MeterID*6+2];
    Send645Buf[4]=SysParam[SP_METERADDR1+MeterID*6+3];
    Send645Buf[5]=SysParam[SP_METERADDR1+MeterID*6+4];
    Send645Buf[6]=SysParam[SP_METERADDR1+MeterID*6+5];
    Send645Buf[7]=0x68;
    Send645Buf[8]=0x93;

    Send645Buf[9]=0;  
    Send645Buf[10]=CSCheck(Send645Buf, 10); 
    Send645Buf[11]=0x16;
   
    Send_645Buf_num = 12;
    //tcp_write(gPcbLastConnectFromClient, Send645Buf,Send_645Buf_num, 1);
    //tcp_output(gPcbLastConnectFromClient);
    //ClearDLT645SendBuf();//clear buffer
}

/************************************************************************/
/*函数名:CommandAnalysis                                                */
/*函数功能:解析645格式的命令，并做好回应buffer的填充，但不发送回应buffer*/
/*                                                                      */
/*                                                                      */
/************************************************************************/
#if 0
void MeterCmdAnalysis()
{
    BYTE ch;
   // if(RxValidDataFrame(Recv645Buf,Recv_645Save_num))
    {
        
        RxDataHandle645(&Recv645Buf[Begin+10],Recv645Buf[Begin+9]);//将数据域还原
       
        ch = Recv645Buf[Begin+8];  //C
        printf("chh =%02x  ",ch);
        switch(ch)
        {
            case 0x08://广播校时
                HandleCmdRadioTime();
               
            break;

            case 0x11://读数据
                if(Recv645Buf[Begin+9]==4)
                {
                    HandleCmdReadData();
                    
                }
            break;

            case 0x12://读后续数据
                    printf("llllllllllll\n");
            break;

            case 0x13://读通信地址,点对点,本项目是读最近使用的那个，也就是MeterID
                 HandleCmdReadAddr();
                
            break;

            case 0x14://写数据

            break;

            case 0x15://写通信地址
                 CmdWriteMeterAddr();
            break;

            case 0x16://冻结命令

            break;
            
            case 0x17://更改通信率

            break;
            
            case 0x18://更改密码

            break;

            case 0x19://最大需量清零

            break;

            case 0x1A://电表清零

            break;

            case 0x1B://事件清零

            break;

            default:
               Recv645Buf[Begin+8]|=0xC0;
               ErrResponseHandle(Recv645Buf[Begin+8],OTHER_ERR);//其他错误
             
            break;    
        }
    }
    
}
#endif
void MeterCmdAnalysis1(BYTE *pchar)
{
    BYTE ch;
    for(ch=0;ch<(pchar[Begin+9]+12);ch++)
    {
        printf("%02x",pchar[Begin+ch]);
    }
    printf("\r\n");
    ch = 0;
   // if(RxValidDataFrame(Recv645Buf,Recv_645Save_num))
    {
        
        RxDataHandle645(&pchar[Begin+10],pchar[Begin+9]);//将数据域还原
        for(ch=0;ch<(pchar[Begin+9]+12);ch++)
        {
            printf("%02x",pchar[Begin+ch]);
        }
        printf("\r\n");
        
        ch = pchar[Begin+8];  //C
        printf("chh =%02x  \r\n",ch);
        switch(ch)
        {
            case 0x08://广播校时
                HandleCmdRadioTime(pchar);
               
            break;

            case 0x11://读数据
                if(pchar[Begin+9]==4)
                {
                    HandleCmdReadData1(pchar);                    
                }
            break;

            case 0x12://读后续数据
                    //printf("llllllllllll\n");
            break;

            case 0x13://读通信地址,点对点,本项目是读最近使用的那个，也就是MeterID
                 HandleCmdReadAddr();
                
            break;

            case 0x14://写数据

            break;

            case 0x15://写通信地址
                // CmdWriteMeterAddr(pchar);
            break;

            case 0x16://冻结命令

            break;
            
            case 0x17://更改通信率

            break;
            
            case 0x18://更改密码

            break;

            case 0x19://最大需量清零

            break;

            case 0x1A://电表清零

            break;

            case 0x1B://事件清零

            break;

            default:
               pchar[Begin+8]|=0xC0;
               ErrResponseHandle(pchar[Begin+8],OTHER_ERR);//其他错误
             
            break;    
        }
    }
    
}


