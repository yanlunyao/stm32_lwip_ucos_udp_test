/*
* Copyright (c) 2006,迈普（四川）通信技术有限公司
* All rights reserved.
*
* 文件名称： cmm.c
* 文件标识： 见配置管理计划书
* 内容摘要： 简要描述本文件的内容
* 词汇解释:  MCU－Micro Controller Unit的缩写，指代单片机
*            DEV－表示设备，比如路由器

* 原作者：   张德强
* 完成日期： 2008年08月30日
*/

/*
$Log: ctype.h,v $
Revision 1.3  2011/11/03 04:52:06  zhangdq
RH03 cmm根除看门狗问题

Revision 1.2  2011/05/20 08:31:28  zhangdq
RH03 CMM单片机单元测试第四次提交

Revision 1.1  2011/04/29 01:05:26  zhangdq
RH03 CMM单片机首次提交

*/

#ifndef CTYPE_H
#define CTYPE_H
typedef unsigned char tBoolean;
#define  bool  uint

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#ifndef ERROR
#define ERROR  (-1)
#endif



#define BOOLEAN tBoolean


/********************************************************************************************************
*                       Date types(Compiler specific)  数据类型（和编译器相关）                         *
********************************************************************************************************/

#define uint unsigned int
#define uchar unsigned char
#define ushort unsigned short
#define ulong unsigned long

#if 0
typedef unsigned char  uint8;          // Unsigned  8 bit quantity  无符号8位整型变量
typedef signed   char  int8;           // Signed    8 bit quantity  有符号8位整型变量
typedef unsigned short uint16;         // Unsigned 16 bit quantity  无符号16位整型变量
typedef signed   short int16;          // Signed   16 bit quantity  有符号16位整型变量
typedef unsigned int   uint32;         // Unsigned 32 bit quantity  无符号32位整型变量
typedef signed   int   int32;          // Signed   32 bit quantity  有符号32位整型变量
typedef float           fp32;          // Single precision floating point 单精度浮点数（32位长度）
typedef double          fp64;          // Double precision floating point 双精度浮点数（64位长度）


typedef unsigned char  INT8U;                   /* 无符号8位整型变量                        */
typedef signed   char  INT8;                    /* 有符号8位整型变量                        */
typedef unsigned short INT16U;                  /* 无符号16位整型变量                       */
typedef signed   short INT16;                   /* 有符号16位整型变量                       */
typedef unsigned int   INT32U;                  /* 无符号32位整型变量                       */
typedef signed   int   INT32;                   /* 有符号32位整型变量                       */
typedef float          FP32;                    /* 单精度浮点数（32位长度）                 */
typedef double         FP64;                    /* 双精度浮点数（64位长度）                 */
#endif

/*

#define  INT8U unsigned char

#define  uint16 unsigned short
#define  INT16U unsigned short
#define  INT32U unsigned int
#define  uint32 unsigned int


#define  int8   signed char
#define  INT8  signed char

#define  int16   signed short
#define  INT16  signed short

#define  int32   signed int
#define  INT32  signed int

#define fp32      float
#define fp64      double
#define FP32      float
#define FP64      double

*/

#define WORD unsigned short
#define BYTE unsigned char

#define  uint8 unsigned char

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL  0
#endif

#endif

