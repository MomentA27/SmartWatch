<p align="center">
  <img src="https://img.shields.io/badge/MCU-STM32F411CEU6-blue" alt="MCU">
  <img src="https://img.shields.io/badge/RTOS-FreeRTOS%20V10.3.1-green" alt="RTOS">
  <img src="https://img.shields.io/badge/Language-C%20(OOP)-orange" alt="Language">
  <img src="https://img.shields.io/badge/Architecture-Layered%20%2B%20Bridge-purple" alt="Architecture">
</p>

<h1 align="center">⌚ SmartWatch STM32F411</h1>

<p align="center">
  <b>架构驱动 · 纯C面向对象 · 生产级代码规范</b><br>
  一款不只是实现功能的智能手表固件，更是嵌入式 C 语言架构设计的最佳实践。
</p>

---

## 🌟 项目亮点

| 特性 | 实现方式 |
|------|----------|
| **纯 C 面向对象** | `struct` + 函数指针 vtable，无需 C++ 也能实现多态与封装 |
| **依赖注入 (DI)** | Driver 构造函数注入 6 类接口（I2C/延时/OS/中断等），驱动与硬件零耦合 |
| **Bridge 模式** | `Wrapper`(抽象接口) ↔ `Adapter`(具体绑定)，更换传感器芯片业务代码零修改 |
| **三层驱动架构** | `Driver`(寄存器级) → `Handler`(RTOS事件线程) → `Adapter/Wrapper`(对外API) |
| **OS 抽象层 (OSAL)** | 全 `void*` 不透明句柄，业务代码绝不包含 FreeRTOS 头文件，可无缝切换 RTOS |
| **声明式配置** | 静态配置表 + 批量创建 Task/Queue/Mutex，告别散落各处的 `xTaskCreate` |
| **弱符号裁剪** | `__WEAK` 修饰的任务入口，通过链接期选择即可实现功能模块的插拔裁剪 |

---

## 1. 整体分层架构

```
┌──────────────────────────────────────────────────────────────────┐
│                  01_APP/  应用层                                  │
│  ┌──────────────────┐ ┌──────────────────┐                       │
│  │   系统初始化      │ │  RTOS配置表      │                       │
│  │   Bootstrap      │ │  声明式资源管理  │                       │
│  └────────┬─────────┘ └────────┬─────────┘                       │
├───────────┼────────────────────┼─────────────────────────────────┤
│           │                    │                                  │
│  ┌────────┴────────────────────┴──────────────────────────────┐  │
│  │              04_Service/  服务层                            │  │
│  │  ┌──────────────┐ ┌──────────────┐ ┌────────────────────┐  │  │
│  │  │Sensor Service│ │Temp/Humi Port│ │  Watchdog Monitor  │  │  │
│  │  │ 传感器轮询   │ │ 温湿度封装   │ │   看门狗监控       │  │  │
│  │  └──────────────┘ └──────────────┘ └────────────────────┘  │  │
│  └────────┬───────────────────────────────────────────────────┘  │
├───────────┼──────────────────────────────────────────────────────┤
│           │                                                       │
│  ┌────────┴──────────────────────────────────────────────────┐   │
│  │              02_BSP_Platform/  BSP驱动层                   │   │
│  │  ┌────────────────┐  ┌──────────────────┐                 │   │
│  │  │ 01_Bsp/Drivers │  │ 02_Adapter/Wrapper│                │   │
│  │  │  驱动+Handler  │  │  适配器+Bridge   │                 │   │
│  │  └────────────────┘  └──────────────────┘                 │   │
│  └────────┬──────────────────────────────────────────────────┘   │
├───────────┼──────────────────────────────────────────────────────┤
│           │                                                       │
│  ┌────────┴──────────────────────────────────────────────────┐   │
│  │              02_MCU_Platform/  MCU外设抽象层               │   │
│  │  ┌──────────┐ ┌──────────┐                                 │   │
│  │  │   GPIO   │ │   I2C    │                                 │   │
│  │  │ 引脚抽象 │ │ 双模总线 │                                 │   │
│  │  └──────────┘ └──────────┘                                 │   │
│  └────────┬──────────────────────────────────────────────────┘   │
├───────────┼──────────────────────────────────────────────────────┤
│           │                                                       │
│  ┌────────┴──────────────────────────────────────────────────┐   │
│  │              02_OS_Platform/  OS抽象层                     │   │
│  │  ┌────────────┐ ┌────────────┐ ┌────────────┐             │   │
│  │  │ Task/Queue │ │ Mutex/Sema │ │Event/System│             │   │
│  │  │ 任务/队列  │ │ 互斥/信号量│ │事件/系统   │             │   │
│  │  └────────────┘ └────────────┘ └────────────┘             │   │
│  └────────┬──────────────────────────────────────────────────┘   │
├───────────┼──────────────────────────────────────────────────────┤
│           │                                                       │
│  ┌────────┴──────────────────────────────────────────────────┐   │
│  │       Drivers/STM32F4xx_HAL_Driver/  STM32 HAL库          │   │
│  │       Core/                           CMSIS + 启动代码    │   │
│  │       Middlewares/                    FreeRTOS V10.3.1    │   │
│  └───────────────────────────────────────────────────────────┘   │
├──────────────────────────────────────────────────────────────────┤
│  03_Libraries/  通用库 (DWT延时 / I2C软件总线 / FIFO / 调试)   │
└──────────────────────────────────────────────────────────────────┘
```

