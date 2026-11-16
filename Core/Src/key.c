#include "key.h"
#include "main.h"

/* 软件消抖：两次边沿间隔小于 25ms 视为抖动丢弃。
   按键接内部上拉 + 下降沿触发，所以"按下"只会产生一次有效事件。 */
#define KEY_DEBOUNCE_MS 25U

static volatile uint32_t s_last_ms[2];
static volatile uint8_t  s_event[2];

void Key_Init(void)
{
    s_last_ms[0] = 0;
    s_last_ms[1] = 0;
    s_event[0]   = 0;
    s_event[1]   = 0;
}

/* 中断回调里只做一件事：记录事件。不解析、不刷屏、不延时。 */
void Key_OnExti(uint16_t pin)
{
    uint8_t id;

    if (pin == GPIO_PIN_11)
    {
        id = KEY_DIR;
    }
    else if (pin == GPIO_PIN_12)
    {
        id = KEY_PAUSE;
    }
    else
    {
        return;
    }

    uint32_t now = HAL_GetTick();
    if ((now - s_last_ms[id]) < KEY_DEBOUNCE_MS)
    {
        return;                 /* 抖动，丢弃 */
    }
    s_last_ms[id] = now;
    s_event[id]   = 1;
}

/* 主循环调用：有事件则返回 1 并清除 */
uint8_t Key_TakeEvent(uint8_t which)
{
    uint8_t e;

    if (which > 1)
    {
        return 0;
    }
    e = s_event[which];
    s_event[which] = 0;
    return e;
}
