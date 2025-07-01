#include <rtthread.h>
#include <rtdevice.h>

//#define UART_NAME "uart2"

void send_to_fpga(rt_device_t serial,rt_uint8_t data)
{
    rt_device_write(serial, 0, &data, 1);
}

