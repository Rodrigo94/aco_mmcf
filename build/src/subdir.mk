################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/main.cpp \
../src/aco.cpp \
../src/ant.cpp \
../src/layer.cpp \
../src/node.cpp 

OBJS += \
./src/main.o \
./src/aco.o \
./src/ant.o \
./src/layer.o \
./src/node.o 

CPP_DEPS += \
./src/main.d \
./src/aco.d \
./src/ant.d \
./src/layer.d \
./src/node.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<" 


