/*
 * @brief   Application entry point.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK66F18.h"
#include "fsl_debug_console.h"

#include <task.h>

#include "fsl_dspi.h"
#include "fsl_dspi_freertos.h"
#include "fsl_gpio.h"
#include "fsl_adc16.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void blinkTask(void *pv);
static void testTask(void *pv);
static void actuatorTask(void *pv);
static void ADCTask(void *pv);

// ADC Interrupt:
void ADC16_IRQ_HANDLER_FUNC(void);
void adcRead(adc16_config_t, adc16_channel_config_t);



/*******************************************************************************
 * ADC Global Variables From Example - Please do non-global after verification
 ******************************************************************************/
volatile bool g_Adc16ConversionDoneFlag = false;
volatile uint32_t g_Adc16ConversionValue;
volatile uint32_t g_Adc16InterruptCounter;
const uint32_t g_Adc16_12bitFullRange = 4096U;

const double adc_m_adjust = 1.0/4349.0;
const double adc_b_adjust = 0.058174292940906;

#define ADC16_BASE          ADC0
#define ADC16_CHANNEL_GROUP 0U
// This sets what ADC Signal you are using:
#define ADC16_USER_CHANNEL  18U

#define ADC16_IRQn             ADC0_IRQn
#define ADC16_IRQ_HANDLER_FUNC ADC0_IRQHandler



/*******************************************************************************
 * Main
 ******************************************************************************/

int main(void) {

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
#endif


    BaseType_t error;

    // Create the BlinkTest
    if ((error = xTaskCreate(
    blinkTask,
	"Blink LED Task",
    1024,
	NULL,
	0,
	NULL)) != pdPASS){
    	printf("Task init failed: %lf\n", error);
    	for (;;)
    		;

    };


    if ((error =  xTaskCreate(testTask,
    "Test Debugging Task",
	1024,
	NULL,
	0,
	NULL)) != pdPASS) {
    	printf("Task init failed: %ld\n", error);
    	for (;;)
    	    ;
    };

    if ((error =  xTaskCreate(actuatorTask,
    "Actuator Task",
	1024,
	NULL,
	0,
	NULL)) != pdPASS) {
    	printf("Task init failed: %ld\n", error);
    	for (;;)
    	    ;
    };

    if ((error =  xTaskCreate(ADCTask,
        "ADC Task",
    	1024,
    	NULL,
    	0,
    	NULL)) != pdPASS) {
        	printf("ADC Task init failed: %ld\n", error);
        	for (;;)
        	    ;
        };

    vTaskStartScheduler();

    for(;;);
}

/*******************************************************************************
 * Tasks
 ******************************************************************************/
static void blinkTask(void *pv) {
	while(1) {

		vTaskDelay(pdMS_TO_TICKS(500));
//		printf("BLINK");
	}
}

static void testTask(void *pv) {
	while(1) {
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

static void actuatorTask(void *pv){
	for(;;){
		// Toggle indefinitely to show they work, will add control loop later
		GPIO_PortToggle(BOARD_INITPINS_HS_SWITCH_B_IN0_GPIO, BOARD_INITPINS_HS_SWITCH_B_IN0_GPIO_PIN_MASK);
		GPIO_PortToggle(BOARD_INITPINS_HS_SWITCH_B_IN1_GPIO, BOARD_INITPINS_HS_SWITCH_B_IN1_GPIO_PIN_MASK);
		vTaskDelay(pdMS_TO_TICKS(200));
	}
}



static void ADCTask(void *pv) {
	adc16_config_t adc16ConfigStruct;
	adc16_channel_config_t adc16ChannelConfigStruct;

	BOARD_InitPins();
	BOARD_BootClockRUN();
	BOARD_InitDebugConsole();
	EnableIRQ(ADC16_IRQn);

	// Configure ADC:
	ADC16_GetDefaultConfig(&adc16ConfigStruct);
	//adc16ConfigStruct.referenceVoltageSource					  = 1U;
	adc16ChannelConfigStruct.channelNumber                        = ADC16_USER_CHANNEL;
	adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = true; /* Enable the interrupt. */

	// Calibration for Positive/Negative (refer to SDK-Example)
	if (kStatus_Success == ADC16_DoAutoCalibration(ADC16_BASE)) {
		PRINTF("ADC16_DoAutoCalibration() Done.\r\n");
	}
	else {
		PRINTF("ADC16_DoAutoCalibration() Failed.\r\n");
	}
	int16_t offset = 0U;
	ADC16_SetOffsetValue(ADC16_BASE, offset);

	g_Adc16InterruptCounter = 0U;

	// Read from ADC
	while (1)
	{
		vTaskDelay(pdMS_TO_TICKS(500));
		adcRead(adc16ConfigStruct, adc16ChannelConfigStruct);

	}
}
void ADC16_IRQ_HANDLER_FUNC(void)
{
    g_Adc16ConversionDoneFlag = true;
    /* Read conversion result to clear the conversion completed flag. */
    g_Adc16ConversionValue = ADC16_GetChannelConversionValue(ADC16_BASE, ADC16_CHANNEL_GROUP);
    g_Adc16InterruptCounter++;
    SDK_ISR_EXIT_BARRIER;
}

void adcRead(adc16_config_t adc16ConfigStruct, adc16_channel_config_t adc16ChannelConfigStruct){
	uint32_t adcValue;
	// Better to define these?
	double adc_corrected;


	g_Adc16ConversionDoneFlag = false;
	ADC16_SetChannelConfig(ADC16_BASE, ADC16_CHANNEL_GROUP, &adc16ChannelConfigStruct);
	while (!g_Adc16ConversionDoneFlag)
	{
	}
	adcValue = g_Adc16ConversionValue;

	// Temporarily resolve the PCB Layer error where VREFL is not grounded:
	// Two's complement anything above our expected maximum.
	if (adcValue > 4100) adcValue = adcValue - (1<<16);
	else adcValue = adcValue;

	adc_corrected = (adc_m_adjust*adcValue+adc_b_adjust);

	PRINTF("ADC Value: %d\r\n", adcValue);
	PRINTF("ADC Corrected: %lf \r\n", adc_corrected);
	PRINTF("ADC Interrupt Count: %d\r\n", g_Adc16InterruptCounter);
}

