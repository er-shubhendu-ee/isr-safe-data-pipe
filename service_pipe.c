/**
 * @file      service_pipe.c
 * @author:   Shubhendu B B
 * @date:     12/03/2026
 * @brief
 * @details
 *
 * @copyright
 *
 **/

#if defined(SERVICE_PIPE_STANDALONE)
#include "suppliment.h"
#else
#include "service_base.h"
// #include "config_board.h"
#include "service_system.h"
#endif

//
#include "service_pipe.h"

//
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Unique identifier for PIPE service handles.
 *
 * Used to verify that a handle belongs to this module and is not corrupted.
 *
 * Note:
 *   - Must match the 'handleMagic' field in control block
 *   - Chosen to be human-readable in hex dumps ("PIPE")
 */
#define service_pipe_HANDLE_MAGIC service_base_MAKE_MAGIC('P', 'I', 'P', 'E')

typedef enum service_pipe_tagState {
    service_pipe_STATE_IDLE = 0,
    service_pipe_STATE_BUSY_READ,
    service_pipe_STATE_BUSY_WRITE
} service_pipe_State_t;

typedef struct service_pipe_tagChannel {
    service_pipe_State_t state;
    uint32_t sequence;
    service_pipe_Message_t message;
} service_pipe_Channel_t;

/* Control block for each PIPE instance.
 *
 * Requirements (for service_base_ASSERT_HANDLE):
 *   - Must contain: uint32_t handleMagic;
 *   - Field name must be exactly 'handleMagic'
 *
 * Notes:
 *   - handleMagic is used for identity validation
 *   - Remaining fields are not validated structurally
 */
typedef struct service_pipe_tagPipeControlBlock {
    service_pipe_Id_t id;
    uint32_t handleMagic;
    uint32_t writeSequence;
    uint32_t nextWriteChannel;
    service_pipe_Channel_t channel[service_pipe_CHANNEL_COUNT_MAX];
} service_pipe_ControlBlock_t;

/* Service-level control block.
 *
 * Maintains:
 *   - pointer to statically allocated pool
 *   - number of instances allocated so far
 *
 * Design:
 *   - No dynamic allocation
 *   - Linear allocation only (no reuse in current implementation)
 */
typedef struct service_pipe_tagServiceControlBlock {
    service_pipe_ControlBlock_t* pPoolPipeControlBlock;
    int instanceUsed;
} service_pipe_ServiceControlBlock_t;

/* Static pool of PIPE control blocks.
 *
 * Assumptions:
 *   - Memory is contiguous
 *   - Properly aligned for service_pipe_ControlBlock_t
 *
 * Used by service_base_is_valid_handle() for range + alignment validation
 */
static service_pipe_ControlBlock_t gPoolPipeControlBlock[service_pipe_INSTANCE_COUNT_MAX];
static service_pipe_ServiceControlBlock_t gServiceControlBlock;

static int32_t acquire_pump_in_channel(service_pipe_ControlBlock_t* pControlBlock,
                                       uint32_t* pChannelId);

static int32_t acquire_pump_out_channel(service_pipe_ControlBlock_t* pControlBlock,
                                        uint32_t* pChannelId);

static int32_t release_channel(service_pipe_ControlBlock_t* pControlBlock, uint32_t channelId);

static int32_t pump_in(service_pipe_ControlBlock_t* pControlBlock,
                       service_pipe_Message_t* pMessage);

static int32_t pump_out(service_pipe_ControlBlock_t* pControlBlock,
                        service_pipe_Message_t* pMessage);

int32_t service_pipe_init(void) {
    memset(gPoolPipeControlBlock, 0, sizeof(gPoolPipeControlBlock));
    memset(&gServiceControlBlock, 0, sizeof(gServiceControlBlock));

    gServiceControlBlock.pPoolPipeControlBlock = gPoolPipeControlBlock;

    return service_pipe_STATUS_NO_ERROR;
}

service_pipe_Handle_t service_pipe_get_handle(service_pipe_Id_t id) {
    if (!gServiceControlBlock.pPoolPipeControlBlock) {
        return NULL;
    }

    service_pipe_ControlBlock_t* hTemp = NULL;
    if (service_pipe_INSTANCE_COUNT_MAX <= gServiceControlBlock.instanceUsed) {
        return NULL;
    }

    hTemp = (service_pipe_ControlBlock_t*)gServiceControlBlock.pPoolPipeControlBlock +
            (gServiceControlBlock.instanceUsed++);

    memset(hTemp, 0, sizeof(service_pipe_ControlBlock_t));
    hTemp->handleMagic = service_pipe_HANDLE_MAGIC;
    hTemp->id = id;

    return (service_pipe_Handle_t)hTemp;
}

int32_t service_pipe_access(service_pipe_Handle_t hPipe, service_pipe_OpType_t opType,
                            service_pipe_Message_t* pMessage) {
    service_pipe_ControlBlock_t* pControlBlock;

    if ((service_pipe_OP_TYPE_INVALID >= opType) || (service_pipe_OP_TYPE_MAX <= opType)) {
        return service_pipe_ERROR_INVALID_OPERATION_TYPE;
    }

    if ((NULL == hPipe) || (NULL == pMessage)) {
        return service_pipe_ERROR_NULL_ARGS;
    }

    if (service_pipe_STATUS_NO_ERROR !=
        service_base_ASSERT_HANDLE(hPipe, gPoolPipeControlBlock, service_pipe_INSTANCE_COUNT_MAX,
                                   service_pipe_ControlBlock_t, service_pipe_HANDLE_MAGIC,
                                   service_pipe_STATUS_NO_ERROR,
                                   service_pipe_ERROR_INVALID_HANDLE)) {
        return service_pipe_ERROR_INVALID_HANDLE;
    }

    pControlBlock = (service_pipe_ControlBlock_t*)hPipe;

    switch (opType) {
        case service_pipe_OP_TYPE_WRITE:
            return pump_in(pControlBlock, pMessage);

        case service_pipe_OP_TYPE_READ:
            return pump_out(pControlBlock, pMessage);

        default:
            return service_pipe_ERROR_INVALID_OPERATION_TYPE;
    }
}

