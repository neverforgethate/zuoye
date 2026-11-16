#include "oled_view.h"
#include "app.h"
#include "OLED.h"
#include "main.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

/* ============================================================
 * 功能 3：OLED 显示当前状态
 *
 * 驱动来源：电控 QQ 群文件《第三次培训所用资料》
 *           OLED.c / OLED.h / OLED_Font.h
 *           8x16 字模，软件 I2C，PB1 = SCL，PB0 = SDA
 *
 * 显示 4 行（128x64 屏，每行最多 16 个字符）：
 *     STATE:RUN       运行 / STOP 暂停
 *     DIR  :FWD       正向 / REV 反向
 *     STEP :0500      相邻 LED 切换间隔（毫秒，补足 4 位）
 *     LED  :1         当前点亮的第几颗
 * ============================================================ */

/* ------------------------------------------------------------
 * 调试开关：1 = 每 2 秒做一次双向 I2C 扫描，结果走串口
 *           0 = 正常使用（提交前改成 0）
 * ------------------------------------------------------------ */
#define OLED_SCAN 0

#if OLED_SCAN

/* 0 = 用 PB0，1 = 用 PB1 */
static uint8_t g_scl_idx;
static uint8_t g_sda_idx;

static void pin_w(uint8_t idx, uint8_t v)
{
    uint16_t pin = idx ? GPIO_PIN_1 : GPIO_PIN_0;
    HAL_GPIO_WritePin(GPIOB, pin, v ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t pin_r(uint8_t idx)
{
    uint16_t pin = idx ? GPIO_PIN_1 : GPIO_PIN_0;
    return (HAL_GPIO_ReadPin(GPIOB, pin) == GPIO_PIN_SET) ? 1 : 0;
}

static void scan_i2c_init(void)
{
    GPIO_InitTypeDef g = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();
    g.Pin   = GPIO_PIN_0 | GPIO_PIN_1;
    g.Mode  = GPIO_MODE_OUTPUT_OD;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &g);
}

/* 软件 I2C 的位间延时：把时钟降到大约 100kHz。
   SSD1306 的 I2C 上限是 400kHz，太快会不应答。 */
static void scan_delay(void)
{
    volatile uint8_t i;
    for (i = 0; i < 40; i++)
    {
        /* 空转，约 3~5 us */
    }
}

/* 用当前 g_scl_idx / g_sda_idx 的引脚分配，探测地址 addr 是否应答 */
static uint8_t scan_probe(uint8_t addr)
{
    uint8_t i;
    uint8_t ack;

    pin_w(g_sda_idx, 1);
    scan_delay();
    pin_w(g_scl_idx, 1);
    scan_delay();

    pin_w(g_sda_idx, 0);        /* 起始：SCL 高时 SDA 下降 */
    scan_delay();
    pin_w(g_scl_idx, 0);
    scan_delay();

    for (i = 0; i < 8; i++)     /* 地址 + 写位 */
    {
        pin_w(g_sda_idx, ((((uint16_t)addr << 1) & (0x80U >> i)) ? 1 : 0));
        scan_delay();
        pin_w(g_scl_idx, 1);
        scan_delay();
        pin_w(g_scl_idx, 0);
        scan_delay();
    }

    pin_w(g_sda_idx, 1);        /* 释放 SDA，读从机应答 */
    scan_delay();
    pin_w(g_scl_idx, 1);
    scan_delay();
    ack = (pin_r(g_sda_idx) == 0) ? 1 : 0;
    pin_w(g_scl_idx, 0);
    scan_delay();

    pin_w(g_sda_idx, 0);        /* 停止 */
    scan_delay();
    pin_w(g_scl_idx, 1);
    scan_delay();
    pin_w(g_sda_idx, 1);
    scan_delay();

    return ack;
}

static void scan_report(void)
{
    char msg[96];
    uint8_t pu_a, ack_a, pu_b, ack_b;
    uint8_t pd_pb0, pd_pb1;
    GPIO_InitTypeDef g = {0};

    /* ---- 用内部下拉判断外部有没有上拉 ----
       外部有 4.7k 上拉 -> 内部 40k 下拉压不住 -> 读到 1
       线悬空          -> 内部下拉生效         -> 读到 0 */
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_PULLDOWN;
    g.Pin  = GPIO_PIN_0;
    HAL_GPIO_Init(GPIOB, &g);
    HAL_Delay(2);
    pd_pb0 = pin_r(0);
    g.Pin = GPIO_PIN_1;
    HAL_GPIO_Init(GPIOB, &g);
    HAL_Delay(2);
    pd_pb1 = pin_r(1);

    scan_i2c_init();

    /* 方案 A：SCL = PB1, SDA = PB0 （按指南定义）/ 读 SDA 线上拉 */
    g_scl_idx = 1;
    g_sda_idx = 0;
    pin_w(g_sda_idx, 1);
    pin_w(g_scl_idx, 1);
    HAL_Delay(1);
    pu_a   = pin_r(g_sda_idx);
    ack_a  = scan_probe(0x78);

    /* 方案 B：SCL = PB0, SDA = PB1 （两根接反的情况） */
    g_scl_idx = 0;
    g_sda_idx = 1;
    pin_w(g_sda_idx, 1);
    pin_w(g_scl_idx, 1);
    HAL_Delay(1);
    pu_b   = pin_r(g_sda_idx);
    ack_b  = scan_probe(0x78);

    snprintf(msg, sizeof(msg),
             "SCAN: PD(B0=%u,B1=%u) A(pu=%u,ack=%u) B(pu=%u,ack=%u)\r\n",
             (unsigned)pd_pb0, (unsigned)pd_pb1,
             (unsigned)pu_a, (unsigned)ack_a,
             (unsigned)pu_b, (unsigned)ack_b);
    HAL_UART_Transmit(&huart1, (uint8_t *)msg, (uint16_t)strlen(msg), 200);
}
#endif /* OLED_SCAN */

/* 记录上一次显示的值，只重写发生变化的那一行 */
static uint8_t  s_last_run  = 0xFF;
static uint8_t  s_last_dir  = 0xFF;
static uint8_t  s_last_led  = 0xFF;
static uint16_t s_last_step = 0xFFFF;

void Oled_Init(void)
{
#if OLED_SCAN
    scan_report();
#endif

    OLED_Init();
    Oled_Refresh(1);
}

void Oled_Refresh(uint8_t force)
{
    char line[17];

#if OLED_SCAN
    {
        static uint32_t s_last_scan = 0;
        uint32_t now = HAL_GetTick();
        if ((now - s_last_scan) >= 2000U)
        {
            s_last_scan = now;
            scan_report();
        }
    }
#endif

    if (force)
    {
        s_last_run  = 0xFF;
        s_last_dir  = 0xFF;
        s_last_led  = 0xFF;
        s_last_step = 0xFFFF;
    }

    if (g_app.run != s_last_run)
    {
        s_last_run = g_app.run;
        snprintf(line, sizeof(line), "STATE:%s", g_app.run ? "RUN " : "STOP");
        OLED_ShowString(1, 1, line);
    }

    if (g_app.dir != s_last_dir)
    {
        s_last_dir = g_app.dir;
        snprintf(line, sizeof(line), "DIR  :%s", g_app.dir ? "REV" : "FWD");
        OLED_ShowString(2, 1, line);
    }

    if (g_app.step_ms != s_last_step)
    {
        s_last_step = g_app.step_ms;
        snprintf(line, sizeof(line), "STEP :%04u", (unsigned int)g_app.step_ms);
        OLED_ShowString(3, 1, line);
    }

    if (g_app.led != s_last_led)
    {
        s_last_led = g_app.led;
        snprintf(line, sizeof(line), "LED  :%u", (unsigned int)(g_app.led + 1));
        OLED_ShowString(4, 1, line);
    }
}
