/**
  ******************************************************************************
  * @file    main.c
  ******************************************************************************
	* 实验平台:stm32开发板+W5500模块
	* 程序包含实现静态和动态IP获取，同时建立1个server和一个client端。
	*当LED(PB8)闪烁时，模块会往已建立的链接发送数据
	*RST    PA2
	*NIT    PA3
	*SCS    PA4
	*SCLK   PA5
	*MISO   PA6
	*MOSI   PA7	
	*LED     PB8	
	*USART1_TX		        PA9	// out
	*USART1_RX		        PA10	// in
	*
	*淘宝  :http://shop118467498.taobao.com
  ******************************************************************************
  */ 
#include "mcu_init.h"
#include "config.h"
#include "device.h"
#include "spi2.h"
#include "socket.h"
#include "w5500.h"
#include "at24c16.h"
#include "util.h"
#include "dhcp.h"
#include "stm32f10x_gpio.h"

#include <stdio.h>

#define __USE_DHCP__  //注释该语句使用静态ip，否则为动态ip

extern uint8 txsize[];
extern uint8 rxsize[];
extern vuint32 sec;
uint32 presec;

uint8 buffer[2048];/*定义一个2KB的缓存*/

uint8 getIP_DHCPS(void) ;
void LED_Init(void);
void LED_OFF(void);
void LED_OFF(void);

