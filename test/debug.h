#ifndef DEBUG_H
#define DEBUG_H
#include <time.h>

time_t begin_t, end_t;
size_t count_t;
#if 1
// 测试性能
#define PERFORMANCE_TEST(prestr, func, times) begin_t = clock();   \
	count_t = times;                                               \
	while (count_t)                                                \
	{                                                              \
		func;                                                      \
		count_t-=1;                                                \
	}                                                              \
	end_t = clock();                                               \
	printf("%s, run %d times, total time: %f s, one time: %f s\n", \
       	   prestr, times, 1.0*(end_t-begin_t)/CLOCKS_PER_SEC, 1.0*(end_t-begin_t)/CLOCKS_PER_SEC/times)
#else
	#define PERFORMANCE_TEST(prestr, func, times) {}
#endif

#endif