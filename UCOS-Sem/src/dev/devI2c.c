/************************************************************************************/
/*作者:张泽坤                                                                       */
/*描述:I2C接口处理 ,                                                                 */
/*修订记录:                                                                         */
/************************************************************************************/

#if 0
#include    <hw_ints.h> 
#include    <interrupt.h> 
#include    <sysctl.h> 
#include    <gpio.h> 
#include    "devI2c.h"

//    定义工作状态 
#define    STAT_IDLE       0          //    空闲状态 
#define    STAT_ADDR      1          //    发送数据地址状态 
#define    STAT_DATA      2          //    接收或发送数据状态 
#define    STAT_FINISH      3          //    收发完成状态 
 
//    定义全局变量 
static unsigned long I2CM_BASE = I2C0_MASTER_BASE;    //    定义 I2C 主机基址，并初始化 
static tI2CM_DEVICE gtDevice;                         //    器件数据接口 
static unsigned char gucStatus = STAT_IDLE;         //    工作状态 
static tBoolean gbSendRecv;                         //    收发操作标志，false 发送，true 接收 
static char gcAddr[4];                          //    数据地址数组 
static unsigned int guiAddrIndex;              //    数据地址数组索引变量 
static unsigned int guiDataIndex;              //    数据缓冲区索引变量 
 
//    对 tI2CM_DEVICE 结构体变量初始化设置所有数据成员 
void I2CM_DeviceInitSet(tI2CM_DEVICE *pDevice, unsigned char ucSLA, 
                      unsigned long ulAddr, 
                      unsigned int uiLen, 
                      char *pcData, 
                      unsigned int uiSize) 
{ 
    pDevice->ucSLA = ucSLA; 
    pDevice->ulAddr = ulAddr; 
    pDevice->uiLen = uiLen; 
    pDevice->pcData = pcData; 
    pDevice->uiSize = uiSize; 
} 
 
//    对 tI2CM_DEVICE 结构体变量设置与数据收发相关的成员（数据地址、数据缓冲区、数据大小） 
void I2CM_DeviceDataSet(tI2CM_DEVICE *pDevice, unsigned long ulAddr, 
                        char *pcData, 
                        unsigned int uiSize) 
{ 
    pDevice->ulAddr = ulAddr; 
    pDevice->pcData = pcData; 
    pDevice->uiSize = uiSize; 
}

//    I2C 主机初始化 
void I2CM_Init(void) 
{ 
    I2CM_DeviceInitSet(&gtDevice, 0, 0, 0, (void *)0, 0); 
 
    if ((I2CM_BASE != I2C0_MASTER_BASE) && (I2CM_BASE != I2C1_MASTER_BASE)) 
    { 
        I2CM_BASE = I2C0_MASTER_BASE; 
    } 
 
    switch (I2CM_BASE) 
    { 
        case I2C0_MASTER_BASE: 
        SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);     //    使能 I2C0 模块 
        SysCtlPeripheralEnable(I2C0SCL_PERIPH);       //    使能 SCL 所在的 GPIO 模块 
        GPIOPinTypeI2C(I2C0SCL_PORT, I2C0SCL_PIN);    //    配置相关管脚为 SCL 功能 
        SysCtlPeripheralEnable(I2C0SDA_PERIPH);       //    使能 SDA 所在的 GPIO 模块 
        GPIOPinTypeI2C(I2C0SDA_PORT, I2C0SDA_PIN);    //    配置相关管脚为 SDA 功能 
        IntEnable(INT_I2C0);                   //    使能 I2C0 中断 
        break; 
        /*
        case I2C1_MASTER_BASE: 
        SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);     //    使能 I2C1 模块 
        SysCtlPeripheralEnable(I2C1SCL_PERIPH);       //    使能 SCL 所在的 GPIO 模块 
        GPIOPinTypeI2C(I2C1SCL_PORT, I2C1SCL_PIN);    //    配置相关管脚为 SCL 功能 
        SysCtlPeripheralEnable(I2C1SDA_PERIPH);       //    使能 SDA 所在的 GPIO 模块 
        GPIOPinTypeI2C(I2C1SDA_PORT, I2C1SDA_PIN);    //    配置相关管脚为 SDA 功能 
        IntEnable(INT_I2C1);               //    使能 I2C1 中断 
        break; 
        */
        default: 
        break; 
    } 
 
    I2CMasterInit(I2CM_BASE, false);            //    I2C 主机模块初始化，100kbps 
    I2CMasterIntEnable(I2CM_BASE);            //    使能 I2C 主模块中断 
    IntMasterEnable( );                     //    使能处理器中断 
    I2CMasterEnable(I2CM_BASE);              //    使能 I2C 主机 
} 

