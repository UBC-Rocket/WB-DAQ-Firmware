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
#define TC_I2C1_BASE     (I2C1_BASE)
#define TC_I2C1_IRQN     (I2C1_IRQn)
#define TC_I2C1_CLK_SRC  (I2C1_CLK_SRC)
#define TC_I2C1_CLK_FREQ CLOCK_GetFreq((I2C3_CLK_SRC))
#define TC_I2C1 ((I2C_Type *)TC_I2C1_BASE)

#define TC_I2C3_BASE     (I2C3_BASE)
#define TC_I2C3_IRQN     (I2C3_IRQn)
#define TC_I2C3_CLK_SRC  (I2C3_CLK_SRC)
#define TC_I2C3_CLK_FREQ CLOCK_GetFreq((I2C3_CLK_SRC))
#define TC_I2C3 ((I2C_Type *)TC_I2C3_BASE)

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

    NVIC_SetPriority(TC_I2C1_IRQN, 3);
    NVIC_SetPriority(TC_I2C3_IRQN, 3);


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


/*
 * Reads from a thermocouple. If we need more configuration I might make this more general to get arbitrary registers
 * @param handle the handle of the i2c instance
 * @param slaveAddress the slave that you are addressing, for the current board this should be 0b1100000 or 0b1100111
 * @return the temperature at the hot junction
 */
static float readTc(i2c_rtos_handle_t *handle, uint8_t slaveAddress){
    i2c_master_transfer_t transfer;
    status_t status;
	uint8_t buf[2] = {0};
	buf[0] = 0b00000000;

	memset(&transfer, 0, sizeof(transfer));
	transfer.slaveAddress   = 0b1100111;
	transfer.direction      = kI2C_Write;
	transfer.subaddress     = 0;
	transfer.subaddressSize = 0;
	transfer.data           = buf;
	transfer.dataSize       = 1;
	transfer.flags          = kI2C_TransferDefaultFlag;

	status = I2C_RTOS_Transfer(handle, &transfer);
	if (status != kStatus_Success)
	{
		printf("I2C master: error during write transaction, %d\n", status);
	}


	for (uint32_t i = 0; i < 2; i++)
	{
		buf[i] = 0;
	}

	transfer.slaveAddress   = 0b1100111;
	transfer.direction      = kI2C_Read;
	transfer.subaddress     = 0;
	transfer.subaddressSize = 0;
	transfer.data           = buf;
	transfer.dataSize       = 2;
	transfer.flags          = kI2C_TransferDefaultFlag;

	status = I2C_RTOS_Transfer(handle, &transfer);
	if (status != kStatus_Success)
	{
		printf("I2C master: error during read transaction, %d\n", status);
	}


	printf("Received: ");
	for (uint32_t i = 0; i < 2; i++)
	{
		if (i % 8 == 0)
		{
			printf("\r\n");
		}
		printf("0x%2x  ", buf[i]);
	}
	printf("\r\n\r\n");

	double temperature;
	uint8_t UpperByte = buf[0];
	uint8_t LowerByte = buf[1];
	if ((UpperByte & 0x80) == 0x80){ //Temperature  0°C
		temperature = (UpperByte * 16 + LowerByte / 16.0) - 4096;
	} else //Temperature  0°C
		temperature = (UpperByte * 16 + LowerByte / 16.0);
	return temperature;
}

static void tcTask(void *pv){
    i2c_rtos_handle_t i2c1_handle;
    i2c_rtos_handle_t i2c3_handle;
    i2c_master_config_t i2c_config;


    // We assume config is same between I2C instances
    I2C_MasterGetDefaultConfig(&i2c_config);
    i2c_config.baudRate_Bps = 10000;

    I2C_RTOS_Init(&i2c1_handle, TC_I2C1, &i2c_config, TC_I2C1_CLK_FREQ);
    I2C_RTOS_Init(&i2c3_handle, TC_I2C3, &i2c_config, TC_I2C3_CLK_FREQ);

    for(;;){
    	float temperature = readTc(&i2c1_handle, 0b1100111);


		printf("Temperature read: %.4f\r\n", temperature);

		vTaskDelay(pdMS_TO_TICKS(500));
    }
}

