/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "fsl_dspi.h"
#include "fsl_dspi_freertos.h"

/*******************************************************************************
 * Tempsorary
 ******************************************************************************/
#define EXAMPLE_DSPI_MASTER_BASE (SPI0_BASE)
#define EXAMPLE_DSPI_MASTER_IRQN (SPI0_IRQn)
#define DSPI_MASTER_CLK_SRC      (DSPI0_CLK_SRC)
#define DSPI_MASTER_CLK_FREQ     CLOCK_GetFreq((DSPI0_CLK_SRC))
#define EXAMPLE_DSPI_MASTER_BASEADDR ((SPI_Type *)EXAMPLE_DSPI_MASTER_BASE)


#define TRANSFER_SIZE     (16)     /*! Transfer size */
#define TRANSFER_BAUDRATE (500000U) /*! Transfer baudrate - 500k */



/*
 * send a signal to turn the highside switch on or off
*/
void switchSignal(void *pv);

