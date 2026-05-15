//
// Created by 35540 on 2026/3/28.
//
//******************************** Includes *********************************//
#include "aht21_adaption.h"

#include "event_groups.h"


//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
#ifdef  LOG_TAG
#undef  LOG_TAG
#define LOG_TAG       "AHT_ADA"
#else // else of LOG_TAG
#define LOG_TAG       "AHT_ADA"
#endif // end of LOG_TAG

#define EVENT_TEMP      (1 << 0)
#define EVENT_HUMI      (1 << 1)
#define EVENT_TEMPHUMI  (1 << 2)
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Variables *********************************//
/* Event Flags */
extern  os_event_hanlder_t xtemphumi_event_flags_handle;

static float s_temperature = 0;
static float s_humi = 0;
//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Macros ***********************************//
#if defined(AHT21Ada_DEBUG) && defined(MYDEBUG)
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
/***************************************************************************
 *                 Static Declarations For temphumionment
 ***************************************************************************/
static void temphumi_drv_init(temphumi_drv_t *dev);
static void temphumi_drv_deinit(temphumi_drv_t *dev);
static void temphumi_drv_read_temp(temphumi_drv_t * dev, float *temp);
static void temphumi_drv_read_humi(temphumi_drv_t * dev, float *humi);
static void temphumi_drv_read_temp_and_humi(temphumi_drv_t * dev, \
                                            float *temp, float *humi);
static void temp_humi_callback(float *temperature, float *humidity);
static aht21_status_t i2c_init_myown(void* bus);
static aht21_status_t i2c_deinit_myown(void* bus);
static aht21_status_t i2c_start_myown(void* bus);
static aht21_status_t i2c_stop_myown(void* bus);
static aht21_status_t i2c_wait_ack_myown(void* bus);
static aht21_status_t i2c_send_ack_myown(void* bus);
static aht21_status_t i2c_send_no_ack_myown(void* bus);
static aht21_status_t i2c_send_byte_myown(void* bus,
                                         const uint8_t send_data);
static aht21_status_t i2c_receive_byte_myown(void* bus,
                                         uint8_t* const rx_data);
static aht21_status_t critical_enter_myown(void);
static aht21_status_t critical_exit_myown(void);
// 获取时基函数实例
static uint32_t get_tick_count_myown(void);
// os延时函数实例

static temp_humi_status_t os_delay_ms_myown(uint32_t ms);
static temp_humi_status_t os_queue_creat_myown(uint32_t num,
                                               uint32_t size,
                                               void** p_queue_handler);
static temp_humi_status_t os_queue_put_myown(void* queue_handler,
                                             void* item,
                                             uint32_t timeout);
static temp_humi_status_t os_queue_get_myown(void* queue_handler,
                                             void* item,
                                             uint32_t timeout);
//******************************** 函数声明   *********************************//
//---------------------------------------------------------------------------//
//******************************** Variables ********************************//
void drv_adapter_temphumi_register(void)
{
  temphumi_drv_t _temphumi_drv = {
    .idx = 0,
    .dev_id = 0,
    .temphumi_drv_init = temphumi_drv_init,
    .temphumi_drv_deinit = temphumi_drv_deinit,
    .temphumi_drv_read_temp = temphumi_drv_read_temp,
    .temphumi_drv_read_humi = temphumi_drv_read_humi,
    .temphumi_drv_read_temp_and_humi = \
                temphumi_drv_read_temp_and_humi,
};

  drv_adapter_temphumi_reg(0, &_temphumi_drv);
}

// 2. 初始化i2c驱动接口结构体
static aht21_i2c_driver_interface_t i2c_driver_interface = {
  .pf_i2c_init         = i2c_init_myown,
  .pf_i2c_deinit       = i2c_deinit_myown,
  .pf_i2c_start        = i2c_start_myown,
  .pf_i2c_stop         = i2c_stop_myown,
  .pf_i2c_wait_ack     = i2c_wait_ack_myown,
  .pf_i2c_send_ack     = i2c_send_ack_myown,
  .pf_i2c_send_nack    = i2c_send_no_ack_myown,
  .pf_i2c_send_byte    = i2c_send_byte_myown,
  .pf_i2c_receive_byte = i2c_receive_byte_myown,
  .pf_critical_enter   = critical_enter_myown,
  .pf_critical_exit    = critical_exit_myown
};
// 3. 初始化获取时基接口结构体
static aht21_timebase_interface_t timebase_interface = {
  .pf_get_tick_count = get_tick_count_myown
};
// 4. 初始化OS延时函数接口结构体
static aht21_yield_interface_t yeiled_interface = {
  .pf_rtos_yield = (void (*)(uint32_t)) os_delay_ms_myown
};
// 5， 初始化os操作接口结构体
static temp_humi_handler_os_api_t os_api= {
  .os_delay = os_delay_ms_myown,
  .os_queue_creat = os_queue_creat_myown,
  .os_queue_put = os_queue_put_myown,
  .os_queue_get = os_queue_get_myown
};
// 2. 组装 Driver 的专属依赖包（引擎组装）
static aht21_driver_input_api_t aht21_driver_api = {
  .p_i2c_driver = &i2c_driver_interface,
  .p_timebase   = &timebase_interface,
  .p_yield      = &yeiled_interface
};

