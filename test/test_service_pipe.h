#ifndef TEST_SERVICE_PIPE_H_
#define TEST_SERVICE_PIPE_H_

int test_init(void);
int test_handle_allocation(void);
int test_handle_exhaustion(void);
int test_single_write_read(void);
int test_multiple_write_read(void);
int test_null_handle(void);
int test_null_message(void);
int test_invalid_operation(void);
int test_dual_pipe_isolation(void);
int test_read_before_first_write(void);

#endif