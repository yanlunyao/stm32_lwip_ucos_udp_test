#include "includes.h"
#include <spi.h>
#include <ssi.h>
#include "rn8209x.h"
#include <Hw_memmap.h>
#include "comm.h"
#include "dev.h"
#include "DLT645.h"
#include "devI2C.h"
#include "devTimer.h"

BYTE EEPROMPARAM[EEPROM_COUNT]={0};
#if 0
rn8209_fmt_t initTable[18] =
{
    {SYSCON,    WIDTH_TWO_B,    *(WORD *)&SysParam[SP_RNSYSCON]},  //0x40  开启B通道电流
    {IARMSOS,   WIDTH_TWO_B,    *(WORD *)&SysParam[SP_RNIARMSOS]},//0xFB96
    {IBRMSOS,   WIDTH_TWO_B,    *(WORD *)&SysParam[SP_RNIBRMSOS]}, 
    {IBGAIN,    WIDTH_TWO_B,    *(WORD *)&SysParam[SP_RNIBGAIN]},
    {GPQA,      WIDTH_TWO_B,    *(WORD *)&SysParam[SP_RNGPQA]}, 
    {GPQB,      WIDTH_TWO_B,    *(WORD *)&SysParam[SP_RNGPQB]},
    {APOSA,     WIDTH_TWO_B,    *(WORD *)&SysParam[SP_RNAPOSA]},
    {APOSB,     WIDTH_TWO_B,    *(WORD *)&SysParam[SP_RNAPOSB]},
    {PHSA,      WIDTH_ONE_B,    *(WORD *)&SysParam[SP_RNPHSA]},
    {PHSB,      WIDTH_ONE_B,    *(WORD *)&SysParam[SP_RNPHSB]},
    {RPOSA,     WIDTH_TWO_B,    *(WORD *)&SysParam[SP_RNRPOSA]},
    {RPOSB,     WIDTH_TWO_B,    *(WORD *)&SysParam[SP_RNRPOSB]},
    {QPHSCAL,   WIDTH_TWO_B,    *(WORD *)&SysParam[SP_RNQPHSCAL]},
    {HFCONST,   WIDTH_TWO_B,    *(WORD *)&SysParam[SP_RNHFCONST]},
    {PSTART,    WIDTH_TWO_B,    *(WORD *)&SysParam[SP_RNPSTART]},
    {QSTART,    WIDTH_TWO_B,    *(WORD *)&SysParam[SP_RNQSTART]},
    {IE,        WIDTH_ONE_B,    *(WORD *)&SysParam[SP_RNIE]},
    {0 ,0 ,0}
};

#else

rn8209_fmt_t initTable[18] =
{
    {SYSCON,    WIDTH_TWO_B,    (WORD *)&SysParam[SP_RNSYSCON]},  //0x40  开启B通道电流  0
    {IARMSOS,   WIDTH_TWO_B,    (WORD *)&SysParam[SP_RNIARMSOS]}, //0xFB96               1
    {IBRMSOS,   WIDTH_TWO_B,    (WORD *)&SysParam[SP_RNIBRMSOS]}, 
    {IBGAIN,    WIDTH_TWO_B,    (WORD *)&SysParam[SP_RNIBGAIN]},
    {GPQA,      WIDTH_TWO_B,    (WORD *)&SysParam[SP_RNGPQA]}, 
    {GPQB,      WIDTH_TWO_B,    (WORD *)&SysParam[SP_RNGPQB]},
    {APOSA,     WIDTH_TWO_B,    (WORD *)&SysParam[SP_RNAPOSA]},
    {APOSB,     WIDTH_TWO_B,    (WORD *)&SysParam[SP_RNAPOSB]},
    {PHSA,      WIDTH_ONE_B,    (WORD *)&SysParam[SP_RNPHSA]},
    {PHSB,      WIDTH_ONE_B,    (WORD *)&SysParam[SP_RNPHSB]},
    {RPOSA,     WIDTH_TWO_B,    (WORD *)&SysParam[SP_RNRPOSA]},
    {RPOSB,     WIDTH_TWO_B,    (WORD *)&SysParam[SP_RNRPOSB]},
    {QPHSCAL,   WIDTH_TWO_B,    (WORD *)&SysParam[SP_RNQPHSCAL]},
    {HFCONST,   WIDTH_TWO_B,    (WORD *)&SysParam[SP_RNHFCONST]},   // 13
    {PSTART,    WIDTH_TWO_B,    (WORD *)&SysParam[SP_RNPSTART]},    // 14
    {QSTART,    WIDTH_TWO_B,    (WORD *)&SysParam[SP_RNQSTART]},
    {IE,        WIDTH_ONE_B,    (WORD *)&SysParam[SP_RNIE]},
    {0 ,0 ,0}
};
#endif

