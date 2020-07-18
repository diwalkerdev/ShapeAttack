#include <stdio.h>

int MELIORATE_NUM_TEST_FUNCTIONS = 4;
bool meliorate_stop_on_error = false;


extern void test_make_matrix();
extern void test_matrix_multiplication();
extern void test_rotations();
extern void test_that_the_result_is_42();

void (*meliorate_test_functions[])() = {
    &test_make_matrix,
    &test_matrix_multiplication,
    &test_rotations,
    &test_that_the_result_is_42
};

const char* meliorate_test_names[] = {
    "test_make_matrix",
    "test_matrix_multiplication",
    "test_rotations",
    "test_that_the_result_is_42"
};