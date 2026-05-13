//******************************** Includes *********************************//
#include "aht21_driver.h"

//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
/**
 * @brief 日志tag
 * @note  用于日志查找模块
 */
#ifdef  LOG_TAG
#undef  LOG_TAG
#define LOG_TAG       "AHT21DRriver"
#else // else of LOG_TAG
#define LOG_TAG       "AHT21DRriver"
#endif // end of LOG_TAG
#define IS_INITED                  (1 == inited)
#define AHT21_MEASURE_WAITING_TIME (80         )
#define AHT21_ID                   (0X18       )
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Variables *********************************//
static uint8_t inited;  // 初始化状态
//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Macros ***********************************//
#if defined(AHT21DRriver) && defined(MYDEBUG)
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
static aht21_status_t read_ID(bsp_aht21_driver_t * const p_aht21,
                             uint8_t * const state_num);
//******************************** 函数声明   *********************************//
//---------------------------------------------------------------------------//
//******************************** Variables ********************************//

//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//
/**
 * 用于读取AHT21的运行状态
 * @param[in] p_aht21  指向AHT21对象的结构体指针
 * @return  表示状态的一字节数据
 */
static uint8_t aht21_read_status(bsp_aht21_driver_t * const p_aht21)
{
  if (!IS_INITED) return AHT21_ERRORRESOURCE;
  uint8_t rx_data = 0;
  /**********************************临界区*************************************/
  p_aht21->p_i2c_driver_interface->pf_critical_enter();
  {
    /** 发送起始信号,发送从机读取地址*/
    p_aht21->p_i2c_driver_interface->pf_i2c_start    (NULL);
    p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL,
                                                      AHT21_REG_READ_ADDR);
    uint8_t ret = p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack(NULL);
    /** 超时检测*/
    ASSERT_CONDITION(ret == AHT21_OK);
    /** 读取从机一字节数据*/
    p_aht21->p_i2c_driver_interface->pf_i2c_receive_byte(NULL,&rx_data);
    p_aht21->p_i2c_driver_interface->pf_i2c_send_nack   (NULL);
    p_aht21->p_i2c_driver_interface->pf_i2c_stop        (NULL);
  }
  p_aht21->p_i2c_driver_interface->pf_critical_exit();
  /********************************临界区结束************************************/
  LOG_DEBUG("rx_data = %#x",rx_data);

  return rx_data;
}

/**
 * 初始化AHT21对象
 * @param[in] p_aht21  指向AHT21对象结构体的指针
 * @return  返回初始化结果
 */
static aht21_status_t aht21_init(bsp_aht21_driver_t * const p_aht21)
{
  LOG_DEBUG("aht21_init start");
  if (IS_INITED) return AHT21_ERRORRESOURCE;
  inited = 1; // 标记初始化成功
  p_aht21->p_yield_interface->pf_rtos_yield(300);
  ASSERT_NOT_NULL(p_aht21);
  ASSERT_NOT_NULL(p_aht21->p_i2c_driver_interface);
  ASSERT_NOT_NULL(p_aht21->p_i2c_driver_interface->pf_i2c_init);

  p_aht21->p_i2c_driver_interface->pf_i2c_init(NULL);
  LOG_DEBUG("aht21_init iic_driver_init");
  uint8_t state_num = 0;
  read_ID(p_aht21, &state_num); // 读取状态ID

  LOG_DEBUG("aht21_is_init,state is [%#X]",state_num);
  return AHT21_OK;
}

/**
 * 用于AHT21的反初始化
 * @param[in] p_aht21  指向AHT21对象结构体的指针
 * @return  返回释放结果
 */
static aht21_status_t aht21_deinit(bsp_aht21_driver_t * const p_aht21)
{
  ASSERT_NOT_NULL(p_aht21);
  inited = 0;
  return AHT21_OK;
}

/**
 * 用于读取AHT21的状态
 * @param[in]  p_aht21   指向AHT21对象结构体的指针
 * @param[out] state_num 读取状态
 * @return  返回初始化结果
 */
static aht21_status_t read_ID(bsp_aht21_driver_t * const p_aht21  ,
                             uint8_t             * const state_num)
{
  /*****************************临界区*******************************************/
  p_aht21->p_i2c_driver_interface->pf_critical_enter();
  {
    p_aht21->p_i2c_driver_interface->pf_i2c_start    (NULL);
    p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL, AHT21_REG_READ_ADDR);
    aht21_status_t ret = p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack(NULL);
    ASSERT_CONDITION(ret == AHT21_OK);
    p_aht21->p_i2c_driver_interface->pf_i2c_receive_byte(NULL, state_num);
    p_aht21->p_i2c_driver_interface->pf_i2c_send_nack   (NULL);
    p_aht21->p_i2c_driver_interface->pf_i2c_stop        (NULL);
  }
  p_aht21->p_i2c_driver_interface->pf_critical_exit();
  /********************************临界区结束*************************************/
  return AHT21_OK;
}

