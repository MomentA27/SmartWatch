cmake_minimum_required(VERSION 3.10)

# 定义 debug_adapter 驱动库
add_library(watchdog_monitor STATIC)

# 添加 debug_adapter 模块的源文件
target_sources(watchdog_monitor PRIVATE
        watchdog_monitor.c
)

# 暴露头文件路径（关键：对外公开 inc 目录，其他模块可引用）
target_include_directories(watchdog_monitor PUBLIC
        .
)

# 链接依赖库（当前模块依赖的库，根据实际需求调整）
# 注意：依赖库必须是已定义的（如 STM32_Drivers、bsp、user_debug）
target_link_libraries(watchdog_monitor PRIVATE
        STM32_Drivers
        libraries
        os_platform
)