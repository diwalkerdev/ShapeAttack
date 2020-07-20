#include <stdio.h>

int MELIORATE_NUM_TEST_FUNCTIONS = 5;
bool meliorate_stop_on_error = false;


extern void test_make_matrix();
extern void test_matrix_multiplication();
extern void test_rotations_vec();
extern void test_rotations_mat();
extern void test_row_iter();

void (*meliorate_test_functions[])() = {
    &test_make_matrix,
    &test_matrix_multiplication,
    &test_rotations_vec,
    &test_rotations_mat,
    &test_row_iter
};

const char* meliorate_test_names[] = {
    "test_make_matrix",
    "test_matrix_multiplication",
    "test_rotations_vec",
    "test_rotations_mat",
    "test_row_iter"
};