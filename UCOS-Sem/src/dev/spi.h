
#ifndef __SPI_H__
#define __SPI_H__

#include <hw_types.h>
#include <cmmtype.h>

#define SPI_UNUSED_CHANNEL  12
#define SPI_BIT_RATE_10K    10000
#define SPI_BIT_RATE_100K   100000
#define SPI_BIT_RATE_1M     1000000
#define SPI_16BIT_WIDTH     16
#define SPI_8BIT_WIDTH      8
#define SPI_BIT_WIDTH       SPI_8BIT_WIDTH

void spiInit(void);
void resetSPI(void);
bool spiRead(BYTE channel, uint add, uint *data, BYTE bytes);
bool spiWrite(BYTE channel, uint add, uint data, BYTE bytes);
void spiRegShow(void);
void channelSelect(int channel);


#endif


