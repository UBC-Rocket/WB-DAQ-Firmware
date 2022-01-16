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
 * Task Prototypes
 ******************************************************************************/
static void blinkTask(void *pv);
static void testTask(void *pv);
static void actuatorTask(void *pv);
static void ADCTask(void *pv);


/*******************************************************************************
 * ADC Prototypes and Interrupt Variables
 ******************************************************************************/
void adcSetup(adc16_config_t, adc16_channel_config_t);
void adcRead(adc16_config_t, adc16_channel_config_t);
void ADC16_IRQ_HANDLER_FUNC(void);

volatile bool g_Adc16ConversionDoneFlag = false;
volatile uint32_t g_Adc16ConversionValue;
volatile uint32_t g_Adc16InterruptCounter;

#define ADC16_BASE          ADC0
#define ADC16_CHANNEL_GROUP 0U
#define ADC16_USER_CHANNEL  18U	// This sets what ADC Signal you are using:

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
    	printf("Task init failed: %ld\n", error);
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

	adcSetup(adc16ConfigStruct, adc16ChannelConfigStruct);

	while (1)
	{
		adcRead(adc16ConfigStruct, adc16ChannelConfigStruct);
	}
}


/*******************************************************************************
 * ADC Functions
 ******************************************************************************/

void ADC16_IRQ_HANDLER_FUNC(void)
{
    g_Adc16ConversionDoneFlag = true;
    /* Read conversion result to clear the conversion completed flag. */
    g_Adc16ConversionValue = ADC16_GetChannelConversionValue(ADC16_BASE, ADC16_CHANNEL_GROUP);
    g_Adc16InterruptCounter++;
    SDK_ISR_EXIT_BARRIER;
}

void adcSetup(adc16_config_t adc16ConfigStruct, adc16_channel_config_t adc16ChannelConfigStruct){
	EnableIRQ(ADC16_IRQn);

	ADC16_GetDefaultConfig(&adc16ConfigStruct);

	adc16ChannelConfigStruct.channelNumber                        = ADC16_USER_CHANNEL;
	adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = true;

	g_Adc16InterruptCounter = 0U;

	if (kStatus_Success == ADC16_DoAutoCalibration(ADC16_BASE))
	{
		PRINTF("ADC16_DoAutoCalibration() Done.\r\n");
	}
	else
	{
		PRINTF("ADC16_DoAutoCalibration() Failed.\r\n");
	}
}

void adcRead(adc16_config_t adc16ConfigStruct, adc16_channel_config_t adc16ChannelConfigStruct){
	g_Adc16ConversionDoneFlag = false;
	ADC16_SetChannelConfig(ADC16_BASE, ADC16_CHANNEL_GROUP, &adc16ChannelConfigStruct);

	// Slow down Print to display
	while (!g_Adc16ConversionDoneFlag)
	{
	}

	PRINTF("ADC Value: %d\r\n", g_Adc16ConversionValue);
	PRINTF("ADC Interrupt Count: %d\r\n", g_Adc16InterruptCounter);

}




