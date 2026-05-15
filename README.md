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

