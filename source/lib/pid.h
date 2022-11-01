//#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "adc.h"
#include "fsl_gpio.h"
#include "pwm.h"
#include "fsl_debug_console.h"
#include "SEGGER_RTT.h"

extern float kp;



void ControlTask(void *pv);