//  组装 Handler 的输入参数（驾驶员就座）
temp_humi_handler_input_api_t humitemp_input_api = {
  .driver_api         = &aht21_driver_api,      // 👈 引擎包直接丢给 Handler
  .timebase_interface = &timebase_interface,    // Handler 自己也要用 Tick
  .os_interface       = &os_api
};
//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//
static aht21_status_t i2c_init_myown(void* bus)
{
  SENSOR_I2C_SOFTWARE_INIT();
  return AHT21_OK;
}
static aht21_status_t i2c_deinit_myown(void* bus)
{
  return AHT21_OK;
}
static aht21_status_t i2c_start_myown(void* bus) {
  SENSOR_I2C_SOFTWARE_START();
  return AHT21_OK;
}
static aht21_status_t i2c_stop_myown(void* bus)
{
  SENSOR_I2C_SOFTWARE_STOP();
  return AHT21_OK;
}
static aht21_status_t i2c_wait_ack_myown(void* bus)
{
  unsigned char ret = SUCCESS; // should be ErrorStatus but IICWaitAck(X)
  ret = SENSOR_I2C_SOFTWARE_WAITACK();
  if(SUCCESS == ret)
  {
    return AHT21_OK;
  } else {
    return AHT21_ERRORTIMEOUT;
  }
}
static aht21_status_t i2c_send_ack_myown(void* bus)
{
  SENSOR_I2C_SOFTWARE_SENDACK();
  return AHT21_OK;
}
static aht21_status_t i2c_send_no_ack_myown(void* bus)
{
  SENSOR_I2C_SOFTWARE_SENDNOACK();
  return AHT21_OK;
}
static aht21_status_t i2c_send_byte_myown(void* bus,
                                         const uint8_t send_data)
{
  SENSOR_I2C_SOFTWARE_SENDBYTE(send_data);
  return AHT21_OK;
}
static aht21_status_t i2c_receive_byte_myown(void* bus,
                                         uint8_t* const rx_data)
{
  SENSOR_I2C_SOFTWARE_RECEIVEBYTE(rx_data);
  return AHT21_OK;
}
static aht21_status_t critical_enter_myown(void)
{
  os_enter_critical();
  return AHT21_OK;
}
static aht21_status_t critical_exit_myown(void)
{
  os_exit_critical();
  return AHT21_OK;
}
static uint32_t get_tick_count_myown(void)
{
  return os_get_tick_ms();
}
static temp_humi_status_t os_delay_ms_myown(uint32_t ms)
{
  os_delay_ms(ms);
  return TEMP_HUMI_OK;
}
static temp_humi_status_t os_queue_creat_myown(uint32_t num, uint32_t size, void** p_queue_handler) {
  ASSERT_NOT_NULL(p_queue_handler);
  return (os_queue_create((os_queue_handler_t*)p_queue_handler, size, num) == SUCCESS) ? TEMP_HUMI_OK:TEMP_HUMI_ERROR;
}
static temp_humi_status_t os_queue_put_myown(void* queue_handler, void* item, uint32_t timeout) {
  ASSERT_NOT_NULL(queue_handler);
  return (os_queue_put((os_queue_handler_t)queue_handler, item, timeout) == SUCCESS) ? TEMP_HUMI_OK : TEMP_HUMI_ERROR;
}
static temp_humi_status_t os_queue_get_myown(void* queue_handler, void* item, uint32_t timeout) {
  ASSERT_NOT_NULL(queue_handler);
  return (os_queue_get((os_queue_handler_t)queue_handler, item, timeout) == SUCCESS) ? TEMP_HUMI_OK : TEMP_HUMI_ERROR;
}

static void temphumi_drv_init(temphumi_drv_t *dev)
{
    //;
}

static void temphumi_drv_deinit(temphumi_drv_t *dev)
{
    //
}
static void temphumi_drv_read_temp(temphumi_drv_t * dev, float *temp)
{
  uint32_t active_bits = 0;
  const uint32_t uxBitsToWaitFor = EVENT_TEMP;
  (void)dev;

  static temp_humi_event_t event = {
    .lifetime = 0,
    .type = TEMP_HUMI_EVENT_TEMP,
    .pf_callback = temp_humi_callback
  };
  if (TEMP_HUMI_OK != bsp_temp_humi_read(&event))
  {
    LOG_ERROR("bsp_temp_humi_read failed, handler not ready");
    return;
  }
  event.lifetime = 1000;

  os_event_wait(
          xtemphumi_event_flags_handle,
          uxBitsToWaitFor,
          true,
          true,
          0xFFFFFFFF,
          &active_bits
      );

  if ((active_bits & uxBitsToWaitFor) == uxBitsToWaitFor)
  {
    *temp = s_temperature;
  }
}
static void temphumi_drv_read_humi(temphumi_drv_t * dev, float *humi)
{
    //读取湿度
}
static void temphumi_drv_read_temp_and_humi(temphumi_drv_t * dev, float *temp, float *humi)
{
  uint32_t active_bits = 0;
  const uint32_t uxBitsToWaitFor = EVENT_TEMPHUMI;
  (void)dev;

  static temp_humi_event_t event = {
    .lifetime = 0,
    .type = TEMP_HUMI_EVENT_TEMP,
    .pf_callback = temp_humi_callback
  };
  if (TEMP_HUMI_OK != bsp_temp_humi_read(&event))
  {
    LOG_ERROR("bsp_temp_humi_read failed, handler not ready");
    return;
  }

  os_event_wait(
          xtemphumi_event_flags_handle,
          uxBitsToWaitFor,
          true,
          true,
          0xFFFFFFFF,
          &active_bits
      );

  if ((active_bits & uxBitsToWaitFor) == uxBitsToWaitFor)
  {
    *temp = s_temperature;
    *humi = s_humi;
  }
}


static void temp_humi_callback(float *temperature, float *humidity)
{
    s_temperature = *temperature;
    s_humi = *humidity;
    os_event_set(xtemphumi_event_flags_handle, EVENT_TEMPHUMI);
}


