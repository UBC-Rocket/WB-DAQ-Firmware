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
#include "fsl_i2c.h"
#include "fsl_i2c_freertos.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define TC_I2C_BASE     (I2C3_BASE)
#define TC_I2C_IRQN     (I2C3_IRQn)
#define TC_I2C_CLK_SRC  (I2C3_CLK_SRC)
#define TC_I2C_CLK_FREQ CLOCK_GetFreq((I2C3_CLK_SRC))


#define TC_I2C ((I2C_Type *)TC_I2C_BASE)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void blinkTask(void *pv);
static void testTask(void *pv);
static void actuatorTask(void *pv);
static void tcTask(void *pv);

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

    NVIC_SetPriority(TC_I2C_IRQN, 3);


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

    if ((error =  xTaskCreate(tcTask,
    "Thermocouple Task",
	1024,
	NULL,
	0,
	NULL)) != pdPASS) {
    	printf("Task init failed: %ld\n", error);
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

uint8_t g_master_buff[32] = {0};

static void tcTask(void *pv){
    i2c_rtos_handle_t master_rtos_handle;
    i2c_master_config_t masterConfig;
    i2c_master_transfer_t masterXfer;
    uint32_t sourceClock;
    status_t status;


	i2c_master_handle_t *g_m_handle;

    /*
     * masterConfig.baudRate_Bps = 100000U;
     * masterConfig.enableStopHold = false;
     * masterConfig.glitchFilterWidth = 0U;
     * masterConfig.enableMaster = true;
     */
    I2C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Bps = 10000;
    sourceClock               = TC_I2C_CLK_FREQ;

    status = I2C_RTOS_Init(&master_rtos_handle, TC_I2C, &masterConfig, sourceClock);



    for(;;){
		g_m_handle = &master_rtos_handle.drv_handle;

//		g_master_buff[0] = 0b00000001;
		g_master_buff[0] = 0b00000000;

		memset(&masterXfer, 0, sizeof(masterXfer));
		masterXfer.slaveAddress   = 0b1100111;
		masterXfer.direction      = kI2C_Write;
		masterXfer.subaddress     = 0;
		masterXfer.subaddressSize = 0;
		masterXfer.data           = g_master_buff;
		masterXfer.dataSize       = 1;
		masterXfer.flags          = kI2C_TransferDefaultFlag;

		status = I2C_RTOS_Transfer(&master_rtos_handle, &masterXfer);
		if (status != kStatus_Success)
		{
			PRINTF("I2C master: error during write transaction, %d\n", status);
		}


		for (uint32_t i = 0; i < 2; i++)
		{
			g_master_buff[i] = 0;
		}

		masterXfer.slaveAddress   = 0b1100111;
		masterXfer.direction      = kI2C_Read;
		masterXfer.subaddress     = 0;
		masterXfer.subaddressSize = 0;
		masterXfer.data           = g_master_buff;
		masterXfer.dataSize       = 2;
		masterXfer.flags          = kI2C_TransferDefaultFlag;

		status = I2C_RTOS_Transfer(&master_rtos_handle, &masterXfer);


		PRINTF("Received: ");
		for (uint32_t i = 0; i < 2; i++)
		{
			if (i % 8 == 0)
			{
				PRINTF("\r\n");
			}
			PRINTF("0x%2x  ", g_master_buff[i]);
		}
		PRINTF("\r\n\r\n");

		double temperature;
		uint8_t UpperByte = g_master_buff[0];
		uint8_t LowerByte = g_master_buff[1];
		if ((UpperByte & 0x80) == 0x80){ //Temperature  0°C
			temperature = (UpperByte * 16 + LowerByte / 16.0) - 4096;
		} else //Temperature  0°C
			temperature = (UpperByte * 16 + LowerByte / 16.0);

		PRINTF("Temperature read: %.4lf\r\n", temperature);

		vTaskDelay(pdMS_TO_TICKS(500));
    }
}