/**
 * 用于AHT21的测温和湿度数据读取
 * @param[in]  p_aht21   指向AHT21对象结构体的指针
 * @param[out] temp      温度数据
 * @param[out] humi      湿度数据
 * @return  返回初始化结果
 */
static aht21_status_t aht21_read_temp_humi(bsp_aht21_driver_t * const p_aht21,
                                           float              * const temp   ,
                                           float              * const humi   )
{
  if (!IS_INITED) return AHT21_ERRORRESOURCE;
  uint8_t cnt = 5;
  uint8_t data_buf [6] = {0};
  uint32_t retu_data = 0;
  // 1.发送测温命令
  /*****************************临界区*******************************************/
  p_aht21->p_i2c_driver_interface->pf_critical_enter();
  {
    p_aht21->p_i2c_driver_interface->pf_i2c_start    (NULL);
    p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL,
                                                      AHT21_REG_WRITE_ADDR);
    p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack (NULL);
    p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL,
                                                      AHT21_REG_MEASURE_CMD);
    p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack (NULL);
    p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL,
                                               AHT21_REG_MEASURE_CMD_ARFS1);
    p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack (NULL);
    p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL,
                                               AHT21_REG_MEASURE_CMD_ARFS2);
    p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack (NULL);
    p_aht21->p_i2c_driver_interface->pf_i2c_stop     (NULL);
  }
  p_aht21->p_i2c_driver_interface->pf_critical_exit();
  /********************************临界区结束*************************************/
  // 2.等待测温完成
  p_aht21->p_yield_interface->pf_rtos_yield(AHT21_MEASURE_WAITING_TIME);

  while (0x80 == (0x80 & aht21_read_status(p_aht21)) && cnt)
  {
    p_aht21->p_yield_interface->pf_rtos_yield(5);
    cnt--;
    if (0 == cnt) return AHT21_ERRORTIMEOUT;
  }
  LOG_DEBUG("read temp start ......");
  // 3.读取测量结果
  /*****************************临界区*******************************************/
  p_aht21->p_i2c_driver_interface->pf_critical_enter();
  {
    p_aht21->p_i2c_driver_interface->pf_i2c_start    (NULL);
    p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL,
                                                      AHT21_REG_READ_ADDR);
    p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack (NULL);
    for (uint8_t i = 0; i < 5; i++)
    {
      p_aht21->p_i2c_driver_interface->pf_i2c_receive_byte(NULL,
                                                           &data_buf[i]);
      p_aht21->p_i2c_driver_interface->pf_i2c_send_ack(NULL);
    }
    p_aht21->p_i2c_driver_interface->pf_i2c_receive_byte(NULL,
                                                         &data_buf[5]);
    p_aht21->p_i2c_driver_interface->pf_i2c_send_nack   (NULL);
    p_aht21->p_i2c_driver_interface->pf_i2c_stop        (NULL);
  }
  p_aht21->p_i2c_driver_interface->pf_critical_exit();
  /********************************临界区结束*************************************/
  retu_data = (retu_data | data_buf[1]) <<8;
  retu_data = (retu_data | data_buf[2]) <<8;
  retu_data = (retu_data | data_buf[3]) >>4;
  retu_data = retu_data & 0xFFFFF;
  *humi = (float)(retu_data * 1000 >> 20);
  *humi /= 10;

  retu_data = 0;
  retu_data = (retu_data | (data_buf[3] & 0x0f)) <<8;
  retu_data = (retu_data | data_buf[4]) <<8;
  retu_data = (retu_data | data_buf[5]);
  retu_data = retu_data & 0xFFFFF;
  *temp = (float)((retu_data* 2000 >> 20) - 500);
  *temp /= 10;

  return AHT21_OK;
}

/**
 * 用于AHT21的只读湿度信息
 * @param[in]  p_aht21   指向AHT21对象结构体的指针
 * @param[out] humi      湿度数据
 * @return  返回初始化结果
 */
