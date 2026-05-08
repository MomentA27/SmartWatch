# 硬件驱动封装层 BSP Platform

## 📌 模块定位
封装所有与具体硬件设备相关的底层驱动，如传感器、Flash、IO扩展等。通过标准接口向上提供访问能力，屏蔽芯片差异。

## 📁 模块结构示例
```
02_BSP_Platform/
├── Bsp_Drivers/           ← 放硬件驱动实现（如 sensor、flash、LED）
├── Platform_Interface/    ← 放上层调用的接口定义（抽象），以及接口实现（适配）
├── Bsp_Integration/       ← 集成BSP_Drivers需要的接口（来自OS, MCU-Core, Middleware）
└── README

## ✅ 命名规范
- 模块前缀建议使用 `Dbg_` 或 `Test_`
- 函数命名建议使用 `MCU_设备_操作`，如 `MCU_UART_Send()`
- 结构体命名建议使用 `MCU_设备_Config_t`，如 `MCU_UART_Config_t`
- 宏定义建议使用 `MCU_设备_XXX`，如 `MCU_UART_TX_PIN`

## 🔄 依赖关系
- 仅依赖 `MCU Platform` 提供的 HAL 层接口
- 不得直接访问操作系统或APP

## 👥 责任人
- BSP平台负责人：@噔噔噔