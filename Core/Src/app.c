#include "app.h"
#include "led.h"

AppState_t g_app;

/* 状态变化标志：由中断/主循环置位，主循环取走后负责刷新 OLED 和串口反馈 */
static volatile uint8_t s_changed;

void App_Init(void)
{
    g_app.run     = 1;      /* 上电默认：运行、正向、500ms、第 1 颗 */
    g_app.dir     = 0;
    g_app.step_ms = 500;
    g_app.led     = 0;
    s_changed     = 1;
    LED_Init();
    LED_Show(g_app.led);
}

/* 串口数据包校验通过后调用：一次更新运行/方向/间隔三项。
   当前 LED 编号保持不变（作业要求：暂停期间的修改恢复后生效） */
void App_Apply(uint8_t run, uint8_t dir, uint16_t step_ms)
{
    g_app.run     = (run != 0) ? 1 : 0;
    g_app.dir     = (dir != 0) ? 1 : 0;
    g_app.step_ms = step_ms;
    s_changed     = 1;
}

void App_ToggleDir(void)
{
    g_app.dir ^= 1;
    s_changed  = 1;
}

void App_ToggleRun(void)
{
    g_app.run ^= 1;
    s_changed  = 1;
}

uint8_t App_TakeChanged(void)
{
    uint8_t c = s_changed;
    s_changed = 0;
    return c;
}
