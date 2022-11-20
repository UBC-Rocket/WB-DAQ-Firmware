#include "pid.h"

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

float kp;

float pressureScaling = 3.3;


void ControlTask(void *pv) {
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
