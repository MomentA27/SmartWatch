cmake_minimum_required(VERSION 3.10)

# 定义 AHT21 驱动库（整合 driver 和 handler）
add_library(bsp_aht21 STATIC)

# 添加 AHT21 模块的源文件
target_sources(bsp_aht21 PRIVATE
        # driver 目录源文件
        AHT21_Driver/aht21_driver.c
        # handler 目录源文件
        AHT21_Handler/aht21_handler.c
)

# 暴露 AHT21 模块的头文件路径
target_include_directories(bsp_aht21 PUBLIC
        # driver 目录头文件
        AHT21_Driver
        # handler 目录头文件
        AHT21_Handler
)

# 链接依赖库（根据实际需求添加，例如硬件驱动库）
target_link_libraries(bsp_aht21 PUBLIC
        STM32_Drivers
        os_platform
        libraries
        service
)