/*
 * @brief   Application entry point.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
/// Need to put it back into the source folder
#include "adc.h"
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

#define BUFFER_SIZE 50
/*******************************************************************************
 * Task Prototypes
 ******************************************************************************/
static void mainTask(void *pv);
static void blinkTask(void *pv);

static void actuatorTask(void *pv);
static void ControlTask(void *pv);
static void tcTask(void *pv);
static void rttReceive(void *pv);



#define VALVE_PIN BOARD_INITPINS_HS_SWITCH_B_IN0_GPIO
#define VALVE_PIN_MASK BOARD_INITPINS_HS_SWITCH_B_IN0_GPIO_PIN_MASK

void PWM(uint32_t, uint32_t);
uint32_t duty_cycle = 10;
uint32_t period = 100;
float kp;



#define VALVE_PIN BOARD_INITPINS_HS_SWITCH_B_IN0_GPIO
#define VALVE_PIN_MASK BOARD_INITPINS_HS_SWITCH_B_IN0_GPIO_PIN_MASK

uint8_t pwm_active = 0;


// DPR Constants
#define PI_MAX 5
#define PI_MIN 0

#define DUTYCYCLE_MAX 100
#define DUTYCYCLE_MIN 0




typedef enum testConfigEnum
    {
        POTENTIOMETER, 
        KULITE
    } testConfigType;

testConfigType testConfig = POTENTIOMETER; // Swap this to Kulite or Potentiometer.
float pressureScaling = 3.3;

struct m_buffer m_buf;

TaskHandle_t actuator_task;

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

	// initialize m_buffer
	m_buffer_init(&m_buf);

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


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
// Main Tasks:
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


// Initialize Semaphores:
SemaphoreHandle_t semaphore_PWMActive;

// Create Message Type that takes specific values using Enumerate:

message_t message;

/*
// Description:	This task reads the current message and sets the program to different settings.
// Note:		This is not a queue and any message written twice before being acted on is overwritten.
//				Future Revision will likely be a queue.
*/
static void mainTask(void *pv){

	// Create Semaphores:
    semaphore_PWMActive = xSemaphoreCreateBinary();
	message_t working_buf[256];
	int8_t working_buf_size;

    // Forever Loop:
	for(;;){
		vTaskDelay(pdMS_TO_TICKS(200));

		xSemaphoreTakeRecursive(m_buf.lock, 0);
		working_buf_size = m_buf.size;
		for (int i = 0; i < working_buf_size; i++) {
			working_buf[i] = m_buffer_pop(&m_buf);
		}
		xSemaphoreGiveRecursive(m_buf.lock);

		// Feature Setting Logic:
		for (int i = 0; i < working_buf_size; i++) {
			switch(working_buf[i]){
				case PWM_Pause:
					vTaskSuspend(actuator_task);
					break;
				case PWM_Resume:
					vTaskResume(actuator_task);
					break;
				case No_Command:
					break;
			}
		}
	}

}

// Tuning Parameters:

