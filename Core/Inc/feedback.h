#ifndef __FEEDBACK_H
#define __FEEDBACK_H

#include <stdint.h>

void FB_Init(void);
void FB_MarkDirty(void);    /* 状态变化后调用，标记"待发送" */
void FB_Poll(void);         /* 主循环调用：空闲时发一次 */
void FB_OnTxDone(void);     /* 在 HAL_UART_TxCpltCallback 里调用 */

#endif
