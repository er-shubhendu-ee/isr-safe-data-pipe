/**
 * @file      service_pipe.h
 * @author    Shubhendu B B
 * @date      01/06/2026
 * @brief     Lightweight snapshot transport service.
 *
 * PIPE Characteristics:
 *   - Static memory allocation
 *   - Fixed-width payload
 *   - Fixed channel count
 *   - Non-blocking access
 *   - Round-robin write allocation
 *   - Latest-snapshot read policy
 *   - ISR-safe ownership transitions
 *   - RTOS-safe ownership transitions
 *
 * Design Notes:
 *   - Not a FIFO
 *   - Not a Queue
 *   - Not a Mailbox
 *   - Not Publish/Subscribe
 *
 * The PIPE is a transport medium for exchanging
 * application snapshots between execution contexts.
 *
 * Data validity, freshness, ownership, and startup
 * sequencing are intentionally outside the scope
 * of this service and remain the responsibility of
 * the application layer.
 */

#ifndef SERVICE_PIPE_H_
#define SERVICE_PIPE_H_

#include <stdbool.h>
#include <stdint.h>

#define service_pipe_INSTANCE_COUNT_MAX 2
#define service_pipe_CHANNEL_COUNT_MAX 2

typedef enum service_pipe_tagStatus {
    service_pipe_STATUS_NO_ERROR = 0,

    service_pipe_ERROR_GENERIC = -1,
    service_pipe_ERROR_NULL_ARGS = -2,
    service_pipe_ERROR_INVALID_RESOURCES = -3,
    service_pipe_ERROR_RESOURCES_UNAVAILABLE = -4,
    service_pipe_ERROR_INVALID_HANDLE = -5,
    service_pipe_ERROR_INVALID_OPERATION_TYPE = -6
} service_pipe_Status_t;

typedef enum service_pipe_tagOpType {
    service_pipe_OP_TYPE_INVALID = 0,
    service_pipe_OP_TYPE_WRITE = 1,
    service_pipe_OP_TYPE_READ = 2,
    service_pipe_OP_TYPE_MAX
} service_pipe_OpType_t;

typedef struct service_pipe_tagSupervisorMessage {
    struct {
        uint32_t readyToOutput : 1;
    } status;
} service_pipe_SupervisorMessage_t;

typedef struct service_pipe_tagProtectionMessage {
    struct {
        uint32_t isSystemHealthy : 1;
    } status;
} service_pipe_ProtectionMessage_t;

typedef struct service_pipe_tagControlMessage {
    int32_t placeHolder;
} service_pipe_ControlMessage_t;

/* Application-defined pipe identifiers.
 *
 * Pipe ownership and direction should be
 * documented at system level.
 */
typedef uint32_t service_pipe_Id_t;

/* Pipe payload.
 *
 * Pipe width is fixed at compile time.
 *
 * Extend this structure as required.
 */
typedef struct service_pipe_tagMessage {
    service_pipe_SupervisorMessage_t supervisorParam;
    service_pipe_ProtectionMessage_t protectionParam;
    service_pipe_ControlMessage_t controlParam;
} service_pipe_Message_t;

/* Pipe width in bytes. */
#define service_pipe_END_SIZE_BYTES sizeof(service_pipe_Message_t)

typedef struct service_pipe_ControlBlock_t* service_pipe_Handle_t;

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize PIPE service.
 *
 * Clears:
 *   - internal control blocks
 *   - channel state
 *   - sequence counters
 *
 * Must be called before any access.
 *
 * Returns:
 *   NO_ERROR
 */
int32_t service_pipe_init(void);

/* Allocates a new PIPE handle from the pool.
 *
 * Behavior:
 *   - Returns pointer to next available control block
 *   - Initializes handleMagic and id
 *   - Clears previous contents to avoid stale data
 *
 * Limitations:
 *   - No reuse of freed instances
 *   - No protection against exhaustion beyond simple bound check
 *
 * Safety:
 *   - memset ensures no stale config/callback data remains
 */
service_pipe_Handle_t service_pipe_get_handle(service_pipe_Id_t id);

/* Access PIPE.
 *
 * Parameters:
 *   pipeId
 *   opType
 *   pMessage
 *
 * WRITE:
 *   Copies pMessage into PIPE.
 *
 * READ:
 *   Copies latest available snapshot
 *   from PIPE into pMessage.
 *
 * Returns:
 *   NO_ERROR
 *   ERROR_NULL_ARGS
 *   ERROR_INVALID_ID
 *   ERROR_INVALID_OPERATION_TYPE
 *   ERROR_NO_RESOURCES_AVAILABLE
 */
int32_t service_pipe_access(service_pipe_Handle_t hPipe, service_pipe_OpType_t opType,
                            service_pipe_Message_t* pMessage);

#ifdef __cplusplus
}
#endif

#endif /* @end  SERVICE_PIPE_H_*/
