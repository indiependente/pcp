#ifndef __MATRIX_UTILS_H__
#define __MATRIX_UTILS_H__

void rand_fill_matrix(int m[N][N]);
void ordered_fill_matrix(int m[N][N]);
void print_matrix(int m[N][N], int rank);
void print_array_as_matrix(int *array, int count, int blocklength, int rank);
void transpose_matrix(int* array, int rows, int cols);

#endif