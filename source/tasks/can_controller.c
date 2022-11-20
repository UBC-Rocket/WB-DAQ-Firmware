#include "can_controller.h"

extern struct mBuffer mBuf;

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
    semaphore_PWMActive = xSemaphoreCreateBinary();
	message_t working_buf[256];
	int8_t working_buf_size;

    // Forever Loop:
	for(;;){
		vTaskDelay(pdMS_TO_TICKS(200));

		xSemaphoreTakeRecursive(mBuf.lock, 0);
		working_buf_size = mBuf.size;
		for (int i = 0; i < working_buf_size; i++) {
			working_buf[i] = mBufferPop(&mBuf);
		}
		xSemaphoreGiveRecursive(mBuf.lock);

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
