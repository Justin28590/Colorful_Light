#include <rtthread.h>
#include <rtdevice.h>
#include <WS2812B.h>


uint32_t WS2812B_Buf[WS2812B_LED_QUANTITY]; // 0xGGRRBB
uint16_t WS2812B_Bit[24 * WS2812B_LED_QUANTITY + 1];
uint8_t WS2812B_Flag;
uint8_t ColorLight_Mode,ColorLight_Flag;
uint16_t ColorLight_Time;
uint8_t speed;  // 新增变量控制步长增量
uint8_t WhiteLight_Brightness;
uint32_t WS2812B_Buf_SPI[91];


static struct rt_spi_device *ws2812b_spi = RT_NULL;
static struct rt_timer ws2812b_timer;
static struct rt_semaphore ws2812b_sem;

void WS2812B_IRQHandler(void);

void WS2812B_Init(void)
{
    ws2812b_spi = (struct rt_spi_device *)rt_device_find("spi2");
    if (ws2812b_spi == RT_NULL)
    {
        rt_kprintf("SPI device not found!\n");
        return;
    }
    rt_sem_init(&ws2812b_sem, "ws2812b_sem", 0, RT_IPC_FLAG_FIFO);
}

void WS2812B_ClearBuf(void)
{
    rt_memset(WS2812B_Buf, 0, sizeof(WS2812B_Buf));
}

void WS2812B_SetBuf(uint32_t Color)
{
    for (int i = 0; i < WS2812B_LED_QUANTITY; i++)
    {
        WS2812B_Buf[i] = Color;
    }
}

void WS2812B_UpdateBuf(void)
{
    for (int j = 0; j < WS2812B_LED_QUANTITY; j++)
    {
        for (int i = 0; i < 24; i++)
        {
            if (WS2812B_Buf[j] & (0x800000 >> i))
            {
                WS2812B_Bit[j * 24 + i + 1] = 60;
            }
            else
            {
                WS2812B_Bit[j * 24 + i + 1] = 30;
            }
        }
    }
    rt_spi_transfer(ws2812b_spi, (const void *)WS2812B_Bit, RT_NULL, 24 * WS2812B_LED_QUANTITY + 1);
    rt_timer_start(&ws2812b_timer);
    rt_sem_take(&ws2812b_sem, RT_WAITING_FOREVER);
}

void WS2812B_IRQHandler(void)
{
    rt_timer_stop(&ws2812b_timer);
    WS2812B_Flag = 1;
    rt_sem_release(&ws2812b_sem);
}

void WS2812B_Close(void)
{
    WS2812B_SetBuf(0x000000);
    WS2812B_UpdateBuf();
}

void WS2812B_WhiteLight(void)
{
    for (int i = 0; i < WS2812B_LED_QUANTITY; i++) {
        uint8_t red   = (WS2812B_Buf[i] >> 16) & 0xFF;
        uint8_t green = (WS2812B_Buf[i] >> 8) & 0xFF;
        uint8_t blue  = WS2812B_Buf[i] & 0xFF;
        if (WhiteLight_Brightness == 1) {
            red   >>= 4;
            green >>= 4;
            blue  >>= 4;
        } else if (WhiteLight_Brightness == 2) {
            red   >>= 2;
            green >>= 2;
            blue  >>= 2;
        }
        WS2812B_Buf[i] = (green << 16) | (red << 8) | blue;
    }
    WS2812B_UpdateBuf();
}

void WS2812B_ColorLight(void)
{
    ColorLight_Time = 60;
    for (int i = 0; i < WS2812B_LED_QUANTITY; i++) {
        WS2812B_Buf[i] = WS2812B_Buf_SPI[i];
    }
    if (ColorLight_Flag) {
        ColorLight_Flag = 0;
        WS2812B_UpdateBuf();
    }
}

void WS2812B_ColorLight_Mode0(void)
{
    static uint8_t j;
    for (int i = WS2812B_LED_QUANTITY - 1; i > 0; i--) {
        WS2812B_Buf[i] = WS2812B_Buf[i - 1];
    }
    WS2812B_Buf[0] = (j == 0) ? WS2812B_Buf_SPI[0] : (j < 10) ? WS2812B_Buf[1] : 0;
    j = (j + 1) % 100;
    if (ColorLight_Flag) {
        ColorLight_Flag = 0;
        WS2812B_UpdateBuf();
    }
}

void WS2812B_ColorLight_Mode1(void)
{
    ColorLight_Time = 500;
    for (int i = 0; i < WS2812B_LED_QUANTITY; i++) {
        WS2812B_Buf[i] = rand() % 0x1000000;
    }
    if (ColorLight_Flag) {
        ColorLight_Flag = 0;
        WS2812B_UpdateBuf();
    }
}

void WS2812B_ColorLight_Mode2(void)
{
    static uint8_t j;
    ColorLight_Time = 20;
    for (int i = WS2812B_LED_QUANTITY; i > 0; i--) {
        WS2812B_Buf[i] = WS2812B_Buf[i - 1];
    }
    WS2812B_Buf[0] = (j == 0) ? rand() % 0x1000000 : WS2812B_Buf[1];
    j = (j + 1) % 10;
    if (ColorLight_Flag) {
        ColorLight_Flag = 0;
        WS2812B_UpdateBuf();
    }
}

void WS2812B_ColorLight_Mode3(void)
{
    static uint8_t i, Color;
    ColorLight_Time = 6;
    WS2812B_SetBuf(((Color & 1) ? 255 - Color : Color) << ((i / 2) * 8));
    Color++;
    if (Color == 0) {
        i = (i + 1) % 14;
    }
    if (ColorLight_Flag) {
        ColorLight_Flag = 0;
        WS2812B_UpdateBuf();
    }
}
