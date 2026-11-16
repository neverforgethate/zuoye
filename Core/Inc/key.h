#ifndef __KEY_H
#define __KEY_H

#include <stdint.h>

#define KEY_DIR     0   /* 按键1：PA11，切换方向 */
#define KEY_PAUSE   1   /* 按键2：PB12，暂停/继续 */

void    Key_Init(void);
void    Key_OnExti(uint16_t pin);   /* 在 HAL_GPIO_EXTI_Callback 里调用 */
uint8_t Key_TakeEvent(uint8_t which);

#endif
