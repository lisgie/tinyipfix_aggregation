################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../networking/networking.c 

OBJS += \
./networking/networking.o 

C_DEPS += \
./networking/networking.d 


# Each subdirectory must supply rules for building sources it contributes
networking/%.o: ../networking/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I/home/livio/workspace/contiki/core -include/home/livio/workspace/contiki/core -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


