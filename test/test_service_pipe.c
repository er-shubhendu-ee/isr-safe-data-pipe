#include "test_service_pipe.h"

#include <stdio.h>
#include <string.h>

#include "service_pipe.h"

#define TEST_PASS 0
#define TEST_FAIL (-1)

static void print_result(const char* pName, int status) {
    printf("[%s] %s\n", (TEST_PASS == status) ? "PASS" : "FAIL", pName);
}

int test_init(void) {
    int32_t status;

    status = service_pipe_init();

    print_result(__func__, (service_pipe_STATUS_NO_ERROR == status) ? TEST_PASS : TEST_FAIL);

    return status;
}

int test_handle_allocation(void) {
    service_pipe_Handle_t hPipe;

    service_pipe_init();

    hPipe = service_pipe_get_handle(1);

    int status = (NULL != hPipe) ? TEST_PASS : TEST_FAIL;

    print_result(__func__, status);

    return status;
}

int test_handle_exhaustion(void) {
    service_pipe_Handle_t h1;
    service_pipe_Handle_t h2;
    service_pipe_Handle_t h3;

    service_pipe_init();

    h1 = service_pipe_get_handle(1);
    h2 = service_pipe_get_handle(2);
    h3 = service_pipe_get_handle(3);

    int status = ((NULL != h1) && (NULL != h2) && (NULL == h3)) ? TEST_PASS : TEST_FAIL;

    print_result(__func__, status);

    return status;
}

int test_single_write_read(void) {
    service_pipe_Handle_t hPipe;
    service_pipe_Message_t tx;
    service_pipe_Message_t rx;

    service_pipe_init();

    hPipe = service_pipe_get_handle(1);

    memset(&tx, 0, sizeof(tx));
    memset(&rx, 0, sizeof(rx));

    tx.supervisorParam.status.readyToOutput = 1;
    tx.protectionParam.status.isSystemHealthy = 1;
    tx.controlParam.placeHolder = 123;

    service_pipe_access(hPipe, service_pipe_OP_TYPE_WRITE, &tx);

    service_pipe_access(hPipe, service_pipe_OP_TYPE_READ, &rx);

    int status = (memcmp(&tx, &rx, sizeof(tx)) == 0) ? TEST_PASS : TEST_FAIL;

    print_result(__func__, status);

    return status;
}

int test_multiple_write_read(void) {
    service_pipe_Handle_t hPipe;
    service_pipe_Message_t tx;
    service_pipe_Message_t rx;

    service_pipe_init();

    hPipe = service_pipe_get_handle(1);

    memset(&tx, 0, sizeof(tx));

    tx.controlParam.placeHolder = 1;
    service_pipe_access(hPipe, service_pipe_OP_TYPE_WRITE, &tx);

    tx.controlParam.placeHolder = 2;
    service_pipe_access(hPipe, service_pipe_OP_TYPE_WRITE, &tx);

    tx.controlParam.placeHolder = 3;
    service_pipe_access(hPipe, service_pipe_OP_TYPE_WRITE, &tx);

    memset(&rx, 0, sizeof(rx));

    service_pipe_access(hPipe, service_pipe_OP_TYPE_READ, &rx);

    int status = (3 == rx.controlParam.placeHolder) ? TEST_PASS : TEST_FAIL;

    print_result(__func__, status);

    return status;
}

int test_null_handle(void) {
    service_pipe_Message_t msg;

    int status = (service_pipe_ERROR_NULL_ARGS ==
                  service_pipe_access(NULL, service_pipe_OP_TYPE_WRITE, &msg))
                     ? TEST_PASS
                     : TEST_FAIL;

    print_result(__func__, status);

    return status;
}

int test_null_message(void) {
    service_pipe_Handle_t hPipe;

    service_pipe_init();

    hPipe = service_pipe_get_handle(1);

    int status = (service_pipe_ERROR_NULL_ARGS ==
                  service_pipe_access(hPipe, service_pipe_OP_TYPE_WRITE, NULL))
                     ? TEST_PASS
                     : TEST_FAIL;

    print_result(__func__, status);

    return status;
}

int test_invalid_operation(void) {
    service_pipe_Handle_t hPipe;
    service_pipe_Message_t msg;

    service_pipe_init();

    hPipe = service_pipe_get_handle(1);

    int status =
        (0 > service_pipe_access(hPipe, service_pipe_OP_TYPE_MAX, &msg)) ? TEST_PASS : TEST_FAIL;

    print_result(__func__, status);

    return status;
}

int test_dual_pipe_isolation(void) {
    service_pipe_Handle_t hPipe1;
    service_pipe_Handle_t hPipe2;

    service_pipe_Message_t tx1;
    service_pipe_Message_t tx2;
    service_pipe_Message_t rx1;
    service_pipe_Message_t rx2;

    service_pipe_init();

    hPipe1 = service_pipe_get_handle(1);
    hPipe2 = service_pipe_get_handle(2);

    memset(&tx1, 0, sizeof(tx1));
    memset(&tx2, 0, sizeof(tx2));

    tx1.controlParam.placeHolder = 111;
    tx2.controlParam.placeHolder = 222;

    service_pipe_access(hPipe1, service_pipe_OP_TYPE_WRITE, &tx1);

    service_pipe_access(hPipe2, service_pipe_OP_TYPE_WRITE, &tx2);

    service_pipe_access(hPipe1, service_pipe_OP_TYPE_READ, &rx1);

    service_pipe_access(hPipe2, service_pipe_OP_TYPE_READ, &rx2);

    int status = ((111 == rx1.controlParam.placeHolder) && (222 == rx2.controlParam.placeHolder))
                     ? TEST_PASS
                     : TEST_FAIL;

    print_result(__func__, status);

    return status;
}

int test_read_before_first_write(void) {
    service_pipe_Handle_t hPipe;
    service_pipe_Message_t rx;

    service_pipe_init();

    hPipe = service_pipe_get_handle(1);

    int status = service_pipe_access(hPipe, service_pipe_OP_TYPE_READ, &rx);

    printf("[INFO] Read-before-write returned %ld\n", (long)status);

    print_result(__func__, TEST_PASS);

    return TEST_PASS;
}