void init8209(void)
{
    unsigned char i = 0;
    unsigned char j = 0;
 
    printf ( "\r\nRN8209 init..." );
    
    while(initTable[i].Bwidth)
    {
    	//有时写第一通道有问题
		spiWrite(0, initTable[0].add,*(initTable[0].value), initTable[0].Bwidth);
        for(j=0;j<9;j++)
        {
            spiWrite(j, initTable[i].add,*(initTable[i].value+j), initTable[i].Bwidth);
            //data = *(WORD *)&SysParam[SP_RNHFCONST+j*2];
            //spiWrite(j, initTable[i].add,data, initTable[i].Bwidth);
        }
        
        i++;
    }
    for(j=0;j<9;j++)
    {
        while(SSIBusy(SSI0_BASE))
        {
            ;
        }
    
        channelSelect(j);
        SSIDataPut(SSI0_BASE, 0xEA);
        SSIDataPut(SSI0_BASE, 0xE5);
    
        SSIDataPut(SSI0_BASE, 0xEA);
        SSIDataPut(SSI0_BASE, 0x5A);//选择A通道电流用着电能计算
        //SSIDataPut(SSI0_BASE, 0xA5);//选择B通道的电流用着电能计算
    
        SSIDataPut(SSI0_BASE, 0xEA);
        SSIDataPut(SSI0_BASE, 0xDC);
    }
    
}

void QF_PF_IRQn_Output(int channel)
{
    if(channel > 15 || channel < 0)
    {
        printf("\n150 channel over range.");
        return ;
    }
    GPIOPinWrite(GPIO_PORTD_BASE , GPIO_PIN_0, (channel & 0x08)? GPIO_PIN_0:0);
    GPIOPinWrite(GPIO_PORTC_BASE , GPIO_PIN_7, (channel & 0x04)? GPIO_PIN_7:0);
    GPIOPinWrite(GPIO_PORTD_BASE , GPIO_PIN_1, (channel & 0x02)? GPIO_PIN_1:0);            
    GPIOPinWrite(GPIO_PORTC_BASE , GPIO_PIN_6, (channel & 0x01)? GPIO_PIN_6:0);   
}


