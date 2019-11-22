__kernel void matrix_mult(__global const float *x, 
                        __global const float *y, 
                        __global float *restrict z,
                        const int N , 
                        const int M)
{
    int row = get_global_id(0);
    int column = get_global_id(1);
    float tmp =0;
    for (int j = 0; j < M; j++)
        tmp += x[row* N + j] * y[j* M + column];
    z[row* M + column] = tmp;
}