---

## 2. 应用层 (01_APP) — 源码映射

### 2.1 系统初始化

| 架构角色 | 源文件 |
|----------|--------|
| 系统入口，时钟配置，外设初始化，启动RTOS调度器 | `Core/Src/main.c` |
| 入口头文件（引脚宏定义、Error_Handler） | `Core/Inc/main.h` |
| FreeRTOS初始化 + 启动任务创建 | `Core/Src/freertos.c` |
| FreeRTOS配置（V10.3.1, CMSIS-RTOS V2, heap_4, 30KB堆） | `Core/Inc/FreeRTOSConfig.h` |
| 用户应用初始化入口（创建 init task → 资源初始化 → 自删除） | `01_APP/User_Init/User_Init_Imple/user_init.c` |
| 全局状态结构体 + OS句柄导出 | `01_APP/User_Init/User_Init_Imple/user_init.h` |
| 外设适配器注册（调用 `drv_adapter_temphumi_register()`） | `01_APP/User_Init/01_Platform_IO_Register/user_periph_setup.c` |
| 外设注册头文件 | `01_APP/User_Init/01_Platform_IO_Register/user_periph_setup.h` |

**启动流程：**

```
Reset_Handler (startup_stm32f411xe.s)
  → SystemInit()                          [FPU使能]
  → 拷贝 .data, 清零 .bss
  → main()                                [Core/Src/main.c]
     → HAL_Init()                         [SysTick, NVIC优先级组]
     → SystemClock_Config()               [HSI→PLL 100MHz]
     → MX_GPIO/DMA/USART1/I2C3/SPI1/SPI2/I2C1/ADC1/RTC/IWDG_Init()
     → user_debug_init()                  [SEGGER RTT + EasyLogger]
     → app_periph_init()                  [注册AHT21适配器]
     → osKernelInitialize()
     → MX_FREERTOS_Init()                 [创建defaultTask + UserAppTask_Init]
     → osKernelStart()                    [调度器启动，永不返回]
```

### 2.2 声明式资源配置表

| 架构角色 | 源文件 |
|----------|--------|
| Task/Queue/Mutex/Semaphore 静态配置表 + 批量创建 + Weak符号桩 | `01_APP/User_Init/02_User_Task_Config/user_task_config.c` |
| 优先级分带宏、配置结构体定义 | `01_APP/User_Init/02_User_Task_Config/user_task_config.h` |

**优先级分带：**

| 优先级带 | 值 | 用途 |
|----------|-----|------|
| `PRI_EMERGENCY` | 55 (最高) | 初始化任务（一次性自删除） |
| `PRI_HARD_REALTIME` | 51 | 硬实时（传感器Handler线程） |
| `PRI_SOFT_REALTIME` | 46 | 软实时（传感器轮询、看门狗） |
| `PRI_NORMAL` | 39 | 普通业务任务 |
| `PRI_BACKGROUND` | 1 | 后台任务 |

**资源配置表内容：**

| 资源类型 | 数量 | 名称 |
|----------|------|------|
| Task（任务） | 3个 | tempHandlerTask, WatchDog_Thread, SensorTask |
| Queue（队列） | 1个 | SensorDataQueue (深度2, `uint32_t`) |
| Mutex（互斥锁） | 1个 | SensorDataMutex |
| Semaphore（信号量） | 0个 | (预留) |