// Task that is constantly waiting for RTT
static void rttReceive(void *pv) {
	char buffer[BUFFER_SIZE];
	unsigned int i = 0;


	// The Offset is where the numerical value begins in the string that I type to enter this command.
	// an example, if I enter "Kp: 100" this will set the Kp Parameter to 100, as 100 starts at index 4
	char period_str[] = "Period: ";
	int period_str_offset = 8;
	char kp_str[] = "Kp: ";
	int kp_str_offset = 4;


	while(1) {
		vTaskDelay(pdMS_TO_TICKS(200));
		//SEGGER_RTT_WriteString(0, "SEGGER Real-Time-Terminal Sample\r\n\r\n");

		i = SEGGER_RTT_Read(0, buffer, BUFFER_SIZE);
		buffer[i]= '\0';
		if(i != 0){
			if( strcmp(buffer, "p") == 0){
				m_buffer_push(&m_buf, PWM_Pause);
				SEGGER_RTT_WriteString(0, "Success: Received PWM_Pause Command./n");
			}
			else if( strcmp(buffer, "o") == 0){
				m_buffer_push(&m_buf, PWM_Resume);
				SEGGER_RTT_WriteString(0, "Success: Received PWM_Resume Command./n");
			}
			// PWM Period
			else if ( strstr(buffer, period_str)){
				period = strtol(buffer+period_str_offset, NULL, 10);
				printf("%P=d\n\r", period);
			}
			// Proportional Constant
			else if ( strstr(buffer, kp_str)){
				kp = strtod(buffer+kp_str_offset, NULL);
				printf("KP=%d\n\r", kp);
			}
			else{
				period = strtol(buffer, NULL, 10);
				printf("P=%d\n\r", period);
			}
			printf("%s\n", buffer);
		}
		else{
			//pass
		}
	}
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //



/*******************************************************************************
 * Tasks
 ******************************************************************************/
static void blinkTask(void *pv) {
	while(1) {

		vTaskDelay(pdMS_TO_TICKS(500));
//		printf("BLINK");
	}
}



static void ControlTask(void *pv) {
	// Create Message Type that takes specific values using Enumerate:
	typedef enum {
	        P,
	        PI,
	        PID
	    } pidMode_t;
	pidMode_t pidMode = P;

	BOARD_InitPins();
	BOARD_BootClockRUN();
	BOARD_InitDebugConsole();
	EnableIRQ(ADC16_IRQn);

	configureADC();

	// Set Pin to ON State
	//GPIO_PortSet(valvePin, valvePinMask);
	GPIO_PortClear(VALVE_PIN, VALVE_PIN_MASK);


	static char data_out[80];
	float error, desired_val;

	float sensor = adcRead(); // Reads in Voltage

	float out;
	// Integral Components:
	float sensor_old;
	float integral;
	float sample_time = 0.5;

	float ki = 0.1;

	int duty_offset = 50;

	switch(testConfig){
		case KULITE:
			// Reading from the Sensor
			desired_val = 10.0; // In ATM
			kp = 150.0;
			sensor = sensor * pressureScaling; // To convert Volts to ATM
			break;
		case POTENTIOMETER:
			// Reading from Potentiometer:
			desired_val = 1.5;
			kp = 1;//50;
			break;
		default:
			PRINTF("NO CONFIG SETUP");
			for(;;);
			break;
	}
  
	while (1)
	{
		vTaskDelay(pdMS_TO_TICKS(sample_time*1000)); // Delay in milliseconds
		sensor_old = sensor;
		sensor = adcRead();

		// TODO: set up logging with the new message buffer.
		// Currently, this will print garbage because message is no longer being used.
		sprintf(data_out, "%s, %f, %d, %d, %f\r", message, sensor*pressureScaling, duty_cycle, period, kp);
		SEGGER_RTT_WriteString(0, data_out);

		//sprintf(data_out, "%f\t\t %d\r", sensor*pressureScaling, duty_cycle);
		//SEGGER_RTT_WriteString(0, data_out);


		error = desired_val - (sensor);
		out = (int) (kp * error);

		switch(pidMode){
			case PI:
				integral = sample_time * (sensor - sensor_old);
				// Limit the Integral from accumulating
				if (integral >= PI_MAX) {
					integral = PI_MAX;
				}
				else if (integral <= PI_MIN) {
					integral = PI_MIN;
				}
				else {
					duty_cycle = duty_offset + (int) kp*(error) + (int) ki*(integral);
				}
				break;
			case P:
				if (duty_offset + out > DUTYCYCLE_MAX) {
					duty_cycle = DUTYCYCLE_MAX;
				}
				else if(duty_offset + out < DUTYCYCLE_MIN)  {
					duty_cycle = DUTYCYCLE_MIN;
				}
				else {
					duty_cycle = duty_offset + out;
				}
				break;
			default:
				printf("Control Task Error: Unknown Control Mode.");
				for(;;)
				break;
		}
	}
}




static void actuatorTask(void *pv){

	for(;;){
		PWM(period, duty_cycle);
	}
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


	// Turn on:
	GPIO_PortSet(VALVE_PIN, VALVE_PIN_MASK);
	vTaskDelay(pdMS_TO_TICKS(t2));
  
	// Turn off:
	GPIO_PortClear(VALVE_PIN, VALVE_PIN_MASK);
	vTaskDelay(pdMS_TO_TICKS(t1));
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

