#ifndef SM9_H
#define SM9_H

#include <stdio.h>

#include "relic.h"

fp_t SM9_ALPHA1, SM9_ALPHA2, SM9_ALPHA3, SM9_ALPHA4, SM9_ALPHA5;
fp2_t SM9_BETA;

typedef uint64_t sm9_bn_t[8];

void sm9_init();
void sm9_clean();
void sm9_pairing();

#endif