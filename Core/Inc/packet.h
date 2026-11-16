#ifndef __PACKET_H
#define __PACKET_H

#include <stdint.h>

#define PKT_HEAD  0xAA      /* 作业规定的帧头 */
#define PKT_LEN   6         /* AA RUN DIR STEP_L STEP_H CHECK */
#define STEP_MIN  100
#define STEP_MAX  2000

/* 全部是 uint8_t，结构体没有填充字节，sizeof == 6，可以按字节直接填充 */
typedef struct
{
    uint8_t head;
    uint8_t run;
    uint8_t dir;
    uint8_t step_l;
    uint8_t step_h;
    uint8_t check;
} Packet_t;

void    Packet_Init(void);
void    Packet_OnByte(uint8_t byte);        /* 在 HAL_UART_RxCpltCallback 里调用 */
uint8_t Packet_TryTake(Packet_t *out);      /* 主循环调用：取走一包 */
uint8_t Packet_Check(const Packet_t *p);    /* 校验和是否正确 */

#endif
