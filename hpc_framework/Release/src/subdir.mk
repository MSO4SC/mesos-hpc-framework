################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/jobsettings.pb.cc 

CPP_SRCS += \
../src/hpc_framework.cpp 

CC_DEPS += \
./src/jobsettings.pb.d 

OBJS += \
./src/hpc_framework.o \
./src/jobsettings.pb.o 

CPP_DEPS += \
./src/hpc_framework.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -D__cplusplus=201103L -I/opt/mesos-1.1.0/include -I/opt/mesos-1.1.0/src -I/opt/mesos-1.1.0/build/3rdparty/protobuf-2.6.1/src -I/opt/mesos-1.1.0/build/3rdparty/glog-0.3.3/src -O3 -Wall -c -fmessage-length=0 -std=gnu++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -D__cplusplus=201103L -I/opt/mesos-1.1.0/include -I/opt/mesos-1.1.0/src -I/opt/mesos-1.1.0/build/3rdparty/protobuf-2.6.1/src -I/opt/mesos-1.1.0/build/3rdparty/glog-0.3.3/src -O3 -Wall -c -fmessage-length=0 -std=gnu++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


