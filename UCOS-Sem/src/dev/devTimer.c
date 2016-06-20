/*************************************************************
成都昊普环保技术有限公司   版权所有
智能水帘新风系统


文件名:
作  者:    潘国义
描  述:    实时钟 HT1381
修订记录: bn

**************************************************************/
#include "includes.h"
#include "Comm.h"

extern BYTE SysTime[7];

void TimerDly(void)
{
   volatile  int i;
    for(i=0;i<20;i++)
        ;
}

/*******************************************************************************
*函数名:
　　()
*功能: 打开时钟
*输入:
*输出:
*说明:
*******************************************************************************/
static void timerSda ( int bHigh )
{
   
    if ( !bHigh )
    {
        signalOut ( GPIO_PORTB_BASE , GPIO_PIN_3,  bHigh );
    }
    else
    {
        signalOut ( GPIO_PORTB_BASE , GPIO_PIN_3 ,GPIO_PIN_3);
    }
  
}

static void timerScl ( int bHigh )
{ 
    GPIOPinWrite ( GPIO_PORTB_BASE,  GPIO_PIN_2 , bHigh? GPIO_PIN_2:0);
}

static int timerGetSda ( void )
{
    int tmp;
    
    tmp = signalIn ( GPIO_PORTB_BASE , GPIO_PIN_3 ) ? 1 : 0;
 
    return tmp;
    
}

static void timerRST ( int bHigh )
{
    GPIOPinWrite ( GPIO_PORTF_BASE,  GPIO_PIN_0 , bHigh? GPIO_PIN_0:0);
}

/*******************************************************************************
*函数名:
　　()
*功能: 写入1byte 
*输入:
*输出:
*说明:
*******************************************************************************/
static void Write_Timer_Byte ( BYTE Data )
{
    static BYTE i;

    timerScl ( 0 );//I2CSCL = 0;
    for ( i = 0; i < 8; i++ )
    {
        if ( ( Data & 0x01 ) == 0 )
        {
            timerSda ( 0 );// I2CSDA = 0; 
        }
        else
        {
            timerSda ( 1 );// I2CSDA = 1; 
        }

        timerScl ( 1 );//I2CSCL = 0; 
        TimerDly();
        timerScl ( 0 );//I2CSCL = 0;
        TimerDly();
        Data = Data >> 1;
    }
}

/*******************************************************************************
*函数名:
　　()
*功能: 读 1 byte
*输入:
*输出:
*说明:
*******************************************************************************/
static BYTE Read_Timer_Byte ( void )
{
    static BYTE InByte = 0, Timer_Count;

    InByte = 0;
    
    for ( Timer_Count = 0; Timer_Count < 8; Timer_Count++ )
    {      
        if ( timerGetSda() )//因为在写完命令后，已经产生了个下降沿，将第一个bit读出了。
        {
            InByte |= ( 1 << Timer_Count );
        }      
        timerScl ( 1 );
        TimerDly(); 
        timerScl ( 0 );// 下降沿读数 
        TimerDly();
        
    }
    
    return ( InByte );
}

/*******************************************************************************
*函数名:
　　()
*功能: 读实时时钟
*输入:
*输出:
*说明:
*******************************************************************************/

void ReadRealtime ( void )
{
    static BYTE i;
 
    timerScl ( 0 );
    timerRST ( 0 );
    TimerDly();
    
    timerRST ( 1 );
    TimerDly();
    Write_Timer_Byte ( 0xBF );    //Read, Burst Mode

    for ( i = 0; i < 7; i++ )
    {
        SysTime[6-i] = Read_Timer_Byte();
    }
    timerScl ( 1 );
    timerRST ( 0 );
    TimerDly();
}

/*******************************************************************************
*函数名:
　　()
*功能: 写实时时钟
*输入:
*输出:
*说明:
*******************************************************************************/
void WriteRealtime ( void )
{
    timerScl ( 0 );
    timerRST ( 0 );
    TimerDly();
    
    timerRST ( 1 );
    TimerDly();
    Write_Timer_Byte ( 0x8E );   //写使能
    Write_Timer_Byte ( 0x00 );    
    timerRST ( 0 );
    TimerDly();
   
    timerRST ( 1 );
    TimerDly();
    Write_Timer_Byte ( 0XBE );       //连续写
    Write_Timer_Byte ( SysTime[6] & 0x7F ); //OSC enable for sure.CH=0;
    Write_Timer_Byte ( SysTime[5] );
    Write_Timer_Byte ( SysTime[4] & 0x7F );   //24 Hour Mode 
    Write_Timer_Byte ( SysTime[3] );
    Write_Timer_Byte ( SysTime[2] );
    Write_Timer_Byte ( SysTime[1]);  // 周
    Write_Timer_Byte ( SysTime[0] );//year
    Write_Timer_Byte ( 0x00 );     
    timerRST ( 0 );
    TimerDly();
    
    timerRST ( 1 );
    TimerDly();
    Write_Timer_Byte ( 0x8E );     //Write,Single Mode
    Write_Timer_Byte ( 0x80 );     // 写保护
    timerRST ( 0 );

}



/*******************************************************************************
*函数名:
　　()
*功能: 初始化
*输入:
*输出:
*说明:
*******************************************************************************/
void Init_Rtc ( void )
{
    timerScl ( 0 );
    timerRST ( 0 );
    TimerDly();
    timerRST ( 1 );
    Write_Timer_Byte ( 0x8E );   //写使能
    Write_Timer_Byte ( 0x00 );   //
    timerRST ( 0 );
    TimerDly();
    
    timerRST ( 1 );
    Write_Timer_Byte ( 0x80 );   //Write,Single Mode
    Write_Timer_Byte ( 0x00 );   //CH=0;晶振起振
    timerRST ( 0 );
   
}
