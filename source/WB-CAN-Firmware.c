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

/*******************************************************************************
 * Temporary
 ******************************************************************************/
#define EXAMPLE_DSPI_MASTER_BASE (SPI1_BASE)
#define EXAMPLE_DSPI_MASTER_IRQN (SPI1_IRQn)
#define DSPI_MASTER_CLK_SRC      (DSPI1_CLK_SRC)
#define DSPI_MASTER_CLK_FREQ     CLOCK_GetFreq((DSPI1_CLK_SRC))
#define EXAMPLE_DSPI_MASTER_BASEADDR ((SPI_Type *)EXAMPLE_DSPI_MASTER_BASE)

#define TRANSFER_SIZE     (2)     /*! Transfer size */
#define TRANSFER_BAUDRATE (500000U) /*! Transfer baudrate - 500k */


uint8_t masterReceiveBuffer[TRANSFER_SIZE] = {0};
uint8_t masterSendBuffer[TRANSFER_SIZE]    = {0};
//uint8_t masterSendBuffer[TRANSFER_SIZE] = {0b01110010, 0b00000010};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void blinkTask(void *pv);
static void testTask(void *pv);
static void actuatorTask(void *pv);
static void adcTask(void *pv);

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

    NVIC_SetPriority(EXAMPLE_DSPI_MASTER_IRQN, 3);


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

    if ((error =  xTaskCreate(adcTask,
    "ADC Task",
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

static void adcTask(void *pv){
//	for(;;){
//		vTaskDelay(pdMS_TO_TICKS(200));
//	}
	dspi_transfer_t masterXfer;
    dspi_rtos_handle_t master_rtos_handle;
    dspi_master_config_t masterConfig;
    uint32_t sourceClock;
    status_t status;


    /*Master config*/
//    masterConfig.whichCtar                                = kDSPI_Ctar0;
//    masterConfig.ctarConfig.baudRate                      = TRANSFER_BAUDRATE;
//    masterConfig.ctarConfig.bitsPerFrame                  = 8;
//    masterConfig.ctarConfig.cpol                          = kDSPI_ClockPolarityActiveHigh;
//    masterConfig.ctarConfig.cpha                          = kDSPI_ClockPhaseFirstEdge;
//    masterConfig.ctarConfig.direction                     = kDSPI_MsbFirst;
//    masterConfig.ctarConfig.pcsToSckDelayInNanoSec        = 2000;
//    masterConfig.ctarConfig.lastSckToPcsDelayInNanoSec    = 2000;
//    masterConfig.ctarConfig.betweenTransferDelayInNanoSec = 1000;
//
//    masterConfig.whichPcs           = kDSPI_Pcs0;
//    masterConfig.pcsActiveHighOrLow = kDSPI_PcsActiveLow;
//
//    masterConfig.enableContinuousSCK        = false;
//    masterConfig.enableRxFifoOverWrite      = false;
//    masterConfig.enableModifiedTimingFormat = false;
//    masterConfig.samplePoint                = kDSPI_SckToSin0Clock;

    DSPI_MasterGetDefaultConfig(&masterConfig);
    masterConfig.ctarConfig.bitsPerFrame = 14;
    masterConfig.ctarConfig.cpol = kDSPI_ClockPolarityActiveLow;
    masterConfig.whichPcs = kDSPI_Pcs0;

    sourceClock = DSPI_MASTER_CLK_FREQ;
    status      = DSPI_RTOS_Init(&master_rtos_handle, EXAMPLE_DSPI_MASTER_BASEADDR, &masterConfig, sourceClock);

    if (status != kStatus_Success)
    {
        PRINTF("DSPI master: error during initialization. \r\n");
        vTaskSuspend(NULL);
    }
    /*Start master transfer*/
    masterXfer.txData      = masterSendBuffer;
    masterXfer.rxData      = masterReceiveBuffer;
    masterXfer.dataSize    = TRANSFER_SIZE;
    masterXfer.configFlags = kDSPI_MasterCtar0 | kDSPI_MasterPcs0 | kDSPI_MasterPcsContinuous;

    status = DSPI_RTOS_Transfer(&master_rtos_handle, &masterXfer);

    printf("RX: ");
    for(int i = 0; i < TRANSFER_SIZE; ++i){
    	printf(" %d ", masterXfer.rxData[i]);
    }
    printf("\n");


	while(1){
		status = DSPI_RTOS_Transfer(&master_rtos_handle, &masterXfer);
		printf("RX: ");
		for(int i = 0; i < TRANSFER_SIZE; ++i){
			printf(" %d ", masterXfer.rxData[i]);
		}
		printf("\n");
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

