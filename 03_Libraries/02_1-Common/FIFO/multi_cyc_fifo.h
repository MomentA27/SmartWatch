//
// Created by 35540 on 2026/5/13.
//

#ifndef SMARTWATCH_STM32F411_MULTI_CYC_FIFO_H
#define SMARTWATCH_STM32F411_MULTI_CYC_FIFO_H
//******************************** Includes *********************************//
#include <stdint.h>
#include <stddef.h>
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
/** @brief 默认开启参数安全检查 */
#define SAFE_LEVEL_O3

///< 支持的FIFO最大数量
#define FIFO_NUM (4)

#ifdef EN_LIN_TP
#define TOTAL_FIFO_BYTES (450u) ///< LIN TP场景下总缓冲区大小
#elif defined EN_CAN_TP
#define TOTAL_FIFO_BYTES (800u) ///< CAN TP场景下总缓冲区大小
#else
#define TOTAL_FIFO_BYTES (100u) ///< 默认场景下总缓冲区大小
#endif
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Typedefs *********************************//
/**
 * @brief 错误码枚举
 */
typedef enum {
    ERRO_NONE = 0u,         ///< 无错误
    ERRO_LEES_MIN,          ///< 小于最小值
    ERRO_NO_NODE,           ///< 未找到节点
    ERRO_OVER_MAX,          ///< 超过最大值
    ERRO_POINTER_NULL,      ///< 指针为空
    ERRO_REGISTERED_SECOND, ///< 定时器重复注册
    ERRO_TIME_TYPE_ERRO,    ///< 时间类型错误
    ERRO_TIME_USEING,       ///< 时间正在使用
    ERRO_TIMEOUT,           ///< 超时
    ERRO_WRITE_ERRO,        ///< 写错误
    ERRO_READ_ERRO          ///< 读错误
} tErrorCode;

typedef unsigned short tId;  ///< FIFO标识符类型
typedef unsigned short tLen; ///< FIFO长度类型
//******************************** Typedefs *********************************//
//---------------------------------------------------------------------------//
//**************************** Interface Structs ****************************//
//**************************** Interface Structs ****************************//
//---------------------------------------------------------------------------//
//******************************** Classes **********************************//
//******************************** Classes **********************************//
//---------------------------------------------------------------------------//
//**************************** extern  Variables ****************************//
//**************************** extern  Variables ****************************//
//---------------------------------------------------------------------------//
//******************************** 函数声明 ***********************************//
/**
 * @brief 申请一个FIFO
 * @param i_xApplyFifoLen 需要申请的FIFO数据区长度
 * @param i_xFifoId FIFO标识符（用于后续查找）
 * @param o_peApplyStatus 申请状态：成功返回ERRO_NONE，失败返回对应错误码
 * @return 无
 * @note 从全局缓冲区分配空间，包含控制结构（tFifoInfo）和数据区
 */
extern void ApplyFifo(tLen i_xApplyFifoLen,
                      tLen i_xFifoId,
                      tErrorCode *o_peApplyStatus);
/**
 * @brief 向FIFO写入数据
 * @param i_xFifoId FIFO标识符
 * @param i_pucWriteDataBuf 待写入数据的缓冲区指针
 * @param i_xWriteDatalen 待写入数据的长度
 * @param o_peWriteStatus 写入状态：成功返回ERRO_NONE，失败返回对应错误码
 * @return 无
 */
extern void WriteDataInFifo(tId i_xFifoId,
                            uint8_t *i_pucWriteDataBuf,
                            tLen i_xWriteDatalen,
                            tErrorCode *o_peWriteStatus);
/**
 * @brief 从FIFO读取数据
 * @param i_xFifoId FIFO标识符
 * @param i_xNeedReadDataLen 期望读取的数据长度
 * @param o_pucReadDataBuf 存储读取数据的缓冲区指针
 * @param o_pxReadLen 实际读取的数据长度
 * @param o_peReadStatus 读取状态：成功返回ERRO_NONE，失败返回对应错误码
 * @return 无
 * @note 若FIFO中数据不足期望长度，则读取全部可用数据
 */
extern void ReadDataFromFifo(tId i_xFifoId,
                             tLen i_xNeedReadDataLen,
                             uint8_t *o_pucReadDataBuf,
                             tLen *o_pxReadLen,
                             tErrorCode *o_peReadStatus);
/**
 * @brief 获取FIFO中可读取的数据长度
 * @param i_xFifoId FIFO标识符
 * @param o_pxCanReadLen 可读取的数据长度
 * @param o_peGetStatus 获取状态：成功返回ERRO_NONE，失败返回对应错误码
 * @return 无
 */
extern void GetCanReadLen(tId i_xFifoId,
                          tLen *o_pxCanReadLen,
                          tErrorCode *o_peGetStatus);
/**
 * @brief 获取FIFO中可写入的空间长度
 * @param i_xFifoId FIFO标识符
 * @param o_pxCanWriteLen 可写入的空间长度
 * @param o_peGetStatus 获取状态：成功返回ERRO_NONE，失败返回对应错误码
 * @return 无
 */
extern void GetCanWriteLen(tId i_xFifoId,
                           tLen *o_pxCanWriteLen,
                           tErrorCode *o_peGetStatus);
/**
 * @brief 清空FIFO（重置读写指针，状态设为空）
 * @param i_xFifoId FIFO标识符
 * @param o_peGetStatus 清空状态：成功返回ERRO_NONE，失败返回对应错误码
 * @return 无
 * @note 仅重置指针，不擦除实际数据
 */
extern void ClearFIFO(tId i_xFifoId, tErrorCode *o_peGetStatus);
//******************************** 函数声明 ***********************************//

#endif //SMARTWATCH_STM32F411_MULTI_CYC_FIFO_H