/*
 * Copyright (C) 2024 EternalChip, Inc. or its affiliates.
 *
 * All Rights Reserved.
 *
 * File name: service_watchdog_monitor.c
 *
 * Description: Watchdog monitor implementation.
 *
 * Processing flow:
 *
 *  1. Initialize watchdog.
 *  2. Register tasks.
 *  3. Monitor task health and feed hardware watchdog.
 *
 * Version: V1.0
 *
 * Modifications:
 *
 * Note: 1 tab == 4 spaces!
 *
 */

//******************************** Includes *********************************//
#include "watchdog_monitor.h"

#include <string.h>

#include "iwdg.h"
#include "rtc.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
#define MAX_WATCHDOG_TASKS 8         /**< 最大监控任务数 */
#define WATCHDOG_TIMEOUT_MS 10000    /**< 默认任务超时时间(ms)，超时未喂狗触发复位 */
#define BOOT_FAIL_THRESHOLD 3        /**< 启动失败阈值，连续N次看门狗复位后进入安全模式 */
#define BACKUP_REGISTER_INDEX RTC_BKP_DR0 /**< 备份寄存器索引，用于跨复位保存启动失败计数 */

typedef struct {
    os_task_handler_t task_handle;          //被监控任务的OS句柄
    uint32_t          last_feed_time;       //最近一次喂狗时间戳(tick ms)
    uint32_t          timeout_ms;           //该任务的超时时间(ms)
    char              task_name[16];        //任务名称，日志输出时用于定位
    bool              is_active;            //该条目是否已被占用
} WatchdogEntry_t;

/**
 * @brief 日志tag
 * @note  用于日志查找模块
 */
#ifdef  LOG_TAG
#undef  LOG_TAG
#define LOG_TAG       "WATCHDOG_MONITOR"
#else // else of LOG_TAG
#define LOG_TAG       "WATCHDOG_MONITOR"
#endif // end of LOG_TAG
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Variables *********************************//
static WatchdogEntry_t g_watchdog_entries[MAX_WATCHDOG_TASKS];  //监控任务列表
static os_mutex_handler_t g_watchdog_mutex;                     //互斥锁，保护并发访问
static volatile bool g_watchdog_paused = false;                 //暂停标志
static uint32_t s_boot_fail_count = 0;                          //启动失败计数，跨复位保持
static bool s_safe_mode = false;                                //安全模式标志
//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Macros ***********************************//
#if defined(WATCHDOG_MONITOR_DEBUG) && defined(MYDEBUG)
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
//******************************** 函数声明   *********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//
/**
 * @brief  初始化看门狗监控器
 * @note   创建互斥锁并初始化任务监控列表，同时读取备份寄存器中的启动失败计数；
 *         判断上次复位是否由独立看门狗(IWDG)触发，如果是则累加失败计数并写回备份寄存器；
 *         当连续失败次数达到阈值 BOOT_FAIL_THRESHOLD 时，系统进入安全模式。
 */
void watchdog_init(void) {
    if (os_mutex_create(&g_watchdog_mutex) != SUCCESS) {
        LOG_DEBUG("Watchdog mutex create failed!\n");
    }
    for (int i = 0; i < MAX_WATCHDOG_TASKS; i++) {
        g_watchdog_entries[i].is_active = false;
    }

    // 从备份寄存器读取启动失败计数，备份寄存器由 VBAT 供电，系统复位后数据不丢失
    s_boot_fail_count = HAL_RTCEx_BKUPRead(&hrtc, BACKUP_REGISTER_INDEX);

    // 通过 RCC 控制状态寄存器(CSR)的 IWDGRSTF 标志位判断上次复位是否由 IWDG 触发
    uint32_t reset_cause = RCC->CSR;
    if (reset_cause & RCC_CSR_IWDGRSTF) {
        s_boot_fail_count++;
        // 将更新后的失败计数写回备份寄存器，以便下次重启后仍可读取
        HAL_RTCEx_BKUPWrite(&hrtc, BACKUP_REGISTER_INDEX, s_boot_fail_count);
        LOG_DEBUG("System reset by Watchdog! Fail count: %d\n", s_boot_fail_count);
    }
    // 清除 RCC 复位标志，避免影响后续启动时的复位原因判断
    __HAL_RCC_CLEAR_RESET_FLAGS();

    // 连续多次看门狗复位后进入安全模式，停止执行可能引发崩溃的常规任务
    if (s_boot_fail_count >= BOOT_FAIL_THRESHOLD) {
        s_safe_mode = true;
        LOG_DEBUG("Entering SAFE MODE due to repeated crashes.\n");
    }
}

/**
 * @brief  注册一个任务到看门狗监控列表
 * @param[in] handle     任务句柄，用于唯一标识被监控任务
 * @param[in] timeout_ms 任务超时时间（毫秒），超时未喂狗将触发系统复位
 * @param[in] name       任务名称，用于日志输出便于定位问题
 */
void watchdog_register(os_task_handler_t handle, uint32_t timeout_ms, const char* name) {
    os_mutex_take(g_watchdog_mutex, OS_MAX_DELAY);
    for (int i = 0; i < MAX_WATCHDOG_TASKS; i++) {
        if (!g_watchdog_entries[i].is_active) {
            g_watchdog_entries[i].task_handle = handle;
            g_watchdog_entries[i].timeout_ms = timeout_ms;
            g_watchdog_entries[i].last_feed_time = os_get_tick_ms();
            strncpy(g_watchdog_entries[i].task_name, name, sizeof(g_watchdog_entries[i].task_name)-1);
            g_watchdog_entries[i].is_active = true;
            break;
        }
    }
    os_mutex_give(g_watchdog_mutex);
}

