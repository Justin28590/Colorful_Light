#ifndef APPLICATIONS_SPI_H_
#define APPLICATIONS_SPI_H_

#include "WS2812B.h"
#include <stdio.h>


extern uint8_t spi_buf[4];
extern uint32_t WS2812B_Buf_SPI[91];
extern uint8_t led_index;


int spi_fpga_interface_init(void);
void update_led_data(rt_uint8_t *spi_rx_buffer);
void spi_receive(void);

#endif
