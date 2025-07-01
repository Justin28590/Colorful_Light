#include <rtdevice.h>
#include <rtdevice.h>
#include <board.h>
#include <ld3320_drv.h>
#include <ap3216c.h>
#include <lm2904.h>
#include <onenet.h>
#include <onenet_mqtt.h>
#include <paho_mqtt.h>
#include <spi.h>
#include <WS2812B.h>
#include <string.h>

#define LM2904_Pin        GET_PIN(A,8)
#define SPI_RX_BUFFER_SIZE 16  // 接收缓冲区大小
#define SPI_BUF1_SIZE 7        // 每次DMA传输的字节数
#define THREAD_STACK_SIZE 1024  // 线程栈大小
#define THREAD_PRIORITY    10   // 线程优先级
#define THREAD_TIMESLICE   10   // 线程时间片

const char *ds_name = "voice";  //字符串常量应该用双引号括起来，而不是直接将字符串字面量作为变量名
const char *str1 = "大";
const char *str2 = "小";
const char *i2c_bus_name = "i2c2";
const char *ssid = "iPhone";       // Wi-Fi 名称
const char *password = "20040909"; // Wi-Fi 密码
struct rt_spi_device *spi_dev;
struct rt_spi_device *ws2812b_spi = RT_NULL;
struct rt_timer ws2812b_timer;
struct rt_semaphore ws2812b_sem;

rt_uint8_t spi_rx_buffer1[SPI_RX_BUFFER_SIZE];
rt_uint8_t spi_rx_buffer2[SPI_RX_BUFFER_SIZE];
rt_uint8_t *current_rx_buffer = spi_rx_buffer1;
rt_uint8_t *next_rx_buffer = spi_rx_buffer2;
rt_device_t serial = RT_NULL; // 串口设备句柄
ap3216c_device_t dev;         // ap3216c 设备句柄
MessageData *msg_data = NULL;
rt_mutex_t mqtt_msg_mutex;
int mqtt_msg_ready;


/* LM2904 线程 */
static void lm2904_thread_entry(void *parameter)
{
    rt_pin_mode(LM2904_Pin, PIN_MODE_INPUT_PULLUP);
    while(1)
    {
        if(rt_pin_read(LM2904_Pin))
        {
            rt_kprintf("loud voice\r\n");
            onenet_mqtt_upload_string(ds_name, str2);
        }
        else
        {
            rt_kprintf("low voice\r\n");
            onenet_mqtt_upload_string(ds_name,str1);
        }
        rt_thread_mdelay(100);
    }
}

/* 光强传感器线程 */
static void ap3216c_thread_entry(void *parameter)
{
    dev = ap3216c_init(i2c_bus_name);
    float brightness;
    /* 读光照强度值 */
    while(1)
    {
        brightness = ap3216c_read_ambient_light(dev);
        rt_kprintf("current brightness: %d.%d(lux).\r\n", (int)brightness, ((int)(10 * brightness) % 10));
        if(brightness < 100)
        {
            WhiteLight_Brightness=0;
        }
        else if(brightness > 100 && brightness < 300)
        {
            WhiteLight_Brightness=1;
        }
        else if(brightness > 300 && brightness < 800)
        {
            WhiteLight_Brightness=2;
        }
        onenet_mqtt_upload_digit("brightness", brightness);
        rt_thread_mdelay(1000);
    }
}

