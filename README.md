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

## 🏗️ 系统分层架构
text
┌─────────────────────────────────────────────────────────────────┐
│ 01_APP/ 应用层 │
│ ┌──────────────┐ ┌──────────────────┐ │
│ │ User Init │ │ Task Config │ │
│ │ 启动引导 │ │ 声明式资源管理 │ │
│ └──────┬───────┘ └────────┬─────────┘ │
├─────────┼───────────────────┼──────────────────────────────────┤
│ │ 04_Service/ 服务层 │
│ ┌──────┴───────────────────┴──────────────────────────────┐ │
│ │ Sensor 传感器轮询 │ Watchdog 看门狗 │ Temp/Humi 封装 │ │
│ └──────┬──────────────────────────────────────────────────┘ │
├─────────┼─────────────────────────────────────────────────────┤
│ │ 02_BSP_Platform/ BSP 驱动层 │
│ ┌──────┴──────────────────────────────────────────────────┐ │
│ │ 01_Bsp (Driver+Handler) │ 02_Adapter/Wrapper (Bridge) │ │
│ └──────┬──────────────────────────────────────────────────┘ │
├─────────┼─────────────────────────────────────────────────────┤
│ │ 02_MCU_Platform/ MCU 外设抽象层 │
│ ┌──────┴──────────────────────────────────────────────────┐ │
│ │ GPIO (枚举+ROM配置表) │ I2C (硬件/软件双模+DMA) │ │
│ └──────┬──────────────────────────────────────────────────┘ │
├─────────┼─────────────────────────────────────────────────────┤
│ │ 02_OS_Platform/ OS 抽象层 (OSAL) │
│ ┌──────┴──────────────────────────────────────────────────┐ │
│ │ Task │ Queue │ Mutex │ Semaphore │ Event │ System │ │
│ └──────┬──────────────────────────────────────────────────┘ │
├─────────┼─────────────────────────────────────────────────────┤
│ Drivers/STM32F4xx_HAL │ Core/CMSIS │ Middlewares/FreeRTOS │
├─────────────────────────────────────────────────────────────────┤
│ 03_Libraries/ DWT延时 │ 软件I2C总线 │ FIFO │ EasyLogger │
└─────────────────────────────────────────────────────────────────┘
---

## 🧩 核心设计：三层驱动 + Bridge 模式

每个外设驱动均遵循 **Driver → Handler → Adapter/Wrapper** 管道，将硬件寄存器操作、RTOS并发处理与业务抽象完美解耦：

text
┌─────────────┐ ┌─────────────┐ ┌────────────────────────┐
│ Driver │ │ Handler │ │ Adapter + Wrapper │
│ (纯寄存器, │────▶│ (RTOS事件, │────▶│ (Bridge 模式) │
│ vtable, DI)│ │ 数据解包) │ │ 抽象接口 ↔ 具体绑定 │
└─────────────┘ └─────────────┘ └────────────────────────┘
▲ ▲
│ 依赖注入 (DI) │
┌───┴────────────────────────┐ ┌─────┴──────┐
│ I2C │ Delay │ Timebase │ OS│ │ Service │
│ Yield │ Interrupt │ Buf │ │ 业务层API │
└────────────────────────────┘ └────────────┘


**实战案例：AHT21 温湿度传感器**
- **Driver**：`aht21_driver.c` - 包含 vtable 与 `aht21_inst()` 构造函数，注入 I2C/Timebase/Yield 接口。
- **Handler**：`aht21_handler.c` - FreeRTOS 事件驱动线程，负责数据接收与解包。
- **Wrapper**：`aht21_wrapper.c` - 定义通用 `temphumi_drv_t` 接口（init/read_temp/read_humi）。
- **Adapter**：`aht21_adaption.c` - 提供 `drv_adapter_temphumi_register()` 将底层实现绑定到抽象接口。

> 💡 **扩展性**：未来若更换为 SHT30 传感器，仅需编写新的 Adapter 与 Driver，**Service 与 APP 层代码零修改**！

---

## 🔄 任务间通信模式

项目中灵活运用了多种 RTOS 通信机制，保证数据流与控制流的高效流转：

