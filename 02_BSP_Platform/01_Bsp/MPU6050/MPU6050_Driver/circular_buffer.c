/******************************************************************************
* Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * All Rights Reserved.
 * @file circular_buffer.c
 * @par dependencies
 * - circular_buffer.h
 * @author liu
 * @brief 简易环形缓冲区，专为定长(1024字节)DMA数据流设计
 * @version V1.0 2024-12-06
 * @note 1 tab == 4 spaces!
 *****************************************************************************/
#include "circular_buffer.h"
#include <stdlib.h>
#include "user_debug.h"
circular_buffer_t circular_buf;

/**
 * @brief 它指向了环形缓冲区中当前可以写入数据的那一包（14字节）的起始物理地址。
 */
uint8_t *get_wbuffer_addr(void *p_ctx)
{
    circular_buffer_t *buffer = (circular_buffer_t *)p_ctx; // 强转回具体类型
    return buffer->buffer + buffer->wflag * buffer->packet_size;
}

/**
 * @brief 获取当前读包的物理首地址 (供CPU读取解析)
 */
uint8_t *get_rbuffer_addr(void *p_ctx)
{
    circular_buffer_t *buffer = (circular_buffer_t *)p_ctx; // 强转回具体类型
    return buffer->buffer + buffer->rflag * buffer->packet_size;
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
    if (NULL == p_ctx)
    {
        log_e("buffer_init: p_ctx is NULL");
        while(1);
    }
    
    circular_buffer_t *buffer = (circular_buffer_t *)p_ctx;
    buffer->size = size;
    buffer->packet_size = CIRCULAR_BUFFER_PACKET_SIZE;
    buffer->rflag = 0;
    buffer->wflag = 0;

    /* 分配数据区：槽位数 * 每槽字节数 */
    buffer->buffer = (uint8_t *)malloc(size * CIRCULAR_BUFFER_PACKET_SIZE);

    /* 绑定操作函数指针 */
    buffer->pfget_rbuffer_addr = get_rbuffer_addr;
    buffer->pfget_wbuffer_addr = get_wbuffer_addr;
    buffer->pfdata_readed      = data_readed;
    buffer->pfdata_writed      = data_writed;
}
