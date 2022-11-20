#ifndef ADC_H
#define ADC_H

#include "adc.h"
#include <stdio.h>
#include "MK66F18.h"
#include "board.h"
#include "pin_mux.h"
#include "peripherals.h"



#include "fsl_adc16.h"




volatile bool g_Adc16ConversionDoneFlag = false;
volatile uint32_t g_Adc16ConversionValue;
volatile uint32_t g_Adc16InterruptCounter;
const uint32_t g_Adc16_12bitFullRange = 4096U;

adc16_config_t adc16ConfigStruct;
adc16_channel_config_t adc16ChannelConfigStruct;


void ADC16_IRQ_HANDLER_FUNC(void)
{
    g_Adc16ConversionDoneFlag = true;
    /* Read conversion result to clear the conversion completed flag. */
    g_Adc16ConversionValue = ADC16_GetChannelConversionValue(ADC16_BASE, ADC16_CHANNEL_GROUP);
    g_Adc16InterruptCounter++;
    SDK_ISR_EXIT_BARRIER;
}



void configureADC (void) {
	// Configure ADC:
	ADC16_GetDefaultConfig(&adc16ConfigStruct);
	adc16ChannelConfigStruct.channelNumber                        = ADC16_USER_CHANNEL;
	adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = true; /* Enable the interrupt. */

	// Calibration for Positive/Negative (refer to SDK-Example)
	if (kStatus_Success == ADC16_DoAutoCalibration(ADC16_BASE))
	{
		printf("ADC16_DoAutoCalibration() Done.\r\n");
	}
	else
	{
		printf("ADC16_DoAutoCalibration() Failed.\r\n");
	}

	g_Adc16InterruptCounter = 0U;

	return;
}

//  Function: adcRead
//	Purpose: Uses the On-Chip ADC to read voltage
//	Outputs Voltage Reading
float adcRead(void){
	float adcValue;


	g_Adc16ConversionDoneFlag = false;
	ADC16_SetChannelConfig(ADC16_BASE, ADC16_CHANNEL_GROUP, &adc16ChannelConfigStruct);
	while (!g_Adc16ConversionDoneFlag)
	{
	}
	adcValue = (float) g_Adc16ConversionValue;
	if (adcValue > 4100U)
	{
		adcValue = 0; //65536U - adcValue;
	}

	// Maps reading to Voltage
	adcValue = adcValue / 4096.0 * 3.3  ;

	//PRINTF("ADC Value: %f[V]\t", adcValue)
	//PRINTF("ADC Value: %f[V]\t ADC Value: %f[ATM]\t", adcValue, adcValue*pressureScaling);
	//PRINTF("Duty: %d\t", duty_cycle);
	//PRINTF("ADC Interrupt Count: %d\r", g_Adc16InterruptCounter);



	return adcValue;
}


#endif
