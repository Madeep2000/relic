/*
 *  Copyright 2014-2022 The GmSSL Project. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the License); you may
 *  not use this file except in compliance with the License.
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sm9.h"
#include "gmssl/error.h"

#include <sys/times.h>
#include <unistd.h>
#include <signal.h>

static int run = 0;

#define TM_START        0
#define TM_STOP         1

#define START        0
#define STOP         1

static void alarmed(int sig)
{
    signal(SIGALRM, alarmed); 
    run = 0;
}

double app_tminterval(int stop)
{
    double ret = 0;
    struct tms rus;
    clock_t now = times(&rus);
    static clock_t tmstart;

    if (stop == TM_START) {
        tmstart = now;
    } else {
        long int tck = sysconf(_SC_CLK_TCK);
        ret = (now - tmstart) / (double)tck;
    }
    return ret;
}

// s为STOP时，返回间隔时间
static double Time_F(int s)
{
    double ret = app_tminterval(s);  // 返回
    if (s == STOP)
        alarm(0);  // 停止闹钟
    return ret;
}

int main(void)
{
	if (core_init() != RLC_OK) {
		core_clean();
		return 1;
	}

	if (pc_param_set_any() != RLC_OK) {
		RLC_THROW(ERR_NO_CURVE);
		core_clean();
		return 0;
	}
	// SM9_SIGN_MASTER_KEY sign_master;
	// SM9_SIGN_MASTER_KEY sign_master_public;
	SM9_SIGN_KEY sign_key;
	SM9_SIGN_CTX sign_ctx;
	const char *id = "Alice";
	uint8_t sig[104];
	size_t siglen;
	uint8_t buf[512];
	uint8_t *p = buf;
	const uint8_t *cp = buf;
	size_t len;
	int ret;
	
	// sm9_sign_master_key_generate(&sign_master);
	// sm9_sign_master_key_extract_key(&sign_master, id, strlen(id), &sign_key);


	sm9_sign_init(&sign_ctx);
	sm9_sign_update(&sign_ctx, (uint8_t *)"hello world", strlen("hello world"));

    unsigned long count;
	int sec = 1;     
    double d = 0.0;
    // 注册计时器
    signal(SIGALRM, alarmed);
    
    // 计时sec秒
    alarm(sec);
    
    run = 1;
/* 
    Time_F(START);

	// 测每秒运行多少次
    for (count = 0; run && count < 0xffffffffffffffff; count++)
    {
        sm9_sign_finish(&sign_ctx, &sign_key, sig, &siglen);
    }
    d = Time_F(STOP);
    printf("relic SM9_sign run %x times in %.2fs\n", count, d);
*/
	

	format_bytes(stdout, 0, 0, "signature", sig, siglen);

	// sm9_sign_master_public_key_to_der(&sign_master, &p, &len);
	// sm9_sign_master_public_key_from_der(&sign_master_public, &cp, &len);

	// sm9_verify_init(&sign_ctx);
	// sm9_verify_update(&sign_ctx, (uint8_t *)"hello world", strlen("hello world"));
	// ret = sm9_verify_finish(&sign_ctx, sig, siglen, &sign_master_public, id, strlen(id));
	// printf("verify %s\n", ret == 1 ? "success" : "failure");
	
	sm9_sign_init(&sign_ctx);
	sm9_sign_update(&sign_ctx, (uint8_t *)"hello world", strlen("hello world"));
    
	// 测一次运行多长时间
    double begin, end;
	begin = omp_get_wtime();	
	sm9_sign_finish(&sign_ctx, &sign_key, sig, &siglen);
   	end = omp_get_wtime();
	count = 1;
    printf("relic SM9_sign run %x times in %f s\n", count, 1.0*(end-begin));

	

	format_bytes(stdout, 0, 0, "signature", sig, siglen);


	return 0;
}
