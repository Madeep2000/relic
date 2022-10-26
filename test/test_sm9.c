#include "sm9.h"
#include <time.h>
#include "debug.h"
#include <pthread.h>
#include <omp.h>

void sm9_pairing_omp_t(fp12_t r_arr[], const ep2_t Q_arr[], const ep_t P_arr[], const size_t arr_size, const size_t threads_num){
	omp_set_num_threads(threads_num);	
	#pragma omp parallel for
	for (size_t i = 0; i < arr_size; i++)
	{
		sm9_pairing(r_arr[(i)], Q_arr[(i)], P_arr[(i)]);
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

#if 1
	// 测试正确性
	sm9_pairing(r, Ppub, g1);
	printf("in: Ppub\n");
	ep2_print(Ppub);
	printf("in: g1\n");
	ep_print(g1);
	printf("out: r\n");
	fp12_print(r);

	// pp_map_tatep_k12(r, g1, Ppub);
	// printf("tatep: r\n");
	// fp12_print(r);

	// pp_map_weilp_k12(r, g1, Ppub);
	// printf("weilp: r\n");
	// fp12_print(r);

	// pp_map_oatep_k12(r, g1, Ppub);
	// printf("oatep: r\n");
	// fp12_print(r);
#endif

// #if 1
// 	PERFORMANCE_TEST("pairing", sm9_pairing(r, Ppub, g1), 1000);
// 	PERFORMANCE_TEST("pp_map_tatep_k12(r, g1, Ppub)", pp_map_tatep_k12(r, g1, Ppub), 1000);
// 	PERFORMANCE_TEST("pp_map_weilp_k12(r, g1, Ppub)", pp_map_weilp_k12(r, g1, Ppub), 1000);
// 	PERFORMANCE_TEST("pp_map_oatep_k12(r, g1, Ppub)", pp_map_oatep_k12(r, g1, Ppub), 1000);
// #endif

#if 1
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
	int threads_num = 5;
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
	printf("run %d times, threads num: %d, total time: %f s, one time: %f s\n", \
       	   count, threads_num, 1.0*(end-begin), 1.0*(end-begin)/count);
	// 清理空间
	for (size_t i = 0; i < count; i++)
	{
		fp12_free(r_arr[i]);
		g1_free(g1_arr[i]);
		ep2_free(Ppub_arr[i]);
	}
#endif

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

int test_alot(){
	g1_t g1;
	ep2_t Ppub;
	fp12_t r;
	bn_t k;
	bn_null(k);
	bn_new(k);
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

	

#if 0
	// 非退化性测试1
	printf("TEST\n");
	ep_rand(g1);
	ep2_rand(Ppub);
	sm9_init();
	
	sm9_pairing(r, Ppub, g1);
	printf("in: Ppub\n");
	ep2_print(Ppub);
	printf("in: g1\n");
	ep_print(g1);
	printf("out: r\n");
	fp12_print(r);
#endif
#if 0
	// 非退化性测试2
	printf("TEST\n");
	ep_rand(g1);
	ep2_set_infty(Ppub);
	sm9_init();
	
	sm9_pairing(r, Ppub, g1);
	printf("in: Ppub\n");
	ep2_print(Ppub);
	printf("in: g1\n");
	ep_print(g1);
	printf("out: r\n");
	fp12_print(r);
#endif
#if 0
	// 非退化性测试3
	printf("TEST\n");
	ep_set_infty(g1);
	ep2_rand(Ppub);
	sm9_init();
	
	sm9_pairing(r, Ppub, g1);
	printf("in: Ppub\n");
	ep2_print(Ppub);
	printf("in: g1\n");
	ep_print(g1);
	printf("out: r\n");
	fp12_print(r);
#endif
#if 0
	// 双线性测试1
	printf("TEST\n");

	g1_t gtemp;
	ep2_t Ptemp;
	g1_null(gtemp);
	g1_get_gen(gtemp);
	ep2_null(Ptemp);
	ep2_new(Ptemp);

	bn_rand(k,RLC_POS,100);
	ep_rand(g1);
	ep2_rand(Ppub);

	ep_mul_basic(gtemp,g1,k);
	ep2_mul_basic(Ptemp,Ppub,k);

	sm9_init();
	
	printf("in: Ppub\n");
	ep2_print(Ppub);
	printf("in: g1\n");
	ep_print(g1);
	printf("in: Ptemp\n");
	ep2_print(Ptemp);
	printf("in: gtemp\n");
	ep_print(gtemp);
	printf("out: r1\n");
	sm9_pairing(r, Ppub, gtemp);
	fp12_print(r);

	
	sm9_pairing(r, Ptemp, g1);
	printf("out: r2\n");
	fp12_print(r);

	g1_free(gtemp);
	ep2_free(Ptemp);
#endif
#if 0
	// 双线性测试2
	printf("TEST\n");

	g1_t gtemp;
	g1_null(gtemp);
	g1_get_gen(gtemp);

	bn_rand(k,RLC_POS,100);
	ep_rand(g1);
	ep2_rand(Ppub);

	ep_mul_basic(gtemp,g1,k);

	sm9_init();
	
	printf("in: Ppub\n");
	ep2_print(Ppub);
	printf("in: g1\n");
	ep_print(g1);
	printf("in: gtemp\n");
	ep_print(gtemp);
	printf("out: r1\n");
	sm9_pairing(r, Ppub, gtemp);
	fp12_print(r);
	
	sm9_pairing(r, Ppub, g1);
	printf("out: r2\n");
	fp12_print(r);
	fp12_pow_t(r,r,k);
	printf("out: r2^k\n");
	fp12_print(r);

	g1_free(gtemp);
#endif
#if 0
	// 双线性测试3
	printf("TEST\n");

	ep2_t Ptemp;
	ep2_null(Ptemp);
	ep2_new(Ptemp);

	bn_rand(k,RLC_POS,255);
	ep2_rand(Ppub);
	ep2_mul_basic(Ptemp,Ppub,k);

	sm9_init();
	
	printf("in: Ppub\n");
	ep2_print(Ppub);
	printf("in: g1\n");
	ep_print(g1);
	printf("in: Ptemp\n");
	ep2_print(Ptemp);
	printf("out: r1\n");
	sm9_pairing(r, Ptemp, g1);
	fp12_print(r);

	sm9_pairing(r, Ppub, g1);
	printf("out: r2\n");
	fp12_print(r);
	fp12_pow_t(r,r,k);
	printf("out: r2^k\n");
	fp12_print(r);

	ep2_free(Ptemp);
#endif
#if 1
	// 双线性测试4
	printf("TEST\n");
	bn_t k2;
	bn_null(k2);
	bn_new(k2);

	g1_t gtemp;
	ep2_t Ptemp;
	g1_null(gtemp);
	g1_get_gen(gtemp);
	ep2_null(Ptemp);
	ep2_new(Ptemp);

	bn_rand(k,RLC_POS,255);
	bn_rand(k2,RLC_POS,255);
	ep_rand(g1);
	ep2_rand(Ppub);

	ep_mul_basic(gtemp,g1,k);
	ep2_mul_basic(Ptemp,Ppub,k2);

	sm9_init();
	
	printf("in: Ppub\n");
	ep2_print(Ppub);
	printf("in: g1\n");
	ep_print(g1);
	printf("in: Ptemp\n");
	ep2_print(Ptemp);
	printf("in: gtemp\n");
	ep_print(gtemp);
	printf("out: r1\n");
	sm9_pairing(r, Ptemp, gtemp);
	fp12_print(r);

	
	sm9_pairing(r, Ppub, g1);
	printf("out: r2\n");
	fp12_print(r);
	fp12_pow_t(r,r,k);
	fp12_pow_t(r,r,k2);
	printf("out: r2^{k1*k2}\n");
	fp12_print(r);

	g1_free(gtemp);
	ep2_free(Ptemp);
	bn_free(k2);
#endif
	sm9_clean();
	g1_free(g1);
	ep2_free(Ppub);
	fp12_free(r);
	bn_free(k);
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

	//test_sm9_pairing();
	for(int i = 0 ; i < 1 ; i++){
		test_alot();
	}
	core_clean();
	return 0;
}