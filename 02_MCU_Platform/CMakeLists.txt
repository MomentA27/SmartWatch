cmake_minimum_required(VERSION 3.10)

# 引入 GPIO 子模块
add_subdirectory(MCU_Core_GPIO)
# 引入 IIC 子模块
add_subdirectory(MCU_Core_I2C)

# ------------------------------
# 定义 BSP 接口库（用于聚合所有 BSP 组件）
# ------------------------------
# 使用 INTERFACE 关键字，表明这是一个接口库
add_library(mcu_platform INTERFACE)

# 任何链接了 "bsp" 的目标，都会自动链接
target_link_libraries(mcu_platform INTERFACE
        mcu_gpio
        mcu_i2c
)

# 暴露 BSP 公共头文件路径（如果 BSP 目录下有顶层的公共头文件）
target_include_directories(mcu_platform INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/MCU_Core_I2C
)