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

void test_sm9_sign_and_verify(const uint8_t data1[],const char id1[]){

	int j = 1;
	SM9_SIGN_KEY sign_key;
	SM9_SIGN_MASTER_KEY sign_master;

	user_key_init(&sign_key);
	master_key_init(&sign_master);

	SM9_SIGN_CTX ctx;
	const char *id = "Alice";
	uint8_t sig[104];
	size_t siglen;

	char ks[] = "130E78459D78545CB54C587E02CF480CE0B66340F319F348A1D5B1F2DC5F4";
	bn_read_str(sign_master.ks,ks,strlen(ks),16);
	//sm9_bn_t ks = {0x1F2DC5F4,0x348A1D5B,0x340F319F,0x80CE0B66,0x87E02CF4,0x45CB54C5,0x8459D785,0x0130E7};
	//bn_to_bn(sign_master.ks,ks);
	ep2_mul_gen(sign_master.Ppubs,sign_master.ks);

	//ep2_copy(sign_master.Ppubs,Ppub);

	// data = "Chinese IBS standard"
	uint8_t data[20] = {0x43, 0x68, 0x69, 0x6E, 0x65, 0x73, 0x65, 0x20, 0x49, 0x42, 0x53, 0x20, 0x73, 0x74, 0x61, 0x6E, 0x64, 0x61, 0x72, 0x64};
	
	// sm9_sign_master_key_generate(&sign_master);
	sm9_sign_master_key_extract_key(&sign_master, id, strlen(id), &sign_key);
	sm9_sign_init(&ctx);
	sm9_sign_update(&ctx,data, sizeof(data));
	sm9_sign_finish(&ctx, &sign_key, sig, &siglen);
	format_bytes(stdout, 0, 0, "signature", sig, siglen);

	sm9_verify_init(&ctx);
	sm9_verify_update(&ctx, data, sizeof(data));
	if (sm9_verify_finish(&ctx, sig, siglen, &sign_master, id, strlen(id)) != 1) goto err; ++j;
	format_bytes(stdout, 0, 0, "signature", sig, siglen);

	master_key_free(&sign_master);
	user_key_free(&sign_key);

	return 1;
err:
	printf("%s test %d failed\n", __FUNCTION__, j);
	error_print();
	return -1;
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
	char *id = "Alice";
	char *data =  "Chinese IBS standard";
	
	test_sm9_sign_and_verify(data,id);
	core_clean();

	return 0;
}

