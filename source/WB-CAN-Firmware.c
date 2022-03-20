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
#include "fsl_i2c.h"
#include "fsl_i2c_freertos.h"

#include "SEGGER_RTT.h"


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
 * Task Prototypes
 ******************************************************************************/
static void blinkTask(void *pv);
static void testTask(void *pv);
static void actuatorTask(void *pv);
static void ControlTask(void *pv);
static void tcTask(void *pv);

// ADC Interrupt:
void ADC16_IRQ_HANDLER_FUNC(void);


/*******************************************************************************
 * ADC Prototypes and Interrupt Variables
 ******************************************************************************/
void adcSetup(adc16_config_t, adc16_channel_config_t);
float adcRead(adc16_config_t, adc16_channel_config_t);
void ADC16_IRQ_HANDLER_FUNC(void);

volatile bool g_Adc16ConversionDoneFlag = false;
volatile uint32_t g_Adc16ConversionValue;
volatile uint32_t g_Adc16InterruptCounter;
const uint32_t g_Adc16_12bitFullRange = 4096U;

#define ADC16_BASE          ADC0
#define ADC16_CHANNEL_GROUP 0U
// This sets what ADC Signal you are using:
#define ADC16_USER_CHANNEL  18U

#define ADC16_IRQn             ADC0_IRQn
#define ADC16_IRQ_HANDLER_FUNC ADC0_IRQHandler


#define valvePin BOARD_INITPINS_HS_SWITCH_B_IN0_GPIO
#define valvePinMask BOARD_INITPINS_HS_SWITCH_B_IN0_GPIO_PIN_MASK

void PWM(uint32_t, uint32_t);
uint32_t duty_cycle = 10;

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


    if ((error =  xTaskCreate(testTask,
		"Test Debugging Task",
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
		NULL)) != pdPASS) {
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

static void testTask(void *pv) {
	while(1) {
		vTaskDelay(pdMS_TO_TICKS(500));
		SEGGER_RTT_WriteString(0, "SEGGER Real-Time-Terminal Sample\r\n\r\n");
	}
}

static void actuatorTask(void *pv){
	uint32_t period = 100;
	for(;;){
		PWM(period, duty_cycle); // Uses Global Variable changed in Control Task
	}
}


static void ControlTask(void *pv) {
	adc16_config_t adc16ConfigStruct;
	adc16_channel_config_t adc16ChannelConfigStruct;


	BOARD_InitPins();
	BOARD_BootClockRUN();
	BOARD_InitDebugConsole();
	EnableIRQ(ADC16_IRQn);

	// Configure ADC:
	ADC16_GetDefaultConfig(&adc16ConfigStruct);
	adc16ChannelConfigStruct.channelNumber                        = ADC16_USER_CHANNEL;
	adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = true; /* Enable the interrupt. */

	// Calibration for Positive/Negative (refer to SDK-Example)
	if (kStatus_Success == ADC16_DoAutoCalibration(ADC16_BASE))
	{
		PRINTF("ADC16_DoAutoCalibration() Done.\r\n");
	}
	else
	{
		PRINTF("ADC16_DoAutoCalibration() Failed.\r\n");
	}

	g_Adc16InterruptCounter = 0U;

	// Set Pin to ON State
	//GPIO_PortSet(valvePin, valvePinMask);
	GPIO_PortClear(valvePin, valvePinMask);
	// PID Loop:
	// Mode: P = 1, PI = 2
	uint32_t mode = 1;
	// Reading from Potentiometer:
	float desired_val = 1.700; // In ATM
	// Reading from Sensor:
	//float desired_val = 20;

	float error;
	float sensor = adcRead(adc16ConfigStruct, adc16ChannelConfigStruct);
	float out;
	// Integral Components:
	float sensor_old;
	float integral;
	float sample_time = 0.5;

	float ki = 0.1;
	float kp = 150.0;
	int duty_offset = 50;

	while (1)
	{
		vTaskDelay(pdMS_TO_TICKS(sample_time*1000)); // Delay in milliseconds
		sensor_old = sensor;
		sensor = adcRead(adc16ConfigStruct, adc16ChannelConfigStruct);

		error = desired_val - (sensor);
		out = (int) (kp * error);


		if (mode == 2){ // PI Controller:
			integral = sample_time * (sensor - sensor_old);
			if (integral >= 5) integral = 5;			// Limit the Integral from accumulating
			if (integral <= 0) integral = 0;
			duty_cycle = duty_offset + (int) kp*(error) + (int) ki*(integral);
		}

		if (mode == 1){ // P Controller with limited DutyCycle:
			if (duty_offset + out > 100) 	duty_cycle = 100;
			else if(duty_offset + out < 0)  duty_cycle = 0;
			else duty_cycle = duty_offset + out;
		}
	}
}

void ADC16_IRQ_HANDLER_FUNC(void)
{
    g_Adc16ConversionDoneFlag = true;
    /* Read conversion result to clear the conversion completed flag. */
    g_Adc16ConversionValue = ADC16_GetChannelConversionValue(ADC16_BASE, ADC16_CHANNEL_GROUP);
    g_Adc16InterruptCounter++;
    SDK_ISR_EXIT_BARRIER;
}

float adcRead(adc16_config_t adc16ConfigStruct, adc16_channel_config_t adc16ChannelConfigStruct){
	float adcValue;
	float adcValueVolts;
	float adcValueATM;

	g_Adc16ConversionDoneFlag = false;
	ADC16_SetChannelConfig(ADC16_BASE, ADC16_CHANNEL_GROUP, &adc16ChannelConfigStruct);
	while (!g_Adc16ConversionDoneFlag)
	{
	}
	adcValue = (float) g_Adc16ConversionValue;
	if (adcValue > 4100U)
	{
		adcValue = 0; //65536U - adcValue;
	}


	// Duty Cycle from 0-100 Used for Potentiometer Setup:
	//adcValue = adcValue / 4096.0 * 100.0; // Reads in Percentage
	adcValue = adcValue / 4096.0 * 3.3  ; // Reads in Voltage

	// When Reading from Pressure Sensor -  Double check this conversion
	adcValueATM = adcValue * 3.30;

	//PRINTF("ADC Value: %f[V]\t", adcValue)
	PRINTF("ADC Value: %f[ATM]\t", adcValueATM);
	PRINTF("Duty: %d\t", duty_cycle);
	//PRINTF("ADC Value: %d\t", (int)(adcValue / 4096.0 * 3300 * 1000 / 330));
	PRINTF("ADC Interrupt Count: %d\r", g_Adc16InterruptCounter);

	return adcValueATM;
}

// PWM
//	- Duty cycle uint32 between 0 and 100
//

void PWM(uint32_t period, uint32_t duty){
	if (duty < 0 || duty > 100){
		printf("Entered Duty Cycle is out of bounds. \n");
		for(;;){
			// Better Debugging later
		}
	}

	// This Should be Integers? Debug and verify T1 and T2
	// Put Breakpoints to verify t1 and t2
	float t1 = period * (100 - duty)/100;
	float t2 = period * (duty)/100;

	// Turn off:
	GPIO_PortClear(valvePin, valvePinMask);
	vTaskDelay(pdMS_TO_TICKS(t1));
	// Turn on:
	GPIO_PortSet(valvePin, valvePinMask);
	vTaskDelay(pdMS_TO_TICKS(t2));
}




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

static void tcTask(void *pv){
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

