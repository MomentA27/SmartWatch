//
// Created by 35540 on 2026/5/13.
//
/*
 * multi_cyc_fifo.c
 *
 *  Created on: 2026年4月23日
 *      Author: redmiX
 */

//******************************** Includes *********************************//
#include "multi_cyc_fifo.h"
#include "platform_os.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
/**
 * @brief FIFO状态枚举
 */
typedef enum
{
    FIFO_EMPTY, ///< FIFO为空
    FIFO_USING, ///< FIFO使用中
    FIFO_FULL   ///< FIFO已满
} tFifoStatus;

/**
 * @brief FIFO控制节点结构
 */
typedef struct
{
    tId xOwnerId;                  ///< FIFO标识符
    tLen xFifoLen;                 ///< FIFO数据区总长度
    tLen xReadAddr;                ///< 读指针（数据区偏移）
    tLen xWriteAddr;               ///< 写指针（数据区偏移）
    tFifoStatus eFifoStatus;       ///< FIFO当前状态
    uint8_t *pStartFifoAddr;       ///< FIFO数据区起始地址
    void *pvNextFifoList;          ///< 链表下一个节点指针
} tFifoInfo;

#define STRUCT_LEN (sizeof(tFifoInfo))
#define TOTAL_BYTES ((STRUCT_LEN) * (FIFO_NUM) + TOTAL_FIFO_BYTES)
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Macros ***********************************//
/**
 * @brief 增加FIFO读写指针计数（循环处理）
 * @param i_xFifoLen FIFO数据区总长度
 * @param m_pxCounter 待增加的指针（读/写指针）
 * @note 指针达到FIFO长度后自动回绕到0
 */
#define AddCounter(i_xFifoLen, m_pxCounter)\
do{                                        \
    (*(m_pxCounter))++;                    \
    if(*(m_pxCounter) >= (i_xFifoLen))     \
    {                                      \
        *(m_pxCounter) -= (i_xFifoLen);    \
    }                                      \
}while(0)

/**
 * @brief 增加FIFO写指针
 * @param m_pstNode FIFO节点指针
 */
#define AddWriteCounter(m_pstNode)                            \
do{                                                           \
    AddCounter(m_pstNode->xFifoLen, &(m_pstNode->xWriteAddr));\
}while(0)

/**
 * @brief 增加FIFO读指针
 * @param m_pstNode FIFO节点指针
 */
#define AddReadCounter(m_pstNode)                            \
do{                                                          \
    AddCounter(m_pstNode->xFifoLen, &(m_pstNode->xReadAddr));\
}while(0)

/**
 * @brief 检查并更新写操作后的FIFO状态
 * @param m_pstNode FIFO节点指针
 * @note 需关中断保护；若写指针==读指针，标记为FIFO_FULL，否则为FIFO_USING
 */
#define CheckAndChangeWriteFIFOStatus(m_pstNode)         \
do{                                                      \
    os_enter_critical();                              \
    if((m_pstNode)->xWriteAddr == (m_pstNode)->xReadAddr)\
    {                                                    \
        (m_pstNode)->eFifoStatus = FIFO_FULL;            \
    }                                                    \
    else                                                 \
    {                                                    \
        (m_pstNode)->eFifoStatus = FIFO_USING;           \
    }                                                    \
    os_exit_critical();                               \
}while(0u)

/**
 * @brief 检查并更新读操作后的FIFO状态
 * @param m_pstNode FIFO节点指针
 * @note 需关中断保护；若写指针==读指针，标记为FIFO_EMPTY，否则为FIFO_USING
 */
#define CheckAndChangeReadFIFOStatus(m_pstNode)          \
do{                                                      \
    os_enter_critical();                              \
    if((m_pstNode)->xWriteAddr == (m_pstNode)->xReadAddr)\
    {                                                    \
        (m_pstNode)->eFifoStatus = FIFO_EMPTY;           \
    }                                                    \
    else                                                 \
    {                                                    \
        (m_pstNode)->eFifoStatus = FIFO_USING;           \
    }                                                    \
    os_exit_critical();                               \
}while(0u)

/**
 * @brief 获取FIFO管理链表头
 * @param o_psListHeader 链表头指针
 */
