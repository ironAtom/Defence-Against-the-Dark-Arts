################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cilk-main.c \
../dictionary.c 

OBJS += \
./cilk-main.o \
./dictionary.o 

C_DEPS += \
./cilk-main.d \
./dictionary.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	/opt/clang/bin/clang -I/opt/clang/lib/ -O0 -g3 -Wall -c -fmessage-length=0 -fcilkplus -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


