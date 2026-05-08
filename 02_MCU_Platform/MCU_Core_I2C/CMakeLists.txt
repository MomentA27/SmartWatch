cmake_minimum_required(VERSION 3.10)

# 定义 AHT21 驱动库（整合 driver 和 handler）
add_library(mcu_i2c STATIC)

# 添加 AHT21 模块的源文件
target_sources(mcu_i2c PRIVATE
        i2c_port.c
)

# 暴露 AHT21 模块的头文件路径
target_include_directories(mcu_i2c PUBLIC
        .
)

# 链接依赖库（根据实际需求添加，例如硬件驱动库）
target_link_libraries(mcu_i2c PRIVATE
        STM32_Drivers
        libraries
        mcu_gpio
        os_platform
)