#define GetListHeader(o_psListHeader)   \
do{                                     \
    (o_psListHeader) = gs_pstListHeader;\
}while(0)
//******************************** Macros ***********************************//
//---------------------------------------------------------------------------//
//******************************** 函数声明   *********************************//
/**
 * @brief 将FIFO节点加入管理链表尾部
 * @param i_pstFifoNode 待加入的FIFO节点指针
 * @param m_ppstHeader 链表头指针的指针（会被修改）
 * @param o_peAddStatus 操作状态码：成功返回ERRO_NONE，失败返回对应错误码
 */
static void AddInList(tFifoInfo *i_pstFifoNode,
                      tFifoInfo **m_ppstHeader,
                      tErrorCode *o_peAddStatus);
                      /**
 * @brief 在FIFO管理链表中按ID查找节点
 * @param i_xFifoId 待查找的FIFO标识符
 * @param o_ppstNode 输出查找到的节点指针（失败时值未定义）
 * @param o_peFindStatus 查找状态：成功返回ERRO_NONE，未找到返回ERRO_NO_NODE
 */
static void FindFifo(tId i_xFifoId,
                     tFifoInfo **o_ppstNode,
                     tErrorCode *o_peFindStatus);
//******************************** 函数声明   *********************************//
//---------------------------------------------------------------------------//
//******************************** Variables ********************************//
static uint8_t gs_ucFifo[TOTAL_BYTES] = {0};
static tFifoInfo *gs_pstListHeader = (tFifoInfo *)0u;
static tLen gs_xCleanFifoLen = TOTAL_BYTES;
//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//
/**
 * @brief 申请一个FIFO
 * @param i_xApplyFifoLen 需要申请的FIFO数据区长度
 * @param i_xFifoId FIFO标识符
 * @param o_peApplyStatus 申请状态
 * @return 无
 */
void ApplyFifo(tLen i_xApplyFifoLen, tLen i_xFifoId, tErrorCode *o_peApplyStatus)
{
    tFifoInfo *pstNode = (tFifoInfo *)0u;
    tLen xNodeNeedSpace = 0;
    uint32_t cleanFifoLenTmp = 0;

    //1. 参数安全检查（SAFE_LEVEL_O3级别）
#ifdef SAFE_LEVEL_O3
    //1.1 检查状态输出指针非空
    if(NULL == o_peApplyStatus)
    {
        return;
    }

    //1.2 计算4字节对齐所需的填充空间（确保控制结构地址对齐）
    cleanFifoLenTmp = (uint32_t)(&gs_ucFifo[TOTAL_BYTES - gs_xCleanFifoLen]) & 0x03u;
    cleanFifoLenTmp = (4 - cleanFifoLenTmp) & 0x03u;

    //1.3 检查剩余空间是否足够（控制结构+数据区+对齐填充）
    if((i_xApplyFifoLen + STRUCT_LEN + cleanFifoLenTmp) > gs_xCleanFifoLen)
    {
        *o_peApplyStatus = ERRO_OVER_MAX;
        return;
    }
#endif

    //2. 检查FIFO ID是否已注册（若FindFifo返回ERRO_NONE说明ID已存在）
    FindFifo(i_xFifoId, &pstNode, o_peApplyStatus);
    if(ERRO_NONE == *o_peApplyStatus)
    {
        *o_peApplyStatus = ERRO_REGISTERED_SECOND;
        return;
    }

    //3. 处理4字节对齐：扣除填充空间
    if(cleanFifoLenTmp)
    {
        gs_xCleanFifoLen -= cleanFifoLenTmp;
    }

    //4. 初始化FIFO控制器
    pstNode = (tFifoInfo*)(&gs_ucFifo[TOTAL_BYTES - gs_xCleanFifoLen]);
    pstNode->xOwnerId = i_xFifoId;
    pstNode->xFifoLen = i_xApplyFifoLen;
    pstNode->xReadAddr = 0;
    pstNode->xWriteAddr = 0;
    pstNode->pvNextFifoList = NULL;
    pstNode->pStartFifoAddr = (uint8_t*)((tFifoInfo*)(&gs_ucFifo[TOTAL_BYTES - gs_xCleanFifoLen])+1);
    pstNode->eFifoStatus = FIFO_EMPTY;

    //5. 计算本次分配的总空间（控制结构大小 + 数据区长度）
    xNodeNeedSpace = (tLen)((uint8_t*)((tFifoInfo *)(&gs_ucFifo[TOTAL_BYTES - gs_xCleanFifoLen])+1) -
                       (uint8_t*)(&gs_ucFifo[TOTAL_BYTES - gs_xCleanFifoLen]));
    xNodeNeedSpace += i_xApplyFifoLen;
    gs_xCleanFifoLen -= xNodeNeedSpace;

    //6. 将FIFO节点加入管理链表
    AddInList(pstNode, &gs_pstListHeader, o_peApplyStatus);
}

