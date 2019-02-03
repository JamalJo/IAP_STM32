#include "tcp_demo.h"
#include "sys.h"
#include "uip.h"
#include <string.h>
#include <stdio.h>	
#include "eth0.h"


#define Perrec_len 1500				//收到的包的最大长度为1500
#define maxrec_len 30*1024

u8 tcp_client_databuf[Perrec_len];   	//发送数据缓存	  
u8 tcp_client_sta;				         //客户端状态
u16 IAP_num = 0;                      //接收到的数据的个数
extern u8 IAP_pro[maxrec_len]; 
extern u8 ready_flag;

//[7]:0,无连接;1,已经连接;
//[6]:0,无数据;1,收到客户端数据
//[5]:0,无数据;1,有数据需要发送

//这是一个TCP 客户端应用回调函数。
//该函数通过UIP_APPCALL(tcp_demo_appcall)调用,实现Web Client的功能.
//当uip事件发生时，UIP_APPCALL函数会被调用,根据所属端口(1400),确定是否执行该函数。
//例如 : 当一个TCP连接被创建时、有新的数据到达、数据已经被应答、数据需要重发等事件
void tcp_client_demo_appcall(void)
{	
  u16 i=0;	
 	struct tcp_demo_appstate *s = (struct tcp_demo_appstate *)&uip_conn->appstate;
	if(uip_aborted())tcp_client_aborted();		//连接终止	   
	if(uip_timedout())tcp_client_timedout();	//连接超时   
	if(uip_closed())tcp_client_closed();		//连接关闭	   
 	if(uip_connected())tcp_client_connected();	//连接成功	    
	if(uip_acked())tcp_client_acked();			//发送的数据成功送达 
 	//接收到一个新的TCP数据包 
	if (uip_newdata()) 
	{
		if((tcp_client_sta&(1<<6))==0)//还未收到数据
		{
			if(uip_len>Perrec_len-1)
			{		   
				((u8*)uip_appdata)[Perrec_len-1]=0;
			}

	   	/********************************************************************************************************/
	    /*   这个部分的程序是用来专门接收.bin文件的，在ready_flag为1后，也就是stm32接收到来自主机的"ready"*/	
	     /*	字符串后，开始将uip_appdata中的数据一个个复制进IAP_pro中去                                     */	
		/********************************************************************************************************/	
			if( ready_flag ) 
			{	   				
			  for(i=0; i<= uip_len-1; i++)
				{
				   IAP_pro[IAP_num] =  (((u8*)uip_appdata)[i]);
				   IAP_num++;
				}
				//如果已经PC服务器已经发送了准备让stm32接收的命令，则准备接收
            }
	   /***************************************************************************************************/
	    
		  strcpy(tcp_client_databuf,uip_appdata); 	          //将 uip_appdata复制到tcp_client_databuf中去，在eth0_receive()函数中会用得到
			
		  tcp_client_sta|=1<<6;//表示收到客户端数据
		}			
	}else if(tcp_client_sta&(1<<5))//有数据需要发送
	{
		s->textptr=tcp_client_databuf;
		s->textlen=strlen((const char*)tcp_client_databuf);
		tcp_client_sta&=~(1<<5);//清除标记
	}  
	//当需要重发、新数据到达、数据包送达、连接建立时，通知uip发送数据 
	if(uip_rexmit()||uip_newdata()||uip_acked()||uip_connected()||uip_poll())
	{
		tcp_client_senddata();
	}
   i = 0;	
}
//这里我们假定Server端的IP地址为:192.168.0.103
//这个IP必须根据Server端的IP修改.
//尝试重新连接
void tcp_client_reconnect()
{
	uip_ipaddr_t ipaddr;
	uip_ipaddr(&ipaddr,192,168,0,103);	//设置IP为192.168.0.103
	uip_connect(&ipaddr,htons(1400)); 	//端口为1400
}
//终止连接				    
void tcp_client_aborted(void)
{
	tcp_client_sta&=~(1<<7);	//标志没有连接
	tcp_client_reconnect();		//尝试重新连接
	uip_log("tcp_client aborted!\r\n");//打印log
}
//连接超时
void tcp_client_timedout(void)
{
	tcp_client_sta&=~(1<<7);	//标志没有连接	   
	uip_log("tcp_client timeout!\r\n");//打印log
}
//连接关闭
void tcp_client_closed(void)
{
	tcp_client_sta&=~(1<<7);	//标志没有连接
	tcp_client_reconnect();		//尝试重新连接
	uip_log("tcp_client closed!\r\n");//打印log
}	 
//连接建立
void tcp_client_connected(void)
{ 
	struct tcp_demo_appstate *s=(struct tcp_demo_appstate *)&uip_conn->appstate;
 	tcp_client_sta|=1<<7;		//标志连接成功
  	uip_log("tcp_client connected!\r\n");//打印log
	s->state=STATE_CMD; 		//指令状态
	s->textlen=0;
	s->textptr="TCP_client Connected Successfully!\r\n";//回应消息
	s->textlen=strlen((char *)s->textptr);	  
}
//发送的数据成功送达
void tcp_client_acked(void)
{											    
	struct tcp_demo_appstate *s=(struct tcp_demo_appstate *)&uip_conn->appstate;
	s->textlen=0;//发送清零
	uip_log("tcp_client acked!\r\n");//表示成功发送		 
}
//发送数据给服务端
void tcp_client_senddata(void)
{
	struct tcp_demo_appstate *s = (struct tcp_demo_appstate *)&uip_conn->appstate;
	//s->textptr:发送的数据包缓冲区指针
	//s->textlen:数据包的大小（单位字节）		   
	if(s->textlen>0)uip_send(s->textptr, s->textlen);//发送TCP数据包	 
}


















