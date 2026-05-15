//
// Created by 35540 on 2026/5/14.
//
//******************************** Includes *********************************//
#include "mpu6050_handler.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
#ifdef  LOG_TAG
#undef  LOG_TAG
#define LOG_TAG       "MPU_HAN"
#else // else of LOG_TAG
#define LOG_TAG       "MPU_HAN"
#endif // end of LOG_TAG

#define HANDLER_UNINITIALIZED 0
#define HANDLER_INITIALIZED   1
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Variables *********************************//
static uint8_t mpu6050_handler_is_initialized = HANDLER_UNINITIALIZED; // Handler 状态
void *g_mpu6050_driver_ctx = NULL;  // ISR回调使用，存储驱动实例指针
bsp_mpu6050_handler_t handler_instance = {0};    // MPU6050 Handler 实例
//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Macros ***********************************//
#if defined(MPUHan_DEBUG) && defined(MYDEBUG)
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
void (*pf_pin_interrupt_callback)(void *, void *) = NULL;
void (*pf_dma_interrupt_callback)(void *, void *) = NULL;
//******************************** 函数声明   *********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//
/*
* @brief 回调函数  注册MPU6050硬件INT中断
* @param 回调函数
*/
void register_callback(void (*callback)(void *, void *))
{
  pf_pin_interrupt_callback = callback;
}

/*
* @brief 回调函数  注册MPU6050DMA中断
* @param 回调函数
*/
void register_callback_dma(void (*callback)(void *, void *))
{
  pf_dma_interrupt_callback = callback;
}

/**
 * @brief 加载实例
 *
 * @param p_handler_instance: The handler to initialize.
 *
 * @return: The status of the initialization.
 */
mpu6050_status_t mpu6050_handler_init(bsp_mpu6050_handler_t *p_handler_instance)
{
  mpu6050_status_t ret = MPU6050_OK;
  mpu6050_os_interface_t *p_os = NULL;
  if (HANDLER_INITIALIZED == mpu6050_handler_is_initialized)
  {
    LOG_DEBUG("mpuxxx_handler_init: handler already initialized\n");
    return MPU6050_ERROR;
  }
  p_os = p_handler_instance->p_input_args->p_os;  //OS层的抽象接口
  ASSERT_NOT_NULL(p_os);
  ASSERT_NOT_NULL(p_os->os_queue_create);
  // 谁往里放？ DMA 完成中断回调（ISR 上下文）。
  // 放什么？ 放一个简单的标志位（比如 uint8_t data = 1），意思是“DMA 搬完 1024 字节了，请处理”。
  // 谁从中取？ mpu6050_handler_thread（RTOS 任务上下文）。
  // 核心作用：中断到任务的唤醒与同步。它非常短小精悍，只传递“信号”，不传大数据，保证 ISR 能以最快速度退出。
  ret = p_os->os_queue_create(p_handler_instance->queue_length,
                                       p_handler_instance->queue_item_size,
                                      &p_handler_instance->p_queue_handle);
  ASSERT_CONDITION(MPU6050_OK == ret);
  ASSERT_NOT_NULL(p_handler_instance->p_queue_handle);
  // 谁往里放？ mpu6050_handler_thread（任务上下文）。
  // 放什么？ 放解析后的、有物理意义的数据结构（比如包含 ax, ay, az, gx, gy, gz 的 mpu6050_data_t 结构体），或者按包切分好的数据索引。
  // 谁从中取？ 更上层的算法任务（如卡尔曼滤波任务、计步算法任务、姿态解算任务）。
  // 核心作用：数据转换与流控缓冲。
  ret = p_os->os_queue_create( p_handler_instance->queue_length,
                                        p_handler_instance->queue_item_size,
                                        &p_handler_instance->p_unpack_queue_handle);
  ASSERT_CONDITION(MPU6050_OK == ret);
  ASSERT_NOT_NULL(p_handler_instance->p_unpack_queue_handle);
  ASSERT_NOT_NULL(p_handler_instance->p_driver);

  ret = p_os->os_semaphore_create_binary(&p_handler_instance->p_semaphore_handle);
  ASSERT_CONDITION(MPU6050_OK == ret);
  ASSERT_NOT_NULL(p_handler_instance->p_semaphore_handle);
  /* 任务通知是绑定任务本身的 所以不需要创建notify 在mpu6050_thread中创建 */
  // ✨【新增】打包由 Handler 动态创建的 OS 句柄
  p_handler_instance->os_handles.queue_handle     = p_handler_instance->p_queue_handle;
  p_handler_instance->os_handles.semaphore_handle = p_handler_instance->p_semaphore_handle;
  p_handler_instance->os_handles.notify_handle    = p_handler_instance->p_notify_handle;
  // ✨【修改】极度清爽的调用！不再手动拆包散装接口
  ret = bsp_mpu6050_driver_inst(
      p_handler_instance->p_driver,
      p_handler_instance->p_input_args->p_driver_api, // 透传依赖包
#ifdef OS_SUPPORTING
      &p_handler_instance->os_handles,                // 透传句柄包
#endif
      register_callback,
      register_callback_dma
  );
  ASSERT_CONDITION(MPU6050_OK == ret);
  mpu6050_handler_is_initialized = HANDLER_INITIALIZED;
  return ret;
}

