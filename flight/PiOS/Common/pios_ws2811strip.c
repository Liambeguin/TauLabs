/**
  ******************************************************************************
  * @addtogroup PIOS PIOS Core hardware abstraction layer
  * @{
  * @addtogroup PIOS_WS2811STRIP WS2811 LED Strip Functions
  * @brief Hardware functions to deal with the intelligent LED stip
  * @{
  *
  * @file       pios_ws2811strip.c
  * @author     Liam BEGUIN <liambeguin@gmail.com>
  * @brief      WS2811 LED Strip Routines based on Cleanflight
  * @see        The GNU Public License (GPL) Version 3
  *
  ******************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* Project Includes */
#include "pios.h"
#include "pios_ws2811strip.h"

// TODO the following was in nvic section
#define NVIC_PRIORITY_GROUPING NVIC_PriorityGroup_2

#define NVIC_BUILD_PRIORITY(base,sub) (((((base)<<(4-(7-(NVIC_PRIORITY_GROUPING>>8))))|((sub)&(0x0f>>(7-(NVIC_PRIORITY_GROUPING>>8)))))<<4)&0xf0)
#define NVIC_PRIORITY_BASE(prio) (((prio)>>(4-(7-(NVIC_PRIORITY_GROUPING>>8))))>>4)
#define NVIC_PRIORITY_SUB(prio) (((prio)&(0x0f>>(7-(NVIC_PRIORITY_GROUPING>>8))))>>4)

#define NVIC_PRIO_WS2811_DMA               NVIC_BUILD_PRIORITY(1, 2)  // TODO - is there some reason to use high priority? (or to use DMA IRQ at all?)
// End of section

#define WS2811_LED_STRIP_LENGTH 32
#define WS2811_BITS_PER_LED 24
#define WS2811_DELAY_BUFFER_LENGTH 42 // for 50us delay

#define WS2811_DATA_BUFFER_SIZE (WS2811_BITS_PER_LED * WS2811_LED_STRIP_LENGTH)
#define WS2811_DMA_BUFFER_SIZE (WS2811_DATA_BUFFER_SIZE + WS2811_DELAY_BUFFER_LENGTH)   // number of bytes needed is #LEDs * 24 bytes + 42 trailing bytes)
#define BIT_COMPARE_1 17 // timer compare value for logical 1
#define BIT_COMPARE_0 9  // timer compare value for logical 0

// TODO: make this generic (this is only for CC3D, output5)
#define PIOS_WS2811STRIP_GPIO_PORT                  GPIOB
#define PIOS_WS2811STRIP_PIN                        GPIO_Pin_4

/* Local Variables */

uint8_t ledStripDMABuffer[WS2811_DMA_BUFFER_SIZE];
volatile uint8_t ws2811LedDataTransferInProgress = 0;

// TODO replace this with RGB color
static rgbColor24bpp_t ledColorBuffer[WS2811_LED_STRIP_LENGTH];

/**
* Initialise the LED Strip
*/
void PIOS_WS2811STRIP_Init(void)
{
	uint16_t prescalerValue;

	/* Init Output pin */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // TODO make this following line depend on GPIO PORT
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = PIOS_WS2811STRIP_PIN;
	GPIO_Init(PIOS_WS2811STRIP_GPIO_PORT, &GPIO_InitStructure);
	PIOS_WS2811STRIP_GPIO_PORT->BSRR = PIOS_WS2811STRIP_PIN; // What is this line for !?

	/* Setup RCC */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	/* Compute the prescaler value */
	prescalerValue = (uint16_t) (SystemCoreClock / 24000000) - 1;
	/* Time base configuration */
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = 29; // 800kHz
	TIM_TimeBaseStructure.TIM_Prescaler = prescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
	TIM_CtrlPWMOutputs(TIM3, ENABLE);

	/* configure DMA */
	memset(&ledStripDMABuffer, 0, WS2811_DMA_BUFFER_SIZE);

	/* DMA clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	/* DMA1 Channel6 Config */
	DMA_DeInit(DMA1_Channel6);

	DMA_InitTypeDef DMA_InitStructure;
	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&TIM3->CCR1;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ledStripDMABuffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = WS2811_DMA_BUFFER_SIZE;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(DMA1_Channel6, &DMA_InitStructure);

	/* TIM3 CC1 DMA Request enable */
	TIM_DMACmd(TIM3, TIM_DMA_CC1, ENABLE);

	DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PRIORITY_BASE(NVIC_PRIO_WS2811_DMA);
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = NVIC_PRIORITY_SUB(NVIC_PRIO_WS2811_DMA);
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// TODO Now would be the time to clear the leds
}

