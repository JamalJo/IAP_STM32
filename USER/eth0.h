


#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"	 
#include "enc28j60.h"
#include "uip.h"
#include "uip_arp.h"
#include "tapdev.h"
#include "timer.h"				   
#include "math.h" 	
#include "string.h"	

void eth0_receive(void);                     //处理EN280J接收到的信息
void eth0_send(unsigned char* eth0_string);	  //通过EN280J发送信息
void uip_polling(void);						  //uip协议栈的函数，必须在主函数的无限循环中

void IAP_run(void);							//执行烧写到flash中的用户程序
void IAP_update(void);	 					//将收到的.bin文件烧写到FLASH中