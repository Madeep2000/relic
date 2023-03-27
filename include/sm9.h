#ifndef SM9_H
#define SM9_H

#include <stdio.h>
#include <omp.h>

#include "relic.h"

#include "gmssl/sm3.h"

#include "gmssl/error.h"
#include "gmssl/mem.h"
#include "gmssl/asn1.h"

fp_t SM9_ALPHA1, SM9_ALPHA2, SM9_ALPHA3, SM9_ALPHA4, SM9_ALPHA5;
fp2_t SM9_BETA;
#define SM9_N		"B640000002A3A6F1D603AB4FF58EC74449F2934B18EA8BEEE56EE19CD69ECF25"
#define SM9_HID_SIGN		0x01
#define SM9_HID_EXCH		0x02
#define SM9_HID_ENC		0x03


#define SM9_HASH1_PREFIX	0x01
#define SM9_HASH2_PREFIX	0x02

#define SM9_MAX_PLAINTEXT_SIZE 255
#define SM9_MAX_CIPHERTEXT_SIZE 367 

#define SM9_ENC_TYPE_XOR	0
#define SM9_ENC_TYPE_ECB	1
#define SM9_ENC_TYPE_CBC	2
#define SM9_ENC_TYPE_OFB	4
#define SM9_ENC_TYPE_CFB	8

typedef uint64_t sm9_bn_t[8];
typedef uint64_t sm9_barrett_bn_t[9];
typedef sm9_bn_t sm9_fn_t;

typedef struct {
	bn_t h;
	ep_t S;
} SM9_SIGNATURE;


typedef struct {
	ep_t ds;
	ep2_t Ppubs;
} SM9_SIGN_KEY;

typedef struct {
	ep2_t Ppubs; // Ppubs = ks * P2
	bn_t ks;     // sm9_fn_t
} SM9_SIGN_MASTER_KEY;

typedef struct {
	SM3_CTX sm3_ctx;
} SM9_SIGN_CTX;

typedef struct {
	ep_t Ppube; // Ppube = ke * P1
	bn_t ke;
} SM9_ENC_MASTER_KEY;

typedef struct {
	ep_t Ppube;
	ep2_t de;
} SM9_ENC_KEY;

void sm9_init();
void sm9_clean();

//void master_key_init(SM9_SIGN_MASTER_KEY key);
//void master_key_free(SM9_SIGN_MASTER_KEY key);
//void user_key_init(SM9_SIGN_KEY key);
//void user_key_free(SM9_SIGN_KEY key);


// sm9配对算法
void sm9_pairing(fp12_t r, const ep2_t Q, const ep_t P);
void sm9_pairing_fast(fp12_t r, const ep2_t Q, const ep_t P);
void sm9_pairing_fastest(fp12_t r, const ep2_t Q, const ep_t P);

// 运行arr_size次配对算法，使用threads_num个线程运行
void sm9_pairing_omp(fp12_t r_arr[], const ep2_t Q_arr[], const ep_t P_arr[], const size_t arr_size, const size_t threads_num);

// sm9 签名
int sm9_sign_init(SM9_SIGN_CTX *ctx);
int sm9_sign_update(SM9_SIGN_CTX *ctx, const uint8_t *data, size_t datalen);
int sm9_sign_finish(SM9_SIGN_CTX *ctx, const SM9_SIGN_KEY *key, uint8_t *sig, size_t *siglen);
int sm9_do_sign(const SM9_SIGN_KEY *key, const SM3_CTX *sm3_ctx, SM9_SIGNATURE *sig);
int sm9_verify_init(SM9_SIGN_CTX *ctx);
int sm9_verify_update(SM9_SIGN_CTX *ctx, const uint8_t *data, size_t datalen);
int sm9_verify_finish(SM9_SIGN_CTX *ctx, const uint8_t *sig, size_t siglen,	const SM9_SIGN_MASTER_KEY *mpk, const char *id, size_t idlen);

#endif