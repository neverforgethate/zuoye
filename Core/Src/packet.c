#include "packet.h"
#include <string.h>

static Packet_t          s_rx;          /* 正在接收的缓冲区 */
static volatile uint8_t  s_rx_len;      /* 已收字节数 */
static Packet_t          s_ready;       /* 收满一包，等主循环来取 */
static volatile uint8_t  s_ready_flag;

void Packet_Init(void)
{
    s_rx_len     = 0;
    s_ready_flag = 0;
}

/* 中断里逐字节喂进来。第一个字节必须是帧头，否则丢弃并重新同步。 */
void Packet_OnByte(uint8_t byte)
{
    if (s_rx_len == 0 && byte != PKT_HEAD)
    {
        return;                         /* 等帧头 */
    }

    ((uint8_t *)&s_rx)[s_rx_len++] = byte;

    if (s_rx_len >= PKT_LEN)
    {
        s_rx_len = 0;
        if (s_ready_flag == 0)          /* 主循环还没取走上一条，就丢掉这一包 */
        {
            memcpy((void *)&s_ready, (const void *)&s_rx, sizeof(Packet_t));
            s_ready_flag = 1;
        }
    }
}

uint8_t Packet_TryTake(Packet_t *out)
{
    if (out == 0 || s_ready_flag == 0)
    {
        return 0;
    }
    memcpy((void *)out, (const void *)&s_ready, sizeof(Packet_t));
    s_ready_flag = 0;
    return 1;
}

/* CHECK = (0xAA + RUN + DIR + STEP_L + STEP_H) % 256
   uint8_t 相加天然按 256 回绕，所以直接比较即可 */
uint8_t Packet_Check(const Packet_t *p)
{
    uint8_t sum;

    if (p == 0)
    {
        return 0;
    }
    sum = (uint8_t)(PKT_HEAD + p->run + p->dir + p->step_l + p->step_h);
    return (sum == p->check) ? 1 : 0;
}
