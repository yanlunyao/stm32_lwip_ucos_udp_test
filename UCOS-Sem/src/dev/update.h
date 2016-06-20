
#ifndef __UPDATE_H__
#define __UPDATE_H__

#define ADDR_BIN_LENGTH     (4 * 8)
#define ADDR_BIN_PASSWORD   (4 * 9)
#define ADDR_BIN_CIPHER     (4 * 9)
#define ADDR_BIN_CRC        (4 * 10)
#define ADDR_BIN_VER        (4 * 13)


//#define SYSPARAMLEN      252   // 必须是4的倍数

#define CMM_UPDATE_PKT_SIZE     128
#define UPDATE_SERVER_PORT      5500

#define PACKTYPE_RESET_MCU  0x03
#define PACKTYPE_GET_INFO   0x04
#define PACKTYPE_UP_MCU     0x05

#define CMM_UPDATE_REUSLT_OK                    0
#define CMM_UPDATE_REUSLT_IDFAIL                1
#define CMM_UPDATE_REUSLT_CHKFAIL               2
#define CMM_UPDATE_REUSLT_OVER                  3
#define CMM_UPDATE_REUSLT_NOT_AGAIN             4
#define CMM_UPDATE_REUSLT_PROGRAM_FAIL          5
#define CMM_UPDATE_REUSLT_CONTENT_FAIL          6
#define CMM_UPDATE_REUSLT_NOT_READY             7

#define COMM_PACK_HEAD_FLAG      0xf0
#define COMM_PACK_HEAD_ACK       0xf1

#define ADDR_IMAGE_FLAG  ( 0x1c )
#define ADDR_IMAGE_SIZE  (0x00F0)
#define ADDR_IMAGE_XOR   (ADDR_IMAGE_SIZE + 4)
#define ADDR_IMAGE_ADD   (ADDR_IMAGE_SIZE+ 8)
#define CMM_IMG_FLAG_VAL 0x10102020

#define FLASH_BANK_SIZE      1024
#define TEMP_MCU_IMAGE_ADDR  (1024 * 127)  //127K

#define SYS_JUST_ONLY_RESET      0x0F
#define SYS_UPDATE_AND_RESET     0xF0

typedef  struct
{
	uchar packHead; //0xf0普通报文 0xf1:应答报文
	uchar packType; //功能号
	uchar packLen;  //整个报文长度
	uchar chksum;   //保证整个报文异或为0
	uchar caller[4];    //ios调用者信息
	uchar SlotId;   //数据目标地址(下行)或数据源地址(上行)
	uchar data[];
} t_cmmCommPack;


typedef struct
{
    uchar packId[3];
    uchar maxPackId[3];
    uchar packdata[CMM_UPDATE_PKT_SIZE];
} tUpdatePack;

typedef struct
{
    uchar result;
    uchar nextPackId[3];
    uchar reserved[3];
} tUpdatePackRet;

extern void updatePeriodChk(void);
extern void upDateServerInit( void );
extern void copyMcuImage ( uint dst , uint src);
extern uint ramProgramFlash( ulong ulAddress, ulong *pulData, ulong ulCount );
extern void RamUpdateApp(void);
extern void RamDecodeApp(ulong Offset);
    
#endif 

