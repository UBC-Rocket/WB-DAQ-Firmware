#include "rtt.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BUFFER_SIZE 50


// Task that is constantly waiting for RTT
void rttReceive(void *pv) {
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
				xSemaphoreTake(semaphore_Message, 10);
				message = PWM_Pause;
				xSemaphoreGive(semaphore_Message);
				SEGGER_RTT_WriteString(0, "Success: Received PWM_Pause Command./n");
			}
			else if( strcmp(buffer, "o") == 0){
				xSemaphoreTake(semaphore_Message, 10);
				message = PWM_Resume;
				xSemaphoreGive(semaphore_Message);
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