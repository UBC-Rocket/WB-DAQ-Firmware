#include "highside_switch.h"
#include "stdio.h"

uint8_t masterReceiveBuffer[TRANSFER_SIZE] = {0};
uint8_t masterSendBuffer[TRANSFER_SIZE]    = {0b00000000000011000, 0b10111101};


dspi_transfer_t masterXfer;
dspi_rtos_handle_t master_rtos_handle;
dspi_master_config_t masterConfig;
uint32_t sourceClock;
status_t status;
SemaphoreHandle_t dspi_sem;


void switchSignal(void *pv) {
    dspi_sem = xSemaphoreCreateBinary();
    DSPI_MasterGetDefaultConfig(&masterConfig);

    masterConfig.ctarConfig.bitsPerFrame = 16;
    masterConfig.ctarConfig.cpol = kDSPI_ClockPolarityActiveLow;
    masterConfig.whichPcs = kDSPI_Pcs1;

    sourceClock = DSPI_MASTER_CLK_FREQ;
    status = DSPI_RTOS_Init(&master_rtos_handle, EXAMPLE_DSPI_MASTER_BASEADDR, &masterConfig, sourceClock);

    if (status != kStatus_Success)
    {
        //PRINTF("DSPI master: error during initialization. \r\n");
        vTaskSuspend(NULL);
    }
    /*Start master transfer*/
    masterXfer.txData      = masterSendBuffer;
    masterXfer.rxData      = masterReceiveBuffer;
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



