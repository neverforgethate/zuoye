#ifndef __OLED_VIEW_H
#define __OLED_VIEW_H

#include <stdint.h>

void Oled_Init(void);
void Oled_Refresh(uint8_t force);   /* force=1 表示状态刚变化，必须刷 */

#endif
