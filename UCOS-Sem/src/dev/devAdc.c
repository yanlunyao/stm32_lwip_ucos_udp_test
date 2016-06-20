#include <includes.h>
#include <sysctl.h>
#include <hw_adc.h>
#include <hw_types.h>

short gAdcValAry[4];

/***********************************************
    温度传感器1 2 3
    下拉1K  ,温度-20 ~ 100

************************************************/

const short AD_Result_Down[121] =
{
    0xD, 0xE, 0xE, 0xF, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
    0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1D, 0x1E, 0x20, 0x21,
    0x22, 0x24, 0x26, 0x27, 0x29, 0x2B, 0x2D, 0x2F, 0x31, 0x33,
    0x35, 0x37, 0x39, 0x3B, 0x3E, 0x40, 0x43, 0x45, 0x48, 0x4B,
    0x4E, 0x51, 0x54, 0x57, 0x5A, 0x5D, 0x60, 0x64, 0x67, 0x6B,
    0x6E, 0x72, 0x76, 0x7A, 0x7E, 0x82, 0x86, 0x8A, 0x8F, 0x93,
    0x98, 0x9C, 0xA1, 0xA5, 0xAA, 0xAF, 0xB4, 0xB9, 0xBE, 0xC3,
    0xC9, 0xCE, 0xD3, 0xD9, 0xDF, 0xE4, 0xEA, 0xF0, 0xF5, 0xFB,
    0x101, 0x107, 0x10D, 0x113, 0x11A, 0x120, 0x126, 0x12C, 0x133, 0x139,
    0x13F, 0x146, 0x14C, 0x153, 0x159, 0x160, 0x166, 0x16D, 0x173, 0x17A,
    0x181, 0x187, 0x18E, 0x194, 0x19B, 0x1A2, 0x1A8, 0x1AF, 0x1B6, 0x1BC,
    0x1C3, 0x1C9, 0x1D0, 0x1D6, 0x1DD, 0x1E3, 0x1EA, 0x1F0, 0x1F7, 0x1FD,
    0x203
};


const signed char Temp_Value[121] =
{
    -20, -19, -18, -17, -16, -15, -14, -13, -12, -11,

    -10, -9, -8, -7, -6, -5, -4, -3, -2, -1,

    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,

    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,

    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,

    30, 31, 32, 33, 34, 35, 36, 37, 38, 39,

    40, 41, 42, 43, 44, 45, 46, 47, 48, 49,

    50, 51, 52, 53, 54, 55, 56, 57, 58, 59,

    60, 61, 62, 63, 64, 65, 66, 67, 68, 69,

    70, 71, 72, 73, 74, 75, 76, 77, 78, 79,

    80, 81, 82, 83, 84, 85, 86, 87, 88, 89,

    90, 91, 92, 93, 94, 95, 96, 97, 98, 99,

    100
};

#define TEMP_TB_CNT sizeof(Temp_Value)


//#define TEMP_BASE -20

char getTempFromAdcTab ( signed short adc30 )
{
    int i;
    signed short adc = ( adc30 * 3 ) / 5 ; /*由于是5付*/

    //调低默认温度值
    if ( adc < AD_Result_Down[0] )
    {
        return 300;//(Temp_Value[0] - 200);
    }

    //调高默认温度值
    if ( adc > AD_Result_Down[TEMP_TB_CNT -1] )
    {
        return 300;//Temp_Value[TEMP_TB_CNT-1] + 200;
    }



    for ( i = 0; i < ( TEMP_TB_CNT - 1 ); i++ )
    {
        if ( ( adc >= AD_Result_Down[i] ) && ( adc < AD_Result_Down[i+1] ) )
        {
            return Temp_Value[i] ;
        }
    }

    return -100 ;
}


/***********************************************
    温度传感器0 ,
    上拉10K  ,温度-20 ~ 100

************************************************/

const  short AD_Result_Down2[121] =
{

    0x3A7, 0x3A2, 0x39D, 0x397, 0x392, 0x38C, 0x385, 0x37F, 0x378, 0x371,
    0x36A, 0x363, 0x35B, 0x353, 0x34B, 0x342, 0x33A, 0x331, 0x328, 0x31F,
    0x315, 0x30C, 0x302, 0x2F8, 0x2ED, 0x2E3, 0x2D8, 0x2CE, 0x2C3, 0x2B8,
    0x2AD, 0x2A1, 0x296, 0x28B, 0x27F, 0x274, 0x268, 0x25C, 0x251, 0x245,
    0x23A, 0x22E, 0x222, 0x217, 0x20B, 0x200, 0x1F5, 0x1E9, 0x1DE, 0x1D3,
    0x1C8, 0x1BD, 0x1B3, 0x1A8, 0x19E, 0x194, 0x18A, 0x180, 0x176, 0x16C,
    0x163, 0x15A, 0x150, 0x148, 0x13F, 0x136, 0x12E, 0x126, 0x11E, 0x116,
    0x10E, 0x107, 0x100, 0xF9, 0xF2, 0xEB, 0xE4, 0xDE, 0xD8, 0xD2,
    0xCC, 0xC6, 0xC1, 0xBB, 0xB6, 0xB1, 0xAC, 0xA7, 0xA2, 0x9E,
    0x99, 0x95, 0x91, 0x8D, 0x89, 0x85, 0x81, 0x7E, 0x7A, 0x77,
    0x73, 0x70, 0x6D, 0x6A, 0x67, 0x64, 0x62, 0x5F, 0x5C, 0x5A,
    0x57, 0x55, 0x53, 0x51, 0x4E, 0x4C, 0x4A, 0x48, 0x46, 0x45,
    0x43
};


