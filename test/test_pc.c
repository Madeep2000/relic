/*
 * RELIC is an Efficient LIbrary for Cryptography
 * Copyright (c) 2010 RELIC Authors
 *
 * This file is part of RELIC. RELIC is legal property of its developers,
 * whose names are not listed here. Please refer to the COPYRIGHT file
 * for contact information.
 *
 * RELIC is free software; you can redistribute it and/or modify it under the
 * terms of the version 2.1 (or later) of the GNU Lesser General Public License
 * as published by the Free Software Foundation; or version 2.0 of the Apache
 * License as published by the Apache Software Foundation. See the LICENSE files
 * for more details.
 *
 * RELIC is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the LICENSE files for more details.
 *
 * You should have received a copy of the GNU Lesser General Public or the
 * Apache License along with RELIC. If not, see <https://www.gnu.org/licenses/>
 * or <https://www.apache.org/licenses/>.
 */

/**
 * @file
 *
 * Tests for the Pairing-Based Cryptography module.
 *
 * @ingroup test
 */

#include <stdio.h>

#include "relic.h"
#include "relic_test.h"

fp_t SM9_ALPHA1, SM9_ALPHA2, SM9_ALPHA3, SM9_ALPHA4, SM9_ALPHA5;
fp2_t SM9_BETA;

static int memory1(void) {
	err_t e;
	int code = RLC_ERR;
	g1_t a;

	g1_null(a);

	RLC_TRY {
		TEST_CASE("memory can be allocated") {
			g1_new(a);
			g1_free(a);
		} TEST_END;
	} RLC_CATCH(e) {
		switch (e) {
			case ERR_NO_MEMORY:
				util_print("FATAL ERROR!\n");
				RLC_ERROR(end);
				break;
		}
	}
	(void)a;
	code = RLC_OK;
  end:
	return code;
}

