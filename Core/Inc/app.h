#ifndef __APP_H
#define __APP_H

#include <stdint.h>

/* 全工程唯一的运行状态。按键、串口、OLED、串口反馈都读这一份，保证三处一致。 */
typedef struct
{
    uint8_t  run;        /* 0=暂停  1=运行 */
    uint8_t  dir;        /* 0=正向  1=反向 */
    uint16_t step_ms;    /* 相邻 LED 切换间隔，100~2000 ms */
    uint8_t  led;        /* 当前点亮的 LED 编号 0~2 */
} AppState_t;

extern AppState_t g_app;

void    App_Init(void);
void    App_Apply(uint8_t run, uint8_t dir, uint16_t step_ms);
void    App_ToggleDir(void);
void    App_ToggleRun(void);
uint8_t App_TakeChanged(void);   /* 返回并清除"状态有变化"标志 */

#endif
