#ifndef __SPI_ENC28J60_H
#define __SPI_ENC28J60_H

#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "misc.h"


void SPI1_Init(void);
unsigned char	SPI1_ReadWriteByte(unsigned char writedat);

#endif /* __SPI_ENC28J60_H */
