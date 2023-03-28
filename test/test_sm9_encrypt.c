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
#include <malloc.h>
#include <sys/times.h>
#include <unistd.h>
#include <signal.h>
#define hex_ke "0001EDEE3778F441F8DEA3D9FA0ACC4E07EE36C93F9A08618AF4AD85CEDE1C22"

int test_sm9_encrypt() {
	SM9_ENC_MASTER_KEY msk;
	SM9_ENC_KEY enc_key;

    //enc_master_key_init(&msk);
    //enc_user_key_init(&enc_key);
	
	ep_null(msk.Ppube);
	ep_new(msk.Ppube);
	bn_null(msk.ke);
	bn_new(msk.ke);
	ep_null(enc_key.Ppube);
	ep_new(enc_key.Ppube);
	ep2_null(enc_key.de);
	ep2_new(enc_key.de);

	ep2_t de;
	ep2_null(de);
    ep2_new(de);

    uint8_t out[1000] = {0};
	size_t outlen = 0;
	int j = 1;

	uint8_t data[20] = {0x43, 0x68, 0x69, 0x6E, 0x65, 0x73, 0x65, 0x20, 0x49, 0x42, 0x53, 0x20, 0x73, 0x74, 0x61, 0x6E, 0x64, 0x61, 0x72, 0x64};
	uint8_t dec[20] = {0};
	size_t declen = 20;
	uint8_t IDB[3] = {0x42, 0x6F, 0x62};

    char x1[] = "94736ACD2C8C8796CC4785E938301A139A059D3537B6414140B2D31EECF41683";
	char x0[] = "115BAE85F5D8BC6C3DBD9E5342979ACCCF3C2F4F28420B1CB4F8C0B59A19B158";
	char y1[] = "7AA5E47570DA7600CD760A0CF7BEAF71C447F3844753FE74FA7BA92CA7D3B55F";
    char y0[] = "27538A62E7F7BFB51DCE08704796D94C9D56734F119EA44732B50E31CDEB75C1";
	char z1[] = "0";
    char z0[] = "1";

    char ke[] = "1EDEE3778F441F8DEA3D9FA0ACC4E07EE36C93F9A08618AF4AD85CEDE1C22";
    bn_read_str(msk.ke,ke,strlen(ke),16);
	//sm9_bn_from_hex(msk.ke, hex_ke);
    ep_mul_gen(msk.Ppube,msk.ke);
    //sm9_point_mul_generator(&(msk.Ppube), msk.ke);


	if (sm9_enc_master_key_extract_key(&msk, (char *)IDB, sizeof(IDB), &enc_key) < 0) goto err; ++j;
	
    fp_read_str(de->x[0], x0, strlen(x0), 16);
	fp_read_str(de->x[1], x1, strlen(x1), 16);
	fp_read_str(de->y[0], y0, strlen(y0), 16);
	fp_read_str(de->y[1], y1, strlen(y1), 16);
	fp_read_str(de->z[0], z0, strlen(z0), 16);
	fp_read_str(de->z[1], z1, strlen(z1), 16);
    //sm9_twist_point_from_hex(&de, hex_de);
    
    if (ep2_cmp(enc_key.de,de)){ 
		goto err;
	} 
	++j;

	if (sm9_encrypt(&msk, (char *)IDB, sizeof(IDB), data, sizeof(data), out, &outlen) < 0) goto err; ++j;
	format_bytes(stdout, 0, 0, "ciphertext", out, outlen);
    if (sm9_decrypt(&enc_key, (char *)IDB, sizeof(IDB), out, outlen, dec, &declen) < 0) goto err; ++j;
	if (memcmp(data, dec, sizeof(data)) != 0) goto err; ++j;
	format_bytes(stdout, 0, 0, "plaintext", dec, declen);
	printf("%s() ok\n", __FUNCTION__);
    ep2_free(de);
	
	ep_free(msk.Ppube);
	bn_free(msk.ke);
	ep_free(enc_key.Ppube);
	ep2_free(enc_key.de);
    //enc_master_key_free(&msk);
    //enc_user_key_free(&enc_key);

	return 1;
err:
    ep2_free(de);
	ep_free(msk.Ppube);
	bn_free(msk.ke);
	ep_free(enc_key.Ppube);
	ep2_free(enc_key.de);

    //enc_master_key_free(&msk);
    //enc_user_key_free(&enc_key);
	printf("%s test %d failed\n", __FUNCTION__, j);
	error_print();
	return -1;
}