/* 语音识别线程 */
static void ld3320_thread_entry(void *parameter)
{
    rt_ld3320_init();
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
                     rt_KD();
                     break;
                 case CODE_GD:
                     rt_GD();
                     break;
                 case CODE_FWD:
                     rt_FWD();
                     break;
                 case CODE_LSD:
                     rt_LSD();
                     break;
                 case CODE_MSY:
                     rt_MSY();
                     break;
                 case CODE_MSE:
                     rt_MSE();
                     break;
                 case CODE_MSS:
                     rt_MSS();
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


/* MQTT 线程 */
static void mqtt_thread_entry(void *parameter)
{
    rt_wlan_connect(ssid, password);
    onenet_mqtt_entry();
    while (1)
   {
       if (mqtt_msg_ready)
       {
            rt_mutex_take(mqtt_msg_mutex, RT_WAITING_FOREVER);
            mqtt_msg_ready = 0;
            CJSON_PUBLIC(cJSON *)raw_json = cJSON_Parse(msg_data->message->payload);
            CJSON_PUBLIC(cJSON *)params_json = cJSON_GetObjectItem(raw_json, "params");
            CJSON_PUBLIC(cJSON *)FWD_json = cJSON_GetObjectItem(params_json, "FWD");
            if(FWD_json != NULL)
            {
               if(FWD_json->type == cJSON_True)
               {
                   rt_FWD();
               }
               else {
                   rt_GD();
               }
            }
            CJSON_PUBLIC(cJSON *)LSD_json = cJSON_GetObjectItem(params_json, "LSD");
            if(LSD_json != NULL)
            {
               if(LSD_json->type == cJSON_True)
               {
                   rt_LSD();
               }
               else {
                   rt_GD();
               }
            }
            CJSON_PUBLIC(cJSON *)DSJB_json = cJSON_GetObjectItem(params_json, "DSJB");
            if(DSJB_json != NULL)
            {
               if(DSJB_json->type == cJSON_True)
               {
                   rt_MSY();
               }
               else {
                   rt_GD();
               }
            }
            CJSON_PUBLIC(cJSON *)GCLD_json = cJSON_GetObjectItem(params_json, "GCLD");
            if(GCLD_json != NULL)
            {
               if(GCLD_json->type == cJSON_True)
               {
                   rt_MSE();
               }
               else {
                   rt_GD();
               }
            }
            CJSON_PUBLIC(cJSON *)HXD_json = cJSON_GetObjectItem(params_json, "HXD");
            if(HXD_json != NULL)
            {
               if(HXD_json->type == cJSON_True)
               {
                   rt_MSS();
               }
               else {
                   rt_GD();
               }
            }
            CJSON_PUBLIC(cJSON *)KD_json = cJSON_GetObjectItem(params_json, "KD");
            if(KD_json != NULL)
            {
               if(KD_json->type == cJSON_True)
               {
                  rt_KD();
               }
            }
            CJSON_PUBLIC(cJSON *)GD_json = cJSON_GetObjectItem(params_json, "GD");
            if(GD_json != NULL)
            {
               if(GD_json->type == cJSON_True)
               {
                   rt_GD();
               }
            }
               cJSON_Delete(raw_json);
       }
           rt_thread_mdelay(100);
   }
}


/* SPI接收线程 */
static void spi_thread_entry(void *parameter)
{
    spi_dev = (struct rt_spi_device *)rt_device_find("spi20");
    struct rt_spi_configuration cfg;
    cfg.data_width = 8;
    cfg.mode = RT_SPI_SLAVE | RT_SPI_MSB | RT_SPI_MODE_0;
    cfg.max_hz = 9000000; // 9 MHz 速率
    rt_spi_configure(spi_dev, &cfg);

    while (1)
    {
       rt_spi_transfer(spi_dev, RT_NULL, current_rx_buffer, SPI_BUF1_SIZE);
       update_led_data(current_rx_buffer);
       // 交换缓冲区
       rt_uint8_t *temp = current_rx_buffer;
       current_rx_buffer = next_rx_buffer;
       next_rx_buffer = temp;
       rt_thread_mdelay(100);
    }
}

/* WS2812B线程 */
static void ws2812b_thread_entry(void *parameter)
{
    ws2812b_spi = (struct rt_spi_device *)rt_device_find("spi2");
    rt_sem_init(&ws2812b_sem, "ws2812b_sem", 0, RT_IPC_FLAG_FIFO);
    while (1)
    {
       // 等待有新数据需要刷新
       rt_sem_take(&ws2812b_sem, RT_WAITING_FOREVER);

       // 根据 WS2812B_Buf转化为 WS2812B_Bit
       for (int j = 0; j < WS2812B_LED_QUANTITY; j++)
       {
           for (int i = 0; i < 24; i++)
           {
               if (WS2812B_Buf[j] & (0x800000 >> i))
                   WS2812B_Bit[j * 24 + i + 1] = 60;
               else
                   WS2812B_Bit[j * 24 + i + 1] = 30;
           }
       }

       rt_spi_transfer(ws2812b_spi, (const void *)WS2812B_Bit, RT_NULL, 24 * WS2812B_LED_QUANTITY + 1);
       rt_timer_start(&ws2812b_timer);
    }
}

int main(void)
{
    rt_thread_t thread;

    /* 创建并启动 LM2904 线程 */
    thread = rt_thread_create("lm2904_thread", lm2904_thread_entry, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (thread != RT_NULL) rt_thread_startup(thread);

    /* 创建并启动光强传感器线程 */
    thread = rt_thread_create("ap3216c_thread", ap3216c_thread_entry, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (thread != RT_NULL) rt_thread_startup(thread);

    /* 创建并启动语音识别初始化线程 */
    thread = rt_thread_create("ld3320_thread", ld3320_thread_entry, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (thread != RT_NULL) rt_thread_startup(thread);

    /* 创建并启动 MQTT 线程 */
    thread = rt_thread_create("mqtt_thread", mqtt_thread_entry, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (thread != RT_NULL) rt_thread_startup(thread);

    /* 创建并启动spi接收线程 */
    thread = rt_thread_create("spi_thread", spi_thread_entry, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (thread != RT_NULL) rt_thread_startup(thread);

    /* 创建并启动ws2812b线程 */
    thread = rt_thread_create("ws2812b_thread", ws2812b_thread_entry, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (thread != RT_NULL) rt_thread_startup(thread);

    return 0;
}
