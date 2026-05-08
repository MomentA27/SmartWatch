//
// Created by 35540 on 2026/4/24.
//
//******************************** Includes *********************************//
#include "service_sensor.h"

//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
/**
 * @brief 日志tag
 * @note  用于日志查找模块
 */
#ifdef  LOG_TAG
#undef  LOG_TAG
#define LOG_TAG       "SERVICE_SENSOR"
#else // else of LOG_TAG
#define LOG_TAG       "SERVICE_SENSOR"
#endif // end of LOG_TAG
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Variables *********************************//
service_sensor_handler_t sensor_state =
{
  .active_sensors = 0,                      //当前活跃的传感器掩码
  .temp_sample_rate = 1000,                 //温度采样率(ms)
  .last_temp_sample = 0,                    //上一次温度采样时间
  .temp_sampling_enabled = true            //温度采样使能
};

//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Macros ***********************************//
#if defined(SERVICE_SENSOR_DEBUG) && defined(MYDEBUG)
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
void sensor_temp_humi(void);
//******************************** 函数声明   *********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//
/**
 * @brief 传感器轮询任务 - 实现传感器数据采集与处理
 * @param[in] argument : 任务参数
 * @return None
 */
void sensor_polling_task(void *argument) {
  LOG_DEBUG("sensor polling task start");
  uint32_t current_time;

  watchdog_register(os_task_get_handle(), 5000, "SensorTask");
  for(;;)
  {
    watchdog_feed(os_task_get_handle());
    current_time = os_get_tick_ms();
    if(sensor_state.temp_sampling_enabled)
    {
      if((current_time - sensor_state.last_temp_sample) >= sensor_state.temp_sample_rate || sensor_state.last_temp_sample == 0)
      {
        sensor_temp_humi();
        sensor_state.last_temp_sample = current_time;
      }
    }
    os_task_delay(100);
  }

}


void sensor_temp_humi(void)
{
  float temp, humi;
  float sensor_data[2];

  LOG_DEBUG("Reading temp/humidity sensor");
  temphumi_read_temp_and_humi(&temp, &humi);
  if(SUCCESS == os_mutex_take(g_sensor_data_mutex, 100))
  {
    if(temp != g_system_status.temperature)
    {
      g_system_status.temperature = temp;
      g_system_status.humidity = humi;
      g_system_status.system_tick = os_get_tick_ms();
    }
  }
  os_mutex_give(g_sensor_data_mutex);
  LOG_DEBUG("Temperature: %d, Humidity: %d", (int)temp,(int)humi);
  sensor_data[0] = temp;
  sensor_data[1] = humi;
  os_queue_put(g_sensor_data_queue, sensor_data, 0);
}

/**
 * @brief 配置并启动指定传感器的采样任务（非阻塞，仅修改状态机）
 *
 * @note  本函数不会直接执行硬件采集操作！它仅修改内部的 sensor_state 状态结构体。
 *        真正的采集动作由后台的 sensor_polling_task 线程在下一个轮询周期内根据此状态执行。
 *
 * @param[in] sensor_mask  传感器的位掩码组合，用于指定要启动哪些传感器。
 *                         可选参数通过按位或(|)组合，例如：
 *                         - SENSOR_TEMP：仅启动温度
 *                         - SENSOR_TEMP | SENSOR_HUMIDITY：启动温湿度
 *                         - SENSOR_HEARTRATE：启动心率
 * @param[in] sample_rate  采样的时间间隔，单位：毫秒。
 *                         - 例如 1000 表示每 1000ms (1秒) 采样一次。
 *                         - 【特殊值】如果传入 0，在下方的轮询逻辑中，由于
 *                           (current_time - last_sample) >= 0 恒成立，会导致
 *                           sensor_polling_task 每次唤醒时都疯狂执行采样（相当于最高频轮询）。
 *
 * @return None
 */
void sensor_start_sampling(uint32_t sensor_mask, uint32_t sample_rate)
{
  if(sensor_mask & SENSOR_TEMP)
  {
    sensor_state.temp_sampling_enabled = true;
    sensor_state.temp_sample_rate = sample_rate;
    sensor_state.active_sensors |= SENSOR_TEMP;
    LOG_DEBUG("Temperature sampling started, rate: %dms", sample_rate);
  }

  if(sensor_mask & SENSOR_HUMIDITY)
  {
    sensor_state.temp_sampling_enabled = true;  /* 锟斤拷湿锟斤拷一锟斤拷锟斤拷锟?*/
    sensor_state.active_sensors |= SENSOR_HUMIDITY;
    LOG_DEBUG("Humidity sampling started");
  }
}