int main(void)
{
	uint16 pc_port=6001;/*定义计算机的一个端口并初始化*/
	uint16 port=30000;/*定义一个端口并初始化，模块做server*/
	uint16 rmport=60001;/*定义一个端口并初始化，模块做server*/
  uint8 pc_ip[4]={192,168,2,100};/*配置服务器的IP地址,模块做client，请设置成对应电脑IP*/
	uint16 anyport=30001;/*定义一个任意端口并初始化*/
	uint16 len=0;
  RCC_Configuration(); /* 配置单片机系统时钟*/
  GPIO_Configuration();/* 配置GPIO*/
  NVIC_Configuration();/* 配置嵌套中断向量*/
  Timer_Configuration();
  Systick_Init(72);/* 初始化Systick工作时钟*/
  USART1_Init(); /*初始化串口通信:115200@8-n-1*/
  printf("W5500 EVB initialization over.\r\n");
  
  Reset_W5500();/*硬重启W5500*/
  WIZ_SPI_Init();/*初始化SPI接口*/
  printf("W5500 initialized!\r\n");
  set_default(); //不能省略,如果静态获取IP,需改成对应路由器可分配的对应IP
  LED_Init();
  LED_OFF();
#ifdef __USE_DHCP__	
  init_dhcp_client();
  if(getIP_DHCPS()){
	  printf("DHCP OK \r\n");
		set_dhcp_network();//把dhcp获取的ip拷贝到要写入的ip结构体		
  }
#endif	
	set_network();
  printf("Network is ready.\r\n");
	socket(0,Sn_MR_TCP,port,Sn_MR_ND);/*打开socket0的一个端口*/
	socket(1,Sn_MR_TCP,anyport,Sn_MR_ND);/*打开socket0的一个端口*/
	socket(2,Sn_MR_UDP,30000,0);/*初始化socket 2的套接字*/
  printf("creating server with port %d .\r\n",port); 	
	printf("conect server with port %d .\r\n",rmport); 
  while(1)
  {
		  switch(getSn_SR(0))/*获取socket0的状态*/
      {
         case SOCK_INIT:/*socket初始化完成*/
           listen(0);/*在TCP模式下向服务器发送连接请求*/
           break;
         case SOCK_ESTABLISHED:/*socket连接建立*/
           if(getSn_IR(0) & Sn_IR_CON)
           {
              setSn_IR(0, Sn_IR_CON);/*Sn_IR的第0位置1*/
            }
           len=getSn_RX_RSR(0);/*len为已接收数据的大小*/
           if(len>0)
           {
              recv(0,buffer,len);/*W5200接收来自Sever的数据*/
              send(0,buffer,len);/*W5200向Server发送数据*/
            }
           break;
         case SOCK_CLOSE_WAIT:/*socket等待关闭状态*/
           break;
         case SOCK_CLOSED:/*socket关闭*/
           socket(0,Sn_MR_TCP,port,Sn_MR_ND);/*打开socket0的一个端口*/
           break;
				 default:
					// printf("err\r\n");
				 break;
     }
		 switch(getSn_SR(1))/*获取socket0的状态*/
    {
       case SOCK_INIT:/*socket初始化完成*/
         connect(1, pc_ip ,rmport);/*在TCP模式下向服务器发送连接请求*/ 
         break;
       case SOCK_ESTABLISHED:/*socket连接建立*/
         if(getSn_IR(1) & Sn_IR_CON)
         {
            setSn_IR(1, Sn_IR_CON);/*Sn_IR的第0位置1*/
          }
        
         len=getSn_RX_RSR(1);/*len为已接收数据的大小*/
         if(len>0)
         {
            recv(1,buffer,len);/*W5500接收来自Sever的数据*/
            printf("receive from pc\r\n");
            send(1,buffer,len);/*W5500向Server发送数据*/
            printf("send to pc\r\n");
          }
         break;
       case SOCK_CLOSE_WAIT:/*socket等待关闭状态*/
         break;
       case SOCK_CLOSED:/*socket关闭*/
         socket(1,Sn_MR_TCP,anyport,Sn_MR_ND);/*打开socket0的一个端口*/
         break;
     }
		    switch(getSn_SR(2))/*获取socket 0的状态*/
      {
         case SOCK_UDP:/*socket 的套接字初始化完成*/
         //  counter++;
           if(getSn_IR(2) & Sn_IR_RECV)
          {
            setSn_IR(2, Sn_IR_RECV);/*Sn_IR的第0位置1*/
          }
           if((len=getSn_RX_RSR(2))>0)
           { 
              len=recvfrom(2, buffer, len, pc_ip,&pc_port);/*W5500接收计算机发送来的数据*/
              printf("%d.%d.%d.%d:%d\r\n", pc_ip[0],pc_ip[1],pc_ip[2],pc_ip[3],pc_port);
              if(len != sendto(2, buffer,len, pc_ip, pc_port)) 
              { 
                  printf("Sendto Fail\r\n"); 
              } 
            }
           break;
         case SOCK_CLOSED:/*socket 关闭*/
           socket(2,Sn_MR_UDP,30000,0);/*初始化socket 0的套接字*/
           break;
       }
	  if(sec!=presec){
			send(0,"receive data from w5500 server: Thank you for using ChuangWei products\r\n",72);/*W5500向clinet发送数据*/
			send(1,"receive data from w5500 client: Thank you for using ChuangWei products\r\n",72);/*W5500向Server发送数据*/
		  presec=sec;
		  GPIOB->ODR ^= GPIO_Pin_8;//LED取反
		}
  }
}
/*
*********************************************************************************************************
*              
*
* Description: Get an IP from the DHCP server.
* Returns    : 0 : timeout, 1 : get dhcp ip
* Note       : 
*********************************************************************************************************
*/
uint8 getIP_DHCPS(void) 
{
  while(1)
  {
    uint8 dhcpret = check_DHCP_state(SOCK_DHCP);
    switch(dhcpret)
    {
      case DHCP_RET_NONE:
//				printf("*****NO_NET_TEST\r\n");
        break;
      case DHCP_RET_TIMEOUT:
        break;
      case DHCP_RET_UPDATE:
        return 1;
      case DHCP_RET_CONFLICT:
        while(1);
      default:
        break;
    }
  }
}
/****************************************************
函数功能初始LED
****************************************************/
void LED_Init(void)
{
	  GPIO_InitTypeDef GPIO_InitStructure;

		 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);
   
    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init( GPIOB, &GPIO_InitStructure);
}

/****************************************************
函数功能：LED开
****************************************************/
void LED_ON(void)
{
	  GPIO_ResetBits(GPIOB, GPIO_Pin_8);
}

/****************************************************
函数功能：LED关
****************************************************/
void LED_OFF(void)
{
	  GPIO_SetBits(GPIOB, GPIO_Pin_8);
}


