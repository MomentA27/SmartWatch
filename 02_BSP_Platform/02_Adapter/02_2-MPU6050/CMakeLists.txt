cmake_minimum_required(VERSION 3.10)

# 定义 mpu6050_adapter 驱动库
add_library(mpu6050_adapter STATIC)

# 添加源文件
target_sources(mpu6050_adapter PRIVATE
        adapter/mpu6050_adaption.c
        wrapper/mpu6050_wrapper.c
)

# 暴露头文件目录（修正：指向目录而不是单个文件）
target_include_directories(mpu6050_adapter PUBLIC
        adapter
        wrapper
)

# 链接依赖库
target_link_libraries(mpu6050_adapter PUBLIC
        STM32_Drivers
        bsp
        libraries
        mcu_platform
        os_platform
)