**Weak符号桩设计：**  
`temp_humi_handler_thread()` 和 `wdg_handler_thread()` 以 `__WEAK` 修饰提供空实现（`while(1)`），当实际Handler模块参与链接时自动覆盖。这使得功能模块可以通过编译期链接选择性地启用/裁剪。

---

## 3. 服务层 (04_Service) — 源码映射

### 3.1 传感器轮询服务

| 架构角色 | 源文件 |
|----------|--------|
| **核心**：传感器轮询任务，周期性读取温湿度，Mutex保护写入全局状态，Queue发布数据 | `04_Service/service_sensor.c` |
| 传感器服务头文件（传感器类型位掩码、状态结构体） | `04_Service/service_sensor.h` |

**传感器类型位掩码（预留扩展）：**

```c
#define SENSOR_TEMP       (1 << 0)   // 温度
#define SENSOR_HUMIDITY   (1 << 1)   // 湿度
#define SENSOR_MOTION     (1 << 2)   // 运动（预留）
#define SENSOR_PRESSURE   (1 << 3)   // 气压（预留）
#define SENSOR_ALTITUDE   (1 << 4)   // 海拔（预留）
#define SENSOR_HEARTRATE  (1 << 5)   // 心率（预留）
```

**轮询状态结构体：**

| 字段 | 类型 | 用途 |
|------|------|------|
| `active_sensors` | `uint32_t` | 当前活跃传感器掩码 |
| `temp_sample_rate` | `uint32_t` | 温度采样间隔(ms) |
| `last_temp_sample` | `uint32_t` | 上次采样时间戳 |
| `temp_sampling_enabled` | `bool` | 温度采样使能标志 |

### 3.2 温湿度封装层

| 架构角色 | 源文件 |
|----------|--------|
| 薄封装层（对 `aht21_wrapper` 的二次封装，提供业务级API） | `04_Service/service_temp_humi_process/temp_humi_port.c` |
| 封装层头文件（init/deinit/read_temp/read_humi/read_temp_and_humi） | `04_Service/service_temp_humi_process/temp_humi_port.h` |

### 3.3 看门狗监控服务

| 架构角色 | 源文件 |
|----------|--------|
| **核心**：硬件IWDG + 软件任务注册/喂狗/巡检 + 安全模式 | `04_Service/service_watchdog_monitor/watchdog_monitor.c` |
| 看门狗头文件（注册/取消/喂狗 API） | `04_Service/service_watchdog_monitor/watchdog_monitor.h` |

**看门狗API：**

| API | 功能 |
|-----|------|
| `watchdog_register(name, timeout)` | 注册任务到监控列表 |
| `watchdog_feed(name)` | 任务喂狗（更新最后活跃时间） |
| `watchdog_unregister(name)` | 取消注册 |
| `watchdog_pause()` / `watchdog_resume()` | 暂停/恢复监控 |
| `watchdog_enter_safe_mode()` | 进入安全模式（停止喂硬件狗） |

---

## 4. BSP驱动层 (02_BSP_Platform) — 源码映射

### 4.1 驱动三层架构总览

每个传感器驱动遵循统一的**三层架构**：

```
Driver (纯寄存器级) → Handler (事件队列 + RTOS线程) → Adapter + Wrapper (Bridge模式)
```

### 4.2 传感器驱动清单

#### AHT21 温湿度传感器（当前最完整实现）

| 层级 | 源文件 |
|------|--------|
| **Driver**：OOP-style vtable、依赖注入接口、I2C寄存器读写 | `02_BSP_Platform/01_Bsp/AHT21/AHT21_Driver/aht21_driver.h` |
| Driver实现：`aht21_inst()` 构造函数、初始化序列、温湿度计算 | `02_BSP_Platform/01_Bsp/AHT21/AHT21_Driver/aht21_driver.c` |
| 寄存器地址定义 | `02_BSP_Platform/01_Bsp/AHT21/AHT21_Driver/aht21_reg.h` |
| **Handler**：FreeRTOS事件线程，管理事件队列、温湿度数据解包 | `02_BSP_Platform/01_Bsp/AHT21/AHT21_Handler/aht21_handler.h` |
| Handler实现 | `02_BSP_Platform/01_Bsp/AHT21/AHT21_Handler/aht21_handler.c` |

**AHT21 Driver 接口依赖注入：**

```
bsp_aht21_driver_t
├── aht21_driver_input_api_t
│   ├── i2c_bus_interface_t    → I2C start/stop/send/recv/ACK
│   ├── timebase_interface_t   → 系统tick获取
│   └── yield_interface_t      → RTOS任务让出
```

