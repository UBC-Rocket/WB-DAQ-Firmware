################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../freertos/freertos_kernel/portable/GCC/ARM_CM4F/port.c 

OBJS += \
./freertos/freertos_kernel/portable/GCC/ARM_CM4F/port.o 

C_DEPS += \
./freertos/freertos_kernel/portable/GCC/ARM_CM4F/port.d 


# Each subdirectory must supply rules for building sources it contributes
freertos/freertos_kernel/portable/GCC/ARM_CM4F/%.o: ../freertos/freertos_kernel/portable/GCC/ARM_CM4F/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCPU_MK66FX1M0VLQ18 -DCPU_MK66FX1M0VLQ18_cm4 -DFSL_RTOS_FREE_RTOS -DSDK_OS_FREE_RTOS -DSERIAL_PORT_TYPE_UART=1 -DSDK_DEBUGCONSOLE=0 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -D__REDLIB__ -I"C:\Users\gtaha\Desktop\UBC Rocket\WB-CAN-Firmware\board" -I"C:\Users\gtaha\Desktop\UBC Rocket\WB-CAN-Firmware\source" -I"C:\Users\gtaha\Desktop\UBC Rocket\WB-CAN-Firmware" -I"C:\Users\gtaha\Desktop\UBC Rocket\WB-CAN-Firmware\freertos\freertos_kernel\include" -I"C:\Users\gtaha\Desktop\UBC Rocket\WB-CAN-Firmware\freertos\freertos_kernel\portable\GCC\ARM_CM4F" -I"C:\Users\gtaha\Desktop\UBC Rocket\WB-CAN-Firmware\drivers" -I"C:\Users\gtaha\Desktop\UBC Rocket\WB-CAN-Firmware\component\uart" -I"C:\Users\gtaha\Desktop\UBC Rocket\WB-CAN-Firmware\component\serial_manager" -I"C:\Users\gtaha\Desktop\UBC Rocket\WB-CAN-Firmware\utilities" -I"C:\Users\gtaha\Desktop\UBC Rocket\WB-CAN-Firmware\component\lists" -I"C:\Users\gtaha\Desktop\UBC Rocket\WB-CAN-Firmware\device" -I"C:\Users\gtaha\Desktop\UBC Rocket\WB-CAN-Firmware\CMSIS" -O0 -fno-common -g3 -Wall -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fmerge-constants -fmacro-prefix-map="../$(@D)/"=. -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