/**
 * @brief  从监控列表中注销指定任务
 * @param[in] handle 要注销的任务句柄
 */
void watchdog_unregister(os_task_handler_t handle) {
    os_mutex_take(g_watchdog_mutex, OS_MAX_DELAY);
    for (int i = 0; i < MAX_WATCHDOG_TASKS; i++) {
        if (g_watchdog_entries[i].is_active && g_watchdog_entries[i].task_handle == handle) {
            g_watchdog_entries[i].is_active = false;
            break;
        }
    }
    os_mutex_give(g_watchdog_mutex);
}

/**
 * @brief  被监控任务调用此函数喂狗，更新其最后活跃时间
 * @note   若看门狗已暂停，则不执行任何操作；正常状态下更新对应任务的 last_feed_time
 * @param[in] handle 喂狗的任务句柄
 */
void watchdog_feed(os_task_handler_t handle) {
    if (g_watchdog_paused) return;

    os_mutex_take(g_watchdog_mutex, OS_MAX_DELAY);
    for (int i = 0; i < MAX_WATCHDOG_TASKS; i++) {
        if (g_watchdog_entries[i].is_active && g_watchdog_entries[i].task_handle == handle) {
            g_watchdog_entries[i].last_feed_time = os_get_tick_ms();
            break;
        }
    }
    os_mutex_give(g_watchdog_mutex);
}

/**
 * @brief  暂停看门狗任务健康检查
 * @note   暂停期间 monitor 任务仍会持续刷新硬件 IWDG，但不再检测各任务的喂狗超时，
 *         适用于 OTA、低功耗等需要停止部分任务但不想触发复位的场景
 */
void watchdog_pause(void) {
    g_watchdog_paused = true;
}

/**
 * @brief  恢复看门狗任务健康检查
 * @note   将所有已注册任务的 last_feed_time 重置为当前时间，避免恢复后立即因
 *         暂停期间未喂狗而误判超时
 */
void watchdog_resume(void) {
    g_watchdog_paused = false;
    uint32_t now = os_get_tick_ms();
    os_mutex_take(g_watchdog_mutex, OS_MAX_DELAY);
    for (int i = 0; i < MAX_WATCHDOG_TASKS; i++) {
        if (g_watchdog_entries[i].is_active) {
            g_watchdog_entries[i].last_feed_time = now;
        }
    }
    os_mutex_give(g_watchdog_mutex);
}

/**
 * @brief  查询看门狗是否处于暂停状态
 * @return true 已暂停，false 未暂停
 */
bool watchdog_is_paused(void) {
    return g_watchdog_paused;
}

/**
 * @brief  查询系统是否处于安全模式
 * @note   安全模式下仅运行最小功能集，避免反复崩溃
 * @return true 安全模式，false 正常模式
 */
bool watchdog_is_safe_mode(void) {
    return s_safe_mode;
}

/**
 * @brief  清除启动失败计数
 * @note   系统稳定运行一段时间后调用，将备份寄存器中的失败计数归零；
 *         下次即使发生看门狗复位也会从零开始计数，避免历史累积误判
 */
void watchdog_clear_boot_fail_count(void) {
    if (s_boot_fail_count > 0) {
        s_boot_fail_count = 0;
        HAL_RTCEx_BKUPWrite(&hrtc, BACKUP_REGISTER_INDEX, 0);
        LOG_DEBUG("System stable, boot fail count cleared.\n");
    }
}

/**
 * @brief  看门狗监控主任务
 * @note   以 100ms 为周期轮询所有注册任务：
 *          - 正常模式下检查每个任务的喂狗超时情况，全部健康则刷新硬件 IWDG，
 *            任一任务超时则通过 NVIC 触发系统复位；
 *          - 暂停模式下仅刷新硬件 IWDG，不做任务健康检查。
 * @param[in] argument 任务参数（未使用）
 */
void server_watchdog_task(void *argument) {
    watchdog_init();

    while (1) {
        if (!g_watchdog_paused) {
            uint32_t now = os_get_tick_ms();
            bool system_healthy = true;

            // 遍历所有已注册任务，检查是否超时未喂狗
            os_mutex_take(g_watchdog_mutex, OS_MAX_DELAY);
            for (int i = 0; i < MAX_WATCHDOG_TASKS; i++) {
                if (g_watchdog_entries[i].is_active) {
                    if (now - g_watchdog_entries[i].last_feed_time > g_watchdog_entries[i].timeout_ms) {
                        system_healthy = false;
                        LOG_ERROR("Watchdog Bite! Task '%s' stuck for %d ms.\n",
                                  g_watchdog_entries[i].task_name,
                                  now - g_watchdog_entries[i].last_feed_time);
                        break;
                    }
                }
            }
            os_mutex_give(g_watchdog_mutex);

            if (system_healthy) {
                // 所有任务健康，刷新独立看门狗计数器，防止硬件 IWDG 复位
                HAL_IWDG_Refresh(&hiwdg);
            } else {
                // 有任务超时，等待 100ms 让日志输出完成后，通过 NVIC 触发软件系统复位
                LOG_DEBUG("System resetting...\n");
                os_task_delay(100);
                NVIC_SystemReset();
            }
        } else {
            // 暂停模式下仅维持硬件看门狗喂狗，不做任务健康检查
            HAL_IWDG_Refresh(&hiwdg);
        }

        os_task_delay(100);
    }
}
//******************************** Function defi4apowTwnitions ********************************//

