#include "Delay.h"
#include "stm32f10x_tim.h"     // ← 必须加
#include "stm32f10x_rcc.h"     // ← RCC_APB1PeriphClockCmd 需要
/**
  * @brief  微秒级延时
  * @param  xus 延时时长，范围：0~233015
  * @retval 无
  */
  
  
 /* delay.c */
#include "Delay.h"

/* 全局毫秒计数器 */
volatile uint32_t tickCount = 0;

/* SysTick 中断服务函数（每 1ms 进一次） */
void SysTick_Handler(void)
{
    tickCount++;
}

/* 对外接口 */
uint32_t GetTick(void)
{
    return tickCount;
}

/* 初始化：配置 SysTick 为 1ms 中断 */
void Delay_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_InitStructure;

    /* 1ms 系统节拍 */
    SysTick_Config(SystemCoreClock / 1000);

    /* TIM3 用于微秒延时，72MHz / 72 = 1MHz */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_InitStructure.TIM_Prescaler = 72 - 1;
    TIM_InitStructure.TIM_Period = 0xFFFF;
    TIM_InitStructure.TIM_ClockDivision = 0;
    TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_InitStructure);
}
  
  
  
//void Delay_us(uint32_t xus)
//{
//	SysTick->LOAD = 72 * xus;				//设置定时器重装值
//	SysTick->VAL = 0x00;					//清空当前计数值
//	SysTick->CTRL = 0x00000005;				//设置时钟源为HCLK，启动定时器
//	while(!(SysTick->CTRL & 0x00010000));	//等待计数到0
//	SysTick->CTRL = 0x00000004;				//关闭定时器
//}

void Delay_us(uint32_t xus)
{
uint16_t target;

    if (xus == 0) return;

    TIM_SetAutoreload(TIM3, 0xFFFF);      // 固定为最大值，不再动态改
    TIM_SetCounter(TIM3, 0);
    TIM_Cmd(TIM3, ENABLE);

    target = (uint16_t)xus;

    while (TIM_GetCounter(TIM3) < target);   // 直接比较计数值


    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    TIM_Cmd(TIM3, DISABLE);
}

/**
  * @brief  毫秒级延时
  * @param  xms 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_ms(uint32_t xms)
{
	while(xms--)
	{
		Delay_us(1000);
	}
}
 
/**
  * @brief  秒级延时
  * @param  xs 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_s(uint32_t xs)
{
	while(xs--)
	{
		Delay_ms(1000);
	}
} 
