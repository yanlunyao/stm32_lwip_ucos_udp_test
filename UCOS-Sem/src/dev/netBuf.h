
#ifndef __INCnetbufH
#define __INCnetbufH

#include <cmmtype.h>

#define NET_BUF_LEN 256
typedef struct net_buf
{
	uint magicNum;
	uint addr;
	struct net_buf * pNext;     /* pointer to the next buf */
	uint dataLen;
	uint pCommCtrl;
	uchar data[NET_BUF_LEN];
} NET_BUF;

typedef NET_BUF * NET_BUF_ID;

extern uint netBufInit ( uint baseAddr, uint memSize );
extern NET_BUF_ID netBufGet ( void );

extern uint netBufFree ( NET_BUF_ID pBuf );

uint bufAllCountGet ( void );
uint bufFreeCountGet ( void );

#endif  /* __INCnetbufH */

