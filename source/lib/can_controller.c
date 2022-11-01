#include "can_controller.h"


SemaphoreHandle_t semaphore_Message;
SemaphoreHandle_t semaphore_PWMActive;

message_t message;


/*
// Description:	This task reads the current message and sets the program to different settings.
// Note:		This is not a queue and any message written twice before being acted on is overwritten.
//				Future Revision will likely be a queue.
*/
void mainTask(void *pv){

	// Create Semaphores:
    semaphore_Message = xSemaphoreCreateBinary();
    semaphore_PWMActive = xSemaphoreCreateBinary();


    // Forever Loop:
	for(;;){
		vTaskDelay(pdMS_TO_TICKS(200));

		// Wait for message
		while(uxSemaphoreGetCount(semaphore_Message) == 0){
		};

		// Semaphore is Available, so now we can take it and read the message:
		xSemaphoreTake(semaphore_Message, 10);

		// Feature Setting Logic:
		switch(message){
			case PWM_Pause:
				xSemaphoreTake( semaphore_PWMActive, 10 );
				printf("Command Executed: PWM_Pause.\n");
				break;
			case PWM_Resume:
				xSemaphoreGive(semaphore_PWMActive);
				printf("Command Executed: PWM_Resume.\n");
				break;
			case No_Command:
				break;
		}
		message = No_Command;
		xSemaphoreGive(semaphore_Message);
	}

}