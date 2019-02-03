#include "tsensor.h"
#include "delay.h"
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//Mini STM32开发板
//ADC 驱动代码			   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2010/6/7 
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 正点原子 2009-2019
//All rights reserved
////////////////////////////////////////////////////////////////////////////////// 	  
 
		   
//初始化ADC
//这里我们仅以规则通道为例
//我们默认将开启通道0~3	
void T_Adc_Init(void)  //ADC通道初始化
{
	ADC_InitTypeDef ADC_InitStructure; 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_ADC1	, ENABLE );	  //使能GPIOA,ADC1通道时钟
  
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //分频因子6时钟为72M/6=12MHz

   	ADC_DeInit(ADC1);  //将外设 ADC1 的全部寄存器重设为缺省值
 
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器

	ADC_TempSensorVrefintCmd(ENABLE); //开启内部温度传感器
	
 
	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1

	ADC_ResetCalibration(ADC1);	//重置指定的ADC1的复位寄存器

    while(ADC_GetResetCalibrationStatus(ADC1));	//获取ADC1重置校准寄存器的状态,设置状态则等待

	ADC_StartCalibration(ADC1);	 //

	while(ADC_GetCalibrationStatus(ADC1));		//获取指定ADC1的校准程序,设置状态则等待
}
u16 T_Get_Adc(u8 ch)   
	{
 
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道3,第一个转换,采样时间为239.5周期	  			    
 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束
	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
	}

//得到ADC采样内部温度传感器的值
//取10次,然后平均
u16 T_Get_Temp(void)
	{
	u16 temp_val=0;
	u8 t;
	for(t=0;t<10;t++)
		{
		temp_val+=T_Get_Adc(ADC_Channel_16);	  //TampSensor
		delay_ms(5);
		}
	return temp_val/10;
	}

 //获取通道ch的转换值
//取times次,然后平均
u16 T_Get_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=T_Get_Adc(ch);
		delay_ms(5);
	}
	return temp_val/times;
} 	   

