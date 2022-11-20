#include "sm9.h"
#include "../test/debug.h"
void sm9_init(){
	// beta   = 0x6c648de5dc0a3f2cf55acc93ee0baf159f9d411806dc5177f5b21fd3da24d011
	// alpha1 = 0x3f23ea58e5720bdb843c6cfa9c08674947c5c86e0ddd04eda91d8354377b698b
	// alpha2 = 0xf300000002a3a6f2780272354f8b78f4d5fc11967be65334
	// alpha3 = 0x6c648de5dc0a3f2cf55acc93ee0baf159f9d411806dc5177f5b21fd3da24d011
	// alpha4 = 0xf300000002a3a6f2780272354f8b78f4d5fc11967be65333
	// alpha5 = 0x2d40a38cf6983351711e5f99520347cc57d778a9f8ff4c8a4c949c7fa2a96686
	char beta[] = "6C648DE5DC0A3F2CF55ACC93EE0BAF159F9D411806DC5177F5B21FD3DA24D011";
	char alpha1[] = "3F23EA58E5720BDB843C6CFA9C08674947C5C86E0DDD04EDA91D8354377B698B";
	char alpha2[] = "F300000002A3A6F2780272354F8B78F4D5FC11967BE65334";
	char alpha3[] = "6C648DE5DC0A3F2CF55ACC93EE0BAF159F9D411806DC5177F5B21FD3DA24D011";
	char alpha4[] = "F300000002A3A6F2780272354F8B78F4D5FC11967BE65333";
	char alpha5[] = "2D40A38CF6983351711E5F99520347CC57D778A9F8FF4C8A4C949C7FA2A96686";

	fp2_null(SM9_BETA);
	fp_null(SM9_ALPHA1);
	fp_null(SM9_ALPHA2);
	fp_null(SM9_ALPHA3);
	fp_null(SM9_ALPHA4);
	fp_null(SM9_ALPHA5);

	fp2_new(SM9_BETA);
	fp_new(SM9_ALPHA1);
	fp_new(SM9_ALPHA2);
	fp_new(SM9_ALPHA3);
	fp_new(SM9_ALPHA4);
	fp_new(SM9_ALPHA5);

	fp_read_str(SM9_BETA[0], beta, strlen(beta), 16);
	fp_set_dig(SM9_BETA[1], 0);

	fp_read_str(SM9_ALPHA1, alpha1, strlen(alpha1), 16);
	fp_read_str(SM9_ALPHA2, alpha2, strlen(alpha2), 16);
	fp_read_str(SM9_ALPHA3, alpha3, strlen(alpha3), 16);
	fp_read_str(SM9_ALPHA4, alpha4, strlen(alpha4), 16);
	fp_read_str(SM9_ALPHA5, alpha5, strlen(alpha5), 16);
}

void sm9_clean(){
	fp2_free(SM9_BETA);
	fp_free(SM9_ALPHA1);
	fp_free(SM9_ALPHA2);
	fp_free(SM9_ALPHA3);
	fp_free(SM9_ALPHA4);
	fp_free(SM9_ALPHA5);
}

static void bn_to_bits(const sm9_bn_t a, char bits[256])
{
	int i, j;
	for (i = 7; i >= 0; i--) {
		uint32_t w = a[i];
		for (j = 0; j < 32; j++) {
			*bits++ = (w & 0x80000000) ? '1' : '0';
			w <<= 1;
		}
	}
}

// a*k = (a1, a2)*k = (a1*k, a2*k)
static void fp2_mul_fp(fp2_t r, const fp2_t a, const fp_t k)
{
	fp_mul(r[0], a[0], k);
	fp_mul(r[1], a[1], k);
}

static void fp2_conjugate(fp2_t r, const fp2_t a)
{
	fp_copy(r[0], a[0]);
	fp_neg(r[1], a[1]);
}

static void fp2_mul_u(fp2_t r, const fp2_t a, const fp2_t b)
{
	fp_t r0, r1, t;

	fp_null(r0);
	fp_null(r1);
	fp_null(t);

	fp_new(r0);
	fp_new(r1);
	fp_new(t);

	// r0 = -2 * (a0 * b1 + a1 * b0)
	fp_mul(r0, a[0], b[1]);
	fp_mul(t,  a[1], b[0]);
	fp_add(r0, r0, t);
	fp_dbl(r0, r0);
	fp_neg(r0, r0);

	// r1 = a0 * b0 - 2 * a1 * b1
	fp_mul(r1, a[0], b[0]);
	fp_mul(t, a[1], b[1]);
	fp_dbl(t, t);
	fp_sub(r1, r1, t);

	fp_copy(r[0], r0);
	fp_copy(r[1], r1);

	fp_free(r0);
	fp_free(r1);
	fp_free(t);
}

static void fp2_sqr_u(fp2_t r, const fp2_t a)
{
	fp_t r0, r1, t;

	fp_null(r0);
	fp_null(r1);
	fp_null(t);

	fp_new(r0);
	fp_new(r1);
	fp_new(t);

	// r0 = -4 * a0 * a1
	fp_mul(r0, a[0], a[1]);
	fp_dbl(r0, r0);
	fp_dbl(r0, r0);
	fp_neg(r0, r0);

	// r1 = a0^2 - 2 * a1^2
	fp_sqr(r1, a[0]);
	fp_sqr(t, a[1]);
	fp_dbl(t, t);
	fp_sub(r1, r1, t);

	fp_copy(r[0], r0);
	fp_copy(r[1], r1);

	fp_free(r0);
	fp_free(r1);
	fp_free(t);
}

static void fp4_sqr_v(fp4_t r, const fp4_t a)
{
	fp2_t r0, r1, t;
	fp2_null(r0);
	fp2_null(r1);
	fp2_null(t);

	fp2_new(r0);
	fp2_new(r1);
	fp2_new(t);

	fp2_mul_u(t, a[0], a[1]);
	fp2_dbl(r0, t);

	fp2_sqr(r1, a[0]);
	fp2_sqr_u(t, a[1]);
	fp2_add(r1, r1, t);

	fp2_copy(r[0], r0);
	fp2_copy(r[1], r1);

	fp2_free(r0);
	fp2_free(r1);
	fp2_free(t);
}

static void fp4_mul_v(fp4_t r, const fp4_t a, const fp4_t b)
{
	fp2_t r0, r1, t;
	fp2_null(r0);
	fp2_null(r1);
	fp2_null(t);

	fp2_new(r0);
	fp2_new(r1);
	fp2_new(t);

	fp2_mul_u(r0, a[0], b[1]);
	fp2_mul_u(t, a[1], b[0]);
	fp2_add(r0, r0, t);

	fp2_mul(r1, a[0], b[0]);
	fp2_mul_u(t, a[1], b[1]);
	fp2_add(r1, r1, t);

	fp2_copy(r[0], r0);
	fp2_copy(r[1], r1);

	fp2_free(r0);
	fp2_free(r1);
	fp2_free(t);
}

static void fp4_mul_fp(fp4_t r, const fp4_t a, const fp_t k)
{
	fp2_mul_fp(r[0], a[0], k);
	fp2_mul_fp(r[1], a[1], k);
}

/* 
void sm9_fp2_u(sm9_fp2_t r, const sm9_fp2_t a){
	sm9_fp_copy(r[1], a[0]);
	sm9_fp_dbl(r[0], a[1]);
	sm9_fp_neg(r[0], r[0]);
}
即fp2_mul_nor(fp2_t c,fp2_t a)
*/

/* r = a*v, 即 r = a0v + a1*u  
void sm9_fp4_v(sm9_fp4_t r, const sm9_fp4_t a){
	sm9_fp2_copy(r[1], a[0]);
	sm9_fp2_u(r[0], a[1]);
}
即 fp4_mul_art(fp4_t r,fp4_t a)
*/

/* (a0+a1*v)*b*v = a1*b*u + a0*b*v */
void fp4_mul_fp2_v(fp4_t r, const fp4_t a, const fp2_t b){
	fp2_mul(r[0], a[1], b);
	fp2_mul_nor(r[0],r[0]);
	fp2_mul(r[1], a[0], b);
}

void fp4_mul_fp2(fp4_t r, const fp4_t a, const fp2_t b){
	fp2_mul(r[0], a[0], b);
	fp2_mul(r[1], a[1], b);
}

static void fp4_conjugate(fp4_t r, const fp4_t a)
{
	fp2_copy(r[0], a[0]);
	fp2_neg(r[1], a[1]);
}

