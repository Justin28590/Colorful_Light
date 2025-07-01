#include <rtthread.h>
#include <rtdevice.h>
#include "spi.h"

#define SPI_RX_BUFFER_SIZE 16  // 接收缓冲区大小
#define SPI_BUF1_SIZE 7        // 每次DMA传输的字节数
#define LED_COUNT 91

static struct rt_spi_device *spi_dev;
static rt_uint8_t spi_rx_buffer1[SPI_RX_BUFFER_SIZE];
static rt_uint8_t spi_rx_buffer2[SPI_RX_BUFFER_SIZE];
static rt_uint8_t *current_rx_buffer = spi_rx_buffer1;
static rt_uint8_t *next_rx_buffer = spi_rx_buffer2;


/* SPI 设备初始化 */
int spi_fpga_interface_init(void)
{
    spi_dev = (struct rt_spi_device *)rt_device_find("spi20"); // 假设 SPI 设备名为 "spi20"
    if (!spi_dev)
    {
        rt_kprintf("SPI device not found!\n");
        return -RT_ERROR;
    }

    struct rt_spi_configuration cfg;
    cfg.data_width = 8;
    cfg.mode = RT_SPI_SLAVE | RT_SPI_MSB | RT_SPI_MODE_0;
    cfg.max_hz = 9000000; // 9 MHz 速率
    rt_spi_configure(spi_dev, &cfg);
    return RT_EOK;
}

/* 处理 LED 数据 */
void update_led_data(rt_uint8_t *spi_rx_buffer)
{
    uint16_t cnt_x = (spi_rx_buffer[0] << 8) | spi_rx_buffer[1];
    uint16_t cnt_y = (spi_rx_buffer[2] << 8) | spi_rx_buffer[3];
    rt_uint8_t rgb_data[3] = {spi_rx_buffer[4], spi_rx_buffer[5], spi_rx_buffer[6]};

    for (int i = 0; i <= 16; i++) {
           if ((cnt_y >= 60 + 960 - 60 * i) && (cnt_y <= 91 + 960 - 60 * i) && (cnt_x == 10)) {
               WS2812B_Buf[i] += (rgb_data[0] << 16) | (rgb_data[1] << 8) | rgb_data[2];
           }
       }

       for (int i = 0; i <= 28; i++) {
           if ((cnt_x >= 32 + 64 * i) && (cnt_x <= 63 + 64 * i) && (cnt_y == 10)) {
               WS2812B_Buf[17 + i] += (rgb_data[0] << 16) | (rgb_data[1] << 8) | rgb_data[2];
           }
       }

       for (int i = 0; i <= 15; i++) {
           if ((cnt_y >= 60 + 62 * i) && (cnt_y <= 91 + 62 * i) && (cnt_x == 1900)) {
               WS2812B_Buf[46 + i] += (rgb_data[0] << 16) | (rgb_data[1] << 8) | rgb_data[2];
           }
       }

       for (int i = 0; i <= 28; i++) {
           if ((cnt_x >= 32 + 1792 - 64 * i) && (cnt_x <= 63 + 1792 - 64 * i) && (cnt_y == 1070)) {
               WS2812B_Buf[62 + i] += (rgb_data[0] << 16) | (rgb_data[1] << 8) | rgb_data[2];
           }
       }

       if ((cnt_x == 1900) && (cnt_y == 1070)) {
           for (int i = 0; i < WS2812B_LED_QUANTITY; i++) {
               WS2812B_Buf[i] = ((WS2812B_Buf[i] >> 5) & 0xFF0000) | ((WS2812B_Buf[i] >> 5) & 0x00FF00) | ((WS2812B_Buf[i] >> 5) & 0x0000FF);
           }
       }
}

/* SPI 接收线程 */
void spi_receive(void)
{
    while (1)
    {
        rt_spi_transfer(spi_dev, RT_NULL, current_rx_buffer, SPI_BUF1_SIZE);
        update_led_data(current_rx_buffer);

        // 交换缓冲区
        rt_uint8_t *temp = current_rx_buffer;
        current_rx_buffer = next_rx_buffer;
        next_rx_buffer = temp;
    }
}


