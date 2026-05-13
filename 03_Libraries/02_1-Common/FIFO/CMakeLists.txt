cmake_minimum_required(VERSION 3.10)

# 定义 fifo 驱动库
add_library(multi_fifo STATIC)

# 添加 fifo 模块的源文件
target_sources(multi_fifo PRIVATE
        i2c_bus.c
)

# 暴露 fifo 模块的头文件路径
target_include_directories(multi_fifo PUBLIC
        .
)

# 链接依赖库（根据实际需求添加，例如硬件驱动库）
target_link_libraries(multi_fifo PRIVATE
        STM32_Drivers
        os_platform
)