#include <stdio.h>
#include "relic/relic.h"

static int ecdsa(void) {
	// 初始化库
	if (core_init() != RLC_OK) {
		core_clean();
		return 1;
	}

	if(ec_param_set_any() == RLC_OK)
		printf("初始化参数成功！\n");
	
	int code = RLC_ERR;
	bn_t d, r, s;
	ec_t q;
	uint8_t m[5] = { 0, 1, 2, 3, 4 }, h[RLC_MD_LEN];

	bn_null(d);
	bn_null(r);
	bn_null(s);
	ec_null(q);

    bn_new(d);
    bn_new(r);
    bn_new(s);
    ec_new(q);
    cp_ecdsa_gen(d, q);
    cp_ecdsa_sig(r, s, m, sizeof(m), 0, d);
    if(cp_ecdsa_ver(r, s, m, sizeof(m), 0, q)){
        printf("verify success!\n");
    }
	// RLC_TRY {
	// 	bn_new(d);
	// 	bn_new(r);
	// 	bn_new(s);
	// 	ec_new(q);
	// 	cp_ecdsa_gen(d, q);
	// 	cp_ecdsa_sig(r, s, m, sizeof(m), 0, d);
	// 	if(cp_ecdsa_ver(r, s, m, sizeof(m), 0, q)){
	// 		printf("verify success!\n");
	// 	}
	// 	TEST_CASE("ecdsa signature is correct") {
	// 		// printf("I=%d\n",i);
	// 		TEST_ASSERT(cp_ecdsa_gen(d, q) == RLC_OK, end);
	// 		TEST_ASSERT(cp_ecdsa_sig(r, s, m, sizeof(m), 0, d) == RLC_OK, end);
	// 		TEST_ASSERT(cp_ecdsa_ver(r, s, m, sizeof(m), 0, q) == 1, end);
	// 		m[0] ^= 1;
	// 		TEST_ASSERT(cp_ecdsa_ver(r, s, m, sizeof(m), 0, q) == 0, end);
	// 		md_map(h, m, sizeof(m));
	// 		TEST_ASSERT(cp_ecdsa_sig(r, s, h, RLC_MD_LEN, 1, d) == RLC_OK, end);
	// 		TEST_ASSERT(cp_ecdsa_ver(r, s, h, RLC_MD_LEN, 1, q) == 1, end);
	// 		h[0] ^= 1;
	// 		TEST_ASSERT(cp_ecdsa_ver(r, s, h, RLC_MD_LEN, 1, q) == 0, end);
	// 		memset(h, 0, RLC_MD_LEN);
	// 		TEST_ASSERT(cp_ecdsa_ver(r, s, h, RLC_MD_LEN, 1, q) == 0, end);
	// 	}
	// 	TEST_END;
	// }
	// RLC_CATCH_ANY {
	// 	RLC_ERROR(end);
	// }
	// code = RLC_OK;

  end:
	bn_free(d);
	bn_free(r);
	bn_free(s);
	ec_free(q);
	return 0;
}
int main() {
    ecdsa();
    // hello();
    return 0;
}