const signed char Temp_Value2[121] =
{

    -20, -19, -18, -17, -16, -15, -14, -13, -12, -11,

    -10, -9, -8, -7, -6, -5, -4, -3, -2, -1,

    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,

    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,

    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,

    30, 31, 32, 33, 34, 35, 36, 37, 38, 39,

    40, 41, 42, 43, 44, 45, 46, 47, 48, 49,

    50, 51, 52, 53, 54, 55, 56, 57, 58, 59,

    60, 61, 62, 63, 64, 65, 66, 67, 68, 69,

    70, 71, 72, 73, 74, 75, 76, 77, 78, 79,

    80, 81, 82, 83, 84, 85, 86, 87, 88, 89,

    90, 91, 92, 93, 94, 95, 96, 97, 98, 99,

    100
};


//#define TEMP_BASE -20
char  getTempFromAdcTab2 ( signed short adc30 )
{
    int i;
    signed short adc = ( adc30 * 30 ) / 33 ; /*由于是3v to 3.3v*/

    //return adc30;
    if ( adc > AD_Result_Down2[0] )
    {
        return Temp_Value2[0];
    }

    if ( adc < AD_Result_Down2[TEMP_TB_CNT -1] )
    {
        return Temp_Value2[TEMP_TB_CNT -1];
    }

    for ( i = 0; i < ( TEMP_TB_CNT - 1 ); i++ )
    {
        if ( ( adc <= AD_Result_Down2[i] ) && ( adc > AD_Result_Down2[i+1] ) )
        {
            return Temp_Value2[i] ;
        }
    }

    return -100 ;
}



/*******************************************************************************
*函数名:
　　()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
void adcInit ( void )
{
    /*
    GPIOPinTypeADC ( GPIO_PORTE_BASE ,
                     GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_5 | GPIO_PIN_4 );
    */
    GPIOPinTypeADC ( GPIO_PORTE_BASE ,
                     GPIO_PIN_7 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_4 );
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_ADC0 );
    //ADCSequenceConfigure ( ADC0_BASE, CMM_ADC_SEQU ,  ADC_TRIGGER_PROCESSOR, 0 );

    HWREG ( ADC0_BASE + ADC_O_ACTSS ) = 0x00; /*disable*/

    HWREG ( ADC0_BASE + ADC_O_IM ) = 0x00;           /*disable int*/
    HWREG ( ADC0_BASE + ADC_O_EMUX ) = 0x00;     /* soft control*/

    HWREG ( ADC0_BASE + ADC_O_SAC ) = 0x03;   /*8次采样平均*/

    HWREG ( ADC0_BASE + ADC_O_SSMUX0 ) =
        ( 0 << 0 ) | ( 1 << 4 ) | ( 2 << 8 ) | ( 3 << 12 )  ;


    HWREG ( ADC0_BASE + ADC_O_SSCTL0 ) = ( 1 << 13 ) |   /*第4个采样序列结束*/
                                         ( 1 << 14 )     /*并置ADCRIS 对应位为1*/
                                         ;//| ( ( uint ) 1 << 31 )   ;  /*第8个读取温度传感器*/

    HWREG ( ADC0_BASE + ADC_O_ACTSS ) = 0x01;   /*enable*/

}





/*******************************************************************************
*函数名:
　　()
*功能:
*输入:
*输出:
*说明:
*******************************************************************************/
void adcGet4Channel ( signed short * shVals )
{
    uint tmp;

    while ( ! ( HWREG ( ADC0_BASE + ADC_O_SSFSTAT0 )  & ( 1 << 8 ) ) )  /*to empty*/
    {
        tmp = HWREG ( ADC0_BASE + ADC_O_SSFIFO0 ) ;
    }

    HWREG ( ADC0_BASE + ADC_O_ISC )  = ( 1 << 0 );
    HWREG ( ADC0_BASE + ADC_O_OSTAT )  = ( 1 << 0 );
    HWREG ( ADC0_BASE + ADC_O_USTAT )  = ( 1 << 0 );

    HWREG ( ADC0_BASE + ADC_O_PSSI )  = ( 1 << 0 ); /*启动采样*/
    //SSOP0 =  HWREG ( ADC0_BASE + ADC_O_SSOP0 )  ;

    while ( ! ( HWREG ( ADC0_BASE + ADC_O_RIS )  & ( 1 << 0 ) ) )
    {
    };

    tmp = 0;

    do
    {

#if 0
        float v;

        uint val = HWREG ( ADC0_BASE + ADC_O_SSFIFO0 );

        v = val;
        v = v * 3.0 / 1023.0;
        v = v * 1000.0;
        voltages[tmp] = v ;
#else
        uint val = HWREG ( ADC0_BASE + ADC_O_SSFIFO0 );
        shVals[tmp] = val ;
#endif

        tmp++;
    }
    while ( ! ( HWREG ( ADC0_BASE + ADC_O_SSFSTAT0 )  & ( 1 << 8 ) ) ) ;

}


short  adcToVoltMs ( signed short sh )
{
    float v;

    signed short retsh;

    v = sh;
    v = v * 3.0 / 1023.0;
    v = v * 1000.0;
    retsh =  v ;

    return retsh;
}

char getHUM ( signed short adcVal )
{
    signed short tmp;
    signed short Vms = adcToVoltMs ( adcVal );

    tmp = Vms / 30 ;

    if ( tmp >= 100 )
        tmp = 99;

    if ( tmp < 0 )
        tmp = 0;

    return tmp;
}


void adcVotsGet ( void )
{
    signed short tmp[8];
    adcGet4Channel ( tmp );

    gAdcValAry[0] = tmp[0];
    gAdcValAry[1] = tmp[1];
    gAdcValAry[2] = tmp[2];
    gAdcValAry[3] = tmp[3];

    return;
}