//方式一:使用中断处理  
//    功能：I2C 主机发送或接收数据 
//    参数：pDevice 是指向 tI2CM_DEVICE 型结构体变量的指针 
//                bFlag 取值 false 表示发送操作，取值 true 表示接收操作 
//    返回：I2C_MASTER_ERR_NONE    没有错误 
//          I2C_MASTER_ERR_ADDR_ACK  地址应答错误 
//          I2C_MASTER_ERR_DATA_ACK  数据应答错误 
//          I2C_MASTER_ERR_ARB_LOST   多主机通信仲裁失败 
//    发送格式：S | SLA+W | addr[1～4] | data[1～n] | P 
//    接收格式：S | SLA+W | addr[1～4] | Sr | SLA+R | data[1～n] | P 
unsigned long I2CM_SendRecv(tI2CM_DEVICE *pDevice, tBoolean bFlag) 
{ 
    //    数据地址长度或收发数据大小不能为 0，否则不执行任何操作 
    if ((pDevice->uiLen <= 0) || (pDevice->uiSize <= 0)) 
    { 
        return(I2C_MASTER_ERR_NONE); 
    } 
 
    gtDevice = *pDevice; 
    if (gtDevice.uiLen > 4) gtDevice.uiLen = 4;          //    数据地址长度不能超过 4B 
 
    gbSendRecv = bFlag;                  //    相关全局变量初始化 
    guiAddrIndex = 0; 
    guiDataIndex = 0; 
 
    switch (gtDevice.uiLen)                //    将数据地址分解成数组 
    { 
        case 1:                      //    1 字节数据地址 
        gcAddr[0] = (char)(gtDevice.ulAddr); 
        break; 
 
        case 2:                      //    2 字节数据地址 
        gcAddr[0] = (char)(gtDevice.ulAddr >> 8); 
        gcAddr[1] = (char)(gtDevice.ulAddr); 
        break; 
 
        case 3:                      //    3 字节数据地址 
        gcAddr[0] = (char)(gtDevice.ulAddr >> 16); 
        gcAddr[1] = (char)(gtDevice.ulAddr >> 8); 
        gcAddr[2] = (char)(gtDevice.ulAddr); 
        break; 
 
        case 4:                      //    4 字节数据地址 
        gcAddr[0] = (char)(gtDevice.ulAddr >> 24); 
        gcAddr[1] = (char)(gtDevice.ulAddr >> 16); 
        gcAddr[2] = (char)(gtDevice.ulAddr >> 8); 
        gcAddr[3] = (char)(gtDevice.ulAddr); 
        break; 
 
        default: 
        break; 
    }

    //如果是多主机通信，则需要首先等待总线空闲 
    //while (I2CMasterBusBusy(I2CM_BASE));        //    等待总线空闲 
 
    I2CMasterSlaveAddrSet(I2CM_BASE, gtDevice.ucSLA, false);  //    设置从机地址，写操作 
    I2CMasterDataPut(I2CM_BASE, gcAddr[guiAddrIndex++]);    //    开始发送数据地址 
    gucStatus = STAT_ADDR;                //    设置状态：发送数据地址 
 
    //    命令：主机突发发送起始 
    I2CMasterControl(I2CM_BASE, I2C_MASTER_CMD_BURST_SEND_START); 
 
    while (gucStatus != STAT_IDLE);             //    等待总线操作完毕 
 
    return(I2CMasterErr(I2CM_BASE));            //    返回可能的错误状态 
} 


