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
#include "debug.h"

#include <sys/times.h>
#include <unistd.h>
#include <signal.h>

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


	sm9_sign_init(&sign_ctx);
	sm9_sign_update(&sign_ctx, (uint8_t *)"hello world", strlen("hello world"));

	// 测试 sm9_do_sign 和 pairing
	// sm9_sign_finish(&sign_ctx, &sign_key, sig, &siglen);

	// 测试sm9_sign
	PERFORMANCE_TEST_NEW("sm9_sign", sm9_sign_finish(&sign_ctx, &sign_key, sig, &siglen));

	
	format_bytes(stdout, 0, 0, "signature", sig, siglen);

	return 0;
}
