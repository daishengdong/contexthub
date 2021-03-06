/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <rsa.h>


static void biMod(uint32_t *num, const uint32_t *denum, uint32_t *tmp) //num %= denum where num is RSA_LEN * 2 and denum is RSA_LEN and tmp is RSA_LEN + limb_sz
{
    uint32_t bitsh = 32, limbsh = RSA_LIMBS - 1;
    int64_t t;
    int32_t i;

    //initially set it up left shifted as far as possible
    memcpy(tmp + 1, denum, RSA_BYTES);
    tmp[0] = 0;
    bitsh = 32;

    while (!(tmp[RSA_LIMBS] & 0x80000000)) {
        for (i = RSA_LIMBS; i > 0; i--) {
            tmp[i] <<= 1;
            if (tmp[i - 1] & 0x80000000)
                tmp[i]++;
        }
        //no need to adjust tmp[0] as it is still zero
        bitsh++;
    }

    while (1) {

        //check if we should subtract (uses less space than subtracting and unroling it later)
        for (i = RSA_LIMBS; i >= 0; i--) {
            if (num[limbsh + i] < tmp[i])
                goto dont_subtract;
            if (num[limbsh + i] > tmp[i])
                break;
        }

        //subtract
        t = 0;
        for (i = 0; i <= RSA_LIMBS; i++) {
            t += (uint64_t)num[limbsh + i];
            t -= (uint64_t)tmp[i];
            num[limbsh + i] = t;
            t >>= 32;
        }

        //carry the subtraction's carry to the end
        for (i = RSA_LIMBS + limbsh + 1; i < RSA_LIMBS * 2; i++) {
            t += (uint64_t)num[i];
            num[i] = t;
            t >>= 32;
        }

dont_subtract:
        //handle bitshifts/refills
        if (!bitsh) {                          // tmp = denum << 32
            if (!limbsh)
                break;

            memcpy(tmp + 1, denum, RSA_BYTES);
            tmp[0] = 0;
            bitsh = 32;
            limbsh--;
        }
        else {                                 // tmp >>= 1
            for (i = 0; i < RSA_LIMBS; i++) {
                tmp[i] >>= 1;
                if (tmp[i + 1] & 1)
                    tmp[i] += 0x80000000;
            }
            tmp[i] >>= 1;
            bitsh--;
        }
    }
}

static void biMul(uint32_t *ret, const uint32_t *a, const uint32_t *b) //ret = a * b
{
    uint32_t i, j, c;
    uint64_t r;

    //zero the result
    memset(ret, 0, RSA_BYTES * 2);

    for (i = 0; i < RSA_LIMBS; i++) {

        //produce a partial sum & add it in
        c = 0;
        for (j = 0; j < RSA_LIMBS; j++) {
            r = (uint64_t)a[i] * b[j] + c + ret[i + j];
            ret[i + j] = r;
            c = r >> 32;
        }

        //carry the carry to the end
        for (j = i + RSA_LIMBS; j < RSA_LIMBS * 2; j++) {
            r = (uint64_t)ret[j] + c;
            ret[j] = r;
            c = r >> 32;
        }
    }
}

const uint32_t* rsaPubOp(struct RsaState* state, const uint32_t *a, const uint32_t *c)
{
    uint32_t i;

    //calculate a ^ 65536 mod c into state->tmpB
    memcpy(state->tmpB, a, RSA_BYTES);
    for (i = 0; i < 16; i++) {
        biMul(state->tmpA, state->tmpB, state->tmpB);
        biMod(state->tmpA, c, state->tmpB);
        memcpy(state->tmpB, state->tmpA, RSA_BYTES);
    }

    //calculate a ^ 65537 mod c into state->tmpA [ at this point this means do state->tmpA = (state->tmpB * a) % c ]
    biMul(state->tmpA, state->tmpB, a);
    biMod(state->tmpA, c, state->tmpB);

    //return result
    return state->tmpA;
}

#if defined(RSA_SUPPORT_PRIV_OP_LOWRAM) || defined (RSA_SUPPORT_PRIV_OP_BIGRAM)
const uint32_t* rsaPrivOp(struct RsaState* state, const uint32_t *a, const uint32_t *b, const uint32_t *c)
{
    uint32_t i;

    memcpy(state->tmpC, a, RSA_BYTES);  //tC will hold our powers of a

    memset(state->tmpA, 0, RSA_BYTES * 2); //tA will hold result
    state->tmpA[0] = 1;

    for (i = 0; i < RSA_LEN; i++) {
        //if the bit is set, multiply the current power of A into result
        if (b[i / 32] & (1 << (i % 32))) {
            memcpy(state->tmpB, state->tmpA, RSA_BYTES);
            biMul(state->tmpA, state->tmpB, state->tmpC);
            biMod(state->tmpA, c, state->tmpB);
        }

        //calculate the next power of a and modulus it
#if defined(RSA_SUPPORT_PRIV_OP_LOWRAM)
        memcpy(state->tmpB, state->tmpA, RSA_BYTES); //save tA
        biMul(state->tmpA, state->tmpC, state->tmpC);
        biMod(state->tmpA, c, state->tmpC);
        memcpy(state->tmpC, state->tmpA, RSA_BYTES);
        memcpy(state->tmpA, state->tmpB, RSA_BYTES); //restore tA
#elif defined (RSA_SUPPORT_PRIV_OP_BIGRAM)
        memcpy(state->tmpB, state->tmpC, RSA_BYTES);
        biMul(state->tmpC, state->tmpB, state->tmpB);
        biMod(state->tmpC, c, state->tmpB);
#endif
    }

    return state->tmpA;
}
#endif