// I2C 中断服务函数 
void I2C_ISR(void) 
{ 
    unsigned long ulStatus; 
 
    ulStatus = I2CMasterIntStatus(I2CM_BASE, true);        //    读取中断状态 
    I2CMasterIntClear(I2CM_BASE);             //    清除中断状态 
 
    if (I2CMasterErr(I2CM_BASE) != I2C_MASTER_ERR_NONE)  //    若遇到错误 
    { 
        gucStatus = STAT_IDLE; 
        return; 
    } 
 
    if (!ulStatus) return; 
 
    switch (gucStatus) 
    { 
        case STAT_ADDR:                  //    发送数据地址状态 
        if (guiAddrIndex < gtDevice.uiLen)          //    若数据地址未发送完毕 
        {    
            //    继续发送数据地址 
            I2CMasterDataPut(I2CM_BASE, gcAddr[guiAddrIndex++]); 
            //    命令：主机突发发送继续 
            I2CMasterControl(I2CM_BASE, I2C_MASTER_CMD_BURST_SEND_CONT); 
 
            break; 
        } 
        else 
        { 
            gucStatus = STAT_DATA;            //    设置状态：收发数据 
 
            if (gbSendRecv)                //    若是接收操作 
            { 
                //设置从机地址，读操作 
                I2CMasterSlaveAddrSet(I2CM_BASE, gtDevice.ucSLA, true); 
 
                if (gtDevice.uiSize == 1)          //    若只准备接收 1 个字节 
                { 
                    gucStatus = STAT_FINISH;        //    设置状态：接收结束 
 
                    //命令：主机单次接收 
                    I2CMasterControl(I2CM_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE); 
                } 
                else 
                { 
                    //    命令：主机突发接收起始 
                    I2CMasterControl(I2CM_BASE, 
                    I2C_MASTER_CMD_BURST_RECEIVE_START); 
                } 
 
                break; 
            } 
        } 
 
        //    直接进入下一条 case 语句 
 
        case STAT_DATA:                  //    收发数据状态 
        if (gbSendRecv)                  //    若是接收操作 
        { 
            //    读取接收到的数据 
            gtDevice.pcData[guiDataIndex++] = I2CMasterDataGet(I2CM_BASE); 
 
            if (guiDataIndex + 1 < gtDevice.uiSize)       //    若数据未接收完毕 
            { 
                //    命令：主机突发接收继续 
                I2CMasterControl(I2CM_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT); 
            } 
            else 
            { 
                gucStatus = STAT_FINISH;          //    设置状态：接收完成 
 
                //    命令：主机突发接收完成 
                I2CMasterControl(I2CM_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH); 
            } 
        } 
        else 
        { 
             //    发送数据 
            I2CMasterDataPut(I2CM_BASE, gtDevice.pcData[guiDataIndex++]); 
 
            if (guiDataIndex < gtDevice.uiSize)        //    若数据未发送完毕 
            { 
                //    命令：主机突发发送继续 
                I2CMasterControl(I2CM_BASE, I2C_MASTER_CMD_BURST_SEND_CONT); 
            } 
            else 
            { 
                gucStatus = STAT_FINISH;          //    设置状态：发送完成 
 
                //    命令：主机突发发送完成 
                I2CMasterControl(I2CM_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH); 
            } 
    } 
 
    break; 
 
    case STAT_FINISH:                  //    收发完成状态 
    if (gbSendRecv)                  //    若是接收操作 
    { 
        //    读取接最后收到的数据 
        gtDevice.pcData[guiDataIndex] = I2CMasterDataGet(I2CM_BASE); 
    } 
 
    gucStatus = STAT_IDLE;              //    设置状态：空闲 
    break; 
 
    default: 
    break; 
    } 
} 