| 模式 | 应用场景 | 关键源文件 |
|------|----------|-----------|
| **Global State + Mutex** | 传感器数据共享（温度/湿度） | `service_sensor.c` 写 ↔ `user_init.h` 读 |
| **Queue (生产者-消费者)** | 异步传感器数据发布 | `SensorDataQueue` |
| **EventGroup** | 外部Flash多源事件分发 | (架构已预留) |
| **Task Suspend/Resume** | 低功耗休眠冻结任务 | (架构已预留) |

---

## 📁 源码目录结构

text
├── Core/ # STM32CubeMX 生成代码 (main, MSP, IRQ, 链接脚本)
├── Drivers/ # STM32 HAL 库
├── Middlewares/ # FreeRTOS 内核及 CMSIS-RTOS V2
├── 01_APP/ # [应用层] 系统初始化、声明式任务配置表
├── 02_BSP_Platform/ # [BSP层] 传感器驱动(01_Bsp) + 桥接适配器(02_Adapter)
├── 02_MCU_Platform/ # [MCU层] GPIO 引脚抽象、I2C 硬件/软件双模总线
├── 02_OS_Platform/ # [OS层] RTOS 抽象层
├── 03_Libraries/ # [通用库] DWT 延时、软件 I2C、FIFO、EasyLogger、SEGGER RTT
├── 04_Service/ # [服务层] 传感器轮询、温湿度封装、看门狗监控
├── cmake/ # CMake 构建配置 (arm-none-eabi-gcc 工具链)
└── 0X_DOC/ # 架构映射文档、学习路径文档


---

## 🛠️ 构建与编译

本项目采用 **CMake + arm-none-eabi-gcc** 构建，脱离对 IDE 的依赖。

1. **准备工具链**
   安装 `arm-none-eabi-gcc` 并确保添加到系统 PATH。

2. **克隆项目**
bash
git clone https://github.com/<your-username>/SmartWatch_STM32F411.git
cd SmartWatch_STM32F411


3. **编译固件**
bash

使用 CMake Presets 编译 (Debug 模式: -O0 -g3)
cmake --preset default
cmake --build build

或者编译 Release 模式: -Os -g0
cmake --preset release
cmake --build build-release


---

## 📈 实现状态

| 模块 | 状态 | 说明 |
|------|:----:|------|
| 启动流程 + 100MHz 时钟 | ✅ | HSI→PLL 100MHz |
| FreeRTOS + CMSIS-RTOS V2 | ✅ | 56优先级, heap_4, 30KB堆 |
| OS 抽象层 (OSAL) | ✅ | 全 opaque handle，支持 Task/Queue/Mutex/Sema/Event |
| GPIO / I2C 抽象 (HW+SW双模) | ✅ | 集中式引脚枚举，DMA支持 |
| AHT21 温湿度 (完整三层+Bridge) | ✅ | 包含依赖注入与事件驱动Handler |
| 看门狗监控 (软硬结合) | ✅ | 注册/喂狗/安全模式 |
| 传感器轮询服务 | ✅ | Mutex保护 + Queue异步发布 |
| MPU6050 六轴 IMU | 🔧 | Driver+Handler接口完成，Adapter装配中 |
| ST7789 LCD / CST816T 触摸 | 📋 | 引脚已配置，待开发 |
| LVGL UI / 低功耗 / OTA | 📋 | 架构已预留，规划中 |

> ✅ 已完成  🔧 开发中  📋 规划中

---

## 📚 扩展阅读

详细的设计思路与源码映射，请参阅 `0X_DOC/` 目录下的文档：

- [架构源码映射表](0X_DOC/SmartWatch.md) - 每个模块与 `.c/.h` 文件的精准对应
- [温湿度传感器调用链](0X_DOC/温湿度传感器调用链.md) - 从 APP 到硬件寄存器的完整调用链路剖析
- [温湿度传感器和陀螺仪驱动的差异](0X_DOC/温湿度传感器和陀螺仪驱动的差异.md) - 简单传感器与复合传感器在驱动架构上的对比

---

## 📜 License

本项目采用 [MIT License](LICENSE) 开源协议。

<p align="center">
  Made with ❤️ and Architecture-First mindset.
</p>


