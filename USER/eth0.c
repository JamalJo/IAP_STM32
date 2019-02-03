/*
TCP客户端主动发送数据和接收数据（在主循环中接收数据）
*/
#include "eth0.h"
#include "iap.h"

#define maxrec_len 30*1024	                                     //表示接收的IAP_Pro[]的最大长度为30KB,可以加大到50KB,但要小于stm32的RAM大小64KB
u8 IAP_pro[maxrec_len] __attribute__ ((at(0X20001000)));         //这表示IAP_Pro的地址为 0X20001000
u8 over_flag =0;				 //stm32接收到“over”字符串后，将此标志位置1,表示接收完成，IAP_num不再因接收到的数据而增加保证了IAP_num的正确性
u8 tcp_client_tsta=0XFF;		//用来对比确认客户端状态
u8  Update_flag  = 0;           //当ready_flag && over_flag同时为1时，Update_flag将变为1这将导致IAP_update()程序运行，将接收到的代码写入flash
u8 ready_flag = 0;              //stm32接受到“ready”字符串后，ready_flag将变为1，将导致stm32开始准备接收程序

extern u8 ready_flag;
extern u16 IAP_num ;
extern u8  Update_flag;
extern u8 run_flag;

/*stm32作为客户端，接收来自pc主机的信号 */

void eth0_receive()
{			
	/*              在这儿处理数据                */		
   if( ready_flag && over_flag)         
	 {  		 
	   // printf("%s",IAP_pro);
		 ready_flag = 0;
		 over_flag =  0;
         Update_flag = 1;		 
	 }
	
	/*客户端收到数据，并在这里处理一些数据*/
		if(tcp_client_tsta != tcp_client_sta)//TCP Client状态改变
	{				
		if(tcp_client_sta&(1<<6))	       //收到新数据
		{ 

	/*********************************************************************************************************************/
	/******************************在以下部分处理接收过来一个TCP包，判断接收到的TCP包中是否有"ready"，"over"，"run_now" *******/
	/**************************tcp_client_databuf是直接用strcpy（）函数从uip_appdata复制过来的********************/ 
      if(!strcmp("ready",tcp_client_databuf))        //收到ready信号
			{
				ready_flag = 1;
				printf("preparing");
      }
			  if(!strcmp("over",tcp_client_databuf))        //收到ready信号
			{
				over_flag = 1;
				printf("final data:%s",tcp_client_databuf);
				printf("UPdate");
           }
           		  if(!strcmp("run_now",tcp_client_databuf))        //收到ready信号
			{
				run_flag = 1;   
           }

	/*********************************************************************************************************************/	
	/*********************************************************************************************************************/		
			tcp_client_sta&=~(1<<6);		                      //标记数据已经被处理		
		}
		tcp_client_tsta=tcp_client_sta;
	} 
	
	//printf("uip_timer: %d",uip_timer);
}
	
			

/*stm32作为tcp客户端向主机发送数据*/
			
void eth0_send(unsigned char* eth0_string)
{
		if(tcp_client_sta&(1<<7))	//连接还存在
	{
		sprintf((char*)tcp_client_databuf,eth0_string);	 
		printf("tcp_client_databuf:%s",tcp_client_databuf);//显示当前发送数据
		tcp_client_sta|=1<<5;//标记有数据需要发送
	} 
}	



