//
// Created by 35540 on 2026/4/14.
//

#ifndef SMARTWATCH_STM32F4_USER_TASK_CONFIG_H
#define SMARTWATCH_STM32F4_USER_TASK_CONFIG_H
//******************************** Includes *********************************//
#include <stdbool.h>
#include "platform_os.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
// 任务优先级分类
#define PRI_EMERGENCY     (configMAX_PRIORITIES-1)  // 紧急优先级（最高），预留2个优先级
#define PRI_HARD_REALTIME (PRI_EMERGENCY-4)         // 硬实时优先级（传感器采样），4级跨度
#define PRI_SOFT_REALTIME (PRI_HARD_REALTIME-5)     // 软实时优先级（协议解析），5级跨度
#define PRI_NORMAL        (PRI_SOFT_REALTIME-7)     // 普通业务任务优先级
#define PRI_BACKGROUND    (1)                       // 后台任务优先级（日志上传）

#define USER_CONFIG_DEBUG 1
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Typedefs *********************************//
//任务枚举 进行统一管理
typedef enum
{
  tempHandlerTask = 0,
  WatchDog_Thread,
  SensorTask,
  MpuHandlerTask,
  USER_IDX_MAX,
}user_task_idx_e;
//队列枚举 进行统一管理
typedef enum
{
  SensorDataQueue = 0,
  Queue_IDX_MAX
}user_queue_idx_e;
//互斥锁枚举 进行统一管理
typedef enum {
  SensorDataMutex = 0,
  USER_MUTEX_NUM      // 复用这个宏作为数组大小
} user_mutex_idx_e;
//信号量枚举 进行统一管理
typedef enum
{
  Sema_IDX_MAX
}user_sema_idx_e;
typedef struct
{
  os_task_func_t    func_pointer;  // 【改】用抽象的函数指针，不用 TaskFunction_t
  const char *      task_name;     // 保持不变
  uint16_t          stack_size;    // 【改】用标准的 uint16_t，不用 configSTACK_DEPTH_TYPE
  void *            argument;      // 【改】去掉多余的 const
  uint32_t          priority;      // 【改】用标准的 uint32_t，不用 UBaseType_t
  os_task_handler_t task_handle;   // 【改】用抽象的句柄，不用 TaskHandle_t
} st_usertaskcfg_t;
//队列配置结构体
typedef struct
{
  const char *        queue_name;     // 队列名称
  size_t              queue_depth;    // 队列深度
  size_t              data_size;      // 数据大小
  os_queue_handler_t  queue_handle;   // 队列句柄
}st_userqueuecfg_t;
//互斥锁配置结构体
typedef struct
{
  const char *        mutex_name;     // 互斥锁名称
  os_mutex_handler_t  mutex_handle;   // 互斥锁句柄
}st_usermutexcfg_t;
//信号量的统一配置描述结构体
typedef struct
{
  const char *        sema_name;      // 信号量名称  用于调试
  uint32_t            max_count;      // 计数信号量专用 最大计数值
  uint32_t            init_count;     // 计数信号量专用 初始计数值
  bool                is_binary;      // true 表示创建二值信号量，false 表示创建计数信号量
  os_sema_handler_t   sema_handle;    // 信号量句柄
}st_usersemacfg_t;
//******************************** Typedefs *********************************//
//---------------------------------------------------------------------------//
//**************************** Interface Structs ****************************//
//**************************** Interface Structs ****************************//
//---------------------------------------------------------------------------//
//******************************** Classes **********************************//
//******************************** Classes **********************************//
//---------------------------------------------------------------------------//
//**************************** Extern Variables *****************************//
// 任务配置数组声明
extern st_usertaskcfg_t st_usertaskcfg[];

// 队列配置数组声明
extern st_userqueuecfg_t st_userqueuecfg[Queue_IDX_MAX];

// 互斥锁配置数组声明
extern st_usermutexcfg_t st_usermutexcfg[USER_MUTEX_NUM];

// 信号量配置数组声明
extern st_usersemacfg_t st_usersemacfg[Sema_IDX_MAX];

// 全局互斥锁句柄（供外部使用）
extern SemaphoreHandle_t Semaphore_ExtFlashState;

// 全局任务句柄（供外部使用）
extern SemaphoreHandle_t DownloadAppData_taskHandle;
//**************************** Extern Variables *****************************//
//---------------------------------------------------------------------------//
//******************************** 函数声明 **********************************//
int32_t user_os_resources_init(void);
//******************************** 函数声明 **********************************//
#endif //SMARTWATCH_STM32F4_USER_TASK_CONFIG_H