void fp12_mul_t1(fp12_t r, const fp12_t a, const fp12_t b)
{
	fp4_t r0, r1, r2, t;

	fp4_null(r0);
	fp4_null(r1);
	fp4_null(r2);
	fp4_null(t);

	fp4_new(r0);
	fp4_new(r1);
	fp4_new(r2);
	fp4_new(t);

	fp4_mul(r0, a[0][0], b[0][0]);
	fp4_mul_v(t, a[0][2], b[1][1]);
	fp4_add(r0, r0, t);
	fp4_mul_v(t, a[1][1], b[0][2]);
	fp4_add(r0, r0, t);

	fp4_mul(r1, a[0][0], b[0][2]);
	fp4_mul(t, a[0][2], b[0][0]);
	fp4_add(r1, r1, t);
	fp4_mul_v(t, a[1][1], b[1][1]);
	fp4_add(r1, r1, t);

	fp4_mul(r2, a[0][0], b[1][1]);
	fp4_mul(t, a[0][2], b[0][2]);
	fp4_add(r2, r2, t);
	fp4_mul(t, a[1][1], b[0][0]);
	fp4_add(r2, r2, t);

	fp4_copy(r[0][0], r0);
	fp4_copy(r[0][2], r1);
	fp4_copy(r[1][1], r2);

	fp4_free(r0);
	fp4_free(r1);
	fp4_free(r2);
	fp4_free(t);
}

static void fp12_mul_unr_t(dv12_t c, fp12_t a, fp12_t b) {
	dv4_t u0, u1, u2, u3, u4;
	fp4_t t0, t1;

	dv4_null(u0);
	dv4_null(u1);
	dv4_null(u2);
	dv4_null(u3);
	dv4_null(u4);
	fp4_null(t0);
	fp4_null(t1);

	RLC_TRY {
		dv4_new(u0);
		dv4_new(u1);
		dv4_new(u2);
		dv4_new(u3);
		dv4_new(u4);
		fp4_new(t0);
		fp4_new(t1);

		/* Karatsuba algorithm. */

		/* u0 = a_0 * b_0. */
		fp4_mul_unr(u0, a[0][0], b[0][0]);
		/* u1 = a_1 * b_1. */
		fp4_mul_unr(u1, a[0][2], b[0][2]);
		/* u2 = a_2 * b_2. */
		fp4_mul_unr(u2, a[1][1], b[1][1]);

		fp4_add(t0, a[0][2], a[1][1]);
		fp4_add(t1, b[0][2], b[1][1]);
		/* u3 = ( a1 + a2 )*( b1 + b2 ) */
		fp4_mul_unr(u3, t0, t1);

		/* u3 = (a1+a2)*(b1+b2) - a1*b1 - a2*b2 = a1*b2 + a2*b1 */
		fp2_subc_low(u3[0], u3[0], u1[0]);
		fp2_subc_low(u3[0], u3[0], u2[0]);
		fp2_subc_low(u3[1], u3[1], u1[1]);
		fp2_subc_low(u3[1], u3[1], u2[1]);

		/* c0 = ( a2*b1 + a1*b2 ) * v + a0*b0 */
		fp2_nord_low(u4[0], u3[1]);
		fp2_addc_low(c[0][0], u4[0], u0[0]);
		fp2_addc_low(c[0][1], u3[0], u0[1]);


		fp4_add(t0, a[0][0], a[0][2]);
		fp4_add(t1, b[0][0], b[0][2]);
		/* u4 = ( a0 + a1 ) * ( b0 + b1 ) */
		fp4_mul_unr(u4, t0, t1);

		/* u4 = ( a0 + a1 ) * ( b0 + b1 ) - a0*b0 - a1*b1 = a0*b1 + a1*b0 */
		for (int i = 0; i < 2; i++) {
			fp2_subc_low(u4[i], u4[i], u0[i]);
			fp2_subc_low(u4[i], u4[i], u1[i]);
		}
		/* c1 = a2*b2*v + a0*b1 + a1*b0 */
		fp2_nord_low(u3[0], u2[1]);
		fp2_addc_low(c[0][2], u4[0], u3[0]);
		fp2_addc_low(c[1][0], u4[1], u2[0]);



		fp4_add(t0, a[0][0], a[1][1]);
		fp4_add(t1, b[0][0], b[1][1]);
		/* u4 = (a0+a2)*(b0+b2) */
		fp4_mul_unr(u4, t0, t1);

		/* c2 = (a0+a2)*(b0+b2) - a0*b0 - a2*b2 + a1b1 = a0b2 + a2b0 + a1b1 */
		for (int i = 0; i < 2; i++) {
			fp2_subc_low(u4[i], u4[i], u0[i]);
			fp2_addc_low(u4[i],u4[i],u1[i]);
			fp2_subc_low(c[1][1+i], u4[i], u2[i]);
		}

	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	} RLC_FINALLY {
		dv4_free(u0);
		dv4_free(u1);
		dv4_free(u2);
		dv4_free(u3);
		dv4_free(u4);
		fp4_free(t0);
		fp4_free(t1);
	}
}

void fp12_mul_t(fp12_t c, fp12_t a, fp12_t b) {
	dv12_t t;

	dv12_null(t);

	RLC_TRY {
		dv12_new(t);
		fp12_mul_unr_t(t, a, b);
		for (int i = 0; i < 3; i++) {
			fp2_rdcn_low(c[0][i], t[0][i]);
			fp2_rdcn_low(c[1][i], t[1][i]);
		}
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	} RLC_FINALLY {
		dv12_free(t);
	}
}


// g is a sparse fp12_t, g = g0 + g2'w^2, g0 = g0' + g3'w^3，g0',g1',g3'都定义在fp2
void fp12_mul_sparse(fp12_t h, const fp12_t f, const fp12_t g){
	fp4_t t0, t1, u0, u1, u2, t, h0, h1, h2;

	// 1. t0 = f0*g0
	fp4_mul(t0, f[0][0], g[0][0]);

	// 2. t1 = Fp4SparseMul(f2, g2')
	fp4_mul_fp2(t1, f[1][1], g[1][1]);

	// 3. u0 = Fp4SparseMul(f1+f2, g2')
	fp4_add(u0, f[0][2], f[1][1]);
	fp4_mul_fp2(u0, u0, g[1][1]);

	// 4. u1 = (f0+f2)*(g0+g2')
	fp4_copy(t, g[0][0]);
	fp2_add(t[0], t[0], g[1][1]);  // t = (g0+g2')
	fp4_add(u1, f[0][0], f[1][1]);  // u1 = (f0+f2)
	fp4_mul(u1, u1, t);

	// 5. u2 = (f0+f1)*g0
	fp4_add(u2, f[0][0], f[0][2]);
	fp4_mul(u2, u2, g[0][0]);

	// 6. h0 = t0 + (u0 - t1)v
	fp4_sub(t, u0, t1);
	fp4_mul_art(h[0][0], t);  // h0 = (u0 - t1)v
	fp4_add(h[0][0], t0, h[0][0]);

	// 7. h1 = u2 - t0 + t1v
	fp4_mul_art(h[0][2], t1);  // h1 = t1v
	fp4_add(h[0][2], h[0][2], u2);
	fp4_sub(h[0][2], h[0][2], t0);

	// 8. h2 = u1 - t0 - t1
	fp4_sub(h[1][1], u1, t0);
	fp4_sub(h[1][1], h[1][1], t1);
}

// r = (a0 + a1*w + a2*w^2)*b3'w^3，其中b3'是fp2上的元素，也就是b0中的高位fp2，即b3'*w^3 = b3'*v
void fp12_mul_sparse2(fp12_t r, fp12_t a, fp12_t b){
	fp4_mul_fp2_v(r[0][0], a[0][0], b[0][1]);
	fp4_mul_fp2_v(r[0][2], a[0][2], b[0][1]);
	fp4_mul_fp2_v(r[1][1], a[1][1], b[0][1]);
}

void fp12_inv_t2(fp12_t c, fp12_t a) {
	fp4_t v0;
	fp4_t v1;
	fp4_t v2;
	fp4_t t0;

	fp4_null(v0);
	fp4_null(v1);
	fp4_null(v2);
	fp4_null(t0);

	RLC_TRY {
		fp4_new(v0);
		fp4_new(v1);
		fp4_new(v2);
		fp4_new(t0);

		/* v0 = a_0^2 - E * a_1 * a_2. */
		fp4_sqr(t0, a[0][0]);
		fp4_mul(v0, a[0][2], a[1][1]);
		fp4_mul_art(v2, v0);
		fp4_sub(v0, t0, v2);

		/* v1 = E * a_2^2 - a_0 * a_1. */
		fp4_sqr(t0, a[1][1]);
		fp4_mul_art(v2, t0);
		fp4_mul(v1, a[0][0], a[0][2]);
		fp4_sub(v1, v2, v1);

		/* v2 = a_1^2 - a_0 * a_2. */
		fp4_sqr(t0, a[0][2]);
		fp4_mul(v2, a[0][0], a[1][1]);
		fp4_sub(v2, t0, v2);

		/* t0 = [a1(a1^2-a0a2)v+a0(a0^2-a1a2v)+a2(a2^2v-a1a2)]^-1*/
		fp4_mul(t0, a[0][2], v2);
		fp4_mul_art(c[0][2], t0);
		fp4_mul(c[0][0], a[0][0], v0);
		fp4_mul(t0, a[1][1], v1);
		fp4_mul_art(c[1][1], t0);
		fp4_add(t0, c[0][0], c[0][2]);
		fp4_add(t0, t0, c[1][1]);
		fp4_inv(t0, t0);

		fp4_mul(c[0][0], v0, t0);
		fp4_mul(c[0][2], v1, t0);
		fp4_mul(c[1][1], v2, t0);
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	} RLC_FINALLY {
		fp4_free(v0);
		fp4_free(v1);
		fp4_free(v2);
		fp4_free(t0);
	}
}

