#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"	 
#include "enc28j60.h"
#include "uip.h"
#include "uip_arp.h"
#include "tapdev.h"
#include "timerx.h"				   
#include "math.h" 	
#include "string.h"	
#include "eth0.h"
#include "iap.h"
#include "key.h"

u8 run_flag = 0;     //在stm32接收到主机tcp服务端发送来的run_now字符串后，run_flag将变为1，并将导致IAP_run()函数执行，跳转到用户区执行程序

void All_init()
{
	uip_ipaddr_t ipaddr;
	delay_init();	    	 //延时函数初始化	  
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(9600);	 	//串口初始化为9600 
 	while(tapdev_init())	//初始化ENC28J60错误
	{								   
		printf("ENC28J60 Init Error!");	 
		delay_ms(200);
	};
	uip_init();				//uIP初始化	     						  	  
 	uip_ipaddr(ipaddr, 192,168,0,16);	//设置本地设置IP地址
	uip_sethostaddr(ipaddr);					    
	uip_ipaddr(ipaddr, 192,168,0,1); 	//设置网关IP地址(其实就是你路由器的IP地址)
	uip_setdraddr(ipaddr);						 
	uip_ipaddr(ipaddr, 255,255,255,0);	//设置网络掩码
	uip_setnetmask(ipaddr);
    tcp_client_reconnect();	   		//尝试连接到TCP Server端,用于TCP Clien，这个函数定义连接TCP服务器的地址:端口为192.168.0.103:1400
}




 int main(void)
 {			
    All_init(); 
    printf("init okay");
	while (1)
	{ 	
		uip_polling();	//处理uip事件，必须插入到用户程序的循环体中 
		eth0_receive();
		IAP_update();
		
        if(run_flag)
		IAP_run();	
	}  
} 