void getRawMeasure(int channel)
{
    unsigned long long  data=0;
    //ulong data=0;
    uint  Flag;
    //float temp = 0.0;
    uint temp =0;
    BYTE NflagPA =0;//有功通道A
//    BYTE NflagPB =0;//有功通道B
//    BYTE NflagPQ =0;//无功通道


    if(channel > 8 || channel < 0)
    {
        printf("\nSPI channel over range.\n");
        return ;
    }
    spiRead(channel, SYSCON, (uint *)&data, WIDTH_TWO_B);
    printf("\n SYSCON = 0x%x", (uint)data);
    
    spiRead(channel, EMUCON, (uint *)&data, WIDTH_TWO_B);
    printf("\n EMUCON = 0x%x", (uint)data);

    spiRead(channel, HFCONST, (uint *)&data, WIDTH_TWO_B);
    printf("\n HFCONST = 0x%x", (uint)data);
    
    spiRead(channel, PSTART, (uint *)&data, WIDTH_TWO_B);
    printf("\n PSTART = 0x%x", (uint)data);
    
    spiRead(channel, PFCNT, (uint *)&data, WIDTH_TWO_B);
    printf("\n PFCNT = 0x%x", (uint)data);
    
    spiRead(channel, QFCNT, (uint *)&data, WIDTH_TWO_B);
    printf("\n QFCNT = 0x%x\n", (uint)data);
 
    spiRead(channel, IARMSOS, (uint *)&data, WIDTH_TWO_B);
    printf("\n IARMSOS = 0x%x", (uint)data);   
    spiRead(channel, IBRMSOS, (uint *)&data, WIDTH_TWO_B);
    printf("\n IBRMSOS = 0x%x", (uint)data);
    
    spiRead(channel, GPQA, (uint *)&data, WIDTH_TWO_B);
    printf("\n GPQA  = 0x%x", (uint)data);   
    spiRead(channel, GPQB, (uint *)&data, WIDTH_TWO_B);
    printf("\n GPQB  = 0x%x", (uint)data);

    spiRead(channel, APOSA, (uint *)&data, WIDTH_TWO_B);
    printf("\n APOSA = 0x%x", (uint)data); 
    spiRead(channel, APOSB, (uint *)&data, WIDTH_TWO_B);
    printf("\n APOSB = 0x%x", (uint)data);
    
    spiRead(channel, RPOSA, (uint *)&data, WIDTH_TWO_B);
    printf("\n RPOSA = 0x%x", (uint)data);
    spiRead(channel, RPOSB, (uint *)&data, WIDTH_TWO_B);
    printf("\n RPOSB = 0x%x", (uint)data);

    spiRead(channel, PHSA, (uint *)&data, WIDTH_ONE_B);
    printf("\n PHSA = 0x%x", (uint)data);
    spiRead(channel, PHSB, (uint *)&data, WIDTH_ONE_B);
    printf("\n PHSB = 0x%x", (uint)data);

    spiRead(channel, QPHSCAL, (uint *)&data, WIDTH_TWO_B);
    printf("\n QPHSCAL = 0x%x\n", (uint)data);
      
    spiRead(channel, IARMS, (uint *)&data, WIDTH_THREE_B);
   
    if((data >= 0x800000)||(data == 0))
    {
        data = 0;
        temp = 0;
    }
    else
    {
        temp = (uint)(((*(WORD *)&SysParam[SP_RNKIA+2*channel])*data)/10000+1);//补偿0.1mA
    }
    printf("\n IARMS = 0x%x current:%d.%d mA  KiA=%d", (uint)data, temp/10,temp%10,*(WORD *)&SysParam[SP_RNKIA+2*channel]);
    printf("\n IARMSV =%dmV ",(uint)(data*2500/(0x7FFFFF+1)));
/*    
    spiRead(channel, IBRMS, &data, WIDTH_THREE_B);
    if((data >= 0x800000)||(data == 0))
    {
        data = 0;
        temp = 0;
    }
    else
    {
        temp = ((*(WORD *)&SysParam[SP_RNKIB+2*channel]) * data)/10000+1;// 补偿0.1mA
    }
    printf("\n IBRMS = 0x%x current:%d.%d mA  KiB=%d", data, temp/10,temp%10,*(WORD *)&SysParam[SP_RNKIB+2*channel]);
*/    
    spiRead(channel, URMS, (uint *)&data, WIDTH_THREE_B);
    printf("\n URMS = 0x%x ", (uint)data);
    
    if((data >= 0x800000)||(data < 0x24B))
    {    
        data = 0;
    }
 
    temp = (uint)(((*(WORD *)&SysParam[SP_RNKU+2*channel])* data)/1000000);
    printf("\n URMS  = 0x%x voltage:%d.%d V Ku=%d", (uint)data, temp/10,temp%10,(*(WORD *)&SysParam[SP_RNKU+2*channel]));
    printf("\n URMSV =%dmV ",(uint)(data*2500/(0x7FFFFF+1)));
    spiRead(channel, UFREQ, (uint *)&data, WIDTH_TWO_B);
    printf("\n UFREQ = 0x%x\n", (uint)data);
    
    spiRead(channel, POWERPA, (uint *)&data, WIDTH_FOUR_B);
    printf("\n POWERPA = 0x%x", (uint)data);
    if(data>0x80000000)//负数
    {   
        spiRead(channel, EMUSTATUS, &Flag, WIDTH_THREE_B);
        if((Flag&0x20000)&&((Flag&0x200000)==0))//REVP=1,chnsel=0
        {
            data = (data-1)^0x7fffffff;
            data =data&0x7fffffff;
            //data = ~data+1;
            NflagPA=1;
        }
        else
        {
            data =0;    
        }
        
    }
    //数据有溢出可能
   
   temp = (uint )(((*(WORD *)&SysParam[SP_RNKAP+2*channel])*data)/1000000);//扩大10
   
    if(NflagPA == 1)
    {
        NflagPA=0;
        printf("\n POWERPA = 0x%x PA=-%d.%d W  KpA=%d", (uint)data,temp/10,temp%10,*(WORD *)&SysParam[SP_RNKAP+2*channel]);   
    }
    else
    {
        printf("\n POWERPA = 0x%x PA= %d.%d W  KpA=%d", (uint)data,temp/10,temp%10,*(WORD *)&SysParam[SP_RNKAP+2*channel]);
    }
    /*
    spiRead(channel, POWERPB, &data, WIDTH_FOUR_B);
    printf("\n POWERPB = 0x%x", data );
    if(data>0x80000000)//负数
    {
        spiRead(channel, EMUSTATUS, &Flag, WIDTH_THREE_B);
        if((Flag&0x20000)&&((Flag&0x200000)==1))//REVP=1,chnsel=1
        {
            data = (data-1)^0x7fffffff;
            data = data&0x7fffffff;
            NflagPB =1;
        }
        else
        {
            data =0;    
        }
    }
    
    temp = ((*(WORD*)&SysParam[SP_RNKBP+2*channel])*data)/1000000;
    if(NflagPB == 1)
    {
        NflagPB =0;
        printf("\n POWERPB = 0x%x PB=-%d.%d W  KpB=%d", data,temp/10,temp%10,*(WORD*)&SysParam[SP_RNKBP+2*channel]);
    }
    else
    {
        printf("\n POWERPB = 0x%x PB= %d.%d W  KpB=%d", data,temp/10,temp%10,*(WORD*)&SysParam[SP_RNKBP+2*channel]);
    }    
    spiRead(channel, POWERQ, &data, WIDTH_FOUR_B);
    printf("\n POWERQ  = 0x%x", data);
    if(data>0x80000000)//负数
    {
        spiRead(channel, EMUSTATUS, &Flag, WIDTH_THREE_B);
        if(Flag&0x30000)//REVQ=1
        {
            data = ((data-1)^0x7fffffff);
            data = data&0x7fffffff;
            NflagPQ =1;
        }
        else
        {
            data =0;    
        }
    }
    if(NflagPQ ==1)
    {
        NflagPQ = 0; 
    }
    else
    {
    
    }
    printf("\n POWERQ  = 0x%x\n", data);
    */
    spiRead(channel, ENERGYP, (uint *)&data, WIDTH_THREE_B);
    temp = (data+ *(DWORD *)&EEPROMPARAM[EEPROM_ENERGY + 4*channel])*100/EC;
    printf("\n ENERGYP = 0x%x , EE= %d.%d%d kWh", (uint)data,temp/100,temp%100/10,temp%10);

    spiRead(channel, ENERGYP2, (uint *)&data, WIDTH_THREE_B);
    temp = data*100/EC;
    printf("\n ENERGYP2 = 0x%x ,EE2= %d.%d%d kWh", (uint)data,temp/100,temp%100/10,temp%10);
        
    spiRead(channel, ENERGYQ, (uint *)&data, WIDTH_THREE_B);
    printf("\n ENERGYQ = 0x%x", (uint)data);
    temp = data*100/EC;
    printf("\n ENERGYPQ = %d kVARh\n", (uint)data);

    
    spiRead(channel, IF, (uint *)&data, WIDTH_ONE_B);
    printf("\n interrupt = 0x%x", (uint)data);

    spiRead(channel, IE, (uint *)&data, WIDTH_ONE_B);
    printf("\n IE = 0x%x", (uint)data);
    
    spiRead(channel, EMUSTATUS, (uint *)&data, WIDTH_THREE_B);
    printf("\n EMUSTATUS = 0x%x", (uint)data);

    spiRead(channel, HFCONST, (uint *)&data, WIDTH_TWO_B);
    printf("\n HFCONST = 0x%x  buf=0x%x", (uint)data,*(WORD *)&SysParam[SP_RNHFCONST+2*channel]);

    QF_PF_IRQn_Output(channel);

}

