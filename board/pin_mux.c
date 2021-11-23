/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Pins v8.0
processor: MK66FX1M0xxx18
package_id: MK66FX1M0VLQ18
mcu_data: ksdk2_0
processor_version: 8.0.1
pin_labels:
- {pin_num: '111', pin_signal: CMP0_IN0/PTC6/LLWU_P10/SPI0_SOUT/PDB0_EXTRG/I2S0_RX_BCLK/FB_AD9/SDRAM_A17/I2S0_MCLK, label: HS_SWITCH_B_IN0, identifier: IN0;HS_SWITCH_IN0;HS_SWITCH_B_IN0}
- {pin_num: '112', pin_signal: CMP0_IN1/PTC7/SPI0_SIN/USB0_SOF_OUT/I2S0_RX_FS/FB_AD8/SDRAM_A16, label: IN1, identifier: IN1}
- {pin_num: '110', pin_signal: PTC5/LLWU_P9/SPI0_SCK/LPTMR0_ALT2/I2S0_RXD0/FB_AD10/SDRAM_A18/CMP0_OUT/FTM0_CH2, label: HS_SWITCH_B_IN1, identifier: IN1;HS_SWITCH_B_IN0;HS_SWITCH_B_IN1}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

#include "fsl_common.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "pin_mux.h"

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitBootPins
 * Description   : Calls initialization functions.
 *
 * END ****************************************************************************************************************/
void BOARD_InitBootPins(void)
{
    BOARD_InitPins();
}

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitPins:
- options: {callFromInitBoot: 'true', coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: '111', peripheral: GPIOC, signal: 'GPIO, 6', pin_signal: CMP0_IN0/PTC6/LLWU_P10/SPI0_SOUT/PDB0_EXTRG/I2S0_RX_BCLK/FB_AD9/SDRAM_A17/I2S0_MCLK, identifier: HS_SWITCH_B_IN0,
    direction: OUTPUT, gpio_init_state: 'true'}
  - {pin_num: '110', peripheral: GPIOC, signal: 'GPIO, 5', pin_signal: PTC5/LLWU_P9/SPI0_SCK/LPTMR0_ALT2/I2S0_RXD0/FB_AD10/SDRAM_A18/CMP0_OUT/FTM0_CH2, identifier: HS_SWITCH_B_IN1,
    direction: OUTPUT, gpio_init_state: 'false'}
  - {pin_num: '91', peripheral: SPI1, signal: PCS0_SS, pin_signal: ADC1_SE14/PTB10/SPI1_PCS0/UART3_RX/FB_AD19/SDRAM_D19/FTM0_FLT1}
  - {pin_num: '92', peripheral: SPI1, signal: SCK, pin_signal: ADC1_SE15/PTB11/SPI1_SCK/UART3_TX/FB_AD18/SDRAM_D18/FTM0_FLT2}
  - {pin_num: '96', peripheral: SPI1, signal: SIN, pin_signal: TSI0_CH10/PTB17/SPI1_SIN/UART0_TX/FTM_CLKIN1/FB_AD16/SDRAM_D16/EWM_OUT_b/TPM_CLKIN1}
  - {pin_num: '95', peripheral: SPI1, signal: SOUT, pin_signal: TSI0_CH9/PTB16/SPI1_SOUT/UART0_RX/FTM_CLKIN0/FB_AD17/SDRAM_D17/EWM_IN/TPM_CLKIN0}
  - {pin_num: '90', peripheral: SPI1, signal: PCS1, pin_signal: PTB9/SPI1_PCS1/UART3_CTS_b/FB_AD20/SDRAM_D20}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitPins(void)
{
    /* Port B Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortB);
    /* Port C Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortC);

    gpio_pin_config_t HS_SWITCH_B_IN1_config = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 0U
    };
    /* Initialize GPIO functionality on pin PTC5 (pin 110)  */
    GPIO_PinInit(BOARD_INITPINS_HS_SWITCH_B_IN1_GPIO, BOARD_INITPINS_HS_SWITCH_B_IN1_PIN, &HS_SWITCH_B_IN1_config);

    gpio_pin_config_t HS_SWITCH_B_IN0_config = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 1U
    };
    /* Initialize GPIO functionality on pin PTC6 (pin 111)  */
    GPIO_PinInit(BOARD_INITPINS_HS_SWITCH_B_IN0_GPIO, BOARD_INITPINS_HS_SWITCH_B_IN0_PIN, &HS_SWITCH_B_IN0_config);

    /* PORTB10 (pin 91) is configured as SPI1_PCS0 */
    PORT_SetPinMux(PORTB, 10U, kPORT_MuxAlt2);

    /* PORTB11 (pin 92) is configured as SPI1_SCK */
    PORT_SetPinMux(PORTB, 11U, kPORT_MuxAlt2);

    /* PORTB16 (pin 95) is configured as SPI1_SOUT */
    PORT_SetPinMux(PORTB, 16U, kPORT_MuxAlt2);

    /* PORTB17 (pin 96) is configured as SPI1_SIN */
    PORT_SetPinMux(PORTB, 17U, kPORT_MuxAlt2);

    /* PORTB9 (pin 90) is configured as SPI1_PCS1 */
    PORT_SetPinMux(PORTB, 9U, kPORT_MuxAlt2);

    /* PORTC5 (pin 110) is configured as PTC5 */
    PORT_SetPinMux(BOARD_INITPINS_HS_SWITCH_B_IN1_PORT, BOARD_INITPINS_HS_SWITCH_B_IN1_PIN, kPORT_MuxAsGpio);

    /* PORTC6 (pin 111) is configured as PTC6 */
    PORT_SetPinMux(BOARD_INITPINS_HS_SWITCH_B_IN0_PORT, BOARD_INITPINS_HS_SWITCH_B_IN0_PIN, kPORT_MuxAsGpio);
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
