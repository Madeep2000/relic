#include "sm9.h"
#include <time.h>
#include "debug.h"
#include <pthread.h>
#include <omp.h>

void sm9_pairing_omp_t(fp12_t r_arr[], const ep2_t Q_arr[], const ep_t P_arr[], const size_t arr_size, const size_t threads_num){
	omp_set_num_threads(threads_num);	
	#pragma omp parallel	
	{
		int id = omp_get_thread_num();
		printf("id=%d\n", id);
		for (size_t i = 0; i < arr_size; i+=threads_num)
		{
			sm9_pairing(r_arr[(i+id)%arr_size], Q_arr[(i+id)%arr_size], P_arr[(i+id)%arr_size]);
		}
	}
}
void test_sm9_pairing(){
	g1_t g1;
	ep2_t Ppub;
	fp12_t r;

	g1_null(g1);
	g1_new(g1);
	g1_get_gen(g1);


	ep2_null(Ppub);
	ep2_new(Ppub);

	char x0[] = "29DBA116152D1F786CE843ED24A3B573414D2177386A92DD8F14D65696EA5E32";
	char x1[] = "9F64080B3084F733E48AFF4B41B565011CE0711C5E392CFB0AB1B6791B94C408";
	char y0[] = "41E00A53DDA532DA1A7CE027B7A46F741006E85F5CDFF0730E75C05FB4E3216D";
	char y1[] = "69850938ABEA0112B57329F447E3A0CBAD3E2FDB1A77F335E89E1408D0EF1C25";
	char z0[] = "1";
	char z1[] = "0";

	fp_read_str(Ppub->x[0], x0, strlen(x0), 16);
	fp_read_str(Ppub->x[1], x1, strlen(x1), 16);
	fp_read_str(Ppub->y[0], y0, strlen(y0), 16);
	fp_read_str(Ppub->y[1], y1, strlen(y1), 16);
	fp_read_str(Ppub->z[0], z0, strlen(z0), 16);
	fp_read_str(Ppub->z[1], z1, strlen(z1), 16);

	fp12_null(r);
	fp12_new(r);

	sm9_init();

	// 测试正确性
	// sm9_pairing(r, Ppub, g1);
	// printf("in: Ppub\n");
	// ep2_print(Ppub);
	// printf("in: g1\n");
	// ep_print(g1);
	// printf("out: r\n");
	// fp12_print(r);
	
	// 测试性能
	// PERFORMANCE_TEST("pairing", sm9_pairing(r, Ppub, g1), 1000);
	pthread_attr_t attr; // 定义线程属性

	size_t count=1000;
	fp12_t r_arr[count];
	g1_t g1_arr[count];
	ep2_t Ppub_arr[count];

	for (size_t i = 0; i < count; i++)
	{
		fp12_null(r_arr[i]);
		fp12_new(r_arr[i]);
		g1_null(g1_arr[i]);
		g1_new(g1_arr[i]);
		ep2_null(Ppub_arr[i]);
		ep2_new(Ppub_arr[i]);
		g1_copy(g1_arr[i], g1);
		ep2_copy(Ppub_arr[i], Ppub);
	}
	
	double begin, end;
	int threads_num = 4;
	omp_set_num_threads(threads_num);
	begin = omp_get_wtime();
	sm9_pairing_omp_t(r_arr, Ppub_arr, g1_arr, count, threads_num);
	// #pragma omp parallel	
	// {
	// 	int id = omp_get_thread_num();
	// 	printf("id=%d\n", id);
	// 	for (size_t i = 0; i < count; i+=threads_num)
	// 	{
	// 		sm9_pairing(r_arr[i+id], Ppub_arr[i+id], g1_arr[i+id]);
	// 	}
	// }
	end = omp_get_wtime();
	printf("run %d times, total time: %f s, one time: %f s\n", \
       	   count, 1.0*(end-begin), 1.0*(end-begin)/count);

	// 清理空间
	for (size_t i = 0; i < count; i++)
	{
		fp12_free(r_arr[i]);
		g1_free(g1_arr[i]);
		ep2_free(Ppub_arr[i]);
	}
	sm9_clean();
	g1_free(g1);
	ep2_free(Ppub);
	fp12_free(r);
}

void test_fp12_mul(){
	fp12_t t1, t2, t3;

	fp12_null(t1);
	fp12_null(t2);
	fp12_null(t3);

	fp12_new(t1);
	fp12_new(t2);
	fp12_new(t3);

	fp12_rand(t1);
	fp12_rand(t2);

	printf("1:\n");
	fp12_mul_lazyr(t3, t1, t2);
	fp12_print(t3);

	printf("2:\n");
	fp12_mul(t3, t1, t2);
	fp12_print(t3);


	fp12_free(t1);
	fp12_free(t2);
	fp12_free(t3);
}


int main(void) {
	if (core_init() != RLC_OK) {
		core_clean();
		return 1;
	}

	if (pc_param_set_any() != RLC_OK) {
		RLC_THROW(ERR_NO_CURVE);
		core_clean();
		return 0;
	}

	pc_param_print();

	test_sm9_pairing();
	// test_fp12_mul();

	core_clean();
	return 0;
}