//8209中断处理函数
//1.采用周期任务去读中断标志寄存器
//2.用中断引脚
#if 0
void taskRnIsr(void *pmsg)
{
    BYTE err;
    uint IFData=0;
    uint PData=0;
    uint channel;
//    static BYTE flag[9]={0};

    printf ( "\r\nIsr task begin..." );
       
    while(1)
    {
        OSSemPend(RNSemIsr,0,&err);
        //printf("insert isr\n");
        for(channel=0;channel<9;channel++)
        {
            spiRead(channel, RIF, &IFData, WIDTH_ONE_B);
            if(IFData & RN_RPEOIF)//溢出中断
            {
                printf("overflow\n");
                SysParam[SP_PEOIF_COUNT+channel]++;//记录每个通道电能寄存器溢出次数   
                SaveSysParam(); 
                //Read_FM24C(FM_address, OVERFLOW_COUNT+channel,&temp[0], 1);
                //temp[0]++;
                //Write_FM24C(FM_address, OVERFLOW_COUNT+channel,&temp[0], 1);
                
                spiRead(channel, ENERGYP2, &PData, WIDTH_THREE_B);//CLEAR ENERGYP2
                PData = 0x00;
                Write_FM24C(FM_address, A_EEAddr+channel*3,(BYTE *)&PData, 3);
            }

            if(IFData & RN_RPFIF)//PF脉冲
            {
                printf("PF%d\n",channel);
                pfcount[channel]++;
                if( pfcount[channel]==32)
                {   
                    pfcount[channel]=0;
                    spiRead(channel, ENERGYP2, &PData, WIDTH_THREE_B);
                    
                    Read_FM24C(FM_address, A_EEAddr+channel*3, (BYTE*)&IFData, 3);
                    PData +=IFData;
                    printf("energyp%d=%d\n",channel,PData);
                    //将读出的电能值存入铁电
                    Write_FM24C(FM_address, A_EEAddr+channel*3, (BYTE*)&PData, 3);
                }
            }

        } 
        //channelSelect(0);   
    }

}