#### MPU6050 六轴IMU（Driver + Handler接口已定义）

| 层级 | 源文件 |
|------|--------|
| **Driver**：20+虚函数vtable、6接口DI、Kalman数据结构 | `02_BSP_Platform/01_Bsp/MPU6050/MPU6050_Driver/mpu6050_driver.h` |
| Driver实现 | `02_BSP_Platform/01_Bsp/MPU6050/MPU6050_Driver/mpu6050_driver.c` |
| 寄存器地址定义 | `02_BSP_Platform/01_Bsp/MPU6050/MPU6050_Driver/mpu6050_reg.h` |
| 寄存器位域宏 | `02_BSP_Platform/01_Bsp/MPU6050/MPU6050_Driver/mpu6050_reg_bit.h` |
| 环形缓冲区（DMA→Buffer→Queue 数据管道） | `02_BSP_Platform/01_Bsp/MPU6050/MPU6050_Driver/circular_buffer.h` |
| 环形缓冲区实现 | `02_BSP_Platform/01_Bsp/MPU6050/MPU6050_Driver/circular_buffer.c` |
| **Handler**：队列/信号量/任务通知，支持DMA中断链 | `02_BSP_Platform/01_Bsp/MPU6050/MPU6050_Handler/mpu6050_handler.h` |
| Handler实现 | `02_BSP_Platform/01_Bsp/MPU6050/MPU6050_Handler/mpu6050_handler.c` |

**MPU6050 接口依赖注入图：**

```
bsp_mpu6050_driver_t
├── mpu6050_iic_driver_interface_t      → I2C总线读/写/DMA读
├── hardware_interrupt_interface_t      → GPIO中断使能/禁用
├── mpu6050_delay_interface_t           → us/ms延时
├── mpu6050_timebase_interface_t         → 系统tick获取
├── mpu6050_yield_interface_t           → RTOS任务让出
├── mpu6050_os_interface_t              → Queue/Mutex/Semaphore (11个方法)
└── buffer_interface_t                  → DMA环形缓冲区管理
```

### 4.3 Adapter + Wrapper（Bridge模式适配层）

**设计模式说明：** Wrapper 定义抽象接口（函数指针结构体），Adapter 提供注册函数绑定具体驱动，使上层服务代码不依赖具体芯片型号。

#### AHT21

| 层 | 源文件 |
|----|--------|
| **Wrapper**：抽象接口 `temphumi_drv_t`（init/deinit/read_temp/read_humi/read_both） | `02_BSP_Platform/02_Adapter/02_1-AHT21/wrapper/aht21_wrapper.h` |
| Wrapper实现 | `02_BSP_Platform/02_Adapter/02_1-AHT21/wrapper/aht21_wrapper.c` |
| **Adapter**：注册函数 `drv_adapter_temphumi_register()`，装配Driver+Handler+OS+I2C | `02_BSP_Platform/02_Adapter/02_1-AHT21/adapter/aht21_adaption.h` |
| Adapter实现 | `02_BSP_Platform/02_Adapter/02_1-AHT21/adapter/aht21_adaption.c` |

#### MPU6050

| 层 | 源文件 |
|----|--------|
| **Wrapper**：抽象运动传感器接口 | `02_BSP_Platform/02_Adapter/02_2-MPU6050/wrapper/mpu6050_wrapper.h` |
| Wrapper实现 | `02_BSP_Platform/02_Adapter/02_2-MPU6050/wrapper/mpu6050_wrapper.c` |
| **Adapter**：注册函数，装配MPU6050全链路 | `02_BSP_Platform/02_Adapter/02_2-MPU6050/adapter/mpu6050_adaption.h` |
| Adapter实现 | `02_BSP_Platform/02_Adapter/02_2-MPU6050/adapter/mpu6050_adaption.c` |

---

## 5. MCU外设抽象层 (02_MCU_Platform) — 源码映射

| 外设 | 源文件 |
|------|--------|
| **GPIO**：集中式引脚枚举表、ROM配置表、动态RCC时钟使能 | `02_MCU_Platform/MCU_Core_GPIO/gpio_port.c` |
| GPIO头文件（引脚枚举 `CORE_GPIO_TEMPHUMI_SDA/SCL` 等） | `02_MCU_Platform/MCU_Core_GPIO/gpio_port.h` |
| GPIO引脚宏定义（传感器I2C引脚 PB6/PB7） | `02_MCU_Platform/MCU_Core_GPIO/gpio_define.h` |
| **I2C**：硬件/软件双模I2C、Mutex保护、DMA读支持 | `02_MCU_Platform/MCU_Core_I2C/i2c_port.c` |
| I2C头文件（总线枚举、便捷宏 `SENSOR_I2C_HARDWARE_WRITE`） | `02_MCU_Platform/MCU_Core_I2C/i2c_port.h` |

