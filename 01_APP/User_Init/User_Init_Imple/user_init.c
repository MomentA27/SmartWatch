//
// Created by 35540 on 2026/4/23.
//

//******************************** Includes *********************************//
#include "user_init.h"
#include "user_task_config.h"

//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
/**
 * @brief 日志tag
 * @note  用于日志查找模块
 */
#ifdef  LOG_TAG
#undef  LOG_TAG
#define LOG_TAG       "USER_INIT"
#else // else of LOG_TAG
#define LOG_TAG       "USER_INIT"
#endif // end of LOG_TAG

#define USER_INTI_DEBUG 1
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Variables *********************************//
/* 初始化任务句柄 */
SemaphoreHandle_t userTaskInitHandle;

/* 操作系统资源句柄创建*/
os_queue_handler_t      g_sensor_data_queue;
os_sema_handler_t       g_sensor_data_mutex;
os_event_hanlder_t      xtemphumi_event_flags_handle;
//全局资源
system_status_t g_system_status = {
  .temperature = 0.0f,
  .humidity = 0.0f,
  .system_tick = 0,
  .step_count = 0
};
//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Macros ***********************************//
#if defined(USER_INTI_DEBUG) && defined(MYDEBUG)
#define LOG_DEBUG(fmt, ...)  log_d("[%s][%s:%d][DEBUG] " fmt "\r\n", \
LOG_TAG, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)  log_e("[%s][%s:%d][ERROR] " fmt "\r\n", \
LOG_TAG, __func__, __LINE__, ##__VA_ARGS__)
#define ASSERT_NOT_NULL(ptr)                 do { \
if ((ptr) == NULL) {                              \
LOG_ERROR("Invalid parameter: %s is NULL", #ptr); \
while(1);}                                        \
}while(0)
#define ASSERT_CONDITION(cond)               do { \
if (!(cond)) {                                    \
LOG_ERROR("Condition failed: %s", #cond);         \
while(1);}                                        \
}while(0)
#else
#define LOG_DEBUG(fmt, ...)   ((void)0)
#define LOG_ERROR(fmt, ...)   ((void)0)
#define ASSERT_NOT_NULL(ptr)  ((void)0)
#define ASSERT_CONDITION(cond) ((void)0)
#endif
//******************************** Macros ***********************************//
//---------------------------------------------------------------------------//
//******************************** 函数声明   *********************************//
//******************************** 函数声明   *********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//
static void userTaskInitFunction(void *argument)
{
  int32_t ret;
  /* ========== 1. OS 资源初始化 (队列/互斥锁/信号量) ========== */
  ret = user_os_resources_init();
  ASSERT_CONDITION(ret == SUCCESS);
  LOG_DEBUG("All OS resources initialized successfully");

  /* ========== 2. GPIO 硬件平台初始化 ========== */
  ret = core_gpio_init();
  ASSERT_CONDITION(ret == SUCCESS);
  LOG_DEBUG("GPIO platform initialized successfully");
  /* ========== 3. 批量创建业务任务 ========== */
  for(uint8_t i = 0; i < USER_IDX_MAX; i++)
  {
    ret = os_task_create(
             st_usertaskcfg[i].task_name,
             st_usertaskcfg[i].func_pointer,
             st_usertaskcfg[i].stack_size,
             st_usertaskcfg[i].argument,
             st_usertaskcfg[i].priority,
             &st_usertaskcfg[i].task_handle  // 取地址传出句柄
         );
    if (ERROR ==  ret) LOG_DEBUG("%s task init failed ",st_usertaskcfg[i].task_name);
    else               LOG_DEBUG("%s task init success ",st_usertaskcfg[i].task_name);
  }
  /* ========== 4. 资源句柄迁移 (将表里的句柄导出给业务层全局变量) ========== */
  // 温湿度事件组
  if (os_event_create(&xtemphumi_event_flags_handle) != SUCCESS)
  {
    LOG_DEBUG("user init event creat fail");
    while(1);
  }

  // 传感器 I2C 互斥锁

  // 队列与数据互斥锁绑定
  g_sensor_data_queue = st_userqueuecfg[0].queue_handle;
  g_sensor_data_mutex = st_usermutexcfg[0].mutex_handle;
  /* ========== 6. 功成身退，删除自身任务 ========== */
  vTaskDelete(NULL);
}

void UserAppTask_Init(void)
{
  xTaskCreate(userTaskInitFunction, "userTask",128 * 4,
    userTaskInitHandle,PRI_EMERGENCY,NULL);
}