#endif
void ShowMeterAddr(void)
{
    printf("\r\n**************************************\r\n");
    printf("*************Set Meter No*************\r\n");
    printf("**************************************\r\n");
    printf("\nnumber=05-04-03-02-01-00");
    printf("\nMAddr1=%02x-%02x-%02x-%02x-%02x-%02x\n",SysParam[SP_METERADDR1+5],SysParam[SP_METERADDR1+4],\
    SysParam[SP_METERADDR1+3],SysParam[SP_METERADDR1+3],SysParam[SP_METERADDR1+1],SysParam[SP_METERADDR1]);
    printf("\nnumber=11-10-09-08-07-06");
    printf("\nMAddr2=%02x-%02x-%02x-%02x-%02x-%02x\n",SysParam[SP_METERADDR2+5],SysParam[SP_METERADDR2+4],\
    SysParam[SP_METERADDR2+3],SysParam[SP_METERADDR2+2],SysParam[SP_METERADDR2+1],SysParam[SP_METERADDR2]);
    printf("\nnumber=17-16-15-14-13-12");
    printf("\nMAddr3=%02x-%02x-%02x-%02x-%02x-%02x\n",SysParam[SP_METERADDR3+5],SysParam[SP_METERADDR3+4],\
    SysParam[SP_METERADDR3+3],SysParam[SP_METERADDR3+2],SysParam[SP_METERADDR3+1],SysParam[SP_METERADDR3]);
#if(EleMeterCnt==6)
    printf("\nnumber=18-19-20-21-22-23");
    printf("\nMAddr4=%02x-%02x-%02x-%02x-%02x-%02x\n",SysParam[SP_METERADDR4+5],SysParam[SP_METERADDR4+4],\
    SysParam[SP_METERADDR4+3],SysParam[SP_METERADDR4+2],SysParam[SP_METERADDR4+1],SysParam[SP_METERADDR4]);
    printf("\nnumber=24-25-26-27-28-29");
    printf("\nMAddr5=%02x-%02x-%02x-%02x-%02x-%02x\n",SysParam[SP_METERADDR5],SysParam[SP_METERADDR5+1],\
    SysParam[SP_METERADDR5+2],SysParam[SP_METERADDR5+3],SysParam[SP_METERADDR5+4],SysParam[SP_METERADDR5+5]);
    printf("\nnumber=30-31-32-33-34-35");
    printf("\nMAddr6=%02x-%02x-%02x-%02x-%02x-%02x\n",SysParam[SP_METERADDR6],SysParam[SP_METERADDR6+1],\
    SysParam[SP_METERADDR6+2],SysParam[SP_METERADDR6+3],SysParam[SP_METERADDR6+4],SysParam[SP_METERADDR6+5]);
#endif
    printf("\r\n");
    printf("Q:Quit \r\n");
    printf("\r\n");

}
void RnCheckSelf()
{
    BYTE i =0;
  
    for(i=0;i<9;i++)
    {   
        RnCheckSelfCh(i);
        
    }    
    
}
void RnCheckSelfCh(BYTE channel)
{
    BYTE j =0;
    uint temp[10]={0};
    uint tempp=0;

    //printf("\r\n channel_%d ",channel);
    //APOSA
    for(j=0;j<10;j++)
    {
        while(1)
        {
            OSTimeDlyHMSM(0, 0, 0, 100);
            spiRead(channel, RIF, &tempp, WIDTH_ONE_B);
            if(tempp&0x01)//更新中断
            {
                break;
            }
        }
        spiRead(channel, POWERPA, &tempp, WIDTH_FOUR_B);
        temp[j] = tempp;    
    }
      
    tempp =MeanValue(temp,10);
    tempp =(tempp^0xffffffff)+1;
    tempp =tempp&0xffff;
    spiWrite(channel, APOSA, tempp, WIDTH_TWO_B);
    *(WORD *)&SysParam[SP_RNAPOSA+channel*2]=tempp;
    printf("#"); 
#if(EleMeterCnt==6)
    //APOSB
    for(j=0;j<10;j++)
    {
        while(1)
        {
            OSTimeDlyHMSM(0, 0, 0, 100);
            spiRead(channel, RIF, &tempp, WIDTH_ONE_B);
            if(tempp&0x01)
            {
                break;
            }
        }
        spiRead(channel, POWERPB, &tempp, WIDTH_FOUR_B);
        temp[j]=tempp;    
    }
        
    tempp =MeanValue(temp,10);
    tempp =(tempp^0xffffffff)+1;
    tempp =tempp&0xffff;
    spiWrite(channel, APOSB, tempp, WIDTH_TWO_B);
    *(WORD *)&SysParam[SP_RNAPOSB+channel*2]=tempp;
    printf("#"); 
#endif
   //rposa/rposb
    for(j=0;j<10;j++)
    {
        while(1)
        {
            OSTimeDlyHMSM(0, 0, 0, 100);
            spiRead(channel, RIF, &tempp, WIDTH_ONE_B);
            if(tempp&0x01)
            {
                break;
            }
        }
        spiRead(channel, POWERQ, &tempp, WIDTH_FOUR_B);
        temp[j]=tempp;    
    }
       
    tempp =MeanValue(temp,10);
    tempp =(tempp^0xffffffff)+1;
    tempp =tempp&0xffff;
    spiRead(channel, EMUSTATUS, &temp[0], WIDTH_THREE_B);
    if(temp[0]&0x200000)
    {
        spiWrite(channel, RPOSB, tempp, WIDTH_TWO_B);
        *(WORD *)&SysParam[SP_RNRPOSB+channel*2]=tempp;
    }
    else
    {
        spiWrite(channel, RPOSA, tempp, WIDTH_TWO_B);
        *(WORD *)&SysParam[SP_RNRPOSA+channel*2]=tempp;
    }
    printf("#"); 
    
    //iarmsos
    for(j=0;j<10;j++)
    {
        while(1)
        {
            OSTimeDlyHMSM(0, 0, 0, 100);
            spiRead(channel, RIF, &tempp, WIDTH_ONE_B);
            if(tempp&0x01)
            {
                break;
            }
        }
        spiRead(channel, IARMS, &tempp, WIDTH_THREE_B);
        temp[j] = tempp;    
    }
        
    tempp =MeanValue(temp,10);
    tempp =tempp*tempp;
    tempp =(tempp^0xffffffff)+1;
    tempp =tempp>>8;
    //tempp =tempp&0xffff;
    tempp =(tempp&0xffff)|((tempp>>8)&0x8000);
    spiWrite(channel, IARMSOS, tempp, WIDTH_TWO_B);
    *(WORD *)&SysParam[SP_RNIARMSOS+channel*2]=tempp;
    printf("#"); 
#if(EleMeterCnt==6)    
    //ibrmsos
    for(j=0;j<10;j++)
    {
        while(1)
        {
            OSTimeDlyHMSM(0, 0, 0, 100);
            spiRead(channel, RIF, &tempp, WIDTH_ONE_B);
            if(tempp&0x01)
            {
                break;
            }
        }
        spiRead(channel, IBRMS, &tempp, WIDTH_THREE_B);
        temp[j] =tempp;    
    }
        
    tempp =MeanValue(temp,10);
    tempp =tempp*tempp;
    tempp =(tempp^0xffffffff)+1;
    tempp =tempp>>8;
    //tempp =tempp&0xffff;
    tempp =(tempp&0xffff)|((tempp>>8)&0x8000);
    spiWrite(channel, IBRMSOS, tempp, WIDTH_TWO_B);
    *(WORD *)&SysParam[SP_RNIBRMSOS+channel*2]=tempp;
#endif
}