**I2C总线分配：**

| 总线 | 模式 | 引脚 | 挂载设备 |
|------|------|------|----------|
| I2C1 | 硬件 | PB6(SCL) / PB7(SDA) | AHT21 + MPU6050（共享总线） |
| I2C3 | 硬件 | PA8(SCL) / PB4(SDA) | CST816T 触摸屏（预留） |

---

## 6. OS抽象层 (02_OS_Platform) — 源码映射

### 6.1 两层结构

```
[Application]          01_APP/  04_Service/
     ↑ 调用平台OS API
[OS Public API]        02_OS_Platform/inc/platform_os.h
     ↑ 委托到各模块实现
[OS Implementation]    02_OS_Platform/src/{Task,Queue,Mutex,Sema,Event,System}/
     ↑ 调用
[FreeRTOS Kernel]      Middlewares/Third_Party/FreeRTOS/
```

> 注意：当前项目采用**扁平两层OSAL**，未引入独立的 `Middlewares/os_adapter/` 中间件层。所有OS句柄为 `void*` opaque handle，业务代码完全不包含FreeRTOS原生头文件。

### 6.2 完整文件映射

| 层级 | 功能 | 源文件 |
|------|------|--------|
| **总入口** | 统一引用所有OSAL子模块 + 便捷API（`os_delay_ms`, `os_enter_critical` 等） | `02_OS_Platform/inc/platform_os.h` |
| **Task API** | `os_task_create/delete/get_handle` | `02_OS_Platform/src/Task/os_task_port.h` |
| **Task实现** | `xTaskCreate/xTaskDelete/xTaskGetCurrentTaskHandle` | `02_OS_Platform/src/Task/os_task_port.c` |
| **Queue API** | `os_queue_create/put/get/delete/put_from_isr` | `02_OS_Platform/src/Queue/os_queue_port.h` |
| **Queue实现** | `xQueueCreate/xQueueSend/xQueueReceive`（含ISR版本路由） | `02_OS_Platform/src/Queue/os_queue_port.c` |
| **Mutex API** | `os_mutex_create/take/give/delete` | `02_OS_Platform/src/Mutex/os_mutex_port.h` |
| **Mutex实现** | `xSemaphoreCreateMutex/xSemaphoreTake/xSemaphoreGive` | `02_OS_Platform/src/Mutex/os_mutex_port.c` |
| **Semaphore API** | `os_sema_create/take/give`（支持Binary/Counting） | `02_OS_Platform/src/Sema/os_sema_port.h` |
| **Semaphore实现** | `xSemaphoreCreateBinary/Counting/Give/Take` | `02_OS_Platform/src/Sema/os_sema_port.c` |
| **EventGroup API** | `os_event_create/wait/set/clear/delete` | `02_OS_Platform/src/Event/os_event_port.h` |
| **EventGroup实现** | `xEventGroupCreate/WaitBits/SetBits/ClearBits/Delete` | `02_OS_Platform/src/Event/os_event_port.c` |
| **System API** | `os_enter_critical/os_exit_critical/os_delay_ms/os_get_tick_ms` | `02_OS_Platform/src/System/os_system_port.h` |
| **System实现** | `taskENTER_CRITICAL/vTaskDelay/xTaskGetTickCount` | `02_OS_Platform/src/System/os_system_port.c` |

---

## 7. 运行时任务映射

| 任务 | 优先级 | 栈大小 | 入口函数 | 角色 |
|------|--------|--------|----------|------|
| userTask (one-shot) | 55 (EMERGENCY) | 512B | `userTaskInitFunction()` | 创建OS资源 → 生成所有业务任务 → 自删除 |
| tempHandlerTask | 52 (HARD_REALTIME+1) | 512B | `temp_humi_handler_thread()` | AHT21事件驱动温湿度处理 |
| WatchDog_Thread | 49 (SOFT_REALTIME+3) | 512B | `server_watchdog_task()` | 软件看门狗：巡检注册任务 → 喂硬件IWDG |
| SensorTask | 49 (SOFT_REALTIME+3) | 512B | `sensor_polling_task()` | 周期性传感器轮询（100ms + 自适应采样率） |
| defaultTask | 24 (Normal) | 512B | `StartDefaultTask()` | 空闲循环（`osDelay(5)`） |

