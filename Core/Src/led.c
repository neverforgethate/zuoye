#include "led.h"
#include "main.h"

/* 引脚映射表：以后改接线只改这里 */
typedef struct
{
    GPIO_TypeDef *port;
    uint16_t      pin;
} LedPin_t;

static const LedPin_t s_led[LED_COUNT] =
{
    { GPIOA, GPIO_PIN_0 },
    { GPIOA, GPIO_PIN_1 },
    { GPIOA, GPIO_PIN_2 },
};

void LED_Init(void)
{
    LED_Off();
}

void LED_Off(void)
{
    for (uint8_t i = 0; i < LED_COUNT; i++)
    {
        HAL_GPIO_WritePin(s_led[i].port, s_led[i].pin, GPIO_PIN_RESET);
    }
}

/* 高电平点亮：只有 index 那一颗给 SET，其余全部 RESET */
void LED_Show(uint8_t index)
{
    if (index >= LED_COUNT)
    {
        return;
    }
    for (uint8_t i = 0; i < LED_COUNT; i++)
    {
        HAL_GPIO_WritePin(s_led[i].port, s_led[i].pin,
                          (i == index) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

/* dir=0 正向 0->1->2->0 ；dir=1 反向 0->2->1->0 */
uint8_t LED_Next(uint8_t cur, uint8_t dir)
{
    if (cur >= LED_COUNT)
    {
        cur = 0;
    }
    if (dir)
    {
        return (uint8_t)((cur + LED_COUNT - 1) % LED_COUNT);
    }
    return (uint8_t)((cur + 1) % LED_COUNT);
}