mpu6050_status_t mpu6050_handler_inst(bsp_mpu6050_handler_t *p_handler_instance,
                                     mpu6050_handler_input_args_t *p_input_args)
{
  mpu6050_status_t ret = MPU6050_OK;
  LOG_DEBUG("mpuxxx_handler_inst start\n");
  ASSERT_NOT_NULL(p_handler_instance);
  ASSERT_NOT_NULL(p_input_args);
  ASSERT_NOT_NULL(p_input_args->p_driver_api); // 只需校验包存在
  ASSERT_NOT_NULL(p_input_args->p_os);

  p_handler_instance->p_input_args = p_input_args;
  ret = mpu6050_handler_init(p_handler_instance);
  ASSERT_CONDITION(MPU6050_OK == ret);
  LOG_DEBUG("mpuxxx_handler_inst end\n");
  return ret;
}

void mpu6050_handler_thread(void *argument)
{
  mpu6050_status_t ret = MPU6050_OK;
  uint8_t data = 0;
  static bsp_mpu6050_driver_t p_driver; // ✅ 不占栈，生命周期全局
  LOG_DEBUG("mpuxxx_handler_thread start\n");
  ASSERT_NOT_NULL(argument);
  mpu6050_handler_input_args_t *p_input_args =(mpu6050_handler_input_args_t *)argument;
  p_input_args->p_driver_api->p_buffer->pf_buffer_init(p_input_args->p_driver_api->p_buffer->p_ctx, 2); // ✨【修改】访问层级

  //将p_handler_instance.p_driver与变量p_driver绑定 此时p_driver中全是0
  handler_instance.p_driver              = &p_driver;
  handler_instance.p_queue_handle        = NULL;
  handler_instance.p_unpack_queue_handle = NULL;
  handler_instance.queue_item_size       = 1;
  handler_instance.queue_length          = 10;
  //通过RTOS任务将接口具体实现作为参数传入handler_inst 并绑定到p_handler_instance
  //整个逻辑流程mpu6050_handler_thread->mpu6050_handler_inst->mpu6050_handler_init->bsp_mpu6050_driver_inst
  ret = mpu6050_handler_inst(&handler_instance, p_input_args);
  ASSERT_CONDITION(MPU6050_OK == ret);
  LOG_DEBUG("=====mpuxxx_handler_inst success=====\n");
  g_mpu6050_driver_ctx = handler_instance.p_driver;  // 导出驱动实例供ISR回调使用

  for (;;)
  {
    ret = handler_instance.p_input_args->p_os->os_queue_get(handler_instance.p_queue_handle,
                                                       &data,
                                                       0xffffffff);
    ASSERT_CONDITION(MPU6050_OK == ret);
    ret = handler_instance.p_input_args->p_os->os_queue_put(handler_instance.p_unpack_queue_handle,
                                                &data,
                                                0);
    ASSERT_CONDITION(MPU6050_OK == ret);
    vTaskDelay(2);
  }
}