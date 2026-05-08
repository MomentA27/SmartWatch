cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME SmartWatch_STM32F411)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)
# ------------------------------
# 新增：引入 APP 模块
# ------------------------------
add_subdirectory(01_APP)
# ------------------------------
# 新增：引入 BSP 模块
# ------------------------------
add_subdirectory(02_BSP_Platform)
# ------------------------------
# 新增：引入 OS_BSP 模块
# ------------------------------
add_subdirectory(02_OS_Platform)
# 新增：引入 MCU_BSP 模块
# ------------------------------
add_subdirectory(02_MCU_Platform)
# ------------------------------
# 新增：引入 LIBRARIES 模块
# ------------------------------
add_subdirectory(03_Libraries)

# ------------------------------
# 新增：引入 Service 模块
# ------------------------------
add_subdirectory(04_Service)
# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
        # ------------------------------
        # 新增：链接 APP 库
        # ------------------------------
        app
        # ------------------------------
        # 新增：链接 BSP 库
        # ------------------------------
        bsp_adapter
        # ------------------------------
        # 新增：链接 OS 库
        # ------------------------------
        os_platform        # ------------------------------
        # 新增：链接 MCU_BSP 库
        # ------------------------------
        mcu_platform
        # ------------------------------
        # 新增：链接 LIBRARIES 库
        # ------------------------------
        libraries
        # ------------------------------
        # 新增：链接 SERVICE 库
        # ------------------------------
        service
)
