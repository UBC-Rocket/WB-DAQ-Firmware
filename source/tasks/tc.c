#include "tc.h"



/*
 * Reads from a thermocouple. If we need more configuration I might make this more general to get arbitrary registers
 * This function effectively implements the implementation example on page 22 of the data sheet:
 * https://ww1.microchip.com/downloads/en/DeviceDoc/MCP960X-Data-Sheet-20005426.pdf
 * @param handle the handle of the i2c instance
 * @param slaveAddress the slave that you are addressing, for the current board this should be 0b1100000 or 0b1100111
 * @return the temperature at the hot junction
 */
static float readTc(i2c_rtos_handle_t *handle, uint8_t slaveAddress){
    i2c_master_transfer_t transfer;
    status_t status;
	uint8_t buf[2] = {0};

	// Send a write command with value Th = 0x00
	buf[0] = 0b00000000;

	memset(&transfer, 0, sizeof(transfer));
	transfer.slaveAddress   = slaveAddress;
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

	transfer.slaveAddress   = slaveAddress;
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

	// Convert back to temperature
	// How to do this is on the datasheet of the TC amplifiers
	float temperature;
	uint8_t UpperByte = buf[0];
	uint8_t LowerByte = buf[1];
	if ((UpperByte & 0x80) == 0x80){ //Temperature  0°C
		temperature = (UpperByte * 16 + LowerByte / 16.0) - 4096;
	} else //Temperature  0°C
		temperature = (UpperByte * 16 + LowerByte / 16.0);
	return temperature;
}

void tcTask(void *pv){
    i2c_rtos_handle_t i2c1_handle;
    i2c_rtos_handle_t i2c3_handle;
    i2c_master_config_t i2c_config;


    // We assume config is same between I2C instances
    I2C_MasterGetDefaultConfig(&i2c_config);
    i2c_config.baudRate_Bps = 10000;

    // Initialize both I2C instances
    I2C_RTOS_Init(&i2c1_handle, TC_I2C1, &i2c_config, TC_I2C1_CLK_FREQ);
    I2C_RTOS_Init(&i2c3_handle, TC_I2C3, &i2c_config, TC_I2C3_CLK_FREQ);

    for(;;){
    	float temperature = readTc(&i2c1_handle, 0b1100111);
		printf("Temperature read: %.4f\r\n", temperature);

		vTaskDelay(pdMS_TO_TICKS(500));
    }
}
