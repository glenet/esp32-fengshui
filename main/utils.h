/* Copyright (C) Michael Yang
 * Written by Michael Yang <czhe.yang@gmail.com>, December 2017
 */

#ifndef UTILS_H
#define UTILS_H

static inline uint32_t getADCChannel(uint32_t gpio)
{
	uint32_t ui32Channel;

	switch (gpio) {
		case ADC1_CHANNEL_0_GPIO_NUM:
			ui32Channel = ADC1_CHANNEL_0;
			break;
		case ADC1_CHANNEL_1_GPIO_NUM:
			ui32Channel = ADC1_CHANNEL_1;
			break;
		case ADC1_CHANNEL_2_GPIO_NUM:
			ui32Channel = ADC1_CHANNEL_2;
			break;
		case ADC1_CHANNEL_3_GPIO_NUM:
			ui32Channel = ADC1_CHANNEL_3;
			break;
		case ADC1_CHANNEL_4_GPIO_NUM:
			ui32Channel = ADC1_CHANNEL_4;
			break;
		case ADC1_CHANNEL_5_GPIO_NUM:
			ui32Channel = ADC1_CHANNEL_5;
			break;
		case ADC1_CHANNEL_6_GPIO_NUM:
			ui32Channel = ADC1_CHANNEL_6;
			break;
		case ADC1_CHANNEL_7_GPIO_NUM:
			ui32Channel = ADC1_CHANNEL_7;
			break;
		default:
			ui32Channel = ADC1_CHANNEL_0;
	}

	return ui32Channel;
}

#endif