int util1(void) {
	int l, code = RLC_ERR;
	g1_t a, b, c;
	uint8_t bin[2 * RLC_PC_BYTES + 1];

	g1_null(a);
	g1_null(b);
	g1_null(c);

	RLC_TRY {
		g1_new(a);
		g1_new(b);
		g1_new(c);

		TEST_CASE("comparison is consistent") {
			g1_rand(a);
			g1_rand(b);
			TEST_ASSERT(g1_cmp(a, b) != RLC_EQ, end);
		}
		TEST_END;

		TEST_CASE("copy and comparison are consistent") {
			g1_rand(a);
			g1_rand(b);
			g1_rand(c);
			/* Compare points in affine coordinates. */
			if (g1_cmp(a, c) != RLC_EQ) {
				g1_copy(c, a);
				TEST_ASSERT(g1_cmp(c, a) == RLC_EQ, end);
			}
			if (g1_cmp(b, c) != RLC_EQ) {
				g1_copy(c, b);
				TEST_ASSERT(g1_cmp(b, c) == RLC_EQ, end);
			}
			/* Compare with one point in projective. */
			g1_dbl(c, a);
			g1_norm(c, c);
			g1_dbl(a, a);
			TEST_ASSERT(g1_cmp(c, a) == RLC_EQ, end);
			TEST_ASSERT(g1_cmp(a, c) == RLC_EQ, end);
			/* Compare with two points in projective. */
			g1_dbl(c, c);
			g1_dbl(a, a);
			TEST_ASSERT(g1_cmp(c, a) == RLC_EQ, end);
			TEST_ASSERT(g1_cmp(a, c) == RLC_EQ, end);
			g1_neg(b, a);
			g1_add(a, a, b);
			g1_set_infty(b);
			TEST_ASSERT(g1_cmp(a, b) == RLC_EQ, end);
		}
		TEST_END;

		TEST_CASE("inversion and comparison are consistent") {
			g1_rand(a);
			g1_neg(b, a);
			TEST_ASSERT(g1_cmp(a, b) != RLC_EQ, end);
		}
		TEST_END;

		TEST_CASE
				("assignment to random/infinity and comparison are consistent")
		{
			g1_rand(a);
			g1_set_infty(c);
			TEST_ASSERT(g1_cmp(a, c) != RLC_EQ, end);
			TEST_ASSERT(g1_cmp(c, a) != RLC_EQ, end);
		}
		TEST_END;

		TEST_CASE("assignment to infinity and infinity test are consistent") {
			g1_set_infty(a);
			TEST_ASSERT(g1_is_infty(a), end);
		}
		TEST_END;

		TEST_CASE("reading and writing a point are consistent") {
			for (int j = 0; j < 2; j++) {
				g1_set_infty(a);
				l = g1_size_bin(a, j);
				g1_write_bin(bin, l, a, j);
				g1_read_bin(b, bin, l);
				TEST_ASSERT(g1_cmp(a, b) == RLC_EQ, end);
				g1_rand(a);
				l = g1_size_bin(a, j);
				g1_write_bin(bin, l, a, j);
				g1_read_bin(b, bin, l);
				TEST_ASSERT(g1_cmp(a, b) == RLC_EQ, end);
				g1_rand(a);
				g1_dbl(a, a);
				l = g1_size_bin(a, j);
				g1_norm(a, a);
				g1_write_bin(bin, l, a, j);
				g1_read_bin(b, bin, l);
				TEST_ASSERT(g1_cmp(a, b) == RLC_EQ, end);
			}
		}
		TEST_END;
	}
	RLC_CATCH_ANY {
		util_print("FATAL ERROR!\n");
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	g1_free(a);
	g1_free(b);
	g1_free(c);
	return code;
}

int addition1(void) {
	int code = RLC_ERR;

	g1_t a, b, c, d, e;

	g1_null(a);
	g1_null(b);
	g1_null(c);
	g1_null(d);
	g1_null(e);

	RLC_TRY {
		g1_new(a);
		g1_new(b);
		g1_new(c);
		g1_new(d);
		g1_new(e);

		TEST_CASE("point addition is commutative") {
			g1_rand(a);
			g1_rand(b);
			g1_add(d, a, b);
			g1_add(e, b, a);
			TEST_ASSERT(g1_cmp(d, e) == RLC_EQ, end);
		} TEST_END;

		TEST_CASE("point addition is associative") {
			g1_rand(a);
			g1_rand(b);
			g1_rand(c);
			g1_add(d, a, b);
			g1_add(d, d, c);
			g1_add(e, b, c);
			g1_add(e, e, a);
			TEST_ASSERT(g1_cmp(d, e) == RLC_EQ, end);
		} TEST_END;

		TEST_CASE("point addition has identity") {
			g1_rand(a);
			g1_set_infty(d);
			g1_add(e, a, d);
			TEST_ASSERT(g1_cmp(e, a) == RLC_EQ, end);
			g1_add(e, d, a);
			TEST_ASSERT(g1_cmp(e, a) == RLC_EQ, end);
		} TEST_END;

		TEST_CASE("point addition has inverse") {
			g1_rand(a);
			g1_neg(d, a);
			g1_add(e, a, d);
			TEST_ASSERT(g1_is_infty(e), end);
		} TEST_END;
	}
	RLC_CATCH_ANY {
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	g1_free(a);
	g1_free(b);
	g1_free(c);
	g1_free(d);
	g1_free(e);
	return code;
}

int subtraction1(void) {
	int code = RLC_ERR;
	g1_t a, b, c, d;

	g1_null(a);
	g1_null(b);
	g1_null(c);
	g1_null(d);

	RLC_TRY {
		g1_new(a);
		g1_new(b);
		g1_new(c);
		g1_new(d);

		TEST_CASE("point subtraction is anti-commutative") {
			g1_rand(a);
			g1_rand(b);
			g1_sub(c, a, b);
			g1_sub(d, b, a);
			g1_neg(d, d);
			TEST_ASSERT(g1_cmp(c, d) == RLC_EQ, end);
		}
		TEST_END;

		TEST_CASE("point subtraction has identity") {
			g1_rand(a);
			g1_set_infty(c);
			g1_sub(d, a, c);
			TEST_ASSERT(g1_cmp(d, a) == RLC_EQ, end);
		}
		TEST_END;

		TEST_CASE("point subtraction has inverse") {
			g1_rand(a);
			g1_sub(c, a, a);
			TEST_ASSERT(g1_is_infty(c), end);
		}
		TEST_END;
	}
	RLC_CATCH_ANY {
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	g1_free(a);
	g1_free(b);
	g1_free(c);
	g1_free(d);
	return code;
}

int doubling1(void) {
	int code = RLC_ERR;
	g1_t a, b, c;

	g1_null(a);
	g1_null(b);
	g1_null(c);

	RLC_TRY {
		g1_new(a);
		g1_new(b);
		g1_new(c);

		TEST_CASE("point doubling is correct") {
			g1_rand(a);
			g1_add(b, a, a);
			g1_dbl(c, a);
			TEST_ASSERT(g1_cmp(b, c) == RLC_EQ, end);
		} TEST_END;
	}
	RLC_CATCH_ANY {
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	g1_free(a);
	g1_free(b);
	g1_free(c);
	return code;
}

static int multiplication1(void) {
	int code = RLC_ERR;
	g1_t p, q, r;
	bn_t n, k;

	bn_null(n);
	bn_null(k);
	g1_null(p);
	g1_null(q);
	g1_null(r);

	RLC_TRY {
		g1_new(p);
		g1_new(q);
		g1_new(r);
		bn_new(n);
		bn_new(k);

		g1_get_gen(p);
		pc_get_ord(n);

		TEST_CASE("generator has the right order") {
			g1_mul(r, p, n);
			TEST_ASSERT(g1_is_infty(r) == 1, end);
		} TEST_END;

		TEST_CASE("generator multiplication is correct") {
			bn_zero(k);
			g1_mul_gen(r, k);
			TEST_ASSERT(g1_is_infty(r), end);
			bn_set_dig(k, 1);
			g1_mul_gen(r, k);
			TEST_ASSERT(g1_cmp(p, r) == RLC_EQ, end);
			bn_rand_mod(k, n);
			g1_mul(q, p, k);
			g1_mul_gen(r, k);
			TEST_ASSERT(g1_cmp(q, r) == RLC_EQ, end);
			bn_neg(k, k);
			g1_mul_gen(r, k);
			g1_neg(r, r);
			TEST_ASSERT(g1_cmp(q, r) == RLC_EQ, end);
		} TEST_END;

		TEST_CASE("random element has the right order") {
			g1_rand(p);
			g1_mul(r, p, n);
			TEST_ASSERT(g1_is_infty(r) == 1, end);
		} TEST_END;

		TEST_CASE("point multiplication by digit is correct") {
			g1_mul_dig(r, p, 0);
			TEST_ASSERT(g1_is_infty(r), end);
			g1_mul_dig(r, p, 1);
			TEST_ASSERT(g1_cmp(p, r) == RLC_EQ, end);
			bn_rand(k, RLC_POS, RLC_DIG);
			g1_mul(q, p, k);
			g1_mul_dig(r, p, k->dp[0]);
			TEST_ASSERT(g1_cmp(q, r) == RLC_EQ, end);
		}
		TEST_END;
	}
	RLC_CATCH_ANY {
		util_print("FATAL ERROR!\n");
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	g1_free(p);
	g1_free(q);
	g1_free(r);
	bn_free(n);
	bn_free(k);
	return code;
}

static int fixed1(void) {
	int code = RLC_ERR;
	g1_t p, q, r;
	g1_t t[RLC_G1_TABLE];
	bn_t n, k;

	bn_null(n);
	bn_null(k);
	g1_null(p);
	g1_null(q);
	g1_null(r);

	for (int i = 0; i < RLC_G1_TABLE; i++) {
		g1_null(t[i]);
	}

	RLC_TRY {
		g1_new(p);
		g1_new(q);
		g1_new(r);
		bn_new(n);
		bn_new(k);

		g1_get_gen(p);
		pc_get_ord(n);

		for (int i = 0; i < RLC_G1_TABLE; i++) {
			g1_new(t[i]);
		}
		TEST_CASE("fixed point multiplication is correct") {
			g1_rand(p);
			g1_mul_pre(t, p);
			bn_zero(k);
			g1_mul_fix(r, (const g1_t *)t, k);
			TEST_ASSERT(g1_is_infty(r), end);
			bn_set_dig(k, 1);
			g1_mul_fix(r, (const g1_t *)t, k);
			TEST_ASSERT(g1_cmp(p, r) == RLC_EQ, end);
			bn_rand_mod(k, n);
			g1_mul(q, p, k);
			g1_mul_fix(q, (const g1_t *)t, k);
			g1_mul(r, p, k);
			TEST_ASSERT(g1_cmp(q, r) == RLC_EQ, end);
			bn_neg(k, k);
			g1_mul_fix(r, (const g1_t *)t, k);
			g1_neg(r, r);
			TEST_ASSERT(g1_cmp(q, r) == RLC_EQ, end);
		} TEST_END;
		for (int i = 0; i < RLC_G1_TABLE; i++) {
			g1_free(t[i]);
		}
	}
	RLC_CATCH_ANY {
		util_print("FATAL ERROR!\n");
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	g1_free(p);
	g1_free(q);
	g1_free(r);
	bn_free(n);
	bn_free(k);
	return code;
}

static int simultaneous1(void) {
	int code = RLC_ERR;
	g1_t p, q, r;
	bn_t n, k, l;

	bn_null(n);
	bn_null(k);
	bn_null(l);
	g1_null(p);
	g1_null(q);
	g1_null(r);

	RLC_TRY {
		bn_new(n);
		bn_new(k);
		bn_new(l);
		g1_new(p);
		g1_new(q);
		g1_new(r);

		g1_get_gen(p);
		pc_get_ord(n);

		TEST_CASE("simultaneous point multiplication is correct") {
			bn_zero(k);
			bn_rand_mod(l, n);
			g1_mul(q, p, l);
			g1_mul_sim(r, p, k, p, l);
			TEST_ASSERT(g1_cmp(q, r) == RLC_EQ, end);
			bn_rand_mod(k, n);
			bn_zero(l);
			g1_mul(q, p, k);
			g1_mul_sim(r, p, k, p, l);
			TEST_ASSERT(g1_cmp(q, r) == RLC_EQ, end);
			bn_rand_mod(k, n);
			bn_rand_mod(l, n);
			g1_mul_sim(r, p, k, q, l);
			g1_mul(p, p, k);
			g1_mul(q, q, l);
			g1_add(q, q, p);
			TEST_ASSERT(g1_cmp(q, r) == RLC_EQ, end);
			bn_neg(k, k);
			g1_mul_sim(r, p, k, q, l);
			g1_mul(p, p, k);
			g1_mul(q, q, l);
			g1_add(q, q, p);
			TEST_ASSERT(g1_cmp(q, r) == RLC_EQ, end);
			bn_neg(l, l);
			g1_mul_sim(r, p, k, q, l);
			g1_mul(p, p, k);
			g1_mul(q, q, l);
			g1_add(q, q, p);
			TEST_ASSERT(g1_cmp(q, r) == RLC_EQ, end);
		} TEST_END;

		TEST_CASE("simultaneous multiplication with generator is correct") {
			bn_zero(k);
			bn_rand_mod(l, n);
			g1_mul(q, p, l);
			g1_mul_sim_gen(r, k, p, l);
			TEST_ASSERT(g1_cmp(q, r) == RLC_EQ, end);
			bn_rand_mod(k, n);
			bn_zero(l);
			g1_mul_gen(q, k);
			g1_mul_sim_gen(r, k, p, l);
			TEST_ASSERT(g1_cmp(q, r) == RLC_EQ, end);
			bn_rand_mod(k, n);
			bn_rand_mod(l, n);
			g1_mul_sim_gen(r, k, q, l);
			g1_get_gen(p);
			g1_mul_sim(q, p, k, q, l);
			TEST_ASSERT(g1_cmp(q, r) == RLC_EQ, end);
			bn_neg(k, k);
			g1_mul_sim_gen(r, k, q, l);
			g1_get_gen(p);
			g1_mul_sim(q, p, k, q, l);
			TEST_ASSERT(g1_cmp(q, r) == RLC_EQ, end);
			bn_neg(l, l);
			g1_mul_sim_gen(r, k, q, l);
			g1_get_gen(p);
			g1_mul_sim(q, p, k, q, l);
			TEST_ASSERT(g1_cmp(q, r) == RLC_EQ, end);
		} TEST_END;
	}
	RLC_CATCH_ANY {
		util_print("FATAL ERROR!\n");
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	bn_free(n);
	bn_free(k);
	bn_free(l);
	g1_free(p);
	g1_free(q);
	g1_free(r);
	return code;
}

static int validity1(void) {
	int code = RLC_ERR;
	g1_t a;

	g1_null(a);

	RLC_TRY {
		g1_new(a);

		TEST_CASE("validity test is correct") {
			g1_set_infty(a);
			TEST_ASSERT(!g1_is_valid(a), end);
			g1_rand(a);
			TEST_ASSERT(g1_is_valid(a), end);
		}
		TEST_END;

		TEST_CASE("blinding is consistent") {
			g1_rand(a);
			g1_blind(a, a);
			TEST_ASSERT(g1_is_valid(a), end);
		} TEST_END;
	}
	RLC_CATCH_ANY {
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	g1_free(a);
	return code;
}

static int hashing1(void) {
	int code = RLC_ERR;
	g1_t a;
	bn_t n;
	uint8_t msg[5];

	g1_null(a);
	bn_null(n);

	RLC_TRY {
		g1_new(a);
		bn_new(n);

		pc_get_ord(n);

		TEST_CASE("point hashing is correct") {
			rand_bytes(msg, sizeof(msg));
			g1_map(a, msg, sizeof(msg));
			TEST_ASSERT(g1_is_valid(a), end);
		}
		TEST_END;

	}
	RLC_CATCH_ANY {
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	g1_free(a);
	bn_free(n);
	return code;
}

static int memory2(void) {
	err_t e;
	int code = RLC_ERR;
	g2_t a;

	g2_null(a);

	RLC_TRY {
		TEST_CASE("memory can be allocated") {
			g2_new(a);
			g2_free(a);
		} TEST_END;
	} RLC_CATCH(e) {
		switch (e) {
			case ERR_NO_MEMORY:
				util_print("FATAL ERROR!\n");
				RLC_ERROR(end);
				break;
		}
	}
	(void)a;
	code = RLC_OK;
  end:
	return code;
}

int util2(void) {
	int l, code = RLC_ERR;
	g2_t a, b, c;
	uint8_t bin[8 * RLC_PC_BYTES + 1];

	g2_null(a);
	g2_null(b);
	g2_null(c);

	RLC_TRY {
		g2_new(a);
		g2_new(b);
		g2_new(c);

		TEST_CASE("comparison is consistent") {
			g2_rand(a);
			g2_rand(b);
			TEST_ASSERT(g2_cmp(a, b) != RLC_EQ, end);
		}
		TEST_END;

		TEST_CASE("copy and comparison are consistent") {
			g2_rand(a);
			g2_rand(b);
			g2_rand(c);
			/* Compare points in affine coordinates. */
			if (g2_cmp(a, c) != RLC_EQ) {
				g2_copy(c, a);
				TEST_ASSERT(g2_cmp(c, a) == RLC_EQ, end);
			}
			if (g2_cmp(b, c) != RLC_EQ) {
				g2_copy(c, b);
				TEST_ASSERT(g2_cmp(b, c) == RLC_EQ, end);
			}
			/* Compare with one point in projective. */
			g2_dbl(c, a);
			g2_norm(c, c);
			g2_dbl(a, a);
			TEST_ASSERT(g2_cmp(c, a) == RLC_EQ, end);
			TEST_ASSERT(g2_cmp(a, c) == RLC_EQ, end);
			/* Compare with two points in projective. */
			g2_dbl(c, c);
			g2_dbl(a, a);
			TEST_ASSERT(g2_cmp(c, a) == RLC_EQ, end);
			TEST_ASSERT(g2_cmp(a, c) == RLC_EQ, end);
		}
		TEST_END;

		TEST_CASE("negation and comparison are consistent") {
			g2_rand(a);
			g2_neg(b, a);
			TEST_ASSERT(g2_cmp(a, b) != RLC_EQ, end);
			g2_neg(b, a);
			g2_add(a, a, b);
			g2_set_infty(b);
			TEST_ASSERT(g2_cmp(a, b) == RLC_EQ, end);
		}
		TEST_END;

		TEST_CASE
				("assignment to random/infinity and comparison are consistent")
		{
			g2_rand(a);
			g2_set_infty(c);
			TEST_ASSERT(g2_cmp(a, c) != RLC_EQ, end);
			TEST_ASSERT(g2_cmp(c, a) != RLC_EQ, end);
		}
		TEST_END;

		TEST_CASE("assignment to infinity and infinity test are consistent") {
			g2_set_infty(a);
			TEST_ASSERT(g2_is_infty(a), end);
		}
		TEST_END;

		TEST_CASE("reading and writing a point are consistent") {
			for (int j = 0; j < 2; j++) {
				g2_set_infty(a);
				l = g2_size_bin(a, j);
				g2_write_bin(bin, l, a, j);
				g2_read_bin(b, bin, l);
				TEST_ASSERT(g2_cmp(a, b) == RLC_EQ, end);
				g2_rand(a);
				l = g2_size_bin(a, j);
				g2_write_bin(bin, l, a, j);
				g2_read_bin(b, bin, l);
				TEST_ASSERT(g2_cmp(a, b) == RLC_EQ, end);
				g2_rand(a);
				g2_dbl(a, a);
				l = g2_size_bin(a, j);
				g2_norm(a, a);
				g2_write_bin(bin, l, a, j);
				g2_read_bin(b, bin, l);
				TEST_ASSERT(g2_cmp(a, b) == RLC_EQ, end);
			}
		}
		TEST_END;
	}
	RLC_CATCH_ANY {
		util_print("FATAL ERROR!\n");
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	g2_free(a);
	g2_free(b);
	g2_free(c);
	return code;
}

int addition2(void) {
	int code = RLC_ERR;

	g2_t a, b, c, d, e;

	g2_null(a);
	g2_null(b);
	g2_null(c);
	g2_null(d);
	g2_null(e);

	RLC_TRY {
		g2_new(a);
		g2_new(b);
		g2_new(c);
		g2_new(d);
		g2_new(e);

		TEST_CASE("point addition is commutative") {
			g2_rand(a);
			g2_rand(b);
			g2_add(d, a, b);
			g2_add(e, b, a);
			TEST_ASSERT(g2_cmp(d, e) == RLC_EQ, end);
		} TEST_END;

		TEST_CASE("point addition is associative") {
			g2_rand(a);
			g2_rand(b);
			g2_rand(c);
			g2_add(d, a, b);
			g2_add(d, d, c);
			g2_add(e, b, c);
			g2_add(e, e, a);
			TEST_ASSERT(g2_cmp(d, e) == RLC_EQ, end);
		} TEST_END;

		TEST_CASE("point addition has identity") {
			g2_rand(a);
			g2_set_infty(d);
			g2_add(e, a, d);
			TEST_ASSERT(g2_cmp(e, a) == RLC_EQ, end);
			g2_add(e, d, a);
			TEST_ASSERT(g2_cmp(e, a) == RLC_EQ, end);
		} TEST_END;

		TEST_CASE("point addition has inverse") {
			g2_rand(a);
			g2_neg(d, a);
			g2_add(e, a, d);
			TEST_ASSERT(g2_is_infty(e), end);
		} TEST_END;
	}
	RLC_CATCH_ANY {
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	g2_free(a);
	g2_free(b);
	g2_free(c);
	g2_free(d);
	g2_free(e);
	return code;
}

int subtraction2(void) {
	int code = RLC_ERR;
	g2_t a, b, c, d;

	g2_null(a);
	g2_null(b);
	g2_null(c);
	g2_null(d);

	RLC_TRY {
		g2_new(a);
		g2_new(b);
		g2_new(c);
		g2_new(d);

		TEST_CASE("point subtraction is anti-commutative") {
			g2_rand(a);
			g2_rand(b);
			g2_sub(c, a, b);
			g2_sub(d, b, a);
			g2_neg(d, d);
			TEST_ASSERT(g2_cmp(c, d) == RLC_EQ, end);
		}
		TEST_END;

		TEST_CASE("point subtraction has identity") {
			g2_rand(a);
			g2_set_infty(c);
			g2_sub(d, a, c);
			TEST_ASSERT(g2_cmp(d, a) == RLC_EQ, end);
		}
		TEST_END;

		TEST_CASE("point subtraction has inverse") {
			g2_rand(a);
			g2_sub(c, a, a);
			TEST_ASSERT(g2_is_infty(c), end);
		}
		TEST_END;
	}
	RLC_CATCH_ANY {
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	g2_free(a);
	g2_free(b);
	g2_free(c);
	g2_free(d);
	return code;
}

int doubling2(void) {
	int code = RLC_ERR;
	g2_t a, b, c;

	g2_null(a);
	g2_null(b);
	g2_null(c);

	RLC_TRY {
		g2_new(a);
		g2_new(b);
		g2_new(c);

		TEST_CASE("point doubling is correct") {
			g2_rand(a);
			g2_add(b, a, a);
			g2_dbl(c, a);
			TEST_ASSERT(g2_cmp(b, c) == RLC_EQ, end);
		} TEST_END;
	}
	RLC_CATCH_ANY {
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	g2_free(a);
	g2_free(b);
	g2_free(c);
	return code;
}

static int multiplication2(void) {
	int code = RLC_ERR;
	g2_t p, q, r;
	bn_t n, k;

	bn_null(n);
	bn_null(k);
	g2_null(p);
	g2_null(q);
	g2_null(r);

	RLC_TRY {
		g2_new(p);
		g2_new(q);
		g2_new(r);
		bn_new(n);
		bn_new(k);

		g2_get_gen(p);
		pc_get_ord(n);

		TEST_CASE("generator has the right order") {
			g2_mul(r, p, n);
			TEST_ASSERT(g2_is_infty(r) == 1, end);
		} TEST_END;

		TEST_CASE("generator multiplication is correct") {
			bn_zero(k);
			g2_mul_gen(r, k);
			TEST_ASSERT(g2_is_infty(r), end);
			bn_set_dig(k, 1);
			g2_mul_gen(r, k);
			TEST_ASSERT(g2_cmp(p, r) == RLC_EQ, end);
			bn_rand_mod(k, n);
			g2_mul(q, p, k);
			g2_mul_gen(r, k);
			TEST_ASSERT(g2_cmp(q, r) == RLC_EQ, end);
			bn_neg(k, k);
			g2_mul_gen(r, k);
			g2_neg(r, r);
			TEST_ASSERT(g2_cmp(q, r) == RLC_EQ, end);
		} TEST_END;

		TEST_CASE("random element has the right order") {
			g2_rand(p);
			g2_mul(r, p, n);
			TEST_ASSERT(g2_is_infty(r) == 1, end);
		} TEST_END;

		TEST_CASE("point multiplication by digit is correct") {
			g2_mul_dig(r, p, 0);
			TEST_ASSERT(g2_is_infty(r), end);
			g2_mul_dig(r, p, 1);
			TEST_ASSERT(g2_cmp(p, r) == RLC_EQ, end);
			bn_rand(k, RLC_POS, RLC_DIG);
			g2_mul(q, p, k);
			g2_mul_dig(r, p, k->dp[0]);
			TEST_ASSERT(g2_cmp(q, r) == RLC_EQ, end);
		}
		TEST_END;
	}
	RLC_CATCH_ANY {
		util_print("FATAL ERROR!\n");
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	g2_free(p);
	g2_free(q);
	g2_free(r);
	bn_free(n);
	bn_free(k);
	return code;
}

static int fixed2(void) {
	int code = RLC_ERR;
	g2_t p, q, r;
	g2_t t[RLC_G2_TABLE];
	bn_t n, k;

	bn_null(n);
	bn_null(k);
	g2_null(p);
	g2_null(q);
	g2_null(r);

	for (int i = 0; i < RLC_G2_TABLE; i++) {
		g2_null(t[i]);
	}

	RLC_TRY {
		g2_new(p);
		g2_new(q);
		g2_new(r);
		bn_new(n);
		bn_new(k);

		g2_get_gen(p);
		pc_get_ord(n);

		for (int i = 0; i < RLC_G2_TABLE; i++) {
			g2_new(t[i]);
		}
		TEST_CASE("fixed point multiplication is correct") {
			g2_rand(p);
			g2_mul_pre(t, p);
			bn_zero(k);
			g2_mul_fix(r, t, k);
			TEST_ASSERT(g2_is_infty(r), end);
			bn_set_dig(k, 1);
			g2_mul_fix(r, t, k);
			TEST_ASSERT(g2_cmp(p, r) == RLC_EQ, end);
			bn_rand_mod(k, n);
			g2_mul(q, p, k);
			g2_mul_fix(q, t, k);
			g2_mul(r, p, k);
			TEST_ASSERT(g2_cmp(q, r) == RLC_EQ, end);
			bn_neg(k, k);
			g2_mul_fix(r, t, k);
			g2_neg(r, r);
			TEST_ASSERT(g2_cmp(q, r) == RLC_EQ, end);
		} TEST_END;
		for (int i = 0; i < RLC_G2_TABLE; i++) {
			g2_free(t[i]);
		}
	}
	RLC_CATCH_ANY {
		util_print("FATAL ERROR!\n");
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	g2_free(p);
	g2_free(q);
	g2_free(r);
	bn_free(n);
	bn_free(k);
	return code;
}

static int simultaneous2(void) {
	int code = RLC_ERR;
	g2_t p, q, r;
	bn_t n, k, l;

	bn_null(n);
	bn_null(k);
	bn_null(l);
	g2_null(p);
	g2_null(q);
	g2_null(r);

	RLC_TRY {
		bn_new(n);
		bn_new(k);
		bn_new(l);
		g2_new(p);
		g2_new(q);
		g2_new(r);

		g2_get_gen(p);
		pc_get_ord(n);

		TEST_CASE("simultaneous point multiplication is correct") {
			bn_zero(k);
			bn_rand_mod(l, n);
			g2_mul(q, p, l);
			g2_mul_sim(r, p, k, p, l);
			TEST_ASSERT(g2_cmp(q, r) == RLC_EQ, end);
			bn_rand_mod(k, n);
			bn_zero(l);
			g2_mul(q, p, k);
			g2_mul_sim(r, p, k, p, l);
			TEST_ASSERT(g2_cmp(q, r) == RLC_EQ, end);
			bn_rand_mod(k, n);
			bn_rand_mod(l, n);
			g2_mul_sim(r, p, k, q, l);
			g2_mul(p, p, k);
			g2_mul(q, q, l);
			g2_add(q, q, p);
			TEST_ASSERT(g2_cmp(q, r) == RLC_EQ, end);
			bn_neg(k, k);
			g2_mul_sim(r, p, k, q, l);
			g2_mul(p, p, k);
			g2_mul(q, q, l);
			g2_add(q, q, p);
			TEST_ASSERT(g2_cmp(q, r) == RLC_EQ, end);
			bn_neg(l, l);
			g2_mul_sim(r, p, k, q, l);
			g2_mul(p, p, k);
			g2_mul(q, q, l);
			g2_add(q, q, p);
			TEST_ASSERT(g2_cmp(q, r) == RLC_EQ, end);
		} TEST_END;

		TEST_CASE("simultaneous multiplication with generator is correct") {
			bn_zero(k);
			bn_rand_mod(l, n);
			g2_mul(q, p, l);
			g2_mul_sim_gen(r, k, p, l);
			TEST_ASSERT(g2_cmp(q, r) == RLC_EQ, end);
			bn_rand_mod(k, n);
			bn_zero(l);
			g2_mul_gen(q, k);
			g2_mul_sim_gen(r, k, p, l);
			TEST_ASSERT(g2_cmp(q, r) == RLC_EQ, end);
			bn_rand_mod(k, n);
			bn_rand_mod(l, n);
			g2_mul_sim_gen(r, k, q, l);
			g2_get_gen(p);
			g2_mul_sim(q, p, k, q, l);
			TEST_ASSERT(g2_cmp(q, r) == RLC_EQ, end);
			bn_neg(k, k);
			g2_mul_sim_gen(r, k, q, l);
			g2_get_gen(p);
			g2_mul_sim(q, p, k, q, l);
			TEST_ASSERT(g2_cmp(q, r) == RLC_EQ, end);
			bn_neg(l, l);
			g2_mul_sim_gen(r, k, q, l);
			g2_get_gen(p);
			g2_mul_sim(q, p, k, q, l);
			TEST_ASSERT(g2_cmp(q, r) == RLC_EQ, end);
		} TEST_END;
	}
	RLC_CATCH_ANY {
		util_print("FATAL ERROR!\n");
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	bn_free(n);
	bn_free(k);
	bn_free(l);
	g2_free(p);
	g2_free(q);
	g2_free(r);
	return code;
}

static int validity2(void) {
	int code = RLC_ERR;
	g2_t a;

	g2_null(a);

	RLC_TRY {
		g2_new(a);

		TEST_CASE("validity test is correct") {
			g2_set_infty(a);
			TEST_ASSERT(!g2_is_valid(a), end);
			g2_rand(a);
			TEST_ASSERT(g2_is_valid(a), end);
		}
		TEST_END;

		TEST_CASE("blinding is consistent") {
			g2_rand(a);
			g2_blind(a, a);
			TEST_ASSERT(g2_is_valid(a), end);
		} TEST_END;
	}
	RLC_CATCH_ANY {
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	g2_free(a);
	return code;
}

#if FP_PRIME != 509

static int hashing2(void) {
	int code = RLC_ERR;
	g2_t a;
	bn_t n;
	uint8_t msg[5];

	g2_null(a);
	bn_null(n);

	RLC_TRY {
		g2_new(a);
		bn_new(n);

		pc_get_ord(n);

		TEST_CASE("point hashing is correct") {
			rand_bytes(msg, sizeof(msg));
			g2_map(a, msg, sizeof(msg));
			TEST_ASSERT(g2_is_valid(a), end);
		}
		TEST_END;

	}
	RLC_CATCH_ANY {
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	g2_free(a);
	bn_free(n);
	return code;
}

#endif

static int memory(void) {
	err_t e;
	int code = RLC_ERR;
	gt_t a;

	gt_null(a);

	RLC_TRY {
		TEST_CASE("memory can be allocated") {
			gt_new(a);
			gt_free(a);
		} TEST_END;
	} RLC_CATCH(e) {
		switch (e) {
			case ERR_NO_MEMORY:
				util_print("FATAL ERROR!\n");
				RLC_ERROR(end);
				break;
		}
	}
	(void)a;
	code = RLC_OK;
  end:
	return code;
}

int util(void) {
	int l, code = RLC_ERR;
	gt_t a, b, c;
	uint8_t bin[24 * RLC_PC_BYTES];

	gt_null(a);
	gt_null(b);
	gt_null(c);

	RLC_TRY {
		gt_new(a);
		gt_new(b);
		gt_new(c);

		TEST_CASE("comparison is consistent") {
			gt_rand(a);
			gt_rand(b);
			TEST_ASSERT(gt_cmp(a, b) != RLC_EQ, end);
		}
		TEST_END;

		TEST_CASE("copy and comparison are consistent") {
			gt_rand(a);
			gt_rand(b);
			gt_rand(c);
			if (gt_cmp(a, c) != RLC_EQ) {
				gt_copy(c, a);
				TEST_ASSERT(gt_cmp(c, a) == RLC_EQ, end);
			}
			if (gt_cmp(b, c) != RLC_EQ) {
				gt_copy(c, b);
				TEST_ASSERT(gt_cmp(b, c) == RLC_EQ, end);
			}
		}
		TEST_END;

		TEST_CASE("inversion and comparison are consistent") {
			gt_rand(a);
			gt_inv(b, a);
			TEST_ASSERT(gt_cmp(a, b) != RLC_EQ, end);
		}
		TEST_END;

		TEST_CASE
				("assignment to random/infinity and comparison are consistent")
		{
			gt_rand(a);
			gt_set_unity(c);
			TEST_ASSERT(gt_cmp(a, c) != RLC_EQ, end);
			TEST_ASSERT(gt_cmp(c, a) != RLC_EQ, end);
		}
		TEST_END;

		TEST_CASE("assignment to unity and unity test are consistent") {
			gt_set_unity(a);
			TEST_ASSERT(gt_is_unity(a), end);
		}
		TEST_END;
	}
	RLC_CATCH_ANY {
		util_print("FATAL ERROR!\n");
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	gt_free(a);
	gt_free(b);
	gt_free(c);
	return code;
}

int multiplication(void) {
	int code = RLC_ERR;

	gt_t a, b, c, d, e;

	gt_null(a);
	gt_null(b);
	gt_null(c);
	gt_null(d);
	gt_null(e);

	RLC_TRY {
		gt_new(a);
		gt_new(b);
		gt_new(c);
		gt_new(d);
		gt_new(e);

		TEST_CASE("multiplication is commutative") {
			gt_rand(a);
			gt_rand(b);
			gt_mul(d, a, b);
			gt_mul(e, b, a);
			TEST_ASSERT(gt_cmp(d, e) == RLC_EQ, end);
		} TEST_END;

		TEST_CASE("multiplication is associative") {
			gt_rand(a);
			gt_rand(b);
			gt_rand(c);
			gt_mul(d, a, b);
			gt_mul(d, d, c);
			gt_mul(e, b, c);
			gt_mul(e, e, a);
			TEST_ASSERT(gt_cmp(d, e) == RLC_EQ, end);
		} TEST_END;

		TEST_CASE("multiplication has identity") {
			gt_rand(a);
			gt_set_unity(d);
			gt_mul(e, a, d);
			TEST_ASSERT(gt_cmp(e, a) == RLC_EQ, end);
			gt_mul(e, d, a);
			TEST_ASSERT(gt_cmp(e, a) == RLC_EQ, end);
		} TEST_END;
	}
	RLC_CATCH_ANY {
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	gt_free(a);
	gt_free(b);
	gt_free(c);
	gt_free(d);
	gt_free(e);
	return code;
}

int squaring(void) {
	int code = RLC_ERR;
	gt_t a, b, c;

	gt_null(a);
	gt_null(b);
	gt_null(c);

	RLC_TRY {
		gt_new(a);
		gt_new(b);
		gt_new(c);

		TEST_CASE("squaring is correct") {
			gt_rand(a);
			gt_mul(b, a, a);
			gt_sqr(c, a);
			TEST_ASSERT(gt_cmp(b, c) == RLC_EQ, end);
		} TEST_END;
	}
	RLC_CATCH_ANY {
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	gt_free(a);
	gt_free(b);
	gt_free(c);
	return code;
}

static int inversion(void) {
	int code = RLC_ERR;
	gt_t a, b, c;

	RLC_TRY {
		gt_new(a);
		gt_new(b);
		gt_new(c);

		TEST_CASE("inversion is correct") {
			gt_rand(a);
			gt_inv(b, a);
			gt_mul(c, a, b);
			gt_set_unity(b);
			TEST_ASSERT(gt_cmp(c, b) == RLC_EQ, end);
		} TEST_END;
	}
	RLC_CATCH_ANY {
		util_print("FATAL ERROR!\n");
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	gt_free(a);
	gt_free(b);
	gt_free(c);
	return code;
}

int exponentiation(void) {
	int code = RLC_ERR;
	gt_t a, b, c;
	bn_t n, d, e;

	gt_null(a);
	gt_null(b);
	gt_null(c);
	bn_null(d);
	bn_null(e);
	bn_null(n);

	RLC_TRY {
		gt_new(a);
		gt_new(b);
		gt_new(c);
		bn_new(d);
		bn_new(e);
		bn_new(n);

		gt_get_gen(a);
		pc_get_ord(n);

		TEST_CASE("generator has the right order") {
			gt_exp(c, a, n);
			TEST_ASSERT(gt_is_unity(c), end);
		} TEST_END;

		TEST_CASE("generator exponentiation is correct") {
			bn_zero(d);
			gt_exp_gen(c, d);
			TEST_ASSERT(gt_is_unity(c), end);
			bn_set_dig(d, 1);
			gt_exp_gen(c, d);
			TEST_ASSERT(gt_cmp(c, a) == RLC_EQ, end);
			bn_add_dig(d, n, 1);
			gt_exp_gen(c, d);
			TEST_ASSERT(gt_cmp(c, a) == RLC_EQ, end);
			gt_exp_gen(c, n);
			TEST_ASSERT(gt_is_unity(c), end);
			bn_rand_mod(d, n);
			gt_exp_gen(b, d);
			bn_neg(d, d);
			gt_exp_gen(c, d);
			gt_inv(c, c);
			TEST_ASSERT(gt_cmp(b, c) == RLC_EQ, end);
		} TEST_END;

		TEST_CASE("random element has the right order") {
			gt_rand(a);
			gt_exp(c, a, n);
			TEST_ASSERT(gt_is_unity(c) == 1, end);
		} TEST_END;

		TEST_CASE("exponentiation is correct") {
			gt_rand(a);
			gt_rand(b);
			bn_rand_mod(d, n);
			bn_rand_mod(e, n);
			gt_exp_sim(c, a, d, b, e);
			gt_exp(a, a, d);
			gt_exp(b, b, e);
			gt_mul(b, a, b);
			TEST_ASSERT(gt_cmp(b, c) == RLC_EQ, end);
			gt_exp_dig(b, a, 0);
			TEST_ASSERT(gt_is_unity(b), end);
			gt_exp_dig(b, a, 1);
			TEST_ASSERT(gt_cmp(a, b) == RLC_EQ, end);
			bn_rand(d, RLC_POS, RLC_DIG);
			gt_exp(b, a, d);
			gt_exp_dig(c, a, d->dp[0]);
			TEST_ASSERT(gt_cmp(b, c) == RLC_EQ, end);
		} TEST_END;
	}
	RLC_CATCH_ANY {
		util_print("FATAL ERROR!\n");
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	gt_free(a);
	gt_free(b);
	gt_free(c);
	bn_free(d);
	bn_free(e);
	bn_free(n);
	return code;
}

static int validity(void) {
	int code = RLC_ERR;
	gt_t a;

	gt_null(a);

	RLC_TRY {
		gt_new(a);

		TEST_CASE("validity check is correct") {
			gt_set_unity(a);
			TEST_ASSERT(!gt_is_valid(a), end);
			gt_rand(a);
			TEST_ASSERT(gt_is_valid(a), end);
		}
		TEST_END;
	}
	RLC_CATCH_ANY {
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	gt_free(a);
	return code;
}

#include <time.h>
static int pairing(void) {
	int j, code = RLC_ERR;
	g1_t p[2];
	g2_t q[2];
	gt_t e1, e2;
	bn_t k, n;

	gt_null(e1);
	gt_null(e2);
	bn_null(k);
	bn_null(n);

	RLC_TRY {
		gt_new(e1);
		gt_new(e2);
		bn_new(k);
		bn_new(n);

		for (j = 0; j < 2; j++) {
			g1_null(p[j]);
			g2_null(q[j]);
			g1_new(p[j]);
			g2_new(q[j]);
		}

		pc_get_ord(n);
		bn_print(n);
		return 0;
		TEST_CASE("pairing non-degeneracy is correct") {
			g1_rand(p[0]);
			g2_rand(q[0]);
			pc_map(e1, p[0], q[0]);
			TEST_ASSERT(gt_cmp_dig(e1, 1) != RLC_EQ, end);
			g1_set_infty(p[0]);
			pc_map(e1, p[0], q[0]);
			TEST_ASSERT(gt_cmp_dig(e1, 1) == RLC_EQ, end);
			g1_rand(p[0]);
			g2_set_infty(q[0]);
			pc_map(e1, p[0], q[0]);
			TEST_ASSERT(gt_cmp_dig(e1, 1) == RLC_EQ, end);
		} TEST_END;

		TEST_CASE("pairing is bilinear") {
			g1_rand(p[0]);
			g2_rand(q[0]);
			bn_rand_mod(k, n);
#if 1
			// 测试曲线配对算法
			clock_t begin, end;
			size_t count=1000;
			begin = clock();
			for (size_t ti = 0; ti < count; ti++)
			{
				pp_map_tatep_k12(e2, p[0], q[0]);
			}
			end = clock();
			printf("total time: %d s, one pairing time: %f s\n", (end-begin)/CLOCKS_PER_SEC, ((double)end-begin)/CLOCKS_PER_SEC/count);
#endif
			g2_mul(q[1], q[0], k);  // q1 = [k]q0
			pc_map(e1, p[0], q[1]);
			pc_map(e2, p[0], q[0]);
			gt_exp(e2, e2, k);
			TEST_ASSERT(gt_cmp(e1, e2) == RLC_EQ, end);
			g1_mul(p[0], p[0], k);
			pc_map(e2, p[0], q[0]);
			TEST_ASSERT(gt_cmp(e1, e2) == RLC_EQ, end);
			g1_dbl(p[0], p[0]);
			pc_map(e2, p[0], q[0]);
			gt_sqr(e1, e1);
			TEST_ASSERT(gt_cmp(e1, e2) == RLC_EQ, end);
			g2_dbl(q[0], q[0]);
			pc_map(e2, p[0], q[0]);
			gt_sqr(e1, e1);
			TEST_ASSERT(gt_cmp(e1, e2) == RLC_EQ, end);
		} TEST_END;

		TEST_CASE("multi-pairing is correct") {
			g1_rand(p[i % 2]);
			g2_rand(q[i % 2]);
			pc_map(e1, p[i % 2], q[i % 2]);
			g1_rand(p[1 - (i % 2)]);
			g2_set_infty(q[1 - (i % 2)]);
			pc_map_sim(e2, p, q, 2);
			TEST_ASSERT(gt_cmp(e1, e2) == RLC_EQ, end);
			g1_set_infty(p[1 - (i % 2)]);
			g2_rand(q[1 - (i % 2)]);
			pc_map_sim(e2, p, q, 2);
			TEST_ASSERT(gt_cmp(e1, e2) == RLC_EQ, end);
			g2_set_infty(q[i % 2]);
			pc_map_sim(e2, p, q, 2);
			TEST_ASSERT(gt_cmp_dig(e2, 1) == RLC_EQ, end);
			g1_rand(p[0]);
			g2_rand(q[0]);
			pc_map(e1, p[0], q[0]);
			g1_rand(p[1]);
			g2_rand(q[1]);
			pc_map(e2, p[1], q[1]);
			gt_mul(e1, e1, e2);
			pc_map_sim(e2, p, q, 2);
			TEST_ASSERT(gt_cmp(e1, e2) == RLC_EQ, end);
		} TEST_END;
	}
	RLC_CATCH_ANY {
		util_print("FATAL ERROR!\n");
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	gt_free(e1);
	gt_free(e2);
	bn_free(k);
	bn_free(n);
	for (j = 0; j < 2; j++) {
		g1_free(p[j]);
		g2_free(q[j]);
	}
	return code;
}

int test1(void) {
	util_banner("Utilities:", 1);

	if (memory1() != RLC_OK) {
		core_clean();
		return 1;
	}

	if (util1() != RLC_OK) {
		return RLC_ERR;
	}

	util_banner("Arithmetic:", 1);

	if (addition1() != RLC_OK) {
		return RLC_ERR;
	}

	if (subtraction1() != RLC_OK) {
		return RLC_ERR;
	}

	if (doubling1() != RLC_OK) {
		return RLC_ERR;
	}

	if (multiplication1() != RLC_OK) {
		return RLC_ERR;
	}

	if (fixed1() != RLC_OK) {
		return RLC_ERR;
	}

	if (simultaneous1() != RLC_OK) {
		return RLC_ERR;
	}

	if (validity1() != RLC_OK) {
		return RLC_ERR;
	}

	if (hashing1() != RLC_OK) {
		return RLC_ERR;
	}

	return RLC_OK;
}

int test2(void) {
	util_banner("Utilities:", 1);

	if (memory2() != RLC_OK) {
		core_clean();
		return 1;
	}

	if (util2() != RLC_OK) {
		return RLC_ERR;
	}

	util_banner("Arithmetic:", 1);

	if (addition2() != RLC_OK) {
		return RLC_ERR;
	}

	if (subtraction2() != RLC_OK) {
		return RLC_ERR;
	}

	if (doubling2() != RLC_OK) {
		return RLC_ERR;
	}

	if (multiplication2() != RLC_OK) {
		return RLC_ERR;
	}

	if (fixed2() != RLC_OK) {
		return RLC_ERR;
	}

	if (simultaneous2() != RLC_OK) {
		return RLC_ERR;
	}

	if (validity2() != RLC_OK) {
		return RLC_ERR;
	}

#if FP_PRIME != 509
	if (hashing2() != RLC_OK) {
		return RLC_ERR;
	}
#endif

	return RLC_OK;
}

int test(void) {
	util_banner("Utilities:", 1);

	// if (memory() != RLC_OK) {
	// 	core_clean();
	// 	return 1;
	// }

	// if (util() != RLC_OK) {
	// 	return RLC_ERR;
	// }

	// util_banner("Arithmetic:", 1);

	// if (multiplication() != RLC_OK) {
	// 	return RLC_ERR;
	// }

	// if (squaring() != RLC_OK) {
	// 	return RLC_ERR;
	// }

	// if (inversion() != RLC_OK) {
	// 	return RLC_ERR;
	// }

	// if (exponentiation() != RLC_OK) {
	// 	return RLC_ERR;
	// }

	// if (validity() != RLC_OK) {
	// 	return RLC_ERR;
	// }

	if (pairing() != RLC_OK) {
		return RLC_ERR;
	}

	return RLC_OK;
}

typedef uint64_t sm9_bn_t[8];
static void init_BETA_ALPHA(){
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

static void free_BETA_ALPHA(){
	fp2_free(SM9_BETA);
	fp_free(SM9_ALPHA1);
	fp_free(SM9_ALPHA2);
	fp_free(SM9_ALPHA3);
	fp_free(SM9_ALPHA4);
	fp_free(SM9_ALPHA5);
}

void bn_to_bits(const sm9_bn_t a, char bits[256])
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
	fp_neg (r[1], a[1]);
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

static void fp4_conjugate(fp4_t r, const fp4_t a)
{
	fp2_copy(r[0], a[0]);
	fp2_neg(r[1], a[1]);
}

static void fp12_mul_t(fp12_t r, const fp12_t a, const fp12_t b)
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

static void fp12_inv_t(fp12_t r, const fp12_t a)
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

static void fp12_sqr_t (fp12_t r, const fp12_t a)
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

	// for (size_t i = 0; i < 8; i++)
	// {
	// 	printf("k[%d]: %d\n", i, k[i]);
	// }
	
	bn_to_bits(k, kbits);
	// printf("bits: %s\n", kbits);
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

static void fp12_frobenius(fp12_t r, const fp12_t x)
{

	const fp2_t *xa = x[0][0];
	const fp2_t *xb = x[0][2];
	const fp2_t *xc = x[1][1];
	fp4_t ra;
	fp4_t rb;
	fp4_t rc;

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
}

static void fp12_frobenius2(fp12_t r, const fp12_t x)
{
	fp4_t a;
	fp4_t b;
	fp4_t c;

	fp4_conjugate(a, x[0][0]);
	fp4_conjugate(b, x[0][2]);
	fp4_mul_fp(b, b, SM9_ALPHA2);
	fp4_conjugate(c, x[1][1]);
	fp4_mul_fp(c, c, SM9_ALPHA4);

	fp4_copy(r[0][0], a);
	fp4_copy(r[0][2], b);
	fp4_copy(r[1][1], c);
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
}

static void ep2_pi1(ep2_t R, const ep2_t P)
{
	//const c = 0x3f23ea58e5720bdb843c6cfa9c08674947c5c86e0ddd04eda91d8354377b698bn;
	// const sm9_fp_t c = {
	// 	0x377b698b, 0xa91d8354, 0x0ddd04ed, 0x47c5c86e,
	// 	0x9c086749, 0x843c6cfa, 0xe5720bdb, 0x3f23ea58,
	// };
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
	// const sm9_fp_t c = {
	// 	0x7be65334, 0xd5fc1196, 0x4f8b78f4, 0x78027235,
	// 	0x02a3a6f2, 0xf3000000, 0, 0,
	// };

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



static void ep2_add_full(ep2_t R, ep2_t P, ep2_t Q)
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
		return;
	}
	if (ep2_is_infty(P)) {
		ep2_copy(R, Q);
		return;
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
}

static void sm9_eval_g_tangent(fp12_t num, fp12_t den, ep2_t P, ep_t Q){
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
	
	// printf("\n num \n");
	// fp12_print(num);

	// printf("\n den \n");
	// fp12_print(den);

	fp2_sqr(t0, ZP);
	fp2_mul(t1, t0, ZP);
	fp2_mul(b1, t1, YP);

	// printf("\n b1 \n");
	// fp2_print(b1);

	fp2_mul_fp(t2, b1, yQ);

	// printf("\n b1 \n");
	// fp2_print(b1);
	// printf("\n yQ \n");
	// fp2_print(yQ);
	// printf("\n t2 \n");
	// fp2_print(t2);

	fp2_neg(a1, t2);
	
	// printf("\n a1 \n");
	// fp2_print(a1);

	fp2_sqr(t1, XP);
	fp2_mul(t0, t0, t1);
	fp2_mul_fp(t0, t0, xQ);
	// printf("\n t0 \n");
	// fp2_print(t0);

	fp2_mul_dig(t0, t0, 3);
	
	// printf("\n t0 \n");
	// fp2_print(t0);

	fp2_mul(a4, t0, two_inv);

	// printf("\n a4 \n");
	// fp2_print(a4);

	fp2_mul(t1, t1, XP);
	fp2_mul_dig(t1, t1, 3);
	fp2_mul(t1, t1, two_inv);
	fp2_sqr(t0, YP);
	fp2_sub(a0, t0, t1);

	// printf("\n a0 \n");
	// fp2_print(a0);
	fp2_free(t0);
	fp2_free(t1);
	fp2_free(t2);
	fp2_free(two_inv);
	bn_free(three);

}

void sm9_final_exponent_hard_part(fp12_t r, const fp12_t f)
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
	// printf("\n hard1 t0 \n");
	// fp12_print(t0);
	fp12_inv_t(t0, t0);
	fp12_frobenius(t1, t0);
	fp12_mul_t(t1, t0, t1);
	// printf("\n hard1 t1 \n");
	// fp12_print(t1);

	fp12_mul_t(t0, t0, t1);
	fp12_frobenius(t2, f);
	fp12_mul_t(t3, t2, f);
	fp12_pow(t3, t3, nine);
	// printf("\n hard2 t3 \n");
	// fp12_print(t3);

	fp12_mul_t(t0, t0, t3);
	fp12_sqr_t(t3, f);
	fp12_sqr_t(t3, t3);
	fp12_mul_t(t0, t0, t3);
	fp12_sqr_t(t2, t2);
	fp12_mul_t(t2, t2, t1);
	fp12_frobenius2(t1, f);
	fp12_mul_t(t1, t1, t2);
	// printf("\n hard3 t1 \n");
	// fp12_print(t1);

	fp12_pow(t2, t1, a2);
	fp12_mul_t(t0, t2, t0);
	fp12_frobenius3(t1, f);
	// printf("\n hard4 frobenius3 \n");
	// fp12_print(t1);
	fp12_mul_t(t1, t1, t0);
	// printf("\n hard4 t1 \n");
	// fp12_print(t1);

	fp12_copy(r, t1);

	fp12_free(t0);
	fp12_free(t1);
	fp12_free(t2);
	fp12_free(t3);
}

void sm9_final_exponent(fp12_t r, const fp12_t f)
{
	fp12_t t0;
	fp12_t t1;

	fp12_null(t0);
	fp12_null(t1);

	fp12_new(t0);
	fp12_new(t1);

	fp12_frobenius6(t0, f);
	// printf("\n epx1 t0 \n");
	// fp12_print(t0);
	
	fp12_inv_t(t1, f);
	// printf("\n f \n");
	// fp12_print(f);
	// printf("\n t1 \n");
	// fp12_print(t1);

	fp12_mul_t(t0, t0, t1);
	// printf("\n epx2 t0 \n");
	// fp12_print(t0);

	fp12_frobenius2(t1, t0);

	// printf("\n epx3 t1 \n");
	// fp12_print(t1);

	fp12_mul_t(t0, t0, t1);

	// printf("\n epx4 t0 \n");
	// fp12_print(t0);

	sm9_final_exponent_hard_part(t0, t0);

	// printf("\n epx5 t0 \n");
	// fp12_print(t0);
	
	fp12_copy(r, t0);

	fp12_free(t0);
	fp12_free(t1);
}

// 调试打印
// 打印ep2
// printf("\n %d: T\n", i);
// ep2_print(T);

// 打印ep
// printf("\n %d: T\n", i);
// ep_print(p);

// 打印fp12
// printf("\n g_num \n", i);
// fp12_print(num);
void test_pairing(fp12_t r, ep2_t Q, ep_t P){
	// a)
	const char *abits = "00100000000000000000000000000000000000010000101011101100100111110";
	// const char *abits = "101";
	
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

	// new
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
		fp12_sqr_t(f_num, f_num);
		fp12_sqr_t(f_den, f_den);

		// printf("\n %d: f_num 1 \n", i);
		// fp12_print(f_num);
		// printf("\n %d: f_den 1 \n", i);
		// fp12_print(f_den);

		sm9_eval_g_tangent(g_num, g_den, T, P);
		fp12_mul_t(f_num, f_num, g_num);
		fp12_mul_t(f_den, f_den, g_den);

		ep2_dbl_projc(T, T);

		// printf("\n %d \n", i);
		// ep2_print(T);

		if (abits[i] == '1')
		{
			sm9_eval_g_line(g_num, g_den, T, Q, P);

			// printf("\n %d: in f_num \n", i);
			// fp12_print(f_num);
			// printf("\n %d: in f_den \n", i);
			// fp12_print(f_den);
			// printf("\n %d: in g_num \n", i);
			// fp12_print(g_num);
			// printf("\n %d: in g_den \n", i);
			// fp12_print(g_den);

			fp12_mul_t(f_num, f_num, g_num);
			fp12_mul_t(f_den, f_den, g_den);

			// printf("\n %d: f_num 2 \n", i);
			// fp12_print(f_num);
			// printf("\n %d: f_den 2 \n", i);
			// fp12_print(f_den);

			ep2_add_full(T, T, Q);  // T = T + Q
		}
	}


	// d)
	ep2_pi1(Q1, Q);  // Q1 = pi_q(Q)
	ep2_pi2(Q2, Q);  // Q2 = pi_{q^2}(Q), Q2 = -Q2

	// printf("\n Q \n");
	// ep2_print(Q);
	// printf("\n Q1 \n");
	// ep2_print(Q1);
	// printf("\n Q2 \n");
	// ep2_print(Q2);
	
	// e)
	sm9_eval_g_line(g_num, g_den, T, Q1, P);  // g = g_{T,Q1}(P)
	fp12_mul_t(f_num, f_num, g_num);  // f = f * g = f * g_{T,Q1}(P)
	fp12_mul_t(f_den, f_den, g_den);
	ep2_add_full(T, T, Q1);  // T = T + Q1

	// f)
	sm9_eval_g_line(g_num, g_den, T, Q2, P);  // g = g_{T,-Q2}(P)
	fp12_mul_t(f_num, f_num, g_num);  // f = f * g = f * g_{T,-Q2}(P)
	fp12_mul_t(f_den, f_den, g_den);
	ep2_add_full(T, T, Q2);  // T = T - Q2

	// printf("\n f_num \n");
	// fp12_print(f_num);
	// printf("\n f_den \n");
	// fp12_print(f_den);

	// g)
	// printf("\n f_den \n");
	// fp12_print(f_den);

	fp12_inv_t(f_den, f_den);  // f_den = f_den^{-1}

	// printf("\n f_den \n");
	// fp12_print(f_den);

	fp12_mul_t(r, f_num, f_den);  // r = f_num*f_den = f

	// printf("\n r \n");
	// fp12_print(r);

	sm9_final_exponent(r, r);  // r = f^{(q^12-1)/r'}

	// printf("\n r \n");
	// fp12_print(r);

	return 0;
}

void init_Ppub(){
	g1_t g1;
	g2_t g2;

	g1_null(g1);
	g1_new(g1);
	g1_get_gen(g1);

	g2_null(g2);
	g2_new(g2);
	g2_get_gen(g2);

	ep2_t Ppub;
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

	fp12_t r;

	fp12_null(r);
	fp12_new(r);

	init_BETA_ALPHA();

	// 测试正确性
	test_pairing(r, Ppub, g1);

	printf("in: Ppub\n");
	ep2_print(Ppub);
	printf("in: g1\n");
	ep_print(g1);
	printf("out: r\n");
	fp12_print(r);
	
	// 测试性能
	clock_t begin, end;
	size_t count=1000;
	begin = clock();
	for (size_t i = 0; i < count; i++)
	{
		test_pairing(r, Ppub, g1);
	}
	end = clock();
	printf("run %d times, total time: %d s, one time: %f s\n", \
       	   count, (end-begin)/CLOCKS_PER_SEC, ((double)end-begin)/CLOCKS_PER_SEC/count);
	free_BETA_ALPHA();
	
}
int main(void) {
	if (core_init() != RLC_OK) {
		core_clean();
		return 1;
	}

	util_banner("Tests for the PC module:", 0);

	if (pc_param_set_any() != RLC_OK) {
		RLC_THROW(ERR_NO_CURVE);
		core_clean();
		return 0;
	}

	pc_param_print();

	init_Ppub();
	return 0;

	// ep_print(ctx->ep_g);
	// util_banner("Group G_1:", 0);
	// if (test1() != RLC_OK) {
	// 	core_clean();
	// 	return 1;
	// }

	// util_banner("Group G_2:", 0);
	// if (test2() != RLC_OK) {
	// 	core_clean();
	// 	return 1;
	// }

	// util_banner("Group G_T:", 0);
	// if (test() != RLC_OK) {
	// 	core_clean();
	// 	return 1;
	// }

	util_banner("All tests have passed.\n", 0);

	core_clean();
	return 0;
}