/**
 * @brief 向FIFO写入数据
 * @param i_xFifoId FIFO标识符
 * @param i_pucWriteDataBuf 待写入数据缓冲区
 * @param i_xWriteDatalen 待写入数据长度
 * @param o_peWriteStatus 写入状态
 * @return 无
 */
void WriteDataInFifo(tId i_xFifoId,
                     uint8_t *i_pucWriteDataBuf,
                     tLen i_xWriteDatalen,
                     tErrorCode *o_peWriteStatus)
{
    tFifoInfo *pstNode = (tFifoInfo *)0u;
    tLen xIndex = 0;
    tLen xCanWriteTotal = 0;

    //1. 参数安全检查
#ifdef SAFE_LEVEL_O3
    //1.1 检查状态输出指针非空
    if(NULL == o_peWriteStatus)
    {
        return;
    }

    //1.2 检查写入数据缓冲区非空
    if(NULL == i_pucWriteDataBuf ||
       0    == i_xWriteDatalen)
    {
        *o_peWriteStatus = ERRO_POINTER_NULL;
        return;
    }
#endif

    //2. 获取FIFO可写入空间
    GetCanWriteLen(i_xFifoId, &xCanWriteTotal, o_peWriteStatus);
    if(ERRO_NONE != *o_peWriteStatus)
    {
        return;
    }

    //3. 检查写入长度是否超过可写空间
    if(i_xWriteDatalen > xCanWriteTotal)
    {
        *o_peWriteStatus = ERRO_OVER_MAX;
        return;
    }

    //4. 查找FIFO节点
    FindFifo(i_xFifoId, &pstNode, o_peWriteStatus);
    if(ERRO_NONE != *o_peWriteStatus)
    {
        return;
    }

    //5. 逐字节写入数据并更新写指针
    for(xIndex = 0; xIndex < i_xWriteDatalen; xIndex++)
    {
        (pstNode->pStartFifoAddr)[pstNode->xWriteAddr] = i_pucWriteDataBuf[xIndex];
        AddWriteCounter(pstNode);
    }

    //6. 检查并更新FIFO状态
    CheckAndChangeWriteFIFOStatus(pstNode);

    *o_peWriteStatus = ERRO_NONE;
}

/**
 * @brief 从FIFO读取数据
 * @param i_xFifoId FIFO标识符
 * @param i_xNeedReadDataLen 期望读取长度
 * @param o_pucReadDataBuf 读取数据缓冲区
 * @param o_pxReadLen 实际读取长度
 * @param o_peReadStatus 读取状态
 * @return 无
 */
void ReadDataFromFifo(tId i_xFifoId,
                      tLen i_xNeedReadDataLen,
                      uint8_t *o_pucReadDataBuf,
                      tLen *o_pxReadLen,
                      tErrorCode *o_peReadStatus)
{
    tFifoInfo *pstNode = (tFifoInfo *)0u;
    tLen xIndex = 0u;
    tLen xCanReadTotal = 0u;

    //1. 参数安全检查
#ifdef SAFE_LEVEL_O3
    //1.1 检查状态输出指针非空
    if(NULL == o_peReadStatus)
    {
        return;
    }

    //1.2 检查读取缓冲区、实际长度指针非空，期望长度非0
    if(NULL == o_pucReadDataBuf ||
       NULL == o_pxReadLen      ||
       0    == i_xNeedReadDataLen)
    {
        *o_peReadStatus = ERRO_POINTER_NULL;
        return;
    }
#endif

    //2. 获取FIFO可读取数据长度
    GetCanReadLen(i_xFifoId, &xCanReadTotal, o_peReadStatus);
    if(ERRO_NONE != *o_peReadStatus)
    {
        return;
    }

    //3. 查找FIFO节点
    FindFifo(i_xFifoId, &pstNode, o_peReadStatus);
    if(ERRO_NONE != *o_peReadStatus)
    {
        return;
    }

    //4. 确定实际读取长度（取期望长度和可用长度的较小值）
    xCanReadTotal = xCanReadTotal > i_xNeedReadDataLen ?
        i_xNeedReadDataLen : xCanReadTotal;
    *o_pxReadLen = xCanReadTotal;

    //5. 逐字节读取数据并更新读指针
    for(xIndex = 0u; xIndex < xCanReadTotal; xIndex++)
    {
        o_pucReadDataBuf[xIndex] = (pstNode->pStartFifoAddr)[pstNode->xReadAddr];
        AddReadCounter(pstNode);
    }

    //6. 检查并更新FIFO状态
    CheckAndChangeReadFIFOStatus(pstNode);

    *o_peReadStatus = ERRO_NONE;
}

