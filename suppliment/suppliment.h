/**
 * @file      suppliment.h
 * @author:   Shubhendu B B
 * @date:     10/06/2026
 * @brief     Standalone compatibility layer
 * @details   Provides fallback implementations when the framework
 *            headers are unavailable.
 *
 * @copyright
 *
 **/

#ifndef SUPPLIMENT_H_
#define SUPPLIMENT_H_

#include <stddef.h>
#include <stdint.h>

#if defined(CONFIG_BOARD_H_)
#include "config_board.h"
#endif

/* --------------------------------------------------------------------------
 * common_base compatibility
 * -------------------------------------------------------------------------- */
#if defined(COMMON_BASE_H_)

#include "common_base.h"

#else

#ifndef common_base_MAKE_MAGIC
#define common_base_MAKE_MAGIC(a, b, c, d) \
    (((uint32_t)(a) << 24U) | ((uint32_t)(b) << 16U) | ((uint32_t)(c) << 8U) | ((uint32_t)(d)))
#endif

static inline int common_base_is_valid_handle(void* handle, void* poolStart, size_t elementCount,
                                              size_t elementSize) {
    uintptr_t h;
    uintptr_t start;
    uintptr_t end;

    if (NULL == handle) {
        return 0;
    }

    h = (uintptr_t)handle;
    start = (uintptr_t)poolStart;
    end = start + (elementCount * elementSize);

    if ((h < start) || (h >= end)) {
        return 0;
    }

    if (((h - start) % elementSize) != 0U) {
        return 0;
    }

    return 1;
}

#define common_base_ASSERT_HANDLE(handle, poolStart, elementCount, type, magicValue, okVal,       \
                                  errVal)                                                         \
    ((common_base_is_valid_handle((void*)(handle), (void*)(poolStart), (elementCount),            \
                                  sizeof(type)) &&                                                \
      (*(uint32_t*)((uint8_t*)(handle) + offsetof(type, handleMagic)) == (uint32_t)(magicValue))) \
         ? (okVal)                                                                                \
         : (errVal))

#endif /* COMMON_BASE_H_ */

/* --------------------------------------------------------------------------
 * service_system compatibility
 * -------------------------------------------------------------------------- */
#if defined(SERVICE_SYSTEM_H_)

#include "service_system.h"

#else

#ifndef service_system_ENTER_CRITICAL
#define service_system_ENTER_CRITICAL() ((void)0)
#endif

#ifndef service_system_EXIT_CRITICAL
#define service_system_EXIT_CRITICAL() ((void)0)
#endif

#endif /* SERVICE_SYSTEM_H_ */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* SUPPLIMENT_H_ */