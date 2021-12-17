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
void DEMO_ADC16_IRQ_HANDLER_FUNC(void);



/*******************************************************************************
 * ADC Global Variables From Example - Please do non-global after verification
 ******************************************************************************/
volatile bool g_Adc16ConversionDoneFlag = false;
volatile uint32_t g_Adc16ConversionValue;
volatile uint32_t g_Adc16InterruptCounter;
const uint32_t g_Adc16_12bitFullRange = 4096U;

#define DEMO_ADC16_BASE          ADC0
#define DEMO_ADC16_CHANNEL_GROUP 0U
// This sets what ADC Signal you are using:
#define DEMO_ADC16_USER_CHANNEL  18U

#define DEMO_ADC16_IRQn             ADC0_IRQn
#define DEMO_ADC16_IRQ_HANDLER_FUNC ADC0_IRQHandler



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
	while(1) {
		//vTaskDelay(pdMS_TO_TICKS(500));
		adc16_config_t adc16ConfigStruct;
		adc16_channel_config_t adc16ChannelConfigStruct;

		BOARD_InitPins();
		BOARD_BootClockRUN();
		BOARD_InitDebugConsole();
		EnableIRQ(DEMO_ADC16_IRQn);

		PRINTF("\r\nADC16 interrupt Example.\r\n");

		ADC16_GetDefaultConfig(&adc16ConfigStruct);

		PRINTF("ADC Full Range: %d\r\n", g_Adc16_12bitFullRange);
		PRINTF("Press any key to get user channel's ADC value ...\r\n");

		adc16ChannelConfigStruct.channelNumber                        = DEMO_ADC16_USER_CHANNEL;
		adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = true; /* Enable the interrupt. */



		// Calibration for Positive/Negative
		if (kStatus_Success == ADC16_DoAutoCalibration(DEMO_ADC16_BASE))
		{
			PRINTF("ADC16_DoAutoCalibration() Done.\r\n");
		}
		else
		{
			PRINTF("ADC16_DoAutoCalibration() Failed.\r\n");
		}



		g_Adc16InterruptCounter = 0U;

		while (1)
		{
			GETCHAR();
			g_Adc16ConversionDoneFlag = false;
			/*
			 When in software trigger mode, each conversion would be launched once calling the "ADC16_ChannelConfigure()"
			 function, which works like writing a conversion command and executing it. For another channel's conversion,
			 just to change the "channelNumber" field in channel configuration structure, and call the function
			 "ADC16_ChannelConfigure()"" again.
			 Also, the "enableInterruptOnConversionCompleted" inside the channel configuration structure is a parameter for
			 the conversion command. It takes affect just for the current conversion. If the interrupt is still required
			 for the following conversion, it is necessary to assert the "enableInterruptOnConversionCompleted" every time
			 for each command.
			*/
			ADC16_SetChannelConfig(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP, &adc16ChannelConfigStruct);
			while (!g_Adc16ConversionDoneFlag)
			{
			}
			PRINTF("ADC Value: %d\r\n", g_Adc16ConversionValue);
			PRINTF("ADC Interrupt Count: %d\r\n", g_Adc16InterruptCounter);
		}
	}
}
void DEMO_ADC16_IRQ_HANDLER_FUNC(void)
{
    g_Adc16ConversionDoneFlag = true;
    /* Read conversion result to clear the conversion completed flag. */
    g_Adc16ConversionValue = ADC16_GetChannelConversionValue(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP);
    g_Adc16InterruptCounter++;
    SDK_ISR_EXIT_BARRIER;
}

