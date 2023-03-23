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

void test_sm9_sign_and_verify(){
	g1_t g1;
	g2_t g2;
	ep2_t Ppub;
	fp12_t r;

	g1_null(g1);
	g1_new(g1);
	g1_get_gen(g1);

	g2_null(g2);
	g2_new(g2);
	g2_get_gen(g2);

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

	SM9_SIGN_KEY sign_key;

	char _x[] = "A5702F05CF1315305E2D6EB64B0DEB923DB1A0BCF0CAFF90523AC8754AA69820";
	char _y[] = "78559A844411F9825C109F5EE3F52D720DD01785392A727BB1556952B2B013D3";
	fp_read_str(sign_key.ds->x,_x,strlen(_x),16);
	fp_read_str(sign_key.ds->y,_y,strlen(_y),16);
	fp_read_str(sign_key.ds->z,z0,strlen(z0),16);
	ep2_copy(sign_key.Ppubs,Ppub); 


	SM9_SIGN_CTX sign_ctx;
	const char *id = "Alice";
	uint8_t sig[104];
	size_t siglen;
	uint8_t buf[512];
	uint8_t *p = buf;
	const uint8_t *cp = buf;
	size_t len;
	int ret;

	SM9_SIGN_MASTER_KEY sign_master;
	//char ks[] = "0130E78459D78545CB54C587E02CF480CE0B66340F319F348A1D5B1F2DC5F4";
	sm9_bn_t ks = {0x1F2DC5F4,0x348A1D5B,0x340F319F,0x80CE0B66,0x87E02CF4,0x45CB54C5,0x8459D785,0x0130E7};
	bn_to_bn(sign_master.ks,ks);
	ep2_copy(sign_master.Ppubs,Ppub);

	uint8_t data[20] = {0x43, 0x68, 0x69, 0x6E, 0x65, 0x73, 0x65, 0x20, 0x49, 0x42, 0x53, 0x20, 0x73, 0x74, 0x61, 0x6E, 0x64, 0x61, 0x72, 0x64};
	
	
	//test_sm9_sign(SM9_P1,SM9_P2);
	// sm9_sign_master_key_generate(&sign_master);
	// sm9_sign_master_key_extract_key(&sign_master, id, strlen(id), &sign_key);
	sm9_sign_init(&sign_ctx);
	sm9_sign_update(&sign_ctx,data, sizeof(data));
	sm9_sign_finish(&sign_ctx, &sign_key, sig, &siglen);
	format_bytes(stdout, 0, 0, "signature", sig, siglen);


	sm9_clean();
	g1_free(g1);
	ep2_free(Ppub);
	fp12_free(r);
	return 1;
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
	test_sm9_sign_and_verify();
	core_clean();

	return 0;
}