/**
 * @brief 获取FIFO可读取数据长度
 * @param i_xFifoId FIFO标识符
 * @param o_pxCanReadLen 可读取长度
 * @param o_peGetStatus 获取状态
 * @return 无
 */
void GetCanReadLen(tId i_xFifoId,
                   tLen *o_pxCanReadLen,
                   tErrorCode *o_peGetStatus)
{
    tFifoInfo *pstNode = (tFifoInfo *)0u;

    //1. 参数安全检查
#ifdef SAFE_LEVEL_O3
    //1.1 检查状态输出指针非空
    if(NULL == o_peGetStatus)
    {
        return;
    }

    //1.2 检查可读取长度输出指针非空
    if(NULL == o_pxCanReadLen)
    {
        *o_peGetStatus = ERRO_POINTER_NULL;
        return;
    }
#endif

    //2. 查找FIFO节点
    FindFifo(i_xFifoId, &pstNode, o_peGetStatus);
    if(ERRO_NONE != *o_peGetStatus)
    {
        return;
    }

    //3. 根据FIFO状态计算可读取长度
    if(FIFO_USING == pstNode->eFifoStatus)
    {
        // 读指针 > 写指针：数据跨边界，长度 = 总长度 + (写指针 - 读指针)
        // 读指针 <= 写指针：长度 = 写指针 - 读指针
        *o_pxCanReadLen = (pstNode->xReadAddr > pstNode->xWriteAddr) ?
                          (pstNode->xFifoLen - pstNode->xReadAddr + pstNode->xWriteAddr) :
                          (pstNode->xWriteAddr - pstNode->xReadAddr);
    }
    else if(FIFO_FULL == pstNode->eFifoStatus)
    {
        *o_pxCanReadLen = pstNode->xFifoLen;
    }
    else
    {
        *o_pxCanReadLen = (tLen)0u;
    }

    *o_peGetStatus = ERRO_NONE;
}

/**
 * @brief 获取FIFO可写入空间长度
 * @param i_xFifoId FIFO标识符
 * @param o_pxCanWriteLen 可写入长度
 * @param o_peGetStatus 获取状态
 * @return 无
 */
void GetCanWriteLen(tId i_xFifoId,
                    tLen *o_pxCanWriteLen,
                    tErrorCode *o_peGetStatus)
{
    tFifoInfo *pstNode = (tFifoInfo *)0u;

    //1. 参数安全检查
#ifdef SAFE_LEVEL_O3
    //1.1 检查状态输出指针非空
    if(NULL == o_peGetStatus)
    {
        return;
    }

    //1.2 检查可写入长度输出指针非空
    if(NULL == o_pxCanWriteLen)
    {
        *o_peGetStatus = ERRO_POINTER_NULL;
        return;
    }
#endif

    //2. 查找FIFO节点
    FindFifo(i_xFifoId, &pstNode, o_peGetStatus);
    if(ERRO_NONE != *o_peGetStatus)
    {
        return;
    }

    //3. 根据FIFO状态计算可写入长度
    if(FIFO_USING == pstNode->eFifoStatus)
    {
        // 读指针 > 写指针：可写长度 = 读指针 - 写指针
        // 读指针 <= 写指针：可写长度 = 总长度 + (读指针 - 写指针)
        *o_pxCanWriteLen = (pstNode->xReadAddr > pstNode->xWriteAddr) ?
                          (pstNode->xReadAddr - pstNode->xWriteAddr) :
                          (pstNode->xFifoLen + pstNode->xReadAddr - pstNode->xWriteAddr);
    }
    else if(FIFO_EMPTY == pstNode->eFifoStatus)
    {
        *o_pxCanWriteLen = pstNode->xFifoLen;
    }
    else
    {
        *o_pxCanWriteLen = (tLen)0u;
    }

    *o_peGetStatus = ERRO_NONE;
}