void fp12_inv_t(fp12_t r, fp12_t a){
	RLC_TRY {
	if (fp4_is_zero(a[1][1])) {
		fp4_t k, t;

		fp4_null(k);
		fp4_null(t);

		fp4_new(k);
		fp4_new(t);

		fp4_sqr(k, a[0][0]);
		fp4_mul(k, k, a[0][0]);
		fp4_sqr(t, a[0][2]);
		fp4_mul_art(t,t);
		fp4_mul(t, t, a[0][2]);
		fp4_add(k, k, t);
		fp4_inv(k, k);

		fp4_sqr(r[1][1], a[0][2]);
		fp4_mul(r[1][1], r[1][1], k);

		fp4_mul(r[0][2], a[0][0], a[0][2]);
		fp4_mul(r[0][2], r[0][2], k);
		fp4_neg(r[0][2], r[0][2]);

		fp4_sqr(r[0][0], a[0][0]);
		fp4_mul(r[0][0], r[0][0], k);

		fp4_free(k);
		fp4_free(t);
	} else {
		fp4_t t0, t1, t2, t3;

		fp4_null(t0);
		fp4_null(t1);
		fp4_null(t2);
		fp4_null(t3);

		fp4_new(t0);
		fp4_new(t1);
		fp4_new(t2);
		fp4_new(t3);

		fp4_sqr(t0, a[0][2]);
		fp4_mul(t1, a[0][0], a[1][1]);
		fp4_sub(t0, t0, t1);

		fp4_mul(t1, a[0][0], a[0][2]);
		fp4_sqr(t2, a[1][1]);
		fp4_mul_art(t2,t2);
		fp4_sub(t1, t1, t2);

		fp4_sqr(t2, a[0][0]);
		fp4_mul(t3, a[0][2], a[1][1]);
		fp4_mul_art(t3,t3);
		fp4_sub(t2, t2, t3);

		fp4_sqr(t3, t1);
		fp4_mul(r[0][0], t0, t2);
		fp4_sub(t3, t3, r[0][0]);
		fp4_inv(t3, t3);
		fp4_mul(t3, a[1][1], t3);

		fp4_mul(r[0][0], t2, t3);

		fp4_mul(r[0][2], t1, t3);
		fp4_neg(r[0][2], r[0][2]);

		fp4_mul(r[1][1], t0, t3);
	
		fp4_free(t0);
		fp4_free(t1);
		fp4_free(t2);
		fp4_free(t3);
	}
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	} RLC_FINALLY {
		fp4_free(v0);
		fp4_free(v1);
		fp4_free(v2);
		fp4_free(t0);
	}

}

static void fp12_inv_t1(fp12_t r, const fp12_t a)
{
	if (fp4_is_zero(a[1][1])) {
		fp4_t k, t;

		fp4_null(k);
		fp4_null(t);

		fp4_new(k);
		fp4_new(t);

		fp4_sqr(k, a[0][0]);
		fp4_mul(k, k, a[0][0]);
		fp4_sqr_v(t, a[0][2]);
		fp4_mul(t, t, a[0][2]);
		fp4_add(k, k, t);
		fp4_inv(k, k);

		fp4_sqr(r[1][1], a[0][2]);
		fp4_mul(r[1][1], r[1][1], k);

		fp4_mul(r[0][2], a[0][0], a[0][2]);
		fp4_mul(r[0][2], r[0][2], k);
		fp4_neg(r[0][2], r[0][2]);

		fp4_sqr(r[0][0], a[0][0]);
		fp4_mul(r[0][0], r[0][0], k);

		fp4_free(k);
		fp4_free(t);
	} else {
		fp4_t t0, t1, t2, t3;

		fp4_null(t0);
		fp4_null(t1);
		fp4_null(t2);
		fp4_null(t3);

		fp4_new(t0);
		fp4_new(t1);
		fp4_new(t2);
		fp4_new(t3);

		fp4_sqr(t0, a[0][2]);
		fp4_mul(t1, a[0][0], a[1][1]);
		fp4_sub(t0, t0, t1);

		fp4_mul(t1, a[0][0], a[0][2]);
		fp4_sqr_v(t2, a[1][1]);
		fp4_sub(t1, t1, t2);

		fp4_sqr(t2, a[0][0]);
		fp4_mul_v(t3, a[0][2], a[1][1]);
		fp4_sub(t2, t2, t3);

		fp4_sqr(t3, t1);
		fp4_mul(r[0][0], t0, t2);
		fp4_sub(t3, t3, r[0][0]);
		fp4_inv(t3, t3);
		fp4_mul(t3, a[1][1], t3);

		fp4_mul(r[0][0], t2, t3);

		fp4_mul(r[0][2], t1, t3);
		fp4_neg(r[0][2], r[0][2]);

		fp4_mul(r[1][1], t0, t3);
	
		fp4_free(t0);
		fp4_free(t1);
		fp4_free(t2);
		fp4_free(t3);
	}
}


static void fp12_sqr_unr_t(dv12_t c, fp12_t a) {
	fp4_t t0, t1;
	dv4_t u0, u1, u2, u3, u4;

	fp4_null(t0);
	fp4_null(t1);
	dv4_null(u0);
	dv4_null(u1);
	dv4_null(u2);
	dv4_null(u3);
	dv4_null(u4);

	RLC_TRY {
		fp4_new(t0);
		fp4_new(t1);
		dv4_new(u0);
		dv4_new(u1);
		dv4_new(u2);
		dv4_new(u3);
		dv4_new(u4);

		/* a0 = (a00, a01). */
		/* a1 = (a02, a10). */
		/* a2 = (a11, a12). */

		/* (t0,t1) = a0^2 */
		fp2_copy(t0[0], a[0][0]);
		fp2_copy(t0[1], a[0][1]);
		fp4_sqr_unr(u0, t0);

		/* (t2,t3) = 2 * a1 * a2 */
		fp2_copy(t0[0], a[0][2]);
		fp2_copy(t0[1], a[1][0]);
		fp2_copy(t1[0], a[1][1]);
		fp2_copy(t1[1], a[1][2]);
		fp4_mul_unr(u1, t0, t1);
		fp2_addc_low(u1[0], u1[0], u1[0]);
		fp2_addc_low(u1[1], u1[1], u1[1]);

		/* (t4,t5) = a2^2. */
		fp4_sqr_unr(u2, t1);

		/* c2 = a0 + a2. */
		fp2_addm_low(t1[0], a[0][0], a[1][1]);
		fp2_addm_low(t1[1], a[0][1], a[1][2]);

		/* (t6,t7) = (a0 + a2 + a1)^2. */
		fp2_addm_low(t0[0], t1[0], a[0][2]);
		fp2_addm_low(t0[1], t1[1], a[1][0]);
		fp4_sqr_unr(u3, t0);

		/* c2 = (a0 + a2 - a1)^2. */
		fp2_subm_low(t0[0], t1[0], a[0][2]);
		fp2_subm_low(t0[1], t1[1], a[1][0]);
		fp4_sqr_unr(u4, t0);

		/* c2 = (c2 + (t6,t7))/2. */
#ifdef RLC_FP_ROOM
		fp2_addd_low(u4[0], u4[0], u3[0]);
		fp2_addd_low(u4[1], u4[1], u3[1]);
#else
		fp2_addc_low(u4[0], u4[0], u3[0]);
		fp2_addc_low(u4[1], u4[1], u3[1]);
#endif
		fp_hlvd_low(u4[0][0], u4[0][0]);
		fp_hlvd_low(u4[0][1], u4[0][1]);
		fp_hlvd_low(u4[1][0], u4[1][0]);
		fp_hlvd_low(u4[1][1], u4[1][1]);

		/* (t6,t7) = (t6,t7) - c2 - (t2,t3). */
		fp2_subc_low(u3[0], u3[0], u4[0]);
		fp2_subc_low(u3[1], u3[1], u4[1]);
		fp2_subc_low(u3[0], u3[0], u1[0]);
		fp2_subc_low(u3[1], u3[1], u1[1]);

		/* c2 = c2 - (t0,t1) - (t4,t5). */
		fp2_subc_low(u4[0], u4[0], u0[0]);
		fp2_subc_low(u4[1], u4[1], u0[1]);
		fp2_subc_low(c[1][1], u4[0], u2[0]);
		fp2_subc_low(c[1][2], u4[1], u2[1]);

		/* c1 = (t6,t7) + (t4,t5) * E. */
		fp2_nord_low(u4[1], u2[1]);
		fp2_addc_low(c[0][2], u3[0], u4[1]);
		fp2_addc_low(c[1][0], u3[1], u2[0]);

		/* c0 = (t0,t1) + (t2,t3) * E. */
		fp2_nord_low(u4[1], u1[1]);
		fp2_addc_low(c[0][0], u0[0], u4[1]);
		fp2_addc_low(c[0][1], u0[1], u1[0]);
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	} RLC_FINALLY {
		fp4_free(t0);
		fp4_free(t1);
		dv4_free(u0);
		dv4_free(u1);
		dv4_free(u2);
		dv4_free(u3);
		dv4_free(u4);
	}
}

