#ifndef _TC_H_
#define _TC_H_

#include "fsl_i2c.h"
#include "fsl_i2c_freertos.h"
#include <stdio.h>


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


void tcTask(void *pv);


#endif