static aht21_status_t aht21_read_humi(bsp_aht21_driver_t * const p_aht21,
                                      float              * const humi   )
{
  if (!IS_INITED) return AHT21_ERRORRESOURCE;
  uint8_t cnt = 5;
  uint8_t data_buf[4] = {0};
  uint32_t retu_data = 0;
  // 1.发送测温命令
  /*****************************临界区*******************************************/
  p_aht21->p_i2c_driver_interface->pf_critical_enter();
  {
    p_aht21->p_i2c_driver_interface->pf_i2c_start    (NULL);
    p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL,
                                                      AHT21_REG_WRITE_ADDR);
    p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack (NULL);
    p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL,
                                                      AHT21_REG_MEASURE_CMD);
    p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack (NULL);
    p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL,
                                               AHT21_REG_MEASURE_CMD_ARFS1);
    p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack (NULL);
    p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL,
                                               AHT21_REG_MEASURE_CMD_ARFS2);
    p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack (NULL);
    p_aht21->p_i2c_driver_interface->pf_i2c_stop     (NULL);
  }
  p_aht21->p_i2c_driver_interface->pf_critical_exit();
  /********************************临界区结束*************************************/
  // 2.等待测温完成
  p_aht21->p_yield_interface->pf_rtos_yield(AHT21_MEASURE_WAITING_TIME);

  while (0x80 == (0x80 & aht21_read_status(p_aht21)) && cnt)
  {
    p_aht21->p_yield_interface->pf_rtos_yield(5);
    cnt--;
    if (0 == cnt) return AHT21_ERRORTIMEOUT;
  }
  LOG_DEBUG("read temp start ......");
  // 3.读取测量结果
  /*****************************临界区*******************************************/
  p_aht21->p_i2c_driver_interface->pf_critical_enter();
  {
    p_aht21->p_i2c_driver_interface->pf_i2c_start    (NULL);
    p_aht21->p_i2c_driver_interface->pf_i2c_send_byte(NULL,
                                                      AHT21_REG_READ_ADDR);
    p_aht21->p_i2c_driver_interface->pf_i2c_wait_ack (NULL);
    for (uint8_t i = 0; i < 3; i++)
    {
      p_aht21->p_i2c_driver_interface->pf_i2c_receive_byte(NULL,
                                                           &data_buf[i]);
      p_aht21->p_i2c_driver_interface->pf_i2c_send_ack    (NULL);
    }
    p_aht21->p_i2c_driver_interface->pf_i2c_receive_byte(NULL,
                                                         &data_buf[3]);
    p_aht21->p_i2c_driver_interface->pf_i2c_send_nack   (NULL);
    p_aht21->p_i2c_driver_interface->pf_i2c_stop        (NULL);
  }
  p_aht21->p_i2c_driver_interface->pf_critical_exit();
  /********************************临界区结束*************************************/
  retu_data = (retu_data | data_buf[1]) <<8;
  retu_data = (retu_data | data_buf[2]) <<8;
  retu_data = (retu_data | data_buf[3]) >>4;
  retu_data = retu_data & 0xFFFFF;
  *humi = (float)(retu_data * 1000 >> 20);
  *humi /= 10;

  return AHT21_OK;
}

/**
 * 用于AHT21的休眠
 * @param[in]  p_aht21   指向AHT21对象结构体的指针
 * @return  返回初始化结果
 */
static aht21_status_t aht21_sleep(bsp_aht21_driver_t * const p_aht21)
{
  if (! IS_INITED)
  {
    return AHT21_ERRORRESOURCE;
  }
  return AHT21_OK;
}

/**
 * 用于AHT21的唤醒
 * @param[in]  p_aht21   指向AHT21对象结构体的指针
 * @return  返回初始化结果
 */
static aht21_status_t aht21_wakeup(bsp_aht21_driver_t * const p_aht21)
{
  if (! IS_INITED)
  {
    return AHT21_ERRORRESOURCE;
  }
  return AHT21_OK;
}

/**
 * 用于AHT21的初始化
 * @param[in]  p_aht21   指向AHT21对象结构体的指针
 * @return  返回初始化结果
 */
aht21_status_t aht21_inst(
                    bsp_aht21_driver_t*     const p_bsp_aht21_inst     ,
                    aht_i2c_driver_interface_t* const p_i2c_driver_inst,
                    timebase_interface_t*   const p_timebase_inst  ,
                    yield_interface_t*      const p_yield_inst     )
{
  ASSERT_NOT_NULL(p_bsp_aht21_inst);
  ASSERT_NOT_NULL(p_i2c_driver_inst);
  ASSERT_NOT_NULL(p_timebase_inst);
  ASSERT_NOT_NULL(p_yield_inst);
  p_bsp_aht21_inst->p_i2c_driver_interface = p_i2c_driver_inst   ;
  p_bsp_aht21_inst->p_timebase_interface   = p_timebase_inst     ;
  p_bsp_aht21_inst->p_yield_interface      = p_yield_inst        ;

  p_bsp_aht21_inst->pf_inst                = aht21_inst          ;
  p_bsp_aht21_inst->pf_init                = aht21_init          ;
  p_bsp_aht21_inst->pf_deinit              = aht21_deinit        ;
  p_bsp_aht21_inst->pf_read_id             = read_ID             ;
  p_bsp_aht21_inst->pf_read_temp_humi      = aht21_read_temp_humi;
  p_bsp_aht21_inst->pf_read_humi           = aht21_read_humi     ;
  p_bsp_aht21_inst->pf_sleep               = aht21_sleep         ;
  p_bsp_aht21_inst->pf_wakeup              = aht21_wakeup       ;

  aht21_status_t ret = aht21_init(p_bsp_aht21_inst);
  ASSERT_CONDITION(ret == AHT21_OK);

  LOG_DEBUG("aht21_inst end");
  return AHT21_OK;
}