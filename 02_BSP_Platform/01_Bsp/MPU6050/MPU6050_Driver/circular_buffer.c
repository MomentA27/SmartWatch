/******************************************************************************
* Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * All Rights Reserved.
 * @file circular_buffer.c
 * @par dependencies
 * - circular_buffer.h
 * @author liu
 * @brief 简易环形缓冲区，专为定长(14字节)DMA数据流设计
 * @version V1.0 2024-12-06
 * @note 1 tab == 4 spaces!
 *****************************************************************************/
#include "circular_buffer.h"
#include <stdlib.h>
#include "Debug.h"

circular_buffer_t circular_buf;

/**
 * @brief 获取当前写包的物理首地址 (供DMA直接写入)
 */
uint8_t *get_wbuffer_addr(void *p_ctx)
{
    circular_buffer_t *buffer = (circular_buffer_t *)p_ctx; // 强转回具体类型
    return buffer->buffer + buffer->wflag * 14;
}

/**
 * @brief 获取当前读包的物理首地址 (供CPU读取解析)
 */
uint8_t *get_rbuffer_addr(void *p_ctx)
{
    circular_buffer_t *buffer = (circular_buffer_t *)p_ctx; // 强转回具体类型
    return buffer->buffer + buffer->rflag * 14;
}

/**
 * @brief 推进写指针 (在DMA传输完成中断中调用)
 */
void data_writed(void *p_ctx)
{
    circular_buffer_t *buffer = (circular_buffer_t *)p_ctx; // 强转回具体类型
    buffer->wflag = (buffer->wflag + 1) % buffer->size;
}

/**
 * @brief 推进读指针 (在CPU解析完一包数据后调用)
 */
void data_readed(void *p_ctx)
{
    circular_buffer_t *buffer = (circular_buffer_t *)p_ctx; // 强转回具体类型
    buffer->rflag = (buffer->rflag + 1) % buffer->size;
}

/**
 * @brief 初始化缓冲区
 */
void buffer_init(void *p_ctx, uint8_t size)
{
    if (NULL == buffer)
    {
#ifdef DEBUG_SENSOR_MPU6050_DRIVER
        DEBUG_OUT("buffer is NULL");
#endif
        return;
    }
    // 1. 将 void* 强制转换回你实际需要的类型
    circular_buffer_t *buffer = (circular_buffer_t *)p_ctx;
    buffer->size = size;
    buffer->rflag = 0;
    buffer->wflag = 0;

    /* 分配数据区：包数 * 14字节/包 */
    buffer->buffer = (uint8_t *)malloc(size * 14);

    /* 绑定操作函数指针 */
    buffer->pfget_rbuffer_addr = get_rbuffer_addr;
    buffer->pfget_wbuffer_addr = get_wbuffer_addr;
    buffer->pfdata_readed      = data_readed;
    buffer->pfdata_writed      = data_writed;
}
