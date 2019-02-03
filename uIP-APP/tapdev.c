/*
 * Copyright (c) 2001, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: tapdev.c,v 1.8 2006/06/07 08:39:58 adam Exp $
 */	    
#include "tapdev.h"
#include "uip.h"
#include "enc28j60.h"
//////////////////////////////////////////////////////////////////////////////////	 
//ALIENTEK战舰STM32开发板
//uIP与ENC28J60的底层接口 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/28
//版本：V1.0			   								  
//////////////////////////////////////////////////////////////////////////////////

//用于固定IP地址开关打开后的IP设置，本例程没有用这个
#define UIP_DRIPADDR0   192
#define UIP_DRIPADDR1   168
#define UIP_DRIPADDR2   1
#define UIP_DRIPADDR3   15

//MAC地址,必须唯一
//如果你有两个战舰开发板,想连入路由器,则需要修改MAC地址不一样!
const u8 mymac[6]={0x04,0x02,0x35,0x00,0x00,0x01};	//MAC地址
																				  
//配置网卡硬件，并设置MAC地址 
//返回值：0，正常；1，失败；
u8 tapdev_init(void)
{   	 
	u8 i,res=0;					  
	res=ENC28J60_Init((u8*)mymac);	   //初始化ENC28J60					  

 	for (i = 0; i < 6; i++)uip_ethaddr.addr[i]=mymac[i];   	//把IP地址和MAC地址写入缓存区
  
	ENC28J60_PHY_Write(PHLCON,0x0476);     //指示灯状态:0x476 is PHLCON LEDA(绿)=links status, LEDB(红)=receive/transmit
                                        	//PHLCON：PHY 模块LED 控制寄存器	    
	return res;	
}
//读取一包数据  
uint16_t tapdev_read(void)
{	
	return  ENC28J60_Packet_Receive(MAX_FRAMELEN,uip_buf);
}
//发送一包数据  
void tapdev_send(void)
{
	ENC28J60_Packet_Send(uip_len,uip_buf);
}












/*---------------------------------------------------------------------------*/