#define BUF ((struct uip_eth_hdr *)&uip_buf[0])	
//uip事件处理函数
//必须将该函数插入用户主循环,循环调用.
void uip_polling(void)
{
	u8 i;
	static struct timer periodic_timer, arp_timer;
	static u8 timer_ok=0;	 
	if(timer_ok==0)//仅初始化一次
	{
		timer_ok = 1;
		timer_set(&periodic_timer,CLOCK_SECOND/2);  //创建1个0.5秒的定时器 
		timer_set(&arp_timer,CLOCK_SECOND*10);	   	//创建1个10秒的定时器 
	}				 
	uip_len=tapdev_read();	//从网络设备读取一个IP包,得到数据长度.uip_len在uip.c中定义
	if(uip_len>0) 			//有数据
	{   
		//处理IP数据包(只有校验通过的IP包才会被接收) 
		if(BUF->type == htons(UIP_ETHTYPE_IP))//是否是IP包? 
		{
			uip_arp_ipin();	//去除以太网头结构，更新ARP表
			uip_input();   	//IP包处理
			//当上面的函数执行后，如果需要发送数据，则全局变量 uip_len > 0
			//需要发送的数据在uip_buf, 长度是uip_len  (这是2个全局变量)		    
			if(uip_len>0)//需要回应数据
			{
				uip_arp_out();//加以太网头结构，在主动连接时可能要构造ARP请求
				tapdev_send();//发送数据到以太网
			}
		}else if (BUF->type==htons(UIP_ETHTYPE_ARP))//处理arp报文,是否是ARP请求包?
		{
			uip_arp_arpin();
 			//当上面的函数执行后，如果需要发送数据，则全局变量uip_len>0
			//需要发送的数据在uip_buf, 长度是uip_len(这是2个全局变量)
 			if(uip_len>0)tapdev_send();//需要发送数据,则通过tapdev_send发送	 
		}
	}else if(timer_expired(&periodic_timer))	//0.5秒定时器超时
	{
		timer_reset(&periodic_timer);		//复位0.5秒定时器 
		//轮流处理每个TCP连接, UIP_CONNS缺省是40个  
		for(i=0;i<UIP_CONNS;i++)
		{
			uip_periodic(i);	//处理TCP通信事件  
	 		//当上面的函数执行后，如果需要发送数据，则全局变量uip_len>0
			//需要发送的数据在uip_buf, 长度是uip_len (这是2个全局变量)
	 		if(uip_len>0)
			{
				uip_arp_out();//加以太网头结构，在主动连接时可能要构造ARP请求
				tapdev_send();//发送数据到以太网
			}
		}
#if UIP_UDP	//UIP_UDP 
		//轮流处理每个UDP连接, UIP_UDP_CONNS缺省是10个
		for(i=0;i<UIP_UDP_CONNS;i++)
		{
			uip_udp_periodic(i);	//处理UDP通信事件
	 		//当上面的函数执行后，如果需要发送数据，则全局变量uip_len>0
			//需要发送的数据在uip_buf, 长度是uip_len (这是2个全局变量)
			if(uip_len > 0)
			{
				uip_arp_out();//加以太网头结构，在主动连接时可能要构造ARP请求
				tapdev_send();//发送数据到以太网
			}
		}
#endif 
		//每隔10秒调用1次ARP定时器函数 用于定期ARP处理,ARP表10秒更新一次，旧的条目会被抛弃
		if(timer_expired(&arp_timer))
		{
			timer_reset(&arp_timer);
			uip_arp_timer();
		}
	}
}


/****************************************************IAP接收到程序后写入Flash************************************************************/

extern u8  Update_flag;
extern u8 IAP_pro[]; 
extern u16 IAP_num;

												
void IAP_update()
{
	u16 i;
	if(Update_flag)
	{
		if(((*(vu32*)(0X20001000+4))&0xFF000000)==0x08000000)//在0X20001000这个地址，也就是IAP_Pro[]数组中，判断是否为0X08XXXXXX.
			{	 											 //使用Ultraedit可以查看到，在接收到的.bin文件中可以看到0x08XXXXXX
				IAP_num = IAP_num - 4 - 1;  //减去4是用来减去多接收的的“over” ，减去1是用来减去在多加的一个1，在tcp_client_demo_appcall（）中可以看到
				printf("total num:%d\r\n",IAP_num);	
				iap_write_appbin(FLASH_APP1_ADDR,IAP_pro,IAP_num);//更新FLASH代码

									
		/***************************打印发送过去的bin文件信息******************************/
//        for (i=0;i <= IAP_num; i++)
//       {	
//        if(!(i%10))
//        printf("\r\n");					
//		printf("%02x",IAP_pro[i]);
//	    }
//	    printf("\r\n");	
//		printf("total num:%d\r\n",IAP_num+1);
	  /***************************打印发送过去的bin文件信息******************************/

			 
		memset(IAP_pro, '\0', sizeof(char));
		IAP_num = 0; 
		delay_ms(100);		
			 }
		else
			{	  
			 
	          printf("not okay\r\n");		
			}
				
	  Update_flag = 0;	     //将更新标志置0
	 }
	
}


/****************************************************IAP写入应用程序后，开始执行************************************************************/

void IAP_run()							
{   
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, DISABLE);      //时钟关闭使能,否则会导致程序写入后，无法正常执行，这一步非常关键
															       //跳转程序前，一定要关闭stm32的中断
		if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//判断FLASH_APP1_ADDR这个地址上写的.bin文件
		{														  //中的，第四个数字是否为0X08XXXXXX.，以此来判断用户程序写入是否正确
			iap_load_app(FLASH_APP1_ADDR);//执行FLASH APP代码
			
		}else 
		{
			printf("not flash app ,can't run\r\n"); 
		}									  
}


