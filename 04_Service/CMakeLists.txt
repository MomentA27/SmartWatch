cmake_minimum_required(VERSION 3.10)

# 引入 temp_humi 子模块
add_subdirectory(service_temp_humi_process)
# 引入 temp_humi 子模块
add_subdirectory(service_motion_process)
# 引入 watchdog 子模块
add_subdirectory(service_watchdog_monitor)

# ------------------------------
# 定义 Libraries 接口库（用于聚合所有 Libraries 组件）
# ------------------------------
# 使用 INTERFACE 关键字，表明这是一个接口库
add_library(service STATIC
            service_sensor.c)

# 链接模块（公开链接）
# 任何链接了 "libraries" 的目标，都会自动链接
target_link_libraries(service PUBLIC
        temp_humi_pro
        watchdog_monitor
        motion_pro

        STM32_Drivers
        app
        bsp_adapter
        os_platform
        libraries
)

# 暴露 libraries 公共头文件路径（如果 libraries 目录下有顶层的公共头文件）
target_include_directories(service PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

