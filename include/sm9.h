#ifndef SM9_H
#define SM9_H

#include <stdio.h>
#include <omp.h>

#include "relic.h"

fp_t SM9_ALPHA1, SM9_ALPHA2, SM9_ALPHA3, SM9_ALPHA4, SM9_ALPHA5;
fp2_t SM9_BETA;

typedef uint64_t sm9_bn_t[8];

void sm9_init();
void sm9_clean();
void sm9_pairing(fp12_t r, const ep2_t Q, const ep_t P);
// 运行arr_size次配对算法，使用threads_num个线程运行
void sm9_pairing_omp(fp12_t r_arr[], const ep2_t Q_arr[], const ep_t P_arr[], const size_t arr_size, const size_t threads_num);
#endif