---

## 8. 通信模式 — 源码映射

### 8.1 模式 A：全局状态 + 互斥锁（共享内存）

```
sensor_polling_task()              任何消费者任务
      │                                  │
      ├─ os_mutex_take(SensorDataMutex)  ├─ os_mutex_take(SensorDataMutex)
      ├─ g_system_status.temperature = x ├─ val = g_system_status.temperature
      ├─ g_system_status.humidity = y    ├─ val = g_system_status.humidity
      └─ os_mutex_give(SensorDataMutex)  └─ os_mutex_give(SensorDataMutex)
```

| 角色 | 源文件 |
|------|--------|
| **全局状态结构体定义** (`system_status_t`) | `01_APP/User_Init/User_Init_Imple/user_init.h` |
| **Mutex句柄初始化** (`SensorDataMutex`) | `01_APP/User_Init/02_User_Task_Config/user_task_config.c` |
| **写入方**：传感器任务更新 `g_system_status` | `04_Service/service_sensor.c` |
| **全局状态实例** | `01_APP/User_Init/User_Init_Imple/user_init.c` |

### 8.2 模式 B：Queue 异步消息

| Queue名称 | 深度 | 数据大小 | 发送方 | 接收方 |
|-----------|------|----------|--------|--------|
| `SensorDataQueue` | 2 | `uint32_t` | `sensor_polling_task()` | 外部消费者 |

**Queue配置表定义：** `01_APP/User_Init/02_User_Task_Config/user_task_config.c`

### 8.3 模式 C：任务挂起/恢复（预留设计）

```
// 低功耗场景下预留的挂起/恢复接口
os_task_create()      → 创建任务并立即调度
os_task_delete()      → 删除任务释放资源
```

> 当前项目尚未实现低功耗管理的Suspend/Resume流程，但OSAL层已提供完整的任务生命周期API。

---

## 9. 库与中间件层 (03_Libraries + Middlewares) — 源码映射

### 9.1 通用工具库 (03_Libraries)

| 模块 | 源文件 | 用途 |
|------|--------|------|
| **软件I2C总线** | `03_Libraries/02_1-Common/IIC_BUS/i2c_bus.c` | 位脉冲式I2C主机，支持单字节/多字节读写 |
| 软件I2C头文件 | `03_Libraries/02_1-Common/IIC_BUS/i2c_bus.h` | `i2c_bus_t` 引脚可配置结构体 |
| **DWT延时** | `03_Libraries/02_1-Common/DWT_Delay/DWT_delay.c` | ARM DWT周期计数器微秒级精确延时 |
| DWT延时头文件 | `03_Libraries/02_1-Common/DWT_Delay/DWT_Delay.h` | `delay_us()` / `DWT_Init()` |
| **FIFO缓冲区** | `03_Libraries/02_1-Common/FIFO/multi_cyc_fifo.c` | 多字节环形FIFO |
| FIFO头文件 | `03_Libraries/02_1-Common/FIFO/multi_cyc_fifo.h` | FIFO结构体与API |

### 9.2 调试工具

| 模块 | 源文件 | 用途 |
|------|--------|------|
| **EasyLogger** (v2.2.0) | `03_Libraries/02_2-ThirdParty/elog/` | 结构化日志框架，`log_d()`/`log_e()`/`log_i()` |
| **SEGGER RTT** (v3.54) | `03_Libraries/02_2-ThirdParty/SEGGER/` | J-Link实时传输，`SEGGER_RTT_printf()` |
| **SEGGER SystemView** (v3.54) | `03_Libraries/02_2-ThirdParty/SEGGER/` | FreeRTOS任务级追踪与性能分析 |
| **Debug初始化** | `02_BSP_Platform/02_Adapter/02_X-Debug/user_debug.c` | 统一初始化RTT + EasyLogger |

### 9.3 第三方RTOS与HAL

| 模块 | 路径 | 用途 |
|------|------|------|
| **FreeRTOS** V10.3.1 | `Middlewares/Third_Party/FreeRTOS/` | 抢占式RTOS内核 (56优先级, heap_4, 30KB堆) |
| **CMSIS-RTOS V2** | `Middlewares/Third_Party/FreeRTOS/CMSIS_RTOS_V2/` | FreeRTOS → CMSIS-RTOS V2 API适配 |
| **STM32F4 HAL** | `Drivers/STM32F4xx_HAL_Driver/` | STM32硬件抽象层 |
| **CMSIS Core** | `Drivers/CMSIS/` | Cortex-M4核心支持 |

