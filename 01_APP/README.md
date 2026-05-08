# 应用层 APP 总体说明

## 📌 模块定位
本目录用于存放业务逻辑相关的代码，包括场景任务、流程控制、用户交互逻辑等。该层负责 orchestrate（协调）各个平台提供的能力模块。

## 📁 建议子目录结构
- `Platform_IO_Register/`：对adaption中的实现绑定的统一管理
- `App_Init/`：应用初始化
- `App_Config/`：应用配置

## ✅ 命名规范
- 模块前缀建议使用 `Dbg_` 或 `Test_`
- 函数命名建议使用 `MCU_设备_操作`，如 `MCU_UART_Send()`
- 结构体命名建议使用 `MCU_设备_Config_t`，如 `MCU_UART_Config_t`
- 宏定义建议使用 `MCU_设备_XXX`，如 `MCU_UART_TX_PIN`

## 🔄 依赖关系
- 依赖 Platform 各个平台能力（OS/BSP/MCU/Middleware）
- 不应包含硬件直接访问代码

## 👥 责任人
- 应用层负责人：@噔噔噔
