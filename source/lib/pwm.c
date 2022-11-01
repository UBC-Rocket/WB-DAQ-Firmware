#include "pwm.h"




uint32_t duty_cycle = 10;
uint32_t period = 100;

void PWM(uint32_t period, uint32_t duty);

void actuatorTask(void *pv){
	for(;;){
		if (uxSemaphoreGetCount( semaphore_PWMActive ) == 1) {
			PWM(period, duty_cycle); // Uses Global Variable changed in Control Task
		}
		else if(uxSemaphoreGetCount( semaphore_PWMActive ) == 0){
			printf("Waiting for Semaphore\n");
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
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