//方式二:不使用中断
void FMWriteData(BYTE FMSLAVE_ADDR, WORD WRITE_ADDR, BYTE *pDat, BYTE datlen)
{
    BYTE ucIndex;
    I2CMasterInitExpClk(I2C0_MASTER_BASE, SysCtlClockGet(), false); 
    
    //  指定从机地址 
    I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, FMSLAVE_ADDR, 0); //写
   
    //  发送子地址Hbyte
    I2CMasterDataPut(I2C0_MASTER_BASE, (BYTE)WRITE_ADDR>>8); 
 
    //  设置主机状态 
    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_START); 
     
    //  等待数据发送结束 
    while(I2CMasterBusy(I2C0_MASTER_BASE)); 


    //  发送子地址Lbyte
    I2CMasterDataPut(I2C0_MASTER_BASE, (BYTE)WRITE_ADDR); 
    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
    //  等待数据发送结束 
    while(I2CMasterBusy(I2C0_MASTER_BASE)); 
               
    //  发送数据 
    for(ucIndex = 0; ucIndex < datlen; ucIndex++)
    { 
        //  将要发送的数据放到数据寄存器 
        I2CMasterDataPut(I2C0_MASTER_BASE, pDat[ucIndex]); 
 
        //  发送数据命令 
        I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT); 
 
        //  等待数据发送结束
        while(I2CMasterBusy(I2C0_MASTER_BASE)); 
    } 
    // 
    //  发送结束命令 
    // 
    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH); 
 
    // 
    //  等待命令发送结束 
    // 
    while(I2CMasterBusy(I2C0_MASTER_BASE)); 
}

void FMReadData(BYTE FMSLAVE_ADDR, WORD READ_ADDR, BYTE *pDat, BYTE datlen)
{

    BYTE ucIndex;
    //  指定从机地址 
    I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, FMSLAVE_ADDR, 0); //写
   
    //  发送子地址Hbyte 
    I2CMasterDataPut(I2C0_MASTER_BASE, (BYTE)READ_ADDR>>8); 
 
    //  设置主机状态 
    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_START); 
     
    //  等待数据发送结束 
    while(I2CMasterBusy(I2C0_MASTER_BASE)); 


    //  发送子地址Lbyte 
    I2CMasterDataPut(I2C0_MASTER_BASE, (BYTE)READ_ADDR); 
    //  设置主机状态
    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
    
    //  等待数据发送结束 
    while(I2CMasterBusy(I2C0_MASTER_BASE)); 
 

    //  指定从机地址，读
    I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, FMSLAVE_ADDR, 1); 
    //
    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
        
    for(ucIndex = 0; ucIndex < datlen-1; ucIndex++)
    {
        //  等待数据接收完成 
        while(I2CMasterBusy(I2C0_MASTER_BASE));
        pDat[ucIndex]=I2CMasterDataGet(I2C0_MASTER_BASE);
        I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
    }

    //  等待数据接收完成 
    while(I2CMasterBusy(I2C0_MASTER_BASE));
    pDat[ucIndex+1]=I2CMasterDataGet(I2C0_MASTER_BASE);

    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
}
#endif

#include    <hw_ints.h> 
#include    <interrupt.h> 
#include    <sysctl.h> 
#include    <gpio.h> 
#include    "devI2c.h"
#include    "rn8209x.h"

//方式三:使用IO口模拟I2C
//以下为I2C驱动程序
/*************************************************************************/ 
/*************************************************************************/ 
/*************************************************************************/ 
void I2C_FM24C_Init(void)
{

    SysCtlPeripheralEnable(SCL_PERIPH);
    GPIOPinTypeGPIOOutput(SCL_PORT,SCL_PIN); 

    SysCtlPeripheralEnable(SDA_PERIPH);
    GPIOPinTypeGPIOOutput(SDA_PORT,SDA_PIN); 
    
    SCL_H;
    SDA_H;
    SDA_out;
   
}

