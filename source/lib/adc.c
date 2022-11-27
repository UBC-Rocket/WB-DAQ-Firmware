#ifndef ADC_H
#define ADC_H

#include <lib/adc.h>
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


/*
 * Function: ConfigureADC
 * Purpose: Setup the ADC on the chip
 * */
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

/*
 * Function: adcRead
 * Purpose: Uses the On-Chip ADC to read voltage
 * Outputs Voltage Reading
 */
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




	return adcValue;
}



#include "adc.h"
#include "stdio.h"

uint8_t SPI1_ReceiveBuffer[TRANSFER_SIZE];// = {0};
uint8_t SPI1_SendBuffer[TRANSFER_SIZE]    = {0b01101000};//{0b1100000000001111};

/*
 * Purpose: Master Task for SPI
 */

void adc_spi(void *pv) {

	dspi_transfer_t masterXfer;
	dspi_rtos_handle_t master_rtos_handle;
	dspi_master_config_t masterConfig;
	uint32_t sourceClock;
	status_t status;
	SemaphoreHandle_t dspi_sem;


	dspi_sem = xSemaphoreCreateBinary();
	DSPI_MasterGetDefaultConfig(&masterConfig);

	masterConfig.ctarConfig.bitsPerFrame = 16;
	masterConfig.ctarConfig.cpol = kDSPI_ClockPolarityActiveLow;
	masterConfig.whichPcs = kDSPI_Pcs1;

	sourceClock = ADC_DSPI1_CLK_FREQ;
	status = DSPI_RTOS_Init(&master_rtos_handle, ADC_DSPI1_BASEADDR, &masterConfig, sourceClock);

	if (status != kStatus_Success)
	{
		//PRINTF("DSPI master: error during initialization. \r\n");
		vTaskSuspend(NULL);
	}
	/*Start master transfer*/
	masterXfer.txData      =  SPI1_SendBuffer;
	masterXfer.rxData      =  SPI1_ReceiveBuffer;
	masterXfer.dataSize    = TRANSFER_SIZE;
	masterXfer.configFlags = kDSPI_MasterCtar0 | kDSPI_MasterPcs0 | kDSPI_MasterPcsContinuous;        // why was this kDSPI_MasterPcs0

	status = DSPI_RTOS_Transfer(&master_rtos_handle, &masterXfer);

	printf("RX: ");
	for(int i = 0; i < TRANSFER_SIZE; ++i){
		printf(" %d ", masterXfer.rxData[i]);
	}
	printf("\n");

	if (status == kStatus_Success)
	{
		xSemaphoreGive(dspi_sem);
	}

	while(1){
		//masterXfer.txData[1] ^= 1UL << 7;
		//masterXfer.txData[1] ^= 1UL << 6;

		// Toggle the Watchdog Bit of the TX Data
		masterXfer.txData[1] ^=  1UL << 16;

		status = DSPI_RTOS_Transfer(&master_rtos_handle, &masterXfer);
		printf("TX: ");
		for(int i = 0; i < TRANSFER_SIZE; ++i){
			printf(" %d ", masterXfer.txData[i]);
		}
		printf("\n");
		printf("RX: ");
				for(int i = 0; i < TRANSFER_SIZE; ++i){
					printf(" %d ", masterXfer.rxData[i]);
				}
				printf("\n");
		vTaskDelay(pdMS_TO_TICKS(100));
	}

}




#endif
