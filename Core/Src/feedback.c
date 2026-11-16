#include "feedback.h"
#include "app.h"
#include "main.h"
#include "usart.h"
#include <stdio.h>

/* 用 HAL_UART_Transmit_IT 发送。发送期间 busy=1；
   这期间若有新变化，s_dirty 会被重新置位，等下一轮再发最新状态，
   避免同时启动两次 IT 发送（HAL 会返回 HAL_BUSY）。 */
static char             s_buf[64];
static volatile uint8_t s_busy;
static volatile uint8_t s_dirty;

void FB_Init(void)
{
    s_busy  = 0;
    s_dirty = 0;
}

void FB_MarkDirty(void)
{
    s_dirty = 1;
}

void FB_Poll(void)
{
    int n;

    if (s_busy || !s_dirty)
    {
        return;
    }

    /* 作业示例格式：STATE=RUN,DIR=FWD,STEP=300,LED=1 */
    n = snprintf(s_buf, sizeof(s_buf),
                 "STATE=%s,DIR=%s,STEP=%u,LED=%u\r\n",
                 g_app.run ? "RUN" : "STOP",
                 g_app.dir ? "REV" : "FWD",
                 (unsigned int)g_app.step_ms,
                 (unsigned int)(g_app.led + 1));

    if (n <= 0)
    {
        s_dirty = 0;
        return;
    }
    if (n > (int)sizeof(s_buf) - 1)
    {
        n = (int)sizeof(s_buf) - 1;      /* 理论上不会发生，兜底 */
    }

    s_dirty = 0;
    if (HAL_UART_Transmit_IT(&huart1, (uint8_t *)s_buf, (uint16_t)n) == HAL_OK)
    {
        s_busy = 1;
    }
    else
    {
        s_dirty = 1;                     /* 启动失败，下轮重试 */
    }
}

void FB_OnTxDone(void)
{
    s_busy = 0;
}
