
#include "board.h"

// ADC Interrupt:



/*******************************************************************************
 * ADC Prototypes and Interrupt Variables
 ******************************************************************************/



#define ADC16_BASE          ADC0
#define ADC16_CHANNEL_GROUP 0U
// This sets what ADC Signal you are using:
#define ADC16_USER_CHANNEL  18U

#define ADC16_IRQn             ADC0_IRQn
#define ADC16_IRQ_HANDLER_FUNC ADC0_IRQHandler

void ADC16_IRQ_HANDLER_FUNC(void);

void configureADC (void);


//void adcSetup(adc16_config_t, adc16_channel_config_t);
float adcRead(void);