void fp12_sqr_t(fp12_t c, const fp12_t a) {
	dv12_t t;

	dv12_null(t);

	RLC_TRY {
		dv12_new(t);
		fp12_sqr_unr_t(t, a);
		for (int i = 0; i < 3; i++) {
			fp2_rdcn_low(c[0][i], t[0][i]);
			fp2_rdcn_low(c[1][i], t[1][i]);
		}
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	} RLC_FINALLY {
		dv12_free(t);
	}
}

static void fp12_sqr_t1(fp12_t r, const fp12_t a)
{
	fp4_t r0, r1, r2, t;

	fp4_null(r0);
	fp4_null(r1);
	fp4_null(r2);
	fp4_null(t);

	fp4_new(r0);
	fp4_new(r1);
	fp4_new(r2);
	fp4_new(t);

	fp4_sqr(r0, a[0][0]);
	fp4_mul_v(t, a[0][2], a[1][1]);
	fp4_dbl(t, t);
	fp4_add(r0, r0, t);

	fp4_mul(r1, a[0][0], a[0][2]);
	fp4_dbl(r1, r1);
	fp4_sqr_v(t, a[1][1]);
	fp4_add(r1, r1, t);

	fp4_mul(r2, a[0][0], a[1][1]);
	fp4_dbl(r2, r2);
	fp4_sqr(t, a[0][2]);
	fp4_add(r2, r2, t);

	fp4_copy(r[0][0], r0);
	fp4_copy(r[0][2], r1);
	fp4_copy(r[1][1], r2);

	fp4_free(r0);
	fp4_free(r1);
	fp4_free(r2);
	fp4_free(t);
}

static void fp12_set(fp12_t r, const fp4_t a0, const fp4_t a1, const fp4_t a2)
{
	fp4_copy(r[0][0], a0);
	fp4_copy(r[0][2], a1);
	fp4_copy(r[1][1], a2);
}

static void fp12_pow(fp12_t r, const fp12_t a, const sm9_bn_t k)
{
	char kbits[257];
	fp12_t t;
	int i;

	fp12_null(t);
	fp12_new(t);

	// assert(sm9_bn_cmp(k, SM9_P_MINUS_ONE) < 0);
	fp12_set_dig(t, 0);
	
	bn_to_bits(k, kbits);
	fp12_set_dig(t, 1);
	for (i = 0; i < 256; i++) {
		fp12_sqr_t(t, t);
		if (kbits[i] == '1') {
			fp12_mul_t(t, t, a);
		}
	}
	fp12_copy(r, t);

	fp12_free(t);
}

