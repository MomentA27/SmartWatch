cmake_minimum_required(VERSION 3.10)


# ------------------------------
# 1. 引入子模块（src/Queue）
# ------------------------------
add_subdirectory(src/System)  # 引入队列子模块
add_subdirectory(src/Queue)  # 引入队列子模块
add_subdirectory(src/Mutex)  # 引入互斥锁子模块
add_subdirectory(src/Task)  # 引入任务子模块
add_subdirectory(src/Sema)  # 引入信号量子模块
add_subdirectory(src/Event)  # 引入事件组子模块
# ------------------------------
# 2. 定义顶层 INTERFACE 库（聚合子模块）
# ------------------------------
# 使用 INTERFACE 库，仅提供接口和依赖，不生成二进制文件
add_library(os_platform INTERFACE)

# 链接子模块
target_link_libraries(os_platform INTERFACE
        os_queue
        os_mutex
        os_task
        os_sema
        os_event
        os_system
)

# ------------------------------
# 3. 暴露公共头文件路径（inc/）
# ------------------------------
# 让上层模块能找到 inc/platform_os.h
target_include_directories(os_platform INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/inc  # 指向根目录的 inc
)