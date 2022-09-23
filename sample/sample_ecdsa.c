#include <stdio.h>
#include "relic.h"
#include "relic_bn.h"
#include "common.h"
#include "relic_test.h"
static int sample_ecdsa(void) {
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

    bn_new(d);
    bn_new(r);
    bn_new(s);
    ec_new(q);

    cp_ecdsa_gen(d, q);
	double time;
	size_t count = 10000;
	printf("----------性能测试----------\n");
	PERFORMANCE_TEST("ecdsa签名-消息", cp_ecdsa_sig(r, s, m, sizeof(m), 0, d), count);
	PERFORMANCE_TEST("ecdsa验证-消息", cp_ecdsa_ver(r, s, m, sizeof(m), 0, q), count);
	PERFORMANCE_TEST("ecdsa签名-哈希", cp_ecdsa_sig(r, s, h, RLC_MD_LEN, 1, d), count);
	PERFORMANCE_TEST("ecdsa验证-哈希", cp_ecdsa_ver(r, s, h, RLC_MD_LEN, 1, q), count);
end:
	bn_free(d);
	bn_free(r);
	bn_free(s);
	ec_free(q);
	return 0;
}

int main() {
    sample_ecdsa();
    return 0;
}