void taskPriod ( void *pParam )
{
 
   static unsigned long long data = 0;
   
   static uint Flag = 0;    
   static uint tempAi[9] ={0};
//   static uint tempBi[9] ={0};
   static uint tempU[9]  ={0};
   static uint tempEE[9] ={0};
   static uint tempPA[9] ={0};
//   static uint tempPB[9] ={0};

//   static ulong tempQE[9] ={0};
   static BYTE channel =0;
   static BYTE i=0;
   static BYTE count=0;
   printf ( "\r\nPriod task begin..." );

    while ( 1 )
    {
        
        OSTimeDly ( OS_TICKS_PER_SEC*2); 
        updatePeriodChk();
        ReadRealtime();
        count++;
        
        for(channel=0;channel<9;channel++)
        {
            spiRead(channel, IARMS, (uint *)&data, WIDTH_THREE_B);
            //printf("IARMS=%d\r\n",*(uint *)&data);
            if(data >= 0x800000)
            {
                data = 0;
                tempAi[channel]= 0;
            }
            else
            {
                tempAi[channel]= (uint)(((*(WORD *)&SysParam[SP_RNKIA+2*channel])*data)/10000+1);// 0.1mA   补偿0.1mA
            }
            #if(EleMeterCnt==6) 
            spiRead(channel, IBRMS, &data, WIDTH_THREE_B);
            if(data >= 0x800000)
            {
                data = 0;
                tempBi[channel] =0;
            }
            else
            {
                tempBi[channel] = ((*(WORD *)&SysParam[SP_RNKIB+2*channel]) * data)/10000+1;// 0.1mA 补偿0.1mA
            }
            #endif
            spiRead(channel, URMS, (uint *)&data, WIDTH_THREE_B);
            //printf("URMS=%d\r\n",*(uint *)&data);
            if((data >= 0x800000)||(data < 0x24B))
            {    
                data = 0;
            }
        
            tempU[channel] = (uint)(((*(WORD *)&SysParam[SP_RNKU+2*channel])* data)/1000000);// 0.1V
            
            //A通道功率
            spiRead(channel, POWERPA, (uint *)&data, WIDTH_FOUR_B);
            //printf("POWERPA=%d\r\n",*(uint *)&data);
            if(data>0x80000000)//负数
            {   
                spiRead(channel, EMUSTATUS, &Flag, WIDTH_THREE_B);
                if(Flag&0x20000)//REVP=1
                {
                    data = (data-1)^0x8fffffff;
                    data = data&0x7fffffff;
                }
                else
                {
                    data =0;    
                }
        
             }
            
            tempPA[channel] = (uint)(((*(WORD *)&SysParam[SP_RNKAP+2*channel])*data)/1000000);// 0.1W
        
            
            #if(EleMeterCnt==6)            
            spiRead(channel, POWERPB, (uint *)&data, WIDTH_FOUR_B);
            
            if(data>0x80000000)//负数
            {
                spiRead(channel, EMUSTATUS, &Flag, WIDTH_THREE_B);
                if(Flag&0x20000)//REVP=1
                {
                    data = (data-1)^0x8fffffff;
                }
                else
                {
                    data =0;    
                }
            }
        
            tempPB[channel] = ((*(WORD*)&SysParam[SP_RNKBP+2*channel])*data)/1000000;
            #endif
            #if 0
            //无功功率
            spiRead(channel, POWERQ, &data, WIDTH_FOUR_B);
            if(data>0x80000000)//负数
            {
                spiRead(channel, EMUSTATUS, &Flag, WIDTH_THREE_B);
                if(Flag&0x40000)//REVQ=1
                {
                    data = (data-1)^0x8fffffff;
                }
                else
                {
                    data =0;    
                }
            }
            #endif
            //读中断标志位
            spiRead(channel, RIF, (uint *)&data, WIDTH_ONE_B);
            if(data & RN_RPEOIF)//溢出中断
            {
                printf("overflow=%d\n",channel);
                
                *(DWORD *)&EEPROMPARAM[EEPROM_ENERGY+4*channel]= 0;
                EEPROMPARAM[EEPROM_OVERFLOW+channel] +=1;
                Write_FM24C(FMSLAVE_ADDR, EEPROM_OVERFLOWADDR+channel,&EEPROMPARAM[EEPROM_OVERFLOW+channel], 1);
            }
            
            spiRead(channel, ENERGYP, (uint *)&data, WIDTH_THREE_B);//
            
            *(DWORD *)&EEPROMPARAM[EEPROM_ENERGYLAST+4*channel] = data;
            
            data += *(DWORD *)&EEPROMPARAM[EEPROM_ENERGY+4*channel] ;
            
            if(count == 5)//10s
            {
                if(channel == 8)
                {
                    count = 0;
                } 
                Write_FM24C(FMSLAVE_ADDR, EEPROM_ENERGY+4*channel,(BYTE *)&data, 4);
                Write_FM24C(FMSLAVE_ADDR, EEPROM_ENERGYLAST+4*channel,(BYTE *)&EEPROMPARAM[EEPROM_ENERGYLAST+4*channel], 4);
            }
            tempEE[channel]=(data+EEPROMPARAM[EEPROM_OVERFLOW + channel]*0xffffff)/(EC/100);    
          
        }
        #if(EleMeterCnt==3)

        for(i=0;i<3;i++)
        {
            EMeterDat[i].Ai = tempAi[i]/10;//mA
            EMeterDat[i].Bi = tempAi[i+3]/10;
            EMeterDat[i].Ci = tempAi[i+6]/10;

            EMeterDat[i].Au = (tempU[0]+tempU[1]+tempU[2])/3;//100mV  0.1V
            EMeterDat[i].Bu = (tempU[3]+tempU[4]+tempU[5])/3;
            EMeterDat[i].Cu = (tempU[6]+tempU[7]+tempU[8])/3;
                                                                                            
            EMeterDat[i].AE = tempEE[i];// 0.01kWh,电表常数3200
            EMeterDat[i].BE = tempEE[i+3];
            EMeterDat[i].CE = tempEE[i+6];
            
            EMeterDat[i].PWRA = tempPA[i];//功率
            EMeterDat[i].PWRB = tempPA[i+3];
            EMeterDat[i].PWRC = tempPA[i+6];
            
            EMeterDat[i].PWR = tempPA[0+i]+tempPA[i+3]+tempPA[i+6];
            
            EMeterDat[i].EE = EMeterDat[i].AE+EMeterDat[i].BE+EMeterDat[i].CE;// 0.01kwh
                
        }
        #else          
        for(i=0;i<6;i++)
        {
            if((i%2)==0)
            {
                EMeterDat[i].Ai=tempAi[i/2]/10;//mA
                EMeterDat[i].Bi=tempAi[i/2+3]/10;
                EMeterDat[i].Ci=tempAi[i/2+6]/10;
            }
            else
            {
                EMeterDat[i].Ai=tempBi[i/2]/10;//mA
                EMeterDat[i].Bi=tempBi[i/2+3]/10;
                EMeterDat[i].Ci=tempBi[i/2+6]/10;
            }

            EMeterDat[i].Au=(tempU[0]+tempU[1]+tempU[2])/3;//100mV  0.1V
            EMeterDat[i].Bu=(tempU[3]+tempU[4]+tempU[5])/3;
            EMeterDat[i].Cu=(tempU[6]+tempU[7]+tempU[8])/3;

            if((i%2)==0)
            {
                EMeterDat[i].AE=tempEE[i/2];//0.01kwh 
                EMeterDat[i].BE=tempEE[i/2+3];
                EMeterDat[i].CE=tempEE[i/2+6];
            }
            else
            {
                EMeterDat[i].AE=EEB[i/2];//0.01kwh 
                EMeterDat[i].BE=EEB[i/2+3];
                EMeterDat[i].CE=EEB[i/2+6];
            }

            EMeterDat[i].EE=EMeterDat[i].AE+EMeterDat[i].BE+EMeterDat[i].CE;// 0.01kwh   
                
        }  
        #endif
    }
}
/*******************************************************/
/*函数功能: 在输入电流为dati( dati 精确到0.1mA)的情况下*/
/*来校正通道A、B的K值                                  */
/*******************************************************/
void Check_KIA_KIB(BYTE channel,uint dati)
{
    BYTE j =0;
    BYTE i =0;
    uint temp=0;
    uint temp1[10]={0};
    uint len=10;
    unsigned long long dbuf = 0;
    
    for(j=0;j<len;j++)
    {
        while(1)
        {
            OSTimeDlyHMSM(0, 0, 0, 150);
            spiRead(channel, RIF, &temp, WIDTH_ONE_B);
            if(temp&0x01)
            {
                break;
            }
        }
        spiRead(channel, IARMS, &temp, WIDTH_THREE_B);
        if(temp < 0x800000)//大于 0x800000 时做零处理
        {
            temp1[i] = temp;
            i++;
        }
        temp = 0;
    }
    
    temp = MeanValue(temp1,i);
    dbuf = dati;
    
    *(WORD *)&SysParam[SP_RNKIA+2*channel] = dbuf*10000/temp; // 最小单位为1mA来计算的K
    i =0;
    printf("#");   
#if(EleMeterCnt==6) 
    for(j=0;j<len;j++)
    {
        while(1)
        {
            OSTimeDlyHMSM(0, 0, 0, 150);
            spiRead(channel, RIF, &temp, WIDTH_ONE_B);
            if(temp&0x01)
            {
                break;
            }
        }
        spiRead(channel, IBRMS, &temp, WIDTH_THREE_B);
        if(temp < 0x800000)//大于 0x800000 时做零处理
        {
            temp1[i] = temp;
            i++;
        }
        temp = 0;
    }
    
    tempp = MeanValue(temp1,i);
    *(WORD *)&SysParam[SP_RNKIB+2*channel] = dati*10000/tempp;
    printf("#");
#endif
 
}

