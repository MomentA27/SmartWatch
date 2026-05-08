cmake_minimum_required(VERSION 3.10)

# 定义 AHT21 驱动库（整合 driver 和 handler）
add_library(os_task STATIC)

# 添加 AHT21 模块的源文件
target_sources(os_task PRIVATE
        os_task_port.c
)

# 暴露 AHT21 模块的头文件路径
target_include_directories(os_task PUBLIC
        .
)

# 链接依赖库（根据实际需求添加，例如硬件驱动库）
target_link_libraries(os_task PRIVATE
        STM32_Drivers
)