### 9.4 硬件初始化代码 (Core/)

| 模块 | 源文件 | 用途 |
|------|--------|------|
| **启动文件** | `startup_stm32f411xe.s` | 中断向量表、Reset_Handler |
| **链接脚本** | `STM32F411XX_FLASH.ld` | 512KB Flash / 128KB RAM 内存布局 |
| **中断服务函数** | `Core/Src/stm32f4xx_it.c` | EXTI/DMA/USART/TIM IRQ Handler |
| **HAL MSP** | `Core/Src/stm32f4xx_hal_msp.c` | 外设GPIO/时钟引脚初始化 |
| **HAL Timebase** | `Core/Src/stm32f4xx_hal_timebase_tim.c` | TIM11作为HAL 1ms时基 |
| **时钟配置** | `Core/Src/main.c` (`SystemClock_Config`) | HSI 16MHz → PLL → 100MHz SYSCLK |

---

## 10. 外设硬件资源表

| 外设 | 实例 | 引脚 | 用途 |
|------|------|------|------|
| I2C1 | 硬件 | PB6(SCL), PB7(SDA) | AHT21 + MPU6050 传感器共享总线 |
| I2C3 | 硬件 | PA8(SCL), PB4(SDA) | CST816T 触摸屏（预留） |
| SPI1 | 硬件 | PA5(CLK), PA7(MOSI), PA4(CS), PA6(DC), PB10(RST), PA1(BL) | ST7789 LCD 显示屏（预留） |
| SPI2 | 硬件 | PB13(CLK), PB15(MOSI), PB14(MISO), PB12(CS) | W25Q128 外部Flash（预留） |
| USART1 | 硬件 | PA9(TX), PA10(RX) | 蓝牙模块（预留） |
| ADC1 | 硬件 | PA2(Ch2), PB0(Ch8) | 环境光传感器 + 电池电压 |
| TIM11 | 硬件 | 内部 | HAL 1ms时基 |
| IWDG | 硬件 | 内部 | 硬件看门狗 |
| RTC | 硬件 | 内部 | 实时时钟 |
| EXTI0 | 外部中断 | PA0 | 用户按键3 |
| EXTI2 | 外部中断 | PB2 | 触摸屏中断（预留） |
| EXTI9_5 | 外部中断 | PB5 | MPU6050 中断（预留） |

---

## 11. 构建系统 — 源码映射

| 角色 | 源文件 |
|------|--------|
| **根构建文件**：可执行目标 `SmartWatch_STM32F411`，链接所有子库 | `CMakeLists.txt` |
| **工具链配置**：`arm-none-eabi-gcc`, Cortex-M4 + hard FP | `cmake/gcc-arm-none-eabi.cmake` |
| **HAL/FreeRTOS构建**：STM32CubeMX生成的源文件编译规则 | `cmake/stm32cubemx/CMakeLists.txt` |
| **CMake预设**：Debug (`-O0 -g3`) / Release (`-Os -g0`) | `CMakePresets.json` |
| **STM32CubeMX工程** | `SmartWatch_STM32F411.ioc` |

**链接库结构：**

```
SmartWatch_STM32F411 (executable)
  ├── stm32cubemx        (interface lib: HAL/FreeRTOS头文件路径 + 编译宏)
  ├── STM32_Drivers      (object lib: 所有HAL .c 文件)
  ├── FreeRTOS           (object lib: FreeRTOS内核 + CMSIS-RTOS V2 + port + heap_4)
  ├── app                (interface lib: 01_APP/)
  ├── bsp_adapter        (02_BSP_Platform/)
  ├── os_platform        (02_OS_Platform/)
  ├── mcu_platform       (02_MCU_Platform/)
  ├── libraries          (03_Libraries/)
  ├── service            (04_Service/)
  └── -lm                (数学库)
```

---

## 12. 关键设计模式速查

