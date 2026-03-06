#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define EPSILON 1.0e-6

void print_mat(double **mat, int n){
    int i, j;
    for (i=0; i<n; i++){
        for (j=0; j<n; j++){
            printf("%lf ", mat[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    return;
}

void multiply_by_scalar(double **mat, int n, int row, double constant){
    int column;
    for (column=0; column<n; column++){
        mat[row][column]*=constant;
    }
    return;
}

void swap_rows(double **mat, int row1, int row2){
    double *tmp = mat[row1];
    mat[row1] = mat[row2];
    mat[row2] = tmp;
    return;
}

void add_row1_to_row2(double **mat, int n, int row1, int row2, double constant){
    int column;
    for (column=0; column<n; column++){
        mat[row2][column] += mat[row1][column]*constant;
    }
    return;
}

double Gauss_alg(double **mat, int n){
    int column, row, base_row, base_row_selected;
    double det = 1.0;
    for (column=0; column<n; column++){
        base_row_selected = 0;
        for (row=column; !base_row_selected && row<n; row++){
            if (fabsl(mat[row][column]) > EPSILON){
                base_row = row;
                base_row_selected = 1;
            }
        }
        if (!base_row_selected) return 0.0;
        det*=mat[base_row][column];
        multiply_by_scalar(mat, n, base_row, 1.0/mat[base_row][column]);
        for (; row<n; row++){
            if (fabs(mat[row][column]) > EPSILON){
                add_row1_to_row2(mat, n, base_row, row, -mat[row][column]);
            }
        }
        if (base_row != column) {
            swap_rows(mat, column, base_row);
        }
    }
    return det;
}

int main(void){
    int n, i, j;
    scanf("%d", &n);
    double *mat[n];
    for (i=0; i<n; i++){
        mat[i] = (double*)malloc(sizeof(double)*n);
    }
    for (i=0; i<n; i++){
        for (j=0; j<n; j++){
            scanf("%lf", mat[i] + j);
        }
    }
    printf("%lf\n", Gauss_alg(mat, n));
    for (i=0; i<n; i++){
        free(mat[i]);
    }
    return 0;
}