void I2CGPIOINIT()
{
    GPIOPinTypeGPIOOutput(SCL_PORT,SCL_PIN);
    GPIOPinTypeGPIOOutput(SDA_PORT,SDA_PIN);
    SCL_H;
    SDA_H;
    SDA_out;
    
}
/*******************************************
函数名称：start
功    能：完成IIC的起始条件操作
参    数：无
返回值  ：无
********************************************/
static void start(void)
{
      SCL_H;
      SDA_H;
      SysCtlDelay(5*(TheSysClock/3000000));//一个循环延时占用3个周期
      SDA_L;
      SysCtlDelay(5*(TheSysClock/3000000));
      SCL_L;
      //SysCtlDelay(4*(TheSysClock/3000000));
}
/*******************************************
函数名称：stop
功    能：完成IIC的终止条件操作
参    数：无
返回值  ：无
********************************************/
static void stop(void)
{
      SDA_L;
      SCL_H;
      SysCtlDelay(5*(TheSysClock/3000000));
      SDA_H;
      SysCtlDelay(5*(TheSysClock/3000000));
      SCL_L;
}
/*******************************************
函数名称：mack
功    能：完成IIC的主机应答操作
参    数：无
返回值  ：无
********************************************/
static void mack(void)
{
      SDA_L;
      SysCtlDelay(2*(TheSysClock/3000000));
      SCL_H;
      SysCtlDelay(5*(TheSysClock/3000000));
      SCL_L;
//      SysCtlDelay(5*(TheSysClock/3000000));
      
}
/*******************************************
函数名称：mnack
功    能：完成IIC的主机无应答操作
参    数：无
返回值  ：无
********************************************/
static void mnack(void)
{
      SDA_H;
      SysCtlDelay(2*(TheSysClock/3000000));
      SCL_H;
      SysCtlDelay(5*(TheSysClock/3000000));
      SCL_L;
//      SysCtlDelay(5*(TheSysClock/3000000));
       
}

/**********检查应答信号函数******************/
/*如果返回值为1则证明有应答信号，反之没有*/
/*******************************************
函数名称：check
功    能：检查从机的应答操作
参    数：无
返回值  ：从机是否有应答：1--有，0--无
********************************************/
static void ack(void)
{
    unsigned char  k;
    k=0;
    SDA_in;
    SCL_H;
    SysCtlDelay(5*(TheSysClock/3000000));
    while((GPIOPinRead(SDA_PORT,SDA_PIN)&SDA_PIN)&&(k<250)) k++;
    SDA_out;
    SCL_L;
//    SysCtlDelay(5*(TheSysClock/3000000));
}

/*******************************************
函数名称：write1byte
功    能：向IIC总线发送一个字节的数据
参    数：wdata--发送的数据
返回值  ：无
********************************************/
static void write1byte(unsigned char  wdata)
{
      unsigned char  i;
      
      for(i=0;i<8;i++)
      {
         SCL_L; //SCL=0;
         SysCtlDelay(2*(TheSysClock/3000000));//等待5个机器周期	
         if((wdata&0x80)==0x80)
             SDA_H;//SDA=1;
         else
             SDA_L;//SDA=0;
             
         SCL_H;//SCL=1;
         wdata=wdata<<1;
         SysCtlDelay(3*(TheSysClock/3000000));//等待5个机器周期	

      }
      SCL_L;//SCL=0;
}
/*******************************************
函数名称：read1byte
功    能：从IIC总线读取一个字节
参    数：无
返回值  ：读取的数据
********************************************/
static unsigned char  read1byte(void)
{
#if 1 //SDA,SCL，没有上拉电阻
    unsigned char  rdata = 0x00,i;
    unsigned char  flag;

    for(i = 0;i < 8;i++)
    {
      SDA_H;
      SCL_H;
      
      SDA_in;
      SysCtlDelay(5*(TheSysClock/3000000));
      flag = SDA_val;
      rdata <<= 1;
      if(flag)  rdata |= 0x01;
      SDA_out;
      SCL_L;
      SysCtlDelay(5*(TheSysClock/3000000));
    }    
    return rdata;
#else//SDA,SCL上拉了电阻
    unsigned char retc,i;  
    retc=0; 
 
    SDA_in;
    for(i=0;i<8;i++)
    {        
        SCL_L;//SCL=0;      
        SysCtlDelay(5*(TheSysClock/3000000));//等待5个机器周期
        SCL_H;//SCL=1;       
        SysCtlDelay(5*(TheSysClock/3000000));//等待5个机器周期
            
        retc=retc<<1;
        if((GPIOPinRead(SDA_PORT,SDA_PIN)&SDA_PIN))retc=retc+1; 
        SysCtlDelay(4*(TheSysClock/3000000));//等待5个机器周期	     
    }
    SCL_L;//SCL=0;    
    SysCtlDelay(5*(TheSysClock/3000000));//等待5个机器周期	
    SDA_out;
    return(retc);
#endif
}