void DMA1_Channel6_IRQHandler(void)
{
    if (DMA_GetFlagStatus(DMA1_FLAG_TC6)) {
        ws2811LedDataTransferInProgress = 0;
        DMA_Cmd(DMA1_Channel6, DISABLE);            // disable DMA channel 6
        DMA_ClearFlag(DMA1_FLAG_TC6);               // clear DMA1 Channel 6 transfer complete flag
    }
}

void ws2811LedStripDMAEnable(void)
{
    DMA_SetCurrDataCounter(DMA1_Channel6, WS2811_DMA_BUFFER_SIZE);  // load number of bytes to be transferred
    TIM_SetCounter(TIM3, 0);
    TIM_Cmd(TIM3, ENABLE);
    DMA_Cmd(DMA1_Channel6, ENABLE);
}

bool isWS2811LedStripReady(void)
{
	return !ws2811LedDataTransferInProgress;
}

static uint16_t dmaBufferOffset;
static int16_t ledIndex;

#define USE_FAST_DMA_BUFFER_IMPL
#ifdef USE_FAST_DMA_BUFFER_IMPL

static void fastUpdateLEDDMABuffer(rgbColor24bpp_t *color)
{
	uint32_t grb = (color->rgb.g << 16) | (color->rgb.r << 8) | (color->rgb.b);

	for (int8_t index = 23; index >= 0; index--) {
		ledStripDMABuffer[dmaBufferOffset++] = (grb & (1 << index)) ? BIT_COMPARE_1 : BIT_COMPARE_0;
	}
}
#else
static void updateLEDDMABuffer(uint8_t componentValue)
{
	uint8_t bitIndex;

	for (bitIndex = 0; bitIndex < 8; bitIndex++)
	{
		if ((componentValue << bitIndex) & 0x80 )    // data sent MSB first, j = 0 is MSB j = 7 is LSB
		{
			ledStripDMABuffer[dmaBufferOffset] = BIT_COMPARE_1;
		}
		else
		{
			ledStripDMABuffer[dmaBufferOffset] = BIT_COMPARE_0;   // compare value for logical 0
		}
		dmaBufferOffset++;
	}
}
#endif

/*
 * This method is non-blocking unless an existing LED update is in progress.
 * it does not wait until all the LEDs have been updated, that happens in the background.
 */
void ws2811UpdateStrip(void)
{
	static uint32_t waitCounter = 0;

	// wait until previous transfer completes
	while(ws2811LedDataTransferInProgress) {
		waitCounter++;
	}

	dmaBufferOffset = 0;      // reset buffer memory index
	ledIndex = 0;             // reset led index

	// fill transmit buffer with correct compare values to achieve
	// correct pulse widths according to color values
	while (ledIndex < WS2811_LED_STRIP_LENGTH)
	{
#ifdef USE_FAST_DMA_BUFFER_IMPL
		fastUpdateLEDDMABuffer(&ledColorBuffer[ledIndex]);
#else
		updateLEDDMABuffer(ledColorBuffer[ledIndex]->rgb.g);
		updateLEDDMABuffer(ledColorBuffer[ledIndex]->rgb.r);
		updateLEDDMABuffer(ledColorBuffer[ledIndex]->rgb.b);
#endif

		ledIndex++;
	}

	ws2811LedDataTransferInProgress = 1;
	ws2811LedStripDMAEnable();
}

void PIOS_WS2811STRIP_SetLedColor(uint16_t index, const rgbColor24bpp_t *color)
{
	ledColorBuffer[index] = *color;
}

void PIOS_WS2811STRIP_SetStripColor(const rgbColor24bpp_t *color)
{
	uint8_t index = 0;
	for (index = 0; index < WS2811_LED_STRIP_LENGTH; index++){
		ledColorBuffer[index] = *color;
	}
}



