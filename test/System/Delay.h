#ifndef __DELAY_H
#define __DELAY_H


#include "stm32f10x.h"

void Delay_us(uint32_t us);
void Delay_ms(uint32_t ms);
void Delay_s(uint32_t s);


/* 对外接口 */
uint32_t GetTick(void);


/* 初始化：配置 SysTick 为 1ms 中断 */
void Delay_Init(void);

#endif
