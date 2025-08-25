################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../component/lists/fsl_component_generic_list.c 

C_DEPS += \
./component/lists/fsl_component_generic_list.d 

OBJS += \
./component/lists/fsl_component_generic_list.o 


# Each subdirectory must supply rules for building sources it contributes
component/lists/%.o: ../component/lists/%.c component/lists/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DCPU_MCXN947VDF -DCPU_MCXN947VDF_cm33 -DCPU_MCXN947VDF_cm33_core0 -DMCUXPRESSO_SDK -DSDK_DEBUGCONSOLE=0 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"C:\Users\cezar\Documents\MCUXpressoIDE_11.9.1_2170\workspace5\frdmmcxn947_pwm\utilities" -I"C:\Users\cezar\Documents\MCUXpressoIDE_11.9.1_2170\workspace5\frdmmcxn947_pwm\drivers" -I"C:\Users\cezar\Documents\MCUXpressoIDE_11.9.1_2170\workspace5\frdmmcxn947_pwm\device" -I"C:\Users\cezar\Documents\MCUXpressoIDE_11.9.1_2170\workspace5\frdmmcxn947_pwm\startup" -I"C:\Users\cezar\Documents\MCUXpressoIDE_11.9.1_2170\workspace5\frdmmcxn947_pwm\component\uart" -I"C:\Users\cezar\Documents\MCUXpressoIDE_11.9.1_2170\workspace5\frdmmcxn947_pwm\component\lists" -I"C:\Users\cezar\Documents\MCUXpressoIDE_11.9.1_2170\workspace5\frdmmcxn947_pwm\CMSIS" -I"C:\Users\cezar\Documents\MCUXpressoIDE_11.9.1_2170\workspace5\frdmmcxn947_pwm\source" -I"C:\Users\cezar\Documents\MCUXpressoIDE_11.9.1_2170\workspace5\frdmmcxn947_pwm\board" -I"C:\Users\cezar\Documents\MCUXpressoIDE_11.9.1_2170\workspace5\frdmmcxn947_pwm\frdmmcxn947\driver_examples\pwm\cm33_core0" -I"/frdmmcxn947_pwm/include" -O0 -fno-common -g3 -gdwarf-4 -mcpu=cortex-m33 -c -ffunction-sections -fdata-sections -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-component-2f-lists

clean-component-2f-lists:
	-$(RM) ./component/lists/fsl_component_generic_list.d ./component/lists/fsl_component_generic_list.o

.PHONY: clean-component-2f-lists

