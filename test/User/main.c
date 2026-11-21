#include "stm32f10x.h"                  // Device header,
#include "Delay.h"
#include "OLED.h"
#include "usart.h"
#include "eth.h"
#include "tcp.h"
#include "udp.h"




#define RX_BUF_SIZE  256

extern uint8_t  rxBuffer[RX_BUF_SIZE];
extern uint8_t  rxIndex;

//UDP
// 对端IP和端口号
uint8_t srcIP[4] = {192,168,44,111};
uint16_t srcPort = 8888;

uint8_t rxBuff[1024];
uint16_t rxLen;

//电脑-串口-》stm32->udp->网线->电脑-》udp网线返回数据


int main(void)
{
	/*模块初始化*/

	
    Delay_Init();
	USART1_Init();
	
    printf("串口打印初始化完成/r/n");
	
	ETH_Init();
	printf("以太网初始化完成/r/n");
	
	while (1)
	{
		
		
	    UDP_Start();
		UDP_RecvData(rxBuff, &rxLen, srcIP, &srcPort);

		// 判断如果长度大于0，表示读取到数据，就原样发回去
		if (rxLen > 0)
		{
			printf("收到数据：%.*s\n", rxLen, rxBuff);

			UDP_SendData(rxBuff, rxLen, srcIP, srcPort);

			// 数据长度清0
			rxLen = 0;
		}
		
	}
}