| 设计模式 | 体现位置 | 核心源文件 |
|----------|----------|-----------|
| **面向对象C（struct + 函数指针vtable）** | 所有BSP Driver | `aht21_driver.h`, `mpu6050_driver.h` |
| **依赖注入（接口注入）** | Driver `_inst()` 构造函数 | `aht21_driver.c`, `mpu6050_adaption.c` |
| **Bridge模式（抽象与实现分离）** | Adapter + Wrapper | `aht21_wrapper.h` + `aht21_adaption.c` |
| **三层架构（Driver/Handler/Adapter-Wrapper）** | 每个BSP外设 | `01_Bsp/*/` + `02_Adapter/*/` |
| **声明式配置（数据即配置）** | RTOS资源创建 | `user_task_config.c` (`st_usertaskcfg_t[]`) |
| **弱符号桩（编译期功能裁剪）** | 任务函数默认实现 | `user_task_config.c` (`__WEAK` 函数) |
| **类型擦除（Opaque Handle）** | OS抽象层 | `platform_os.h` (`void*` 句柄) |
| **生产者-消费者（Queue解耦）** | 传感器数据发布 | `service_sensor.c` → `g_sensor_data_queue` |
| **注册/回调模式** | 看门狗任务监控 | `watchdog_monitor.c` |
| **事件驱动Handler** | 传感器数据处理 | `aht21_handler.c` (事件队列 + RTOS线程) |

---

## 13. 实现状态总览

### 已完整实现

| 模块 | 状态 | 说明 |
|------|------|------|
| 启动流程 + 时钟配置 | ✅ 完成 | HSI→PLL 100MHz |
| FreeRTOS + CMSIS-RTOS V2 | ✅ 完成 | 56优先级, heap_4, 30KB堆 |
| OS抽象层 (Task/Queue/Mutex/Sema/Event/System) | ✅ 完成 | 全opaque handle |
| GPIO抽象层 | ✅ 完成 | 集中式引脚枚举 + ROM配置表 |
| I2C抽象层 (硬件+软件双模) | ✅ 完成 | Mutex保护, DMA读 |
| AHT21传感器 (Driver→Handler→Adapter→Wrapper) | ✅ 完成 | 完整三层+Bridge |
| SEGGER RTT + EasyLogger 调试 | ✅ 完成 | RTT printf + 结构化日志 |
| 传感器轮询服务 | ✅ 完成 | Mutex保护 + Queue发布 |
| 看门狗监控服务 | ✅ 完成 | 注册/喂狗/安全模式 |
| 声明式RTOS资源配置 | ✅ 完成 | 批量创建 + Weak桩 |
| 软件I2C位脉冲驱动 | ✅ 完成 | 引脚可配置 |

### 接口已定义，实现进行中

| 模块 | 状态 | 说明 |
|------|------|------|
| MPU6050 六轴IMU | 🔧 Driver+Handler接口完整 | 20+vtable, 6DI接口, DMA环形缓冲 |
| MPU6050 Adapter/Wrapper | 🔧 接口已定义 | 待装配 |

### 预留/规划中

| 模块 | 状态 | 说明 |
|------|------|------|
| ST7789 LCD显示驱动 | 📋 预留 | SPI1引脚已配置 |
| CST816T 触摸驱动 | 📋 预留 | I2C3已初始化 |
| W25Q128 外部Flash | 📋 预留 | SPI2引脚已配置 |
| BLE蓝牙通信 | 📋 预留 | USART1已配置 |
| LVGL UI框架 | 📋 预留 | — |
| 5ms周期调度器 + 8状态系统状态机 | 📋 预留 | — |
| 低功耗管理 (STOP模式) | 📋 预留 | — |
| OTA固件升级 | 📋 预留 | — |
| 计步算法 / 心率算法 | 📋 预留 | — |
| SPL06气压 / EM7028心率 / LIS2MDL磁力计 | 📋 预留 | — |
| HP4570充电管理 / WT588语音 | 📋 预留 | — |

---

## 14. 项目文档

| 文档 | 路径 |
|------|------|
| 本架构映射文档 | `0X_DOC/SmartWatch.md` |
| 参考架构（EC_S100 Watch） | `0X_DOC/项目架构.md` |
| 学习路径 | `0X_DOC/学习路径.md` |
| 温湿度传感器和陀螺仪驱动的差异 | `0X_DOC/温湿度传感器和陀螺仪驱动的差异.md` |
| 温湿度传感器调用链 | `0X_DOC/温湿度传感器调用链.md` |

---

> **使用建议：** 当你在流程图中看到一个模块/模式，用此表快速定位到对应的 `.c`/`.h` 文件。本文档随每次涉及架构变动的 git commit 增量更新，始终保持与代码同步。


## 📜 License

本项目采用 [MIT License](LICENSE) 开源协议。

<p align="center">
  Made with ❤️ and Architecture-First mindset.
</p>


