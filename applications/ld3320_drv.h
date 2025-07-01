#ifndef _LD3320_DRV_H_
#define _LD3320_DRV_H_

#include <rtthread.h>
#include <rtdevice.h>
#include "drv_spi.h"
#include <board.h>
#include <errno.h>

extern rt_uint8_t nAsrStatus;
extern volatile rt_uint8_t ld3320_flag;

#define LD3320_WR  GET_PIN(E, 3)
#define LD3320_RST GET_PIN(E, 4)
#define LD3320_IRQ GET_PIN(E, 5)
#define LD3320_CS  GET_PIN(A, 4)

#define LD3320_WR_Func(x) x ? rt_pin_write(LD3320_WR, PIN_HIGH) : rt_pin_write(LD3320_WR, PIN_LOW)
#define LD3320_RST_Func(x) x ? rt_pin_write(LD3320_RST, PIN_HIGH) : rt_pin_write(LD3320_RST, PIN_LOW)
#define LD3320_CS_Func(x) x ? rt_pin_write(LD3320_CS, PIN_HIGH) : rt_pin_write(LD3320_CS, PIN_LOW)

//The following five states are defined to record which state the program is in while running ASR recognition
#define LD_ASR_NONE 0x00      //Indicates that ASR recognition is not being made
#define LD_ASR_RUNING 0x01    //Indicates that LD3320 is in ASR identification
#define LD_ASR_FOUNDOK 0x10   //Represents the end of an identification process, with an identification result
#define LD_ASR_FOUNDZERO 0x11 //Represents the end of an identification process, no identification results
#define LD_ASR_ERROR 0x31     //Represents an incorrect state occurring inside the LD3320 chip in a recognition process

//Identification code (Customer Modification Office)
#define CODE_KD     1   //commond code for LED waterfall
#define CODE_GD     2   //commond code for LED flash
#define CODE_FWD    3 //commond code for LED reversal
#define CODE_LSD    4  //commond code for PLAY MP3
#define CODE_MSY    5  //commond code for PLAY MP3
#define CODE_MSE    6  //commond code for PLAY MP3
#define CODE_MSS    7  //commond code for PLAY MP3

//LD chip fixed values.
#define RESUM_OF_MUSIC 0x01
#define CAUSE_MP3_SONG_END 0x20

#define MASK_INT_SYNC 0x10
#define MASK_INT_FIFO 0x04
#define MASK_AFIFO_INT 0x01
#define MASK_FIFO_STATUS_AFULL 0x08

//The following three states are defined to record whether the program is running ASR recognition or MP3
#define LD_MODE_IDLE 0x00
#define LD_MODE_ASR_RUN 0x08
#define LD_MODE_MP3 0x40

//LD clock set
#define CLK_IN 22 //crystal oscillator
#define LD_PLL_11 (rt_uint8_t)((CLK_IN / 2.0) - 1)
#define LD_PLL_MP3_19 0x0f
#define LD_PLL_MP3_1B 0x18
#define LD_PLL_MP3_1D (rt_uint8_t)(((90.0 * ((LD_PLL_11) + 1)) / (CLK_IN)) - 1)
#define LD_PLL_ASR_19 (rt_uint8_t)(CLK_IN * 32.0 / (LD_PLL_11 + 1) - 0.51)
#define LD_PLL_ASR_1B 0x48
#define LD_PLL_ASR_1D 0x1f

//VOL reg
#define MIC_VOL 0x43
#define SPEAKER_VOL 0x07

void rt_ProcessInt();
void rt_LD_Adjust_Volume(rt_uint8_t val);
void rt_LD_play(rt_uint8_t *mp3name);
void rt_LD_init(void);
void rt_LD_Init_MP3(void);
int rt_ld3320_init(void);
void ld3320_sample(void);

void rt_LD3320_test(void);
void rt_Board_text(rt_uint8_t Code_Val);
void rt_KD(void);
void rt_GD(void);
void rt_FWD(void);
void rt_LSD(void);
void rt_MSY(void);
void rt_MSE(void);
void rt_MSS(void);
void rt_ld3320_irq_pin_init(void);

rt_uint8_t  ld3320_statusCheck(void);


rt_uint8_t rt_LD_GetResult(void);
rt_uint8_t rt_LD_ASR(void);

#endif /* USER_DEVICE_LD3320_DRIVER_LD330_DRV_C_LD3320_DRV_H_ */
