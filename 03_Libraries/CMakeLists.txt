cmake_minimum_required(VERSION 3.10)

# 引入 Common-I2C 子模块
add_subdirectory(02_1-Common/IIC_BUS)
# 引入 ThirdParty-elog 子模块
add_subdirectory(02_2-ThirdParty/elog)
# 引入 ThirdParty-SEGGER 子模块
add_subdirectory(02_2-ThirdParty/SEGGER)



# ------------------------------
# 定义 Libraries 接口库（用于聚合所有 Libraries 组件）
# ------------------------------
# 使用 INTERFACE 关键字，表明这是一个接口库
add_library(libraries INTERFACE)

# 链接模块（公开链接）
# 任何链接了 "libraries" 的目标，都会自动链接
target_link_libraries(libraries INTERFACE
        ##Common
        soft_i2c
        ##ThirdParty
        elog
        SEGGER
)

# 暴露 libraries 公共头文件路径（如果 libraries 目录下有顶层的公共头文件）
target_include_directories(libraries INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)