int test_sm9_encrypt_cmd(uint8_t data[],size_t datalen,char id[],size_t idlen) {
	
	SM9_ENC_MASTER_KEY msk;
	SM9_ENC_KEY enc_key;

    //enc_master_key_init(&msk);
    //enc_user_key_init(&enc_key);
	
	ep_null(msk.Ppube);
	ep_new(msk.Ppube);
	bn_null(msk.ke);
	bn_new(msk.ke);
	ep_null(enc_key.Ppube);
	ep_new(enc_key.Ppube);
	ep2_null(enc_key.de);
	ep2_new(enc_key.de);

	ep2_t de;
	ep2_null(de);
    ep2_new(de);

    uint8_t out[1000] = {0};
	size_t outlen = 0;
	int j = 1;

	//uint8_t data[20] = {0x43, 0x68, 0x69, 0x6E, 0x65, 0x73, 0x65, 0x20, 0x49, 0x42, 0x53, 0x20, 0x73, 0x74, 0x61, 0x6E, 0x64, 0x61, 0x72, 0x64};
	uint8_t dec[20] = {0};
	size_t declen = 20;
	//uint8_t IDB[3] = {0x42, 0x6F, 0x62};

    char x1[] = "94736ACD2C8C8796CC4785E938301A139A059D3537B6414140B2D31EECF41683";
	char x0[] = "115BAE85F5D8BC6C3DBD9E5342979ACCCF3C2F4F28420B1CB4F8C0B59A19B158";
	char y1[] = "7AA5E47570DA7600CD760A0CF7BEAF71C447F3844753FE74FA7BA92CA7D3B55F";
    char y0[] = "27538A62E7F7BFB51DCE08704796D94C9D56734F119EA44732B50E31CDEB75C1";
	char z1[] = "0";
    char z0[] = "1";

    char ke[] = "1EDEE3778F441F8DEA3D9FA0ACC4E07EE36C93F9A08618AF4AD85CEDE1C22";
    bn_read_str(msk.ke,ke,strlen(ke),16);
	//sm9_bn_from_hex(msk.ke, hex_ke);
    ep_mul_gen(msk.Ppube,msk.ke);
    //sm9_point_mul_generator(&(msk.Ppube), msk.ke);


	if (sm9_enc_master_key_extract_key(&msk, (char *)id, idlen, &enc_key) < 0) goto err; ++j;
	
    fp_read_str(de->x[0], x0, strlen(x0), 16);
	fp_read_str(de->x[1], x1, strlen(x1), 16);
	fp_read_str(de->y[0], y0, strlen(y0), 16);
	fp_read_str(de->y[1], y1, strlen(y1), 16);
	fp_read_str(de->z[0], z0, strlen(z0), 16);
	fp_read_str(de->z[1], z1, strlen(z1), 16);
    //sm9_twist_point_from_hex(&de, hex_de);
    
    //if (ep2_cmp(enc_key.de,de)) goto err; ++j;

	if (sm9_encrypt(&msk, (char *)id, idlen, data, datalen, out, &outlen) < 0) goto err; ++j;
	format_bytes(stdout, 0, 0, "ciphertext", out, outlen);
    if (sm9_decrypt(&enc_key, (char *)id, idlen, out, outlen, dec, &declen) < 0) goto err; ++j;
	if (memcmp(data, dec, sizeof(data)) != 0) goto err; ++j;
	format_bytes(stdout, 0, 0, "plaintext", dec, declen);
	printf("%s() ok\n", __FUNCTION__);
    ep2_free(de);
	
	ep_free(msk.Ppube);
	bn_free(msk.ke);
	ep_free(enc_key.Ppube);
	ep2_free(enc_key.de);
    //enc_master_key_free(&msk);
    //enc_user_key_free(&enc_key);

	return 1;
err:
    ep2_free(de);
	ep_free(msk.Ppube);
	bn_free(msk.ke);
	ep_free(enc_key.Ppube);
	ep2_free(enc_key.de);

    //enc_master_key_free(&msk);
    //enc_user_key_free(&enc_key);
	printf("%s test %d failed\n", __FUNCTION__, j);
	error_print();
	return -1;
}

#include <stdint.h>
#include <getopt.h>

void print_usage(char *program_name) {
    printf("Usage: %s [-L plaintext_len] [-P plaintext] [-l id_len] [-i id]\n", program_name);
    printf("Options:\n");
	printf("  -L plaintext_len     Specify plaintext_len (int)\n");
    printf("  -P plaintext         Specify plaintext (uint8_t[])\n");
	printf("  -l idlen             Specify idlen (int)\n");
    printf("  -i id                Specify id (uint8_t[])\n");
    printf("  -h                   Print this help message\n");
	printf("EXAMPLE: ./test_sm9_encrypt -L 20 -P \"Chinese IBS Standard\" -l 3 -i \"Bob\"");
}

int main(int argc, char *argv[]) {
    int datalen = 0;
    int idlen = 0;
    uint8_t *data;
    char *id;

    int opt;
    while ((opt = getopt(argc, argv, "P:i:L:l:h")) != -1) {
        switch (opt) {
            case 'P':
                data = (uint8_t *)malloc((datalen) * sizeof(uint8_t));
				memcpy((uint8_t *)data, optarg,datalen);
                if (data == NULL) {
                    printf("Error: failed to allocate memory for plaintext.\n");
                    return 1;
                }
                // Parse data from optarg
                break;
            case 'i':
                id = (char *)malloc(idlen * sizeof(char));
				strcpy((char *)id, optarg);
                if (id == NULL) {
                    printf("Error: failed to allocate memory for id.\n");
                    return 1;
                }
                // Parse id from optarg
                break;
            case 'L':
                datalen = atoi(optarg);
                break;
            case 'l':
                idlen = atoi(optarg);
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    if (datalen <= 0 || idlen <= 0) {
		print_usage(argv[0]);
        printf("Error: datalen and idlen must be greater than 0.\n");
        return 1;
    }

    // Do something with data and id
		if (core_init() != RLC_OK) {
		core_clean();
		return 1;
	}

	if (pc_param_set_any() != RLC_OK) {
		RLC_THROW(ERR_NO_CURVE);
		core_clean();
		return 0;
	}
	
	//uint8_t data1[20] =  "Chinese IBS standard";

	//test_sm9_sign_and_verify(data,sizeof(data),id,strlen(id));

	test_sm9_encrypt_cmd(data,datalen,id,idlen);
	core_clean();

    free(data);
    free(id);

    return 0;
}

/*
int main(){
    if (core_init() != RLC_OK) {
		core_clean();
		return 1;
	}

	if (pc_param_set_any() != RLC_OK) {
		RLC_THROW(ERR_NO_CURVE);
		core_clean();
		return 0;
	}

    test_sm9_encrypt();

    core_clean();
    return 0;
}
*/