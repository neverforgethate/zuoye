#ifndef __LED_H
#define __LED_H

#include <stdint.h>

#define LED_COUNT 3

void    LED_Init(void);
void    LED_Off(void);
void    LED_Show(uint8_t index);              /* 同一时刻只点亮一颗 */
uint8_t LED_Next(uint8_t cur, uint8_t dir);   /* 计算下一颗编号 */

#endif
