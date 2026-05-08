//
// Created by 35540 on 2026/4/14.
//
//******************************** Includes *********************************//
#include "user_task_config.h"

#include "aht21_handler.h"
#include "service_sensor.h"
#include "watchdog_monitor.h"


//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
#ifdef  LOG_TAG
#undef  LOG_TAG
#define LOG_TAG       "USER_CONFIG"
#else // else of LOG_TAG
#define LOG_TAG       "USER_CONFIG"
#endif // end of LOG_TAG
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Variables *********************************//
extern temp_humi_handler_input_api_t humitemp_input_api;
//队列设置
st_userqueuecfg_t st_userqueuecfg[Queue_IDX_MAX] =
{
                   /*   名称                  深度                    数据大小                    句柄  */
  {"SensorDataQueue",     2,         sizeof(uint32_t), NULL},// "Sensor Data Queue"
};
//互斥锁设置
st_usermutexcfg_t st_usermutexcfg[USER_MUTEX_NUM] =
{
  /*   NawpTow2kzme               Handle */
  {"SensorDataMutex" ,NULL},
};
//信号量设置
st_usersemacfg_t st_usersemacfg[Sema_IDX_MAX] =
{
  /*   Name               Max     Init     IsBinary   Handle */

};
//信号量设置
st_usertaskcfg_t st_usertaskcfg[USER_IDX_MAX] =
{   /*    任务函数指针                任务名称                                  堆栈大小                      任务参数                     任务优先级                   任务句柄*/
  {temp_humi_handler_thread,  "tempHandlerTask" ,           512,        &humitemp_input_api,    		      PRI_HARD_REALTIME + 1,            NULL},  // 温度传感器线程
  {server_watchdog_task,      "WatchDog_Thread" ,           512,        NULL,                             PRI_SOFT_REALTIME + 3,            NULL},  // 看门狗线程
  {sensor_polling_task,       "SensorTask"      ,           512,        NULL,                             PRI_SOFT_REALTIME + 3,            NULL}   // 传感器轮询线程
};

__WEAK void temp_humi_handler_thread(void *argument)
{
  while(1);
}

__WEAK void wdg_handler_thread(void *argument)
{
  while(1);
}
//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Macros ***********************************//
#if defined(USER_CONFIG_DEBUG) && defined(MYDEBUG)
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
int32_t user_os_resources_init(void) {
  uint32_t i;
  uint8_t ret = 0;
  // 初始化队列资源
  for(i = 0; i < Queue_IDX_MAX; i++)
  {
    // 【修改这里】调用你封装的平台化 API，而不是 FreeRTOS 原生 API
    ret = os_queue_create(
              &st_userqueuecfg[i].queue_handle,                   // 传入句柄
              (uint32_t)st_userqueuecfg[i].data_size,             //数据大小
              (uint32_t)st_userqueuecfg[i].queue_depth            //队列深度
                         );

    // 判断你抽象层返回的错误码，而不是去判断 NULL
    if(ERROR == ret)
    {
      LOG_DEBUG("Queue %s create failed!", st_userqueuecfg[i].queue_name);
      return ERROR;
    }
  }

    // 初始化互斥锁资源
    for(i = 0; i < USER_MUTEX_NUM; i++)
    {
     ret  = os_mutex_create(&st_usermutexcfg[i].mutex_handle);

      if(ERROR == ret)
      {
        LOG_DEBUG("Mutex %s create failed!\n", st_usermutexcfg[i].mutex_name);
        return ERROR;
      }
    }

    // 初始化信号量资源
  for(i = 0; i < Sema_IDX_MAX; i++)
  {
    // 直接将结构体里的字段拆开，传入你的抽象 API
    // 没有调用 xSemaphoreCreate，也没有使用 FreeRTOS 的类型，完美解耦！
    ret = os_sema_create(
              &st_usersemacfg[i].sema_handle,
              st_usersemacfg[i].is_binary,
              st_usersemacfg[i].max_count,
              st_usersemacfg[i].init_count
          );

    if(ret != SUCCESS)
    {
      LOG_DEBUG("Semaphore %s create failed!", st_usersemacfg[i].sema_name);
      return ERROR;
    }
  }

  return 0;
}
