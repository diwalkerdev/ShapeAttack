#include <stdio.h>

int MELIORATE_NUM_TEST_FUNCTIONS = 3;
bool meliorate_stop_on_error = false;


extern void test_that_something_is_true();
extern void test_that_error_is_thrown();
extern void test_that_the_result_is_42();

void (*meliorate_test_functions[])() = {
    &test_that_something_is_true,
    &test_that_error_is_thrown,
    &test_that_the_result_is_42
};

const char* meliorate_test_names[] = {
    "test_that_something_is_true",
    "test_that_error_is_thrown",
    "test_that_the_result_is_42"
};