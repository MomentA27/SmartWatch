cmake_minimum_required(VERSION 3.10)

# 定义 aht21_adapter 驱动库
add_library(aht21_adapter STATIC)

# 添加源文件
target_sources(aht21_adapter PRIVATE
        adapter/aht21_adaption.c
        wrapper/aht21_wrapper.c
)

# 暴露头文件目录（修正：指向目录而不是单个文件）
target_include_directories(aht21_adapter PUBLIC
        adapter
        wrapper
)

# 链接依赖库
target_link_libraries(aht21_adapter PUBLIC
        STM32_Drivers
        bsp
        libraries
        mcu_platform
        os_platform
)