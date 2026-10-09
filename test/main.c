#include "test_service_pipe.h"

int main(void) {
    test_init();
    test_handle_allocation();
    test_handle_exhaustion();
    test_single_write_read();
    test_multiple_write_read();
    test_null_handle();
    test_null_message();
    test_invalid_operation();
    test_dual_pipe_isolation();
    test_read_before_first_write();

    return 0;
}