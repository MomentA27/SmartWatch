# MCU 平台抽象层说明

## 📌 模块定位
封装与芯片架构相关的能力，如异常处理、系统初始化、外设集群、启动文件等。

## 📁 子模块建议
- `MCU_Startup/`：启动流程
- `MCU_Cluster_UART/`：UART组封装
- `MCU_ExceptionHandler/`：异常统一处理

## ✅ 命名规范
- 模块前缀建议使用 `Dbg_` 或 `Test_`

## 🔄 依赖关系
- 不依赖 OS、不依赖 BSP
- 提供能力给 BSP / OSAL / APP 使用

## 👥 责任人
- MCU平台负责人：@Jack,Simon