/*******************************************************/
/*函数功能: 在输入电压为datu(精确到0.1V)的情况下       */
/*来校正Ku值                                           */
/*******************************************************/

void Check_KU(BYTE channel,uint datu)
{
    BYTE j =0;
    BYTE i =0;

    uint temp =0;
    uint temp1[10]={0};
    uint tempp=0;
    uint len=10;
    for(j=0;j<len;j++) //读取10次
    {
        while(1)
        {
            OSTimeDlyHMSM(0, 0, 0, 150);
            spiRead(channel, RIF, &temp, WIDTH_ONE_B);
            if(temp&0x01)
            {
                break;
            }
        }
        spiRead(channel, URMS, &temp, WIDTH_THREE_B);
        if(temp < 0x800000)//大于 0x800000 时做零处理
        {
            temp1[i] = temp;
            i++;
        }
        temp = 0;

    }
    
    tempp = MeanValue(temp1,i);
    *(WORD *)&SysParam[SP_RNKU+2*channel] = datu*1000000/tempp; // 
    printf("#");
}
/*******************************************************/
/*函数功能: 在功率为datp(精确到0.1W)时                 */
/*校正通道A、B的Kp值                                   */
/*******************************************************/

void Check_KPA_KPB(BYTE channel,uint datp)
{
    BYTE j =0;
    BYTE i =0;

    uint temp =0;
    uint temp1[10]={0};    
    uint len=10;
	unsigned long long dbuf=0;
	
    for(j=0;j<len;j++)
    {
        while(1)
        {
            OSTimeDlyHMSM(0, 0, 0, 150);
            spiRead(channel, RIF, &temp, WIDTH_ONE_B);
            if(temp&0x01)
            {
                break;
            }
        }
        spiRead(channel, POWERPA, &temp, WIDTH_FOUR_B);
        if(temp < 0x80000000)//大于 0x80000000 时为负
        {
            temp1[i] = temp;
            i++;
        }
        temp = 0;
    }
    
    temp = MeanValue(temp1,i);
    dbuf = datp;//避免数据溢出
    
    *(WORD *)&SysParam[SP_RNKAP+2*channel] = dbuf*1000000/temp;//最小单位为0.1W 来计算的K
    i =0;
    printf("#");

    /*
    for(j=0;j<len;j++)
    {
        while(1)
        {
            OSTimeDlyHMSM(0, 0, 0, 100);
            spiRead(channel, RIF, &temp, WIDTH_ONE_B);
            if(temp&0x01)
            {
                break;
            }
        }
        spiRead(channel, POWERPB, &temp, WIDTH_FOUR_B);
        if(tempp < 0x80000000)//大于 0x80000000 时为负
        {
            temp1[i] = temp;
            i++;
        }
        temp = 0;
    }
    
    tempp = MeanValue(temp1,i);
    *(WORD *)&SysParam[SP_RNKBP+2*channel] = datp*1000000/tempp; //最小单位0.1W 来计算的K
    printf("*");*/
}
/**************************************************************/
/*函数功能: 能量脉冲校正 ，电表常数3200                       */
/*校验方法: 将电流、电压、功率校正好后，外挂标志的100W        */
/*的功率源，连接8小时时间，即可完成校验(在PE0中断函数中断用)  */
/*需先用命令Checkhf enable将变量CheckHFConstFlag设置为1       */
/**************************************************************/
//方式一:用标准功率源(100W)
//pBuf每小时脉冲个数
#if 0
void Check_HFConst(uint *pBuf,BYTE len ,BYTE channel)
{
    uint temp;
     
    temp = MeanValue(pBuf, len);
    *(WORD *)&SysParam[SP_RNHFCONST+2*channel] = *(WORD *)&SysParam[SP_RNHFCONST+2*channel]*temp/32;
    spiWrite(channel, HFCONST, *(WORD *)&SysParam[SP_RNHFCONST+2*channel], WIDTH_TWO_B);
    SaveSysParam();
    
}
#endif
//方式二:跟标准表对照
//原理:因为PFCNT的绝对值的2倍大于HFConst的值时ENERGYP的值加1，所以HFConst'*ENERGYP就应该等于消耗的实际能量值，
//而实际消耗的能量值可以用另外的表读出(datE)，datE*期望的电表常数EC,就是我们期望的能量脉冲值
//HFConst'(表中的值)*ENERGYP(读出829)=HFConst(期望的)*(datE*EC)
//EBuf被校表电能寄存器的值，datE对照表的电能值，以0.01KWh为单位，比如0.05KWh，则datE为5
void Check_HFConst1(BYTE channel,uint datE)
{
    unsigned long long EBuf=0;
    uint temp=0;
    spiRead(channel, ENERGYP, &temp, WIDTH_THREE_B);
    EBuf = temp;

    //扩大10
    temp = (uint)((*(WORD *)&SysParam[SP_RNHFCONST+2*channel])*EBuf*10/(EC/100*datE));//EC=200
    printf("temp%d=0x%x ",channel,temp);
    //4舍5入
    if(temp%10 >= 5)
    {
        *(WORD *)&SysParam[SP_RNHFCONST+2*channel] = (WORD)(temp/10+1);        
    }
    else
    {
        *(WORD *)&SysParam[SP_RNHFCONST+2*channel] = (WORD)(temp/10);
    }
    OSTimeDly(OS_TICKS_PER_SEC/50);
    printf("\nSP_HCONST%d=0x%x ",channel,*(WORD *)&SysParam[SP_RNHFCONST+2*channel]);
    spiWrite(channel, HFCONST, *(WORD *)&SysParam[SP_RNHFCONST+2*channel], WIDTH_TWO_B);
    
    //SaveSysParam();
}
#if 0
//方式二:跟标准表对照
//EBuf被校表电能寄存器的值，datE对照表的电能值，以0.01KWh为单位，比如0.05KWh，则datE为5
void Check_HFConst2(uint EBuf, uint datE,BYTE channel)
{
    *(WORD *)&SysParam[SP_RNHFCONST+2*channel] = EBuf * (*(WORD *)&SysParam[SP_RNHFCONST+2*channel])/(320*datE);
    spiWrite(channel, HFCONST, *(WORD *)&SysParam[SP_RNHFCONST+2*channel], WIDTH_TWO_B);
    SaveSysParam();
}
#endif