/**
 * @brief 清空FIFO
 * @param i_xFifoId FIFO标识符
 * @param o_peGetStatus 清空状态
 * @return 无
 */
void ClearFIFO(tId i_xFifoId, tErrorCode *o_peGetStatus)
{
    tFifoInfo *pstNode = (tFifoInfo *)0u;

    //1. 参数安全检查
#ifdef SAFE_LEVEL_O3
    if(NULL == o_peGetStatus)
    {
        return;
    }
#endif

    //2. 查找FIFO节点
    FindFifo(i_xFifoId, &pstNode, o_peGetStatus);
    if(ERRO_NONE != *o_peGetStatus)
    {
        return;
    }

    //3. 关中断重置读写指针和状态
    os_enter_critical();
    pstNode->eFifoStatus = FIFO_EMPTY;
    pstNode->xReadAddr = pstNode->xWriteAddr;
    os_exit_critical();

    *o_peGetStatus = ERRO_NONE;
}

/**
 * @brief 将FIFO节点加入管理链表
 * @param i_pstFifoNode 待加入的FIFO节点
 * @param m_ppstHeader 链表头指针的指针
 * @param o_peAddStatus 加入状态
 * @return 无
 */
static void AddInList(tFifoInfo *i_pstFifoNode,
                      tFifoInfo **m_ppstHeader,
                      tErrorCode *o_peAddStatus)
{
    tFifoInfo *pstTemp = (tFifoInfo *)0u;

    //1. 参数安全检查
#ifdef SAFE_LEVEL_O3
    //1.1 检查状态输出指针非空
    if(NULL == o_peAddStatus)
    {
        return;
    }

    //1.2 检查链表头指针和待加入节点非空
    if(NULL == m_ppstHeader ||
       NULL == i_pstFifoNode)
    {
        *o_peAddStatus = ERRO_POINTER_NULL;
        return;
    }
#endif

    //2. 若链表为空，直接将节点设为头
    if((tFifoInfo *)0u == *m_ppstHeader)
    {
        *m_ppstHeader = i_pstFifoNode;
        *o_peAddStatus = ERRO_NONE;
        return;
    }

    //3. 遍历链表到尾部
    pstTemp = *m_ppstHeader;
    while((void *)0u != pstTemp->pvNextFifoList)
    {
        //3.1 检查节点是否已在链表中
        if(i_pstFifoNode == pstTemp)
        {
            *o_peAddStatus = ERRO_REGISTERED_SECOND;
            return;
        }
        pstTemp = (tFifoInfo *)(pstTemp->pvNextFifoList);
    }

    //4. 将节点加入链表尾部
    pstTemp->pvNextFifoList = (void *)i_pstFifoNode;
    i_pstFifoNode->pvNextFifoList = (tErrorCode *)0u;

    *o_peAddStatus = ERRO_NONE;
}

/**
 * @brief 在FIFO链表中查找指定ID的节点
 * @param i_xFifoId 待查找的FIFO标识符
 * @param o_ppstNode 查找到的FIFO节点指针
 * @param o_peFindStatus 查找状态
 * @return 无
 */
static void FindFifo(tId i_xFifoId,
                     tFifoInfo **o_ppstNode,
                     tErrorCode *o_peFindStatus)
{
    tFifoInfo *pstNode = (tFifoInfo *)0u;

    //1. 参数安全检查
#ifdef SAFE_LEVEL_O3
    //1.1 检查状态输出指针非空
    if(NULL == o_peFindStatus)
    {
        return;
    }

    //1.2 检查节点输出指针非空
    if(NULL == o_ppstNode)
    {
        *o_peFindStatus = ERRO_POINTER_NULL;
        return;
    }
#endif

    //2. 遍历链表查找匹配ID的节点
    GetListHeader(pstNode);
    while((tFifoInfo *)0u != pstNode)
    {
        if(i_xFifoId == pstNode->xOwnerId)
        {
            *o_peFindStatus = ERRO_NONE;
            *o_ppstNode = pstNode;
            return;
        }
        pstNode = (tFifoInfo *)pstNode->pvNextFifoList;
    }

    //3. 未找到节点
    *o_peFindStatus = ERRO_NO_NODE;
}