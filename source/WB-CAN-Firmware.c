/*
 * @brief   Application entry point.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
/// Need to put it back into the source folder
#include <tasks/adc.h>
#include <tasks/tc.h>
#include <tasks/rtt.h>
#include <tasks/pwm.h>
#include <tasks/pid.h>
#include <tasks/can_controller.h>
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

#include "fsl_i2c.h"
#include "fsl_i2c_freertos.h"

#include "SEGGER_RTT.h"

#include "message_types.h"
#include "buffer.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************
 * Task Prototypes
 ******************************************************************************/
static void blinkTask(void *pv);
/*******************************************************************************
 * Main
 ******************************************************************************/

int main(void) {

    /* Init board hardware. */
   	BOARD_InitPins();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
#endif

    NVIC_SetPriority(TC_I2C1_IRQN, 3);
    NVIC_SetPriority(TC_I2C3_IRQN, 3);


    BaseType_t error;

	// initialize mBuffer
	mBufferInit(&mBuf);

    // Create the BlinkTest
    if ((error = xTaskCreate(blinkTask,
		"Blink LED Task",
		512,
		NULL,
		0,
		NULL)) != pdPASS){
			printf("Task init failed: %ld\n", error);
			for (;;)
				;
    };


    if ((error =  xTaskCreate(mainTask,
		"Main Task",
		512,
		NULL,
		0,
		NULL)) != pdPASS) {
			printf("Task init failed: %ld\n", error);
			for (;;)
				;
    };


    if ((error =  xTaskCreate(rttReceive,
    		"RTT Receive Task",
    		512,
    		NULL,
    		0,
    		NULL)) != pdPASS) {
    			printf("Task init failed: %ld\n", error);
    			for (;;)
    				;
        };

    if ((error =  xTaskCreate(actuatorTask,
		"Actuator Task",
		512,
		NULL,
		0,
		&actuator_task)) != pdPASS) {
			printf("Task init failed: %ld\n", error);
			for (;;)
				;
		};

    if ((error =  xTaskCreate(tcTask,
        "Thermocouple Task",
    	512,
    	NULL,
    	0,
    	NULL)) != pdPASS) {
        	printf("Task init failed: %ld\n", error);
        	for (;;)
        	    ;
        };

    if ((error =  xTaskCreate(ControlTask,
        "Control Task",
    	512,
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
