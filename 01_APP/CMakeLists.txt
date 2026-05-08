cmake_minimum_required(VERSION 3.10)

# 引入 Platform_IO_Register 子模块
add_subdirectory(User_Init/01_Platform_IO_Register)
# 引入 User_Task_Config 子模块
add_subdirectory(User_Init/02_User_Task_Config)
# 引入 User_Init_Imple 子模块
add_subdirectory(User_Init/User_Init_Imple)


# ------------------------------
# 定义 Libraries 接口库（用于聚合所有 Libraries 组件）
# ------------------------------
# 使用 INTERFACE 关键字，表明这是一个接口库
add_library(app INTERFACE)

# 链接模块（公开链接）
# 任何链接了 "libraries" 的目标，都会自动链接
target_link_libraries(app INTERFACE
        user_init
        periph_setup
        task_config
)

# 暴露 libraries 公共头文件路径（如果 libraries 目录下有顶层的公共头文件）
target_include_directories(app INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)