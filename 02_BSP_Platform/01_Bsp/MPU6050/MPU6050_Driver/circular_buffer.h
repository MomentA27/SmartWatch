/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file circular_buffer.h
 *
 * @par dependencies
 *
 * - stdint.h
 *
 * @author liu
 *
 * @brief Provide the circular buffer APIs.
 *
 * Processing flow:
 *
 * call directly.
 *
 * @version V1.0 2024-12-06
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/
#ifndef __CIRCULAR_BUFFER_H__
#define __CIRCULAR_BUFFER_H__
//******************************** Includes *********************************//
#include <stdint.h>

typedef struct circular_buffer
{
  uint8_t *buffer; // 数据缓冲区基地址
  uint8_t rflag;   // 读位置(包索引)
  uint8_t wflag;   // 写位置(包索引)
  uint8_t size;    // 缓冲区容量(可容纳的数据包个数)

  /* 抽象接口：形参统一为 void* 以适配 Driver 层的 buffer_interface_t */
  uint8_t *(*pfget_wbuffer_addr)(void *p_ctx); // 获取写缓冲区地址
  uint8_t *(*pfget_rbuffer_addr)(void *p_ctx); // 获取读缓冲区地址
  void     (*pfdata_writed)    (void *p_ctx);  // 写数据完成，推进写指针
  void     (*pfdata_readed)    (void *p_ctx);  // 读数据完成，推进读指针
} circular_buffer_t;

/**
 * @brief 初始化缓冲区
 * @param[in] buffer 缓冲区实例指针
 * @param[in] size   缓冲区可容纳的数据包个数(每包固定14字节)
 */
void buffer_init(void *p_ctx, uint8_t size);
uint8_t *get_wbuffer_addr(void *p_ctx);
uint8_t *get_rbuffer_addr(void *p_ctx);
void data_writed(void *p_ctx);
void data_readed(void *p_ctx);
#endif /* __CIRCULAR_BUFFER_H__ */
