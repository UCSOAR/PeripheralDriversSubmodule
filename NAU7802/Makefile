# ==============================================================================
# NAU7802 Driver Makefile
# as not working, using Makefile to test compiler
# ==============================================================================

# Compiler and Flags
CXX      = arm-none-eabi-g++
CC       = arm-none-eabi-gcc
MCU      = -mcpu=cortex-m4 -mthumb -DSTM32F407xx -DUSE_HAL_DRIVER
CFLAGS   = -c -Wall $(MCU) $(INCLUDES)
CXXFLAGS = -c -Wall $(MCU) $(INCLUDES)


# Include Directories
INCLUDES = \
    -I. \
    -I/home/BerryByte/SOAR/general/AvionicsSoftware/Core/Inc \
    -I/home/BerryByte/SOAR/general/dependencies/STM32CubeF4/Drivers/CMSIS/Core/Include \
    -I/home/BerryByte/SOAR/general/dependencies/cmsis_device_f4/Include \
    -I/home/BerryByte/SOAR/general/dependencies/stm32f4xx_hal_driver/Inc \
    -I/home/BerryByte/SOAR/general/dependencies/stm32f4xx_hal_driver/Inc/Legacy \
    -I/home/BerryByte/SOAR/general/dependencies/STM32CubeF4/Drivers/CMSIS/RTOS/Template \
    -I/home/BerryByte/SOAR/general/AvionicsTemplateRepository/SoarOS/Core/Inc \
    -I/home/BerryByte/SOAR/general/AvionicsTemplateRepository/SoarOS \
    -I/home/BerryByte/SOAR/general/AvionicsDisco/AvionicsSoftware/Components \
    -I/home/BerryByte/SOAR/general/AvionicsDisco/AvionicsSoftware/Core/Inc \
    -I/home/BerryByte/SOAR/general/AvionicsDisco/Middlewares/Third_Party/FreeRTOS/Source/include/ \
    -I/home/BerryByte/SOAR/general/AvionicsDisco/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/ \


# Source Files
SOURCES_CPP = \
    NAU7802Task.cpp \
    NAU7802.cpp \
    i2c_wrapper.cpp

# Object Files
OBJECTS = $(SOURCES_CPP:.cpp=.o)

# Targets
all: $(OBJECTS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f *.o