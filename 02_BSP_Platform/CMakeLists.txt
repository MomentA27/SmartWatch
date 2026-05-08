cmake_minimum_required(VERSION 3.10)

# ------------------------------
# 1. 引入BSP和Adapter子目录
# ------------------------------
add_subdirectory(01_Bsp)  # 引入BSP子目录
add_subdirectory(02_Adapter)       # 引入Adapter子目录

# ------------------------------
# 2. 定义顶层接口库（聚合BSP和Adapter）
# ------------------------------
# 使用INTERFACE库，仅提供头文件和依赖，不生成实际二进制文件
add_library(bsp_adapter INTERFACE)

# 链接BSP和Adapter的接口库（BSP的CMakeLists.txt中定义了"bsp"库，Adapter定义了"adapter"库）
target_link_libraries(bsp_adapter INTERFACE
        bsp
        adapter
)

# ------------------------------
# 3. 暴露BSP和Adapter的公共头文件路径
# ------------------------------
# ------------------------------
# 3. 动态收集BSP子目录的include路径
# ------------------------------
target_include_directories(bsp_adapter INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)