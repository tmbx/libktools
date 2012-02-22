#include <kerror.h>
#include "test.h"
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#define TEST_MODULE 0
#define TEST_LEVEL_ERROR 0

#define error(...)\
    KERROR_PUSH(TEST_MODULE, TEST_LEVEL_ERROR, __VA_ARGS__)

void bug() {
    error("bug");
}


UNIT_TEST(kerror) {
    kerror_reset();
    TASSERT(!kerror_has_error());

    TASSERT((bug(), error("an error happend"), kerror_has_error() && kerror_get_current()->stack.size == 2));

    kerror_reset();
    TASSERT(!kerror_has_error());


    /*
    KERROR_PUSH(0, 0, "an error has happened");
    KERROR_PUSH(0, 0, "the error fired another error");
    KERROR_PUSH(0, 0, "top of the error stack, we don't care about that one.");

    kstr *err_msg0 = kerror_str_n(0);
    fprintf(stderr, "kerror_str_n(0) returns:\n");
    fprintf(stderr, "%s\n", err_msg0->data);
    kstr_destroy(err_msg0);

    kstr *err_msg2 = kerror_str_n(2);
    fprintf(stderr, "kerror_str_n(2) returns:\n");
    fprintf(stderr, "%s\n", err_msg2->data);
    kstr_destroy(err_msg2);

    kstr *err_msg1000 = kerror_str_n(1000);
    fprintf(stderr, "kerror_str_n(1000) returns:\n");
    fprintf(stderr, "%s\n", err_msg1000->data);
    kstr_destroy(err_msg1000);
    */
}
