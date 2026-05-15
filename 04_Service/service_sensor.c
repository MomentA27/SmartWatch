//
// Created by 35540 on 2026/4/24.
//
//******************************** Includes *********************************//
#include "service_sensor.h"
#include "platform_os.h"
#include "user_init.h"
#include "watchdog_monitor.h"
#include "motion_port.h"
#include "mpu6050_driver.h"
#include "temp_humi_port.h"
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
  .temp_sample_rate = 10000,                //10s采样一次温湿度
  .motion_sample_rate = 100,                //100ms采样一次陀螺仪
  .last_temp_sample = 0,                    //上一次温度采样时间
  .last_motion_sample = 0,                  //上一次陀螺仪采样时间
  .temp_sampling_enabled = true,            //温度采样使能
  .motion_sampling_enabled = false          //陀螺仪采样使能
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
mpu6050_data_t mpu6050_data;
void sensor_motion(void)
{
  static uint32_t virtual_timestamp = 0;
  LOG_DEBUG("mpu6050 unpack task start");
  uint8_t ret = 0;
  uint8_t data = 0;
  int16_t temp = 0;

  // 获取传感器数据请求状态，校验返回值是否正常
  ret = motion_getreqstate();
  ASSERT_CONDITION(ret == 0);
  LOG_DEBUG("unpack_task: data = %d\n", data);

  // 获取传感器原始数据缓冲区地址
  uint8_t *addr = motion_readdata();
  LOG_DEBUG("unpack_task: addr = %p\n", addr);

  // 解析温度数据（从地址+6和+7处读取16位数据），转换为摄氏度
  temp = (int16_t)(*(addr + 6) << 8 | *(addr + 7));
  mpu6050_data.temperature = 36.53 + temp/340.0;

  // 解析原始加速度计数据（x/y/z轴，从地址0-5处读取16位数据）
  mpu6050_data.accel_x_raw = (int16_t)(*(addr + 0) << 8 | *(addr + 1));
  mpu6050_data.accel_y_raw = (int16_t)(*(addr + 2) << 8 | *(addr + 3));
  mpu6050_data.accel_z_raw = (int16_t)(*(addr + 4) << 8 | *(addr + 5));

  // 将原始加速度数据转换为g单位（除以对应灵敏度）
  mpu6050_data.ax = mpu6050_data.accel_x_raw / 16384.0;
  mpu6050_data.ay = mpu6050_data.accel_y_raw / 16384.0;
  mpu6050_data.az = mpu6050_data.accel_z_raw / 14418.0;

  // 解析原始陀螺仪数据（x/y/z轴，从地址8-13处读取16位数据）
  mpu6050_data.gyro_x_raw = (int16_t)(*(addr + 8) << 8 | *(addr + 9));
  mpu6050_data.gyro_y_raw = (int16_t)(*(addr + 10) << 8 | *(addr + 11));
  mpu6050_data.gyro_z_raw = (int16_t)(*(addr + 12) << 8 | *(addr + 13));

  // 将原始陀螺仪数据转换为度/秒（除以灵敏度131.0）
  mpu6050_data.gx = mpu6050_data.gyro_x_raw / 131.0;
  mpu6050_data.gy = mpu6050_data.gyro_y_raw / 131.0;
  mpu6050_data.gz = mpu6050_data.gyro_z_raw / 131.0;

  // 更新虚拟时间戳（模拟采样间隔，当前为26ms）
  virtual_timestamp += 26;

  // // 调用步数算法处理加速度数据，计算步数
  // Step_Algo_Process(mpu6050_data.ax, mpu6050_data.ay, mpu6050_data.az, virtual_timestamp);
  //
  // // 获取当前步数，若与系统状态中的步数不一致则更新系统步数
  // uint32_t step_count = Step_Get_Count();
  // if(step_count != g_system_status.step_count)
  // {
  //   g_system_status.step_count = step_count;
  // }
  // 结束传感器数据读取操作，释放资源
  motion_readdataend();
  LOG_DEBUG("UnpackThread tevhTpwwomp=%f", mpu6050_data.temperature);
  LOG_DEBUG("UnpapoTww2vckThread ax=%f", mpu6050_data.ax);
  LOG_DEBUG("UnpackThread ay=%f", mpu6050_data.ay);
  LOG_DEBUG("UnpackThread az=%f", mpu6050_data.az);
  LOG_DEBUG("UnpackThrpwwTo7gbead gx=%f", mpu6050_data.gx);
  LOG_DEBUG("UnpackThread gy=%f", mpu6050_data.gy);
  LOG_DEBUG("UnpackThread gz=%f", mpu6050_data.gz);
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