void fp12_pow_t(fp12_t c, fp12_t a, bn_t b) {
	fp12_t t;

	if (bn_is_zero(b)) {
		fp12_set_dig(c, 1);
		return ;
	}

	fp12_null(t);

	RLC_TRY {
		fp12_new(t);

		if (fp12_test_cyc(a)) {
			fp12_exp_cyc(c, a, b);
		} else {
			fp12_copy(t, a);

			for (int i = bn_bits(b) - 2; i >= 0; i--) {
				fp12_sqr_t(t, t);
				if (bn_get_bit(b, i)) {
					fp12_mul_t(t, t, a);
				}
			}

			if (bn_sign(b) == RLC_NEG) {
				fp12_inv_t(c, t);
			} else {
				fp12_copy(c, t);
			}
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		fp12_free(t);
	}
}

static void fp12_frobenius(fp12_t r, const fp12_t x)
{

	const fp2_t *xa = x[0][0];
	const fp2_t *xb = x[0][2];
	const fp2_t *xc = x[1][1];
	fp4_t ra;
	fp4_t rb;
	fp4_t rc;

	fp4_null(ra);
	fp4_null(rb);
	fp4_null(rc);

	fp4_new(ra);
	fp4_new(rb);
	fp4_new(rc);

	fp2_conjugate(ra[0], xa[0]);
	fp2_conjugate(ra[1], xa[1]);
	fp2_mul_fp(ra[1], ra[1], SM9_ALPHA3);

	fp2_conjugate(rb[0], xb[0]);
	fp2_mul_fp(rb[0], rb[0], SM9_ALPHA1);
	fp2_conjugate(rb[1], xb[1]);
	fp2_mul_fp(rb[1], rb[1], SM9_ALPHA4);

	fp2_conjugate(rc[0], xc[0]);
	fp2_mul_fp(rc[0], rc[0], SM9_ALPHA2);
	fp2_conjugate(rc[1], xc[1]);
	fp2_mul_fp(rc[1], rc[1], SM9_ALPHA5);

	fp12_set(r, ra, rb, rc);

	fp4_free(ra);
	fp4_free(rb);
	fp4_free(rc);
}

static void fp12_frobenius2(fp12_t r, const fp12_t x)
{
	fp4_t a;
	fp4_t b;
	fp4_t c;
	
	fp4_null(a);
	fp4_null(b);
	fp4_null(c);

	fp4_new(a);
	fp4_new(b);
	fp4_new(c);
	
	fp4_conjugate(a, x[0][0]);
	fp4_conjugate(b, x[0][2]);
	fp4_mul_fp(b, b, SM9_ALPHA2);
	fp4_conjugate(c, x[1][1]);
	fp4_mul_fp(c, c, SM9_ALPHA4);

	fp4_copy(r[0][0], a);
	fp4_copy(r[0][2], b);
	fp4_copy(r[1][1], c);

	fp4_free(a);
	fp4_free(b);
	fp4_free(c);
}

static void fp12_frobenius3(fp12_t r, const fp12_t x)
{
	const fp2_t *xa = x[0][0];
	const fp2_t *xb = x[0][2];
	const fp2_t *xc = x[1][1];
	fp4_t ra;
	fp4_t rb;
	fp4_t rc;

	fp4_null(ra);
	fp4_null(rb);
	fp4_null(rc);

	fp4_new(ra);
	fp4_new(rb);
	fp4_new(rc);

	fp2_conjugate(ra[0], xa[0]);
	fp2_conjugate(ra[1], xa[1]);
	fp2_mul(ra[1], ra[1], SM9_BETA);
	fp2_neg(ra[1], ra[1]);

	fp2_conjugate(rb[0], xb[0]);
	fp2_mul(rb[0], rb[0], SM9_BETA);
	fp2_conjugate(rb[1], xb[1]);

	fp2_conjugate(rc[0], xc[0]);
	fp2_neg(rc[0], rc[0]);
	fp2_conjugate(rc[1], xc[1]);
	fp2_mul(rc[1], rc[1], SM9_BETA);

	fp4_copy(r[0][0], ra);
	fp4_copy(r[0][2], rb);
	fp4_copy(r[1][1], rc);

	fp4_free(ra);
	fp4_free(rb);
	fp4_free(rc);
}

static void fp12_frobenius6(fp12_t r, const fp12_t x)
{
	fp4_t a;
	fp4_t b;
	fp4_t c;

	fp4_null(a);
	fp4_null(b);
	fp4_null(c);

	fp4_new(a);
	fp4_new(b);
	fp4_new(c);

	fp4_copy(a, x[0][0]);
	fp4_copy(b, x[0][2]);
	fp4_copy(c, x[1][1]);

	fp4_conjugate(a, a);
	fp4_conjugate(b, b);
	fp4_neg(b, b);
	fp4_conjugate(c, c);

	fp4_copy(r[0][0], a);
	fp4_copy(r[0][2], b);
	fp4_copy(r[1][1], c);

	fp4_free(a);
	fp4_free(b);
	fp4_free(c);
}

static void ep2_pi1(ep2_t R, const ep2_t P)
{
	//const c = 0x3f23ea58e5720bdb843c6cfa9c08674947c5c86e0ddd04eda91d8354377b698bn;
	fp_t c;
	fp_null(c);
	fp_new(c);

	char c_str[] = "3F23EA58E5720BDB843C6CFA9C08674947C5C86E0DDD04EDA91D8354377B698B";

	fp_read_str(c, c_str, strlen(c_str), 16);

	fp2_conjugate(R->x, P->x);  // X[0], -X[1]
	fp2_conjugate(R->y, P->y);
	fp2_conjugate(R->z, P->z);
	fp2_mul_fp(R->z, R->z, c);

	fp_free(c);
}

static void ep2_pi2(ep2_t R, const ep2_t P)
{
	//c = 0xf300000002a3a6f2780272354f8b78f4d5fc11967be65334
	fp_t c;
	fp_null(c);
	fp_new(c);

	char c_str[] = "F300000002A3A6F2780272354F8B78F4D5FC11967BE65334";

	fp_read_str(c, c_str, strlen(c_str), 16);

	fp2_copy(R->x, P->x);
	fp2_neg(R->y, P->y);
	fp2_mul_fp(R->z, P->z, c);

	fp_free(c);
}
/* 即ep2_add */
void ep2_add_full(ep2_t R, ep2_t P, ep2_t Q)
{
	const fp_t *X1 = P->x;
	const fp_t *Y1 = P->y;
	const fp_t *Z1 = P->z;
	const fp_t *X2 = Q->x;
	const fp_t *Y2 = Q->y;
	const fp_t *Z2 = Q->z;
	fp2_t T1, T2, T3, T4, T5, T6, T7, T8;

	fp2_null(T1);
	fp2_null(T2);
	fp2_null(T3);
	fp2_null(T4);
	fp2_null(T5);
	fp2_null(T6);
	fp2_null(T7);
	fp2_null(T8);

	fp2_new(T1);
	fp2_new(T2);
	fp2_new(T3);
	fp2_new(T4);
	fp2_new(T5);
	fp2_new(T6);
	fp2_new(T7);
	fp2_new(T8);

	if (ep2_is_infty(Q)) {
		ep2_copy(R, P);
		return ;
	}
	if (ep2_is_infty(P)) {
		ep2_copy(R, Q);
		return ;
	}

	fp2_sqr(T1, Z1);
	fp2_sqr(T2, Z2);
	fp2_mul(T3, X2, T1);
	fp2_mul(T4, X1, T2);
	fp2_add(T5, T3, T4);
	fp2_sub(T3, T3, T4);
	fp2_mul(T1, T1, Z1);
	fp2_mul(T1, T1, Y2);
	fp2_mul(T2, T2, Z2);
	fp2_mul(T2, T2, Y1);
	fp2_add(T6, T1, T2);
	fp2_sub(T1, T1, T2);

	if (fp2_is_zero(T1) && fp2_is_zero(T3)) {
		return ep2_dbl_projc(R, P);
	}
	
	if (fp2_is_zero(T1) && fp2_is_zero(T6)) {
		return ep2_set_infty(R);
	}

	fp2_sqr(T6, T1);
	fp2_mul(T7, T3, Z1);
	fp2_mul(T7, T7, Z2);
	fp2_sqr(T8, T3);
	fp2_mul(T5, T5, T8);
	fp2_mul(T3, T3, T8);
	fp2_mul(T4, T4, T8);
	fp2_sub(T6, T6, T5);
	fp2_sub(T4, T4, T6);
	fp2_mul(T1, T1, T4);
	fp2_mul(T2, T2, T3);
	fp2_sub(T1, T1, T2);

	fp2_copy(R->x, T6);
	fp2_copy(R->y, T1);
	fp2_copy(R->z, T7);

	fp2_free(T1);
	fp2_free(T2);
	fp2_free(T3);
	fp2_free(T4);
	fp2_free(T5);
	fp2_free(T6);
	fp2_free(T7);
	fp2_free(T8);
}

/* 特殊加法 当Q.Z =1时适用 */
void ep2_add_t(ep2_t R, ep2_t P, ep2_t Q){
	
	const fp_t *X1 = P->x;
	const fp_t *Y1 = P->y;
	const fp_t *Z1 = P->z;
	const fp_t *X2 = Q->x;
	const fp_t *Y2 = Q->y;
	fp2_t T1, T2, T3, T4, X3,Y3,Z3;

	fp2_null(T1);
	fp2_null(T2);
	fp2_null(T3);
	fp2_null(T4);
	fp2_null(X3);
	fp2_null(Y3);
	fp2_null(Z3);

	fp2_new(T1);
	fp2_new(T2);
	fp2_new(T3);
	fp2_new(T4);
	fp2_new(X3);
	fp2_new(Y3);
	fp2_new(Z3);

	if (ep2_is_infty(Q)) {
		ep2_copy(R, P);
		return ;
	}
	if (ep2_is_infty(P)) {
		ep2_copy(R, Q);
		return ;
	}

	fp2_sqr(T1, Z1);
	fp2_mul(T2, T1, Z1);
	fp2_mul(T1, T1, X2);
	fp2_mul(T2, T2, Y2);
	fp2_sub(T1, T1, X1);
	fp2_sub(T2, T2, Y1);

	if (fp2_is_zero(T1)) {
		if(fp2_is_zero(T2)){
			ep2_dbl_projc(R, Q);
			return ;
		}
		else{
			ep2_set_infty(R);
			return ;
		}
	}

	fp2_mul(Z3, Z1, T1);
	fp2_sqr(T3, T1);
	fp2_mul(T4, T3, T1);
	fp2_mul(T3, T3, X1);
	fp2_dbl(T1, T3);
	fp2_sqr(X3, T2);
	fp2_sub(X3, X3, T1);
	fp2_sub(X3, X3, T4);
	fp2_sub(T3, T3, X3);
	fp2_mul(T3, T3, T2);
	fp2_mul(T4, T4, Y1);
	fp2_sub(Y3, T3, T4);

	fp2_copy(R->x, X3);
	fp2_copy(R->y, Y3);
	fp2_copy(R->z, Z3);

	fp2_free(T1);
	fp2_free(T2);
	fp2_free(T3);
	fp2_free(T4);
	fp2_free(X3);
	fp2_free(Y3);
	fp2_free(Z3);
}

static void sm9_eval_g_line(fp12_t num, fp12_t den, ep2_t T, ep2_t P, ep_t Q){
	const fp_t *XP = P->x;
	const fp_t *YP = P->y;
	const fp_t *ZP = P->z;
	const fp_t *XT = T->x;
	const fp_t *YT = T->y;
	const fp_t *ZT = T->z;
	const uint64_t *xQ = Q->x;
	const uint64_t *yQ = Q->y;

	fp_t *a0 = num[0][0];
	fp_t *a1 = num[0][1];
	fp_t *a4 = num[1][1];
	fp_t *b1 = den[0][1];

	fp2_t T0, T1, T2, T3, T4;
	fp2_t two_inv;
	bn_t three;

	fp2_null(T0);
	fp2_null(T1);
	fp2_null(T2);
	fp2_null(T3);
	fp2_null(T4);

	fp2_null(two_inv);
	bn_null(three);

	fp2_new(T0);
	fp2_new(T1);
	fp2_new(T2);
	fp2_new(T3);
	fp2_new(T4);
	fp2_new(two_inv);
	bn_new(three);

	fp12_set_dig(num, 0);
	fp12_set_dig(den, 0);
	fp2_set_dig(two_inv, 2);
	fp2_inv(two_inv, two_inv);
	bn_set_dig(three, 3);

	fp2_sqr(T0, ZP);
	fp2_mul(T1, T0, XT);
	fp2_mul(T0, T0, ZP);
	fp2_sqr(T2, ZT);
	fp2_mul(T3, T2, XP);
	fp2_mul(T2, T2, ZT);
	fp2_mul(T2, T2, YP);
	fp2_sub(T1, T1, T3);
	fp2_mul(T1, T1, ZT);
	fp2_mul(T1, T1, ZP);
	fp2_mul(T4, T1, T0);
	fp2_copy(b1, T4);

	fp2_mul(T1, T1, YP);
	fp2_mul(T3, T0, YT);
	fp2_sub(T3, T3, T2);
	fp2_mul(T0, T0, T3);
	fp2_mul_fp(T0, T0, xQ);
	fp2_copy(a4, T0);

	fp2_mul(T3, T3, XP);
	fp2_mul(T3, T3, ZP);
	fp2_sub(T1, T1, T3);
	fp2_copy(a0, T1);

	fp2_mul_fp(T2, T4, yQ);
	fp2_neg(T2, T2);
	fp2_copy(a1, T2);

	fp2_free(T0);
	fp2_free(T1);
	fp2_free(T2);
	fp2_free(T3);
	fp2_free(T4);
	fp2_free(two_inv);
	bn_free(three);
}

void sm9_eval_g_tangent(fp12_t num, fp12_t den, ep2_t P, ep_t Q){
	// fp_t *x, *y;
	// x = Q->x;
	// y = Q->y;
	const fp_t *XP = P->x;
	const fp_t *YP = P->y;
	const fp_t *ZP = P->z;
	const uint64_t *xQ = Q->x;
	const uint64_t *yQ = Q->y;

	fp_t *a0 = num[0][0];
	fp_t *a1 = num[0][1];
	fp_t *a4 = num[1][1];
	fp_t *b1 = den[0][1];

	fp2_t t0;
	fp2_t t1;
	fp2_t t2;
	fp2_t two_inv;
	bn_t three;

	fp2_null(t0);
	fp2_null(t1);
	fp2_null(t2);
	fp2_null(two_inv);
	bn_null(three);

	fp2_new(t0);
	fp2_new(t1);
	fp2_new(t2);
	fp2_new(two_inv);
	bn_new(three);

	fp12_set_dig(num, 0);
	fp12_set_dig(den, 0);
	fp2_set_dig(two_inv, 2);
	fp2_inv(two_inv, two_inv);
	bn_set_dig(three, 3);
	
	fp2_sqr(t0, ZP);
	fp2_mul(t1, t0, ZP);
	fp2_mul(b1, t1, YP);
	fp2_mul_fp(t2, b1, yQ);
	fp2_neg(a1, t2);
	fp2_sqr(t1, XP);
	fp2_mul(t0, t0, t1);
	fp2_mul_fp(t0, t0, xQ);
	fp2_mul_dig(t0, t0, 3);
	fp2_mul(a4, t0, two_inv);
	fp2_mul(t1, t1, XP);
	fp2_mul_dig(t1, t1, 3);
	fp2_mul(t1, t1, two_inv);
	fp2_sqr(t0, YP);
	fp2_sub(a0, t0, t1);

	fp2_free(t0);
	fp2_free(t1);
	fp2_free(t2);
	fp2_free(two_inv);
	bn_free(three);
}

static void sm9_final_exponent_hard_part(fp12_t r, const fp12_t f)
{
	// a2 = 0xd8000000019062ed0000b98b0cb27659
	// a3 = 0x2400000000215d941
	const sm9_bn_t a2 = {0xcb27659, 0x0000b98b, 0x019062ed, 0xd8000000, 0, 0, 0, 0};
	const sm9_bn_t a3 = {0x215d941, 0x40000000, 0x2, 0, 0, 0, 0, 0};
	const sm9_bn_t nine = {9,0,0,0,0,0,0,0};
	fp12_t t0, t1, t2, t3;

	fp12_null(t0);
	fp12_null(t1);
	fp12_null(t2);
	fp12_null(t3);

	fp12_new(t0);
	fp12_new(t1);
	fp12_new(t2);
	fp12_new(t3);

	fp12_pow(t0, f, a3);
	// PERFORMANCE_TEST("fp12_pow(t0, f, a3)",fp12_pow(t0, f, a3),1000);
	fp12_inv_t(t0, t0);
	// PERFORMANCE_TEST("fp12_inv_t(t0, t0)",fp12_inv_t(t0, t0),1000);
	fp12_frobenius(t1, t0);
	// PERFORMANCE_TEST("fp12_frobenius(t1, t0)",fp12_frobenius(t1, t0),1000);
	fp12_mul_t(t1, t0, t1);

	fp12_mul_t(t0, t0, t1);
	fp12_frobenius(t2, f);
	fp12_mul_t(t3, t2, f);
	fp12_pow(t3, t3, nine);

	fp12_mul_t(t0, t0, t3);
	fp12_sqr_t(t3, f);
	fp12_sqr_t(t3, t3);
	fp12_mul_t(t0, t0, t3);
	fp12_sqr_t(t2, t2);
	fp12_mul_t(t2, t2, t1);
	fp12_frobenius2(t1, f);
	// PERFORMANCE_TEST("fp12_frobenius2(t1, f)",fp12_frobenius2(t1, f),1000);
	fp12_mul_t(t1, t1, t2);

	fp12_pow(t2, t1, a2);
	// PERFORMANCE_TEST("fp12_pow(t2, t1, a2)",fp12_pow(t2, t1, a2),1000);
	fp12_mul_t(t0, t2, t0);
	fp12_frobenius3(t1, f);
	fp12_mul_t(t1, t1, t0);

	fp12_copy(r, t1);

	fp12_free(t0);
	fp12_free(t1);
	fp12_free(t2);
	fp12_free(t3);
}

static void sm9_final_exponent(fp12_t r, const fp12_t f)
{
	fp12_t t0;
	fp12_t t1;

	fp12_null(t0);
	fp12_null(t1);

	fp12_new(t0);
	fp12_new(t1);

	fp12_frobenius6(t0, f);
	// PERFORMANCE_TEST("fp12_frobenius6",fp12_frobenius6(t0, f),1000);
	fp12_inv_t(t1, f);

	fp12_mul_t(t0, t0, t1);

	fp12_frobenius2(t1, t0);
	// PERFORMANCE_TEST("fp12_frobenius2",fp12_frobenius2(t1, t0),1000);
	fp12_mul_t(t0, t0, t1);

	sm9_final_exponent_hard_part(t0, t0);
	// PERFORMANCE_TEST("sm9_final_exponent_hard_part",sm9_final_exponent_hard_part(t0, t0),1000);
	fp12_copy(r, t0);
	
	fp12_free(t0);
	fp12_free(t1);
}

#if 1 // crude final exponent

static void sm9_final_exponent_hard_part1(fp12_t r, const fp12_t f)
{
	// a2 = 0xd8000000019062ed0000b98b0cb27659
	// a3 = 0x2400000000215d941
	const sm9_bn_t a2 = {0xcb27659, 0x0000b98b, 0x019062ed, 0xd8000000, 0, 0, 0, 0};
	const sm9_bn_t a3 = {0x215d941, 0x40000000, 0x2, 0, 0, 0, 0, 0};
	const sm9_bn_t nine = {9,0,0,0,0,0,0,0};
	fp12_t t0, t1, t2, t3;

	fp12_null(t0);
	fp12_null(t1);
	fp12_null(t2);
	fp12_null(t3);

	fp12_new(t0);
	fp12_new(t1);
	fp12_new(t2);
	fp12_new(t3);

	fp12_pow(t0, f, a3);
	// PERFORMANCE_TEST("fp12_pow(t0, f, a3)",fp12_pow(t0, f, a3),1000);
	fp12_inv_t(t0, t0);
	// PERFORMANCE_TEST("fp12_inv_t(t0, t0)",fp12_inv_t(t0, t0),1000);
	fp12_frobenius(t1, t0);
	// PERFORMANCE_TEST("fp12_frobenius(t1, t0)",fp12_frobenius(t1, t0),1000);
	fp12_mul_t1(t1, t0, t1);

	fp12_mul_t1(t0, t0, t1);
	fp12_frobenius(t2, f);
	fp12_mul_t1(t3, t2, f);
	fp12_pow(t3, t3, nine);

	fp12_mul_t1(t0, t0, t3);
	fp12_sqr_t1(t3, f);
	fp12_sqr_t1(t3, t3);
	fp12_mul_t1(t0, t0, t3);
	fp12_sqr_t1(t2, t2);
	fp12_mul_t1(t2, t2, t1);
	fp12_frobenius2(t1, f);
	// PERFORMANCE_TEST("fp12_frobenius2(t1, f)",fp12_frobenius2(t1, f),1000);
	fp12_mul_t1(t1, t1, t2);

	fp12_pow(t2, t1, a2);
	// PERFORMANCE_TEST("fp12_pow(t2, t1, a2)",fp12_pow(t2, t1, a2),1000);
	fp12_mul_t1(t0, t2, t0);
	fp12_frobenius3(t1, f);
	fp12_mul_t1(t1, t1, t0);

	fp12_copy(r, t1);

	fp12_free(t0);
	fp12_free(t1);
	fp12_free(t2);
	fp12_free(t3);
}

static void sm9_final_exponent1(fp12_t r, const fp12_t f)
{
	fp12_t t0;
	fp12_t t1;

	fp12_null(t0);
	fp12_null(t1);

	fp12_new(t0);
	fp12_new(t1);

	fp12_frobenius6(t0, f);
	// PERFORMANCE_TEST("fp12_frobenius6",fp12_frobenius6(t0, f),1000);
	fp12_inv_t(t1, f);

	fp12_mul_t1(t0, t0, t1);

	fp12_frobenius2(t1, t0);
	// PERFORMANCE_TEST("fp12_frobenius2",fp12_frobenius2(t1, t0),1000);
	fp12_mul_t1(t0, t0, t1);

	sm9_final_exponent_hard_part1(t0, t0);
	// PERFORMANCE_TEST("sm9_final_exponent_hard_part",sm9_final_exponent_hard_part(t0, t0),1000);
	fp12_copy(r, t0);
	
	fp12_free(t0);
	fp12_free(t1);
}

#endif


static void sm9_twist_point_neg(ep2_t R,const ep2_t Q){
	fp2_copy(R->x, Q->x);
	fp2_neg(R->y, Q->y);
	fp2_copy(R->z, Q->z);
}


//original pairing 
void sm9_pairing(fp12_t r, const ep2_t Q, const ep_t P){
	// a)
	const char *abits = "00100000000000000000000000000000000000010000101011101100100111110";
	// const char *abits = "1";
	
	fp12_t f, g, f_num, f_den, g_num, g_den, fp12_tmp;
	ep2_t T, Q1, Q2, ep2_tmp, ep2_tmp2;
	ep_t _p;
	bn_t n;

	// null
	ep_null(_p);
	bn_null(n);
	ep2_null(T);
	ep2_null(Q1);
	ep2_null(Q2);
	ep2_null(ep2_tmp);
	ep2_null(ep2_tmp2);
	fp12_null(f);
	fp12_null(g);
	fp12_null(f_num);
	fp12_null(f_den);
	fp12_null(g_num);
	fp12_null(g_den);
	fp12_null(fp12_tmp);

	ep_new(_p);
	bn_new(n);
	ep2_new(T);
	ep2_new(Q1);
	ep2_new(Q2);
	ep2_new(ep2_tmp);
	ep2_new(ep2_tmp2);
	fp12_new(f);
	fp12_new(g);
	fp12_new(f_num);
	fp12_new(f_den);
	fp12_new(g_num);
	fp12_new(g_den);
	fp12_new(fp12_tmp);

	// b)
	ep2_copy(T, Q);
	fp12_set_dig(f_num, 1);
	fp12_set_dig(f_den, 1);

	for(size_t i = 0; i < strlen(abits); i++)
	{
		// c)
		fp12_sqr_t1(f_num, f_num);
		fp12_sqr_t1(f_den, f_den);

		sm9_eval_g_tangent(g_num, g_den, T, P);
		// PERFORMANCE_TEST("sm9_eval_g_tangent",sm9_eval_g_tangent(g_num, g_den, T, P),10000);

		fp12_mul_t1(f_num, f_num, g_num);
		fp12_mul_t1(f_den, f_den, g_den);

		ep2_dbl_projc(T, T);
		// c.2)
		if (abits[i] == '1')
		{
			sm9_eval_g_line(g_num, g_den, T, Q, P);
			// PERFORMANCE_TEST("sm9_eval_g_line",sm9_eval_g_line(g_num, g_den, T, Q, P),10000);

			fp12_mul_t1(f_num, f_num, g_num);
			fp12_mul_t1(f_den, f_den, g_den);

			ep2_add_full(T, T, Q);  // T = T + Q
		}
	}
	// d)
	ep2_pi1(Q1, Q);  // Q1 = pi_q(Q)
	ep2_pi2(Q2, Q);  // Q2 = pi_{q^2}(Q), Q2 = -Q2
	
	// e)
	sm9_eval_g_line(g_num, g_den, T, Q1, P);  // g = g_{T,Q1}(P)
	fp12_mul_t1(f_num, f_num, g_num);  // f = f * g = f * g_{T,Q1}(P)
	fp12_mul_t1(f_den, f_den, g_den);
	ep2_add_full(T, T, Q1);  // T = T + Q1

	// f)
	sm9_eval_g_line(g_num, g_den, T, Q2, P);  // g = g_{T,-Q2}(P)
	fp12_mul_t1(f_num, f_num, g_num);  // f = f * g = f * g_{T,-Q2}(P)
	fp12_mul_t1(f_den, f_den, g_den);
	ep2_add_full(T, T, Q2);  // T = T - Q2

	// g)
	fp12_inv_t(f_den, f_den);  // f_den = f_den^{-1}

	fp12_mul_t1(r, f_num, f_den);  // r = f_num*f_den = f

	sm9_final_exponent1(r, r);  // r = f^{(q^12-1)/r'}
	// PERFORMANCE_TEST("sm9_final_exponent", sm9_final_exponent(r, r), 1000);

	ep_free(_p);
	bn_free(n);
	ep2_free(T);
	ep2_free(Q1);
	ep2_free(Q2);
	ep2_free(ep2_tmp);
	ep2_free(ep2_tmp2);
	fp12_free(f);
	fp12_free(g);
	fp12_free(f_num);
	fp12_free(f_den);
	fp12_free(g_num);
	fp12_free(g_den);
	fp12_free(fp12_tmp);

	return ;
}

void sm9_pairing_fast(fp12_t r, const ep2_t Q, const ep_t P){
	// a)
	const char *abits = "00100000000000000000000000000000000000010001020200020200101000020";
	
	fp12_t f, g, f_num, f_den, g_num, g_den, fp12_tmp;
	ep2_t T, Q1, Q2, ep2_tmp, neg_Q;
	ep_t _p;
	bn_t n;

	// null
	ep_null(_p);
	bn_null(n);
	ep2_null(T);
	ep2_null(Q1);
	ep2_null(Q2);
	ep2_null(ep2_tmp);
	ep2_null(neg_Q);
	fp12_null(f);
	fp12_null(g);
	fp12_null(f_num);
	fp12_null(f_den);
	fp12_null(g_num);
	fp12_null(g_den);
	fp12_null(fp12_tmp);

	ep_new(_p);
	bn_new(n);
	ep2_new(T);
	ep2_new(Q1);
	ep2_new(Q2);
	ep2_new(ep2_tmp);
	ep2_new(neg_Q);
	fp12_new(f);
	fp12_new(g);
	fp12_new(f_num);
	fp12_new(f_den);
	fp12_new(g_num);
	fp12_new(g_den);
	fp12_new(fp12_tmp);

	sm9_twist_point_neg(neg_Q,Q);


	// b)
	ep2_copy(T, Q);
	fp12_set_dig(f_num, 1);
	fp12_set_dig(f_den, 1);

	for(size_t i = 0; i < strlen(abits); i++)
	{
		// c)
		fp12_sqr_t(f_num, f_num);
		fp12_sqr_t(f_den, f_den);

		sm9_eval_g_tangent(g_num, g_den, T, P);
		// PERFORMANCE_TEST("sm9_eval_g_tangent",sm9_eval_g_tangent(g_num, g_den, T, P),10000);

		fp12_mul_t(f_num, f_num, g_num);
		fp12_mul_t(f_den, f_den, g_den);

		ep2_dbl_projc(T, T);
		// c.2)
		if (abits[i] == '1'){
			sm9_eval_g_line(g_num, g_den, T, Q, P);
			// PERFORMANCE_TEST("sm9_eval_g_line",sm9_eval_g_line(g_num, g_den, T, Q, P),10000);
			fp12_mul_t(f_num, f_num, g_num);
			fp12_mul_t(f_den, f_den, g_den);
			ep2_add_t(T, T, Q);  // T = T + Q
		}
		else if(abits[i] == '2'){
			sm9_eval_g_line(g_num, g_den, T, neg_Q, P);
			fp12_mul_t(f_num, f_num, g_num);
			fp12_mul_t(f_den, f_den, g_den);
			ep2_add_t(T, T, neg_Q);  // T = T - Q
		}
	}
	// d)
	ep2_pi1(Q1, Q);  // Q1 = pi_q(Q)
	ep2_pi2(Q2, Q);  // Q2 = pi_{q^2}(Q), Q2 = -Q2
	
	// e)
	sm9_eval_g_line(g_num, g_den, T, Q1, P);  // g = g_{T,Q1}(P)
	fp12_mul_t(f_num, f_num, g_num);  // f = f * g = f * g_{T,Q1}(P)
	fp12_mul_t(f_den, f_den, g_den);
	ep2_add(T, T, Q1);  // T = T + Q1

	// f)
	sm9_eval_g_line(g_num, g_den, T, Q2, P);  // g = g_{T,-Q2}(P)
	fp12_mul_t(f_num, f_num, g_num);  // f = f * g = f * g_{T,-Q2}(P)
	fp12_mul_t(f_den, f_den, g_den);
//	ep2_add(T, T, Q2);  // T = T - Q2

	// g)
	fp12_inv_t(f_den, f_den);  // f_den = f_den^{-1}

	fp12_mul_t(r, f_num, f_den);  // r = f_num*f_den = f

	sm9_final_exponent(r, r);  // r = f^{(q^12-1)/r'}
	// PERFORMANCE_TEST("sm9_final_exponent", sm9_final_exponent(r, r), 1000);

	ep_free(_p);
	bn_free(n);
	ep2_free(T);
	ep2_free(Q1);
	ep2_free(Q2);
	ep2_free(ep2_tmp);
	ep2_free(neg_Q);
	fp12_free(f);
	fp12_free(g);
	fp12_free(f_num);
	fp12_free(f_den);
	fp12_free(g_num);
	fp12_free(g_den);
	fp12_free(fp12_tmp);

	return ;
}

void sm9_pairing_omp(fp12_t r_arr[], const ep2_t Q_arr[], const ep_t P_arr[], const size_t arr_size, const size_t threads_num){
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

void sm9_pairing_function_test(fp12_t r, const ep2_t Q, const ep_t P)
{
	// a)
	const char *abits = "00100000000000000000000000000000000000010000101011101100100111110";
	// const char *abits = "1";

	fp12_t f, g, f_num, f_den, g_num, g_den, fp12_tmp;
	ep2_t T, Q1, Q2, ep2_tmp, ep2_tmp2;
	ep_t _p;
	bn_t n;

	// null
	ep_null(_p);
	bn_null(n);
	ep2_null(T);
	ep2_null(Q1);
	ep2_null(Q2);
	ep2_null(ep2_tmp);
	ep2_null(ep2_tmp2);
	fp12_null(f);
	fp12_null(g);
	fp12_null(f_num);
	fp12_null(f_den);
	fp12_null(g_num);
	fp12_null(g_den);
	fp12_null(fp12_tmp);

	ep_new(_p);
	bn_new(n);
	ep2_new(T);
	ep2_new(Q1);
	ep2_new(Q2);
	ep2_new(ep2_tmp);
	ep2_new(ep2_tmp2);
	fp12_new(f);
	fp12_new(g);
	fp12_new(f_num);
	fp12_new(f_den);
	fp12_new(g_num);
	fp12_new(g_den);
	fp12_new(fp12_tmp);

	// b)
	ep2_copy(T, Q);
	fp12_set_dig(f_num, 1);
	fp12_set_dig(f_den, 1);

	PERFORMANCE_TEST_NEW("SM9 square ", fp12_sqr_t1(f_num, f_den));
	// fp12_mul_t(f_num, f_num, g_num);
	PERFORMANCE_TEST_NEW("SM9 multiplication ",fp12_mul_t1(f_num, f_num, g_num) );

	PERFORMANCE_TEST_NEW("SM9 square improved", fp12_sqr_t(f_num, f_den));
	// fp12_mul_t(f_num, f_num, g_num);
	PERFORMANCE_TEST_NEW("SM9 multiplication improved",fp12_mul_t(f_num, f_num, g_num) );

	PERFORMANCE_TEST_NEW("ep2_dbl_projc  ",ep2_dbl_projc(T, T) );
	PERFORMANCE_TEST_NEW("SM9 evaluation of g_line ",sm9_eval_g_line(g_num, g_den, T, Q, P) );
	PERFORMANCE_TEST_NEW("SM9 evaluation of g_tangent ",sm9_eval_g_tangent(g_num, g_den, T, P));
	PERFORMANCE_TEST_NEW("SM9 add full ",ep2_add_full(T, T, Q1));
	PERFORMANCE_TEST_NEW("SM9 final exponentiation ",sm9_final_exponent1(r, r));
	PERFORMANCE_TEST_NEW("SM9 final exponentiation improved",sm9_final_exponent(r, r));
	
	ep_free(_p);
	bn_free(n);
	ep2_free(T);
	ep2_free(Q1);
	ep2_free(Q2);
	ep2_free(ep2_tmp);
	ep2_free(ep2_tmp2);
	fp12_free(f);
	fp12_free(g);
	fp12_free(f_num);
	fp12_free(f_den);
	fp12_free(g_num);
	fp12_free(g_den);
	fp12_free(fp12_tmp);

	return 0;
}


void sm9_TEST(fp12_t r, const ep2_t Q, const ep_t P){
	PERFORMANCE_TEST_NEW("SM9 RELIC Pairing ",sm9_pairing(r,Q,P));


}


void sm9_pairing_steps_test(fp12_t r, const ep2_t Q, const ep_t P)
{
	// a)
	const char *abits = "00100000000000000000000000000000000000010000101011101100100111110";
	// const char *abits = "1";

	fp12_t f, g, f_num, f_den, g_num, g_den, fp12_tmp;
	ep2_t T, Q1, Q2, ep2_tmp, ep2_tmp2;
	ep_t _p;
	bn_t n;

	// null
	ep_null(_p);
	bn_null(n);
	ep2_null(T);
	ep2_null(Q1);
	ep2_null(Q2);
	ep2_null(ep2_tmp);
	ep2_null(ep2_tmp2);
	fp12_null(f);
	fp12_null(g);
	fp12_null(f_num);
	fp12_null(f_den);
	fp12_null(g_num);
	fp12_null(g_den);
	fp12_null(fp12_tmp);

	ep_new(_p);
	bn_new(n);
	ep2_new(T);
	ep2_new(Q1);
	ep2_new(Q2);
	ep2_new(ep2_tmp);
	ep2_new(ep2_tmp2);
	fp12_new(f);
	fp12_new(g);
	fp12_new(f_num);
	fp12_new(f_den);
	fp12_new(g_num);
	fp12_new(g_den);
	fp12_new(fp12_tmp);

	int  count = 0;
	int second = 3;
	double d=0.0;
	signal(SIGALRM,alarmed_t);
	alarm(second);
	run_t = 1;
	TIME_F(START);
	for(count=0;run_t&&count<0x7fffffff;count++){
		
	// b)
	ep2_copy(T, Q);
	fp12_set_dig(f_num, 1);
	fp12_set_dig(f_den, 1);




	for (size_t i = 0; i < strlen(abits); i++)
	{
		// c)
		fp12_sqr_t1(f_num, f_num);
		fp12_sqr_t1(f_den, f_den);

		sm9_eval_g_tangent(g_num, g_den, T, P);
		// PERFORMANCE_TEST_NEW("sm9_eval_g_tangent",sm9_eval_g_tangent(g_num, g_den, T, P));

		fp12_mul_t1(f_num, f_num, g_num);
		fp12_mul_t1(f_den, f_den, g_den);

		ep2_dbl_projc(T, T);
		// c.2)
		if (abits[i] == '1')
		{
			sm9_eval_g_line(g_num, g_den, T, Q, P);
			// PERFORMANCE_TEST("sm9_eval_g_line",sm9_eval_g_line(g_num, g_den, T, Q, P),10000);

			fp12_mul_t1(f_num, f_num, g_num);
			fp12_mul_t1(f_den, f_den, g_den);

			ep2_add_full(T, T, Q); // T = T + Q
		}
	}
	// d)
	ep2_pi1(Q1, Q); // Q1 = pi_q(Q)
	ep2_pi2(Q2, Q); // Q2 = pi_{q^2}(Q), Q2 = -Q2

	// e)
	sm9_eval_g_line(g_num, g_den, T, Q1, P); // g = g_{T,Q1}(P)
	fp12_mul_t1(f_num, f_num, g_num);		 // f = f * g = f * g_{T,Q1}(P)
	fp12_mul_t1(f_den, f_den, g_den);
	ep2_add_full(T, T, Q1); // T = T + Q1

	// f)
	sm9_eval_g_line(g_num, g_den, T, Q2, P); // g = g_{T,-Q2}(P)
	fp12_mul_t1(f_num, f_num, g_num);		 // f = f * g = f * g_{T,-Q2}(P)
	fp12_mul_t1(f_den, f_den, g_den);
	ep2_add_full(T, T, Q2); // T = T - Q2

	// g)
	fp12_inv_t(f_den, f_den); // f_den = f_den^{-1}

	fp12_mul_t1(r, f_num, f_den); // r = f_num*f_den = f



	}


	d=TIME_F(STOP);
	printf("SM9 RELIC Miller part \n\t\t\t run %d times in %.2fs \n",count/second,d/second);

	alarm(second);
	run_t = 1;
	d=0.0;
	TIME_F(START);
	for(count=0;run_t&&count<0x7fffffff;count++){
		
		sm9_final_exponent1(r, r); // r = f^{(q^12-1)/r'}
		// PERFORMANCE_TEST("sm9_final_exponent", sm9_final_exponent(r, r), 1000);
	}
	d = TIME_F(STOP);
	printf("SM9 RELIC Final Exp part \n\t\t\t run %d times in %.2fs \n",count/second,d/second);

	ep_free(_p);
	bn_free(n);
	ep2_free(T);
	ep2_free(Q1);
	ep2_free(Q2);
	ep2_free(ep2_tmp);
	ep2_free(ep2_tmp2);
	fp12_free(f);
	fp12_free(g);
	fp12_free(f_num);
	fp12_free(f_den);
	fp12_free(g_num);
	fp12_free(g_den);
	fp12_free(fp12_tmp);

	return 0;
}