//以下为FM24CL04应用程序
/***********************************************************************************/ 
/*因为FM24CL04内部空间大小为512byte,所以是Word 地址,0x0000--0x01FF,但是分***********/ 
/*成了2页，每页256byte大小，对于0x00--0xFF的操作是对第0叶的操作，对于0x0100--0x01FF*/ 
/*的操作是对第1页的操作。在写入操作地址时应注意*************************************/ 
/***********************************************************************************/ 

//多字节数据写入
void  Write_FM24C(BYTE sla,WORD suba,BYTE *s,BYTE no)
{
    unsigned char  i,slabuf;
    unsigned int subabuf;

    I2C_FM24C_Init(); //应为与时钟共用GPIO所以在每次读写前使用前初始化
    
    slabuf = sla;   //A0=0(page=0)
    subabuf = suba;
    if(subabuf >= 0x100)
    {
        slabuf = sla|0x02;//A0=1(page = 1);
    }
    start();             
    write1byte(slabuf);    //发控制字A0,R/W=0 允许写         
    ack();
    write1byte((BYTE)subabuf);      //器件内单元地址,取低8位，           
    ack();
    for(i=0;i<no;i++)
    {   
        write1byte(*s);               
        ack();
        s++;
    } 
    stop();             

} 
//多字节数据读出
void  Read_FM24C(BYTE sla,WORD suba,BYTE *s,BYTE num)
{
    unsigned char  i,slabuf;
    unsigned int subabuf;

    I2C_FM24C_Init();
    
    slabuf = sla;
    subabuf = suba;
    if(subabuf>=0x100)
    {
        slabuf = sla |0x02;//A0=1;page = 1;
    }
    start();             
    write1byte(slabuf);     //发控制字A0,R/W=0 允许写        
    ack();
    write1byte((BYTE)subabuf);      //器件内单元地址 高字节          
    ack();
    
    start();   
    write1byte(slabuf+1);  //发控制字A0,R/W=1 允许读  
    ack();
    
    for(i=0;i<num-1;i++)
    {   
        *s=read1byte();               
        mack();                
        s++;
    } 
    *s=read1byte();
    mnack();                
    stop();                
}

//连续清除多字节数据
void  Clear_FM24C(BYTE sla,WORD suba,BYTE no)
{
    unsigned char  i,slabuf;
    unsigned int subabuf;

    I2C_FM24C_Init();
    
    slabuf = sla;   //A0=0(page=0)
    subabuf = suba;
    if(subabuf>=0x100)
    {
        slabuf = sla|0x02;//A0=1(page = 1);
    }
    start();             
    write1byte(slabuf);    //发控制字A0,R/W=0 允许写         
    ack();
    write1byte((BYTE)subabuf);      //器件内单元地址,取低8位，           
    ack();
    for(i=0;i<no;i++)
    {   
        write1byte(0x00);               
        ack();
    } 
    stop();                 
}

/*************************************************************************************************
**                                  END FILE
**************************************************************************************************/

 