static int32_t acquire_pump_in_channel(service_pipe_ControlBlock_t* pControlBlock,
                                       uint32_t* pChannelId) {
    uint32_t scanIndex;
    uint32_t activeChannel;

    if ((NULL == pControlBlock) || (NULL == pChannelId)) {
        return service_pipe_ERROR_NULL_ARGS;
    }

    service_system_ENTER_CRITICAL();

    for (scanIndex = 0; scanIndex < service_pipe_CHANNEL_COUNT_MAX; scanIndex++) {
        activeChannel =
            (pControlBlock->nextWriteChannel + scanIndex) % service_pipe_CHANNEL_COUNT_MAX;

        if (service_pipe_STATE_IDLE == pControlBlock->channel[activeChannel].state) {
            pControlBlock->channel[activeChannel].state = service_pipe_STATE_BUSY_WRITE;

            pControlBlock->nextWriteChannel = (activeChannel + 1U) % service_pipe_CHANNEL_COUNT_MAX;

            *pChannelId = activeChannel;

            service_system_EXIT_CRITICAL();

            return service_pipe_STATUS_NO_ERROR;
        }
    }

    service_system_EXIT_CRITICAL();

    return service_pipe_ERROR_RESOURCES_UNAVAILABLE;
}

static int32_t acquire_pump_out_channel(service_pipe_ControlBlock_t* pControlBlock,
                                        uint32_t* pChannelId) {
    uint32_t activeChannel;
    uint32_t selectedChannel;
    uint32_t highestSequence;
    bool channelFound;

    if ((NULL == pControlBlock) || (NULL == pChannelId)) {
        return service_pipe_ERROR_NULL_ARGS;
    }

    highestSequence = 0U;
    selectedChannel = 0U;
    channelFound = false;

    service_system_ENTER_CRITICAL();

    for (activeChannel = 0U; activeChannel < service_pipe_CHANNEL_COUNT_MAX; activeChannel++) {
        if (service_pipe_STATE_IDLE != pControlBlock->channel[activeChannel].state) {
            continue;
        }

        if ((false == channelFound) ||
            (pControlBlock->channel[activeChannel].sequence > highestSequence)) {
            highestSequence = pControlBlock->channel[activeChannel].sequence;

            selectedChannel = activeChannel;

            channelFound = true;
        }
    }

    if (false == channelFound) {
        service_system_EXIT_CRITICAL();
        return service_pipe_ERROR_RESOURCES_UNAVAILABLE;
    }

    pControlBlock->channel[selectedChannel].state = service_pipe_STATE_BUSY_READ;

    *pChannelId = selectedChannel;

    service_system_EXIT_CRITICAL();

    return service_pipe_STATUS_NO_ERROR;
}

static int32_t release_channel(service_pipe_ControlBlock_t* pControlBlock, uint32_t channelId) {
    if (NULL == pControlBlock) {
        return service_pipe_ERROR_NULL_ARGS;
    }

    if (channelId >= service_pipe_CHANNEL_COUNT_MAX) {
        return service_pipe_ERROR_INVALID_RESOURCES;
    }

    service_system_ENTER_CRITICAL();

    pControlBlock->channel[channelId].state = service_pipe_STATE_IDLE;

    service_system_EXIT_CRITICAL();

    return service_pipe_STATUS_NO_ERROR;
}

static int32_t pump_in(service_pipe_ControlBlock_t* pControlBlock,
                       service_pipe_Message_t* pMessage) {
    int32_t exeStatus;
    uint32_t channelId;

    if ((NULL == pControlBlock) || (NULL == pMessage)) {
        return service_pipe_ERROR_NULL_ARGS;
    }

    exeStatus = acquire_pump_in_channel(pControlBlock, &channelId);

    if (service_pipe_STATUS_NO_ERROR != exeStatus) {
        return exeStatus;
    }

    memcpy(&pControlBlock->channel[channelId].message, pMessage, sizeof(service_pipe_Message_t));

    pControlBlock->channel[channelId].sequence = ++pControlBlock->writeSequence;

    exeStatus = release_channel(pControlBlock, channelId);

    return exeStatus;
}

static int32_t pump_out(service_pipe_ControlBlock_t* pControlBlock,
                        service_pipe_Message_t* pMessage) {
    int32_t exeStatus;
    uint32_t channelId;

    if ((NULL == pControlBlock) || (NULL == pMessage)) {
        return service_pipe_ERROR_NULL_ARGS;
    }

    exeStatus = acquire_pump_out_channel(pControlBlock, &channelId);

    if (service_pipe_STATUS_NO_ERROR != exeStatus) {
        return exeStatus;
    }

    memcpy(pMessage, &pControlBlock->channel[channelId].message, sizeof(service_pipe_Message_t));

    exeStatus = release_channel(pControlBlock, channelId);

    return exeStatus;
}
