#include <rtthread.h>
#include <rtdevice.h>
#include "drv_spi.h"
#include <board.h>
#include <uart_tx.h>
#include <ld3320_drv.h>
#include <lm2904.h>
#include <ap3216c.h>
#include <onenet.h>
#include <WS2812B.h>

#define UART_NAME "uart2"
#define THREAD_STACK_SIZE   1024
#define THREAD_PRIORITY     10
#define THREAD_TIMESLICE    10


extern rt_uint32_t nMp3Size;
extern rt_uint8_t bMp3Play;
extern rt_uint8_t nLD_Mode;

extern rt_device_t serial;  // 声明串口设备句柄为外部变量
extern ap3216c_device_t dev; // 声明ap3216c设备句柄为外部变量


/********************************************************************************
function:
                LD3320 test demo main function
********************************************************************************/
void rt_LD3320_test(void)
{
    rt_uint8_t nAsrRes = 0;
    while (1)
    {
        if (bMp3Play)
        {
            rt_kprintf("*********playing*********\r\n");
            continue;
        }
        //      printf("nAsrStatus is %x, nLD_Mode is %x \r\n", nAsrStatus, nLD_Mode);
        switch (nAsrStatus)
        {
        case LD_ASR_RUNING:
        case LD_ASR_ERROR:
            break;
        case LD_ASR_NONE:
            nAsrStatus = LD_ASR_RUNING;
            if (rt_LD_ASR() == 0) //Start the ASR process once
                nAsrStatus = LD_ASR_ERROR;
            break;
        case LD_ASR_FOUNDOK:
            nAsrRes = rt_LD_GetResult(); //once ASR process end, get the result
            switch (nAsrRes)
            { //show the commond
            case CODE_KD:
                rt_kprintf("deng yi da kai\r\n");
                break;
            case CODE_GD:
                rt_kprintf("reversal led\r\n");
                break;
            case CODE_FWD:
                rt_kprintf("flash led\r\n");
                break;
            case CODE_LSD:
                rt_kprintf("play mp3\r\n");
                break;
            case CODE_MSY:
                rt_kprintf("play mp3\r\n");
                break;
            case CODE_MSE:
                rt_kprintf("play mp3\r\n");
                break;
            case CODE_MSS:
                rt_kprintf("play mp3\r\n");
                break;
            default:
                break;
            }
            nAsrStatus = LD_ASR_NONE;
            break;
        case LD_ASR_FOUNDZERO:
        default:
            nAsrStatus = LD_ASR_NONE;
            break;
        }
        rt_Board_text(nAsrRes); //do the commond
        nAsrRes = 0;
    }
}
/********************************************************************************
function:
                LED waterfall
********************************************************************************/
void rt_KD(void)
{
    WS2812B_WhiteLight();
}


void rt_GD(void)
{
    WS2812B_Close();
}

void rt_FWD(void)
{
    WS2812B_ColorLight();
}

void rt_LSD()
{
    WS2812B_ColorLight_Mode0();
}

void rt_MSY(void)
{
    WS2812B_ColorLight_Mode1();
}

void rt_MSE(void)
{
    WS2812B_ColorLight_Mode2();
}

void rt_MSS(void)
{
    WS2812B_ColorLight_Mode3();
}

void rt_Board_text(rt_uint8_t Code_Val)
{
    switch (Code_Val)
    {
    case CODE_KD: //Commond "liu shui deng"
        rt_KD();
        break;
    case CODE_GD: //Commond "an jian"
        rt_GD();
        break;
    case CODE_FWD: //Commond "shan shuo"
        rt_FWD();
        break;
    case CODE_LSD: //Commond "bo fang"
        rt_LSD();
        break;
    case CODE_MSY: //Commond "bo fang"
        rt_MSY();
        break;
    case CODE_MSE: //Commond "bo fang"
        rt_MSE();
        break;
    case CODE_MSS: //Commond "bo fang"
        rt_MSS();
        break;
    default:
        break;
    }
}




void ld3320_sample(void)
{
    rt_uint8_t nAsrRes = 0;
    int i;
    nAsrStatus = LD_ASR_NONE;
    while (1)
    {
        if(ld3320_flag)
        {
            ld3320_flag=0;
            rt_ProcessInt();
        }
        else
        {
            ld3320_flag=0;
            i++;
            if(i%500==0)
            {
                rt_kprintf("\r\n\rvoice checking,please speak...\r\n");
                i=0;
            }
            rt_thread_delay(1);
        }
      switch (nAsrStatus)
      {
      case LD_ASR_RUNING:
      case LD_ASR_ERROR:
          break;
      case LD_ASR_NONE:
          nAsrStatus = LD_ASR_RUNING;
          if (rt_LD_ASR() == 0) //Start the ASR process once
              nAsrStatus = LD_ASR_ERROR;
          break;
      case LD_ASR_FOUNDOK:
          nAsrRes = rt_LD_GetResult(); //once ASR process end, get the result
          switch (nAsrRes)
          { //show the commond
          case CODE_KD:
              rt_kprintf("deng yi da kai\r\n");
              send_to_fpga(serial,6);
              break;
          case CODE_GD:
              rt_kprintf("deng yi guan bi\r\n");
              send_to_fpga(serial,7);
              break;
          case CODE_FWD:
              rt_kprintf("fen wei deng yi kai qi\r\n");
              send_to_fpga(serial,8);
              break;
          case CODE_LSD:
              rt_kprintf("liu shui deng yi kai qi\r\n");
              send_to_fpga(serial,9);
              break;
          case CODE_MSY:
              rt_kprintf("dan se jian bian deng yi kai qi\r\n");
              send_to_fpga(serial,10);
              break;
          case CODE_MSE:
              rt_kprintf("cai hong deng yi kai qi\r\n");
              send_to_fpga(serial,11);
              break;
          case CODE_MSS:
              rt_kprintf("hu xi deng yi kai qi\r\n");
              send_to_fpga(serial,12);
              break;
          default:
              break;
          }
          nAsrStatus = LD_ASR_NONE;
          break;
      case LD_ASR_FOUNDZERO:
      default:
          nAsrStatus = LD_ASR_NONE;
          break;
      }
      rt_thread_delay(rt_tick_from_millisecond(100));
    }

}












