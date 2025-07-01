#ifndef APPLICATIONS_WS2812B_H_
#define APPLICATIONS_WS2812B_H_

#include <spi.h>


#define WS2812B_LED_QUANTITY 91
extern uint32_t WS2812B_Buf[];  //0xGGRRBB
extern uint8_t WhiteLight_Brightness;
extern uint16_t WS2812B_Bit[24 * WS2812B_LED_QUANTITY + 1];

void WS2812B_Init(void);
void WS2812B_ClearBuf(void);
void WS2812B_SetBuf(uint32_t Color);
void WS2812B_UpdateBuf(void);
void WS2812B_IRQHandler(void);
void WS2812B_Close(void);
void WS2812B_WhiteLight(void);
void WS2812B_ColorLight(void);
void WS2812B_ColorLight_Mode0(void);
void WS2812B_ColorLight_Mode1(void);
void WS2812B_ColorLight_Mode2(void);
void WS2812B_ColorLight_Mode3(void);

#endif /* APPLICATIONS_WS2812B_H_ */
