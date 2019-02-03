#include "spi_enc28j60.h"

/*
 * 函数名：SPI1_Init
 * 描述  ：ENC28J60 SPI 接口初始化
 * 输入  ：无 
 * 输出  ：无
 * 返回  ：无
 */																						  
void SPI1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;	
     NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		   // 复用输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);
   
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	   // 推免输出
   GPIO_Init(GPIOB, &GPIO_InitStructure);						  
   GPIO_SetBits(GPIOB, GPIO_Pin_12);										 // 先把片选拉高，真正用的时候再拉低 
//
//     /* PC6:ENC28J60_INT */						// 中断引脚
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	   // 浮空输入
//    GPIO_Init(GPIOC, &GPIO_InitStructure);						 
//    
    /* NVIC设置 */
    /* Configure one bit for preemption priority */
      NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);  
//    /* Enable the USART1 Interrupt */
//    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//    NVIC_Init(&NVIC_InitStructure); 
//
//    /* 外部中断设置 */
//    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource6);
//    EXTI_InitStructure.EXTI_Line = EXTI_Line6;
//    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
//    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//    EXTI_Init(&EXTI_InitStructure);

	/* SPI2 配置 */ 
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &SPI_InitStructure);
	
	/* 使能 SPI2 */
	SPI_Cmd(SPI2, ENABLE); 
}


unsigned char SPI1_ReadWriteByte(unsigned char writedat)
{
	/* Loop while DR register in not emplty */
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE) == RESET);
	
	/* Send byte through the SPI1 peripheral */
	SPI_I2S_SendData(SPI2, writedat);
	
	/* Wait to receive a byte */
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
	
	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(SPI2);
}

