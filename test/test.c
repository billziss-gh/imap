/*
 * test.c
 *
 * Copyright 2023 Bill Zissimopoulos
 */
/*
 * This file is part of imap.
 *
 * It is licensed under the MIT license. The full license text can be found
 * in the License.txt file at the root of this project.
 */

#include <tlib/testsuite.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//#define IMAP_USE_SIMD
#define IMAP_ASSERT(expr)               ASSERT(expr)
#include "imap.h"
#include "iset.h"
#include "ivmap.h"

static imap_u64_t seed = 0;
static void test_srand(imap_u64_t s)
{
    seed = s;
}
static imap_u64_t test_rand(void)
{
    /* constants used by Knuth and MUSL */
    seed = seed * 6364136223846793005ULL + 1;
    return seed;
}

static imap_u32_t *test_bsearch(imap_u32_t x, imap_u32_t *array, int count)
{
    int lo = 0, hi = count - 1, mi;
    int di;

    while (lo <= hi)
    {
        mi = (unsigned)(lo + hi) >> 1;
        di = array[mi] - x;
        if (0 > di)
            lo = mi + 1;
        else if (0 < di)
            hi = mi - 1;
        else
            return &array[mi];
    }
    return &array[lo];
}

static int test_concat_sprintf(void *ctx, const char *format, ...)
{
    char **pstr = (char **)ctx;
    va_list ap, ap2;
    va_start(ap, format);
    va_copy(ap2, ap);
    size_t len = *pstr ? strlen(*pstr) : 0;
    int newlen = vsnprintf(0, 0, format, ap);
    char *newstr = (char *)realloc(*pstr, len + newlen + 1);
    if (newstr)
    {
        vsnprintf(newstr + len, newlen + 1, format, ap2);
        *pstr = newstr;
    }
    va_end(ap);
    return newlen;
}

static void imap_primitives_test(void)
{
    imap_u32_t vec32[16];
    imap_u32_t val32;
    imap_u64_t val64;

    memset(vec32, 0, sizeof vec32);
    val64 = 0xFEDCBA9876543210;
    imap__deposit_lo4__(vec32, val64);
    val64 = imap__extract_lo4__(vec32);
    ASSERT(0xFEDCBA9876543210ull == val64);

    memset(vec32, 0, sizeof vec32);
    ASSERT(0 == imap__popcnt_hi28__(vec32, &val32));
    memset(vec32, 0, sizeof vec32);
    vec32[0] = 0xff;
    ASSERT(1 == imap__popcnt_hi28__(vec32, &val32) && 0xff == val32);
    memset(vec32, 0, sizeof vec32);
    vec32[1] = 0xef, vec32[3] = 0xd0;
    ASSERT(2 == imap__popcnt_hi28__(vec32, &val32));
    memset(vec32, 0, sizeof vec32);
    vec32[3] = 0xd0;
    ASSERT(1 == imap__popcnt_hi28__(vec32, &val32) && 0xd0 == val32);
}

static void imap_ensure_test(void)
{
    imap_node_t *tree, *tree2;

    tree = 0;
    tree = imap_ensure(tree, 0);
    ASSERT(0 == tree);
    imap_free(tree);

    tree = 0;
    tree = imap_ensure(tree, +1);
    ASSERT(0 != tree);
    imap_free(tree);

    tree = 0;
    tree = imap_ensure(tree, +1);
    ASSERT(0 != tree);
    tree2 = imap_ensure(tree, +1);
    ASSERT(tree == tree2);
    imap_free(tree);
}

static void imap_assign_test(void)
{
    imap_node_t *tree;
    imap_slot_t *slot;

    tree = 0;
    tree = imap_ensure(tree, +1);
    ASSERT(0 != tree);
    slot = imap_lookup(tree, 0xA0000056);
    ASSERT(0 == slot);
    slot = imap_assign(tree, 0xA0000056);
    ASSERT(0 != slot);
    ASSERT(!imap_hasval(tree, slot));
    ASSERT(0 == imap_getval(tree, slot));
    imap_setval(tree, slot, 0x56);
    ASSERT(imap_hasval(tree, slot));
    ASSERT(0x56 == imap_getval(tree, slot));
    slot = imap_lookup(tree, 0xA0000056);
    ASSERT(0 != slot);
    imap_delval(tree, slot);
    ASSERT(!imap_hasval(tree, slot));
    slot = imap_lookup(tree, 0xA0000056);
    ASSERT(0 == slot);
    imap_free(tree);

    tree = 0;
    tree = imap_ensure(tree, +2);
    ASSERT(0 != tree);
    slot = imap_assign(tree, 0xA0000056);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x56);
    slot = imap_assign(tree, 0xA0000057);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x57);
    slot = imap_lookup(tree, 0xA0000056);
    ASSERT(0 != slot);
    ASSERT(0x56 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0000057);
    ASSERT(0 != slot);
    ASSERT(0x57 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0000056);
    ASSERT(0 == slot);
    slot = imap_lookup(tree, 0xA0000057);
    ASSERT(0 == slot);
    imap_free(tree);

    tree = 0;
    tree = imap_ensure(tree, +3);
    ASSERT(0 != tree);
    slot = imap_assign(tree, 0xA0000056);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x56);
    slot = imap_assign(tree, 0xA0000057);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x57);
    slot = imap_assign(tree, 0xA0008009);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x8009);
    slot = imap_lookup(tree, 0xA0000056);
    ASSERT(0 != slot);
    ASSERT(0x56 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0000057);
    ASSERT(0 != slot);
    ASSERT(0x57 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0008009);
    ASSERT(0 != slot);
    ASSERT(0x8009 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0000056);
    ASSERT(0 == slot);
    slot = imap_lookup(tree, 0xA0000057);
    ASSERT(0 == slot);
    slot = imap_lookup(tree, 0xA0008009);
    ASSERT(0 == slot);
    imap_free(tree);

    tree = 0;
    tree = imap_ensure(tree, +4);
    ASSERT(0 != tree);
    slot = imap_assign(tree, 0xA0000056);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x56);
    slot = imap_assign(tree, 0xA0000057);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x57);
    slot = imap_assign(tree, 0xA0008009);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x8009);
    slot = imap_assign(tree, 0xA0008059);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x8059);
    slot = imap_lookup(tree, 0xA0000056);
    ASSERT(0 != slot);
    ASSERT(0x56 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0000057);
    ASSERT(0 != slot);
    ASSERT(0x57 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0008009);
    ASSERT(0 != slot);
    ASSERT(0x8009 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0008059);
    ASSERT(0 != slot);
    ASSERT(0x8059 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0000056);
    ASSERT(0 == slot);
    slot = imap_lookup(tree, 0xA0000057);
    ASSERT(0 == slot);
    slot = imap_lookup(tree, 0xA0008009);
    ASSERT(0 == slot);
    slot = imap_lookup(tree, 0xA0008059);
    ASSERT(0 == slot);
    imap_free(tree);

    tree = 0;
    tree = imap_ensure(tree, +5);
    ASSERT(0 != tree);
    slot = imap_assign(tree, 0xA0000056);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x56);
    slot = imap_assign(tree, 0xA0000057);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x57);
    slot = imap_assign(tree, 0xA0008009);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x8009);
    slot = imap_assign(tree, 0xA0008059);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x8059);
    slot = imap_assign(tree, 0xA0008069);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x8069);
    slot = imap_lookup(tree, 0xA0000056);
    ASSERT(0 != slot);
    ASSERT(0x56 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0000057);
    ASSERT(0 != slot);
    ASSERT(0x57 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0008009);
    ASSERT(0 != slot);
    ASSERT(0x8009 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0008059);
    ASSERT(0 != slot);
    ASSERT(0x8059 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0008069);
    ASSERT(0 != slot);
    ASSERT(0x8069 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0000056);
    ASSERT(0 == slot);
    slot = imap_lookup(tree, 0xA0000057);
    ASSERT(0 == slot);
    slot = imap_lookup(tree, 0xA0008009);
    ASSERT(0 == slot);
    slot = imap_lookup(tree, 0xA0008059);
    ASSERT(0 == slot);
    slot = imap_lookup(tree, 0xA0008069);
    ASSERT(0 == slot);
    imap_free(tree);
}

static void imap_assign_bigval_test(void)
{
    const unsigned N = 100;
    imap_node_t *tree = 0;
    imap_slot_t *slot;

    for (unsigned i = 0; N > i; i++)
    {
        tree = imap_ensure(tree, +1);
        ASSERT(0 != tree);
        slot = imap_assign(tree, i);
        ASSERT(0 != slot);
        imap_setval(tree, slot, 0x8000000000000000ull | i);
    }
    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 != slot);
        ASSERT(imap_hasval(tree, slot));
        ASSERT((0x8000000000000000ull | i) == imap_getval(tree, slot));
    }

    for (unsigned i = 0; N > i; i++)
    {
        tree = imap_ensure(tree, +1);
        ASSERT(0 != tree);
        slot = imap_assign(tree, i);
        ASSERT(0 != slot);
        imap_setval(tree, slot, i);
    }
    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 != slot);
        ASSERT(imap_hasval(tree, slot));
        ASSERT(i == imap_getval(tree, slot));
    }

    for (unsigned i = 0; N > i; i++)
    {
        tree = imap_ensure(tree, +1);
        ASSERT(0 != tree);
        slot = imap_assign(tree, i);
        ASSERT(0 != slot);
        imap_setval(tree, slot, 0x8000000000000000ull | i);
    }
    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 != slot);
        ASSERT(imap_hasval(tree, slot));
        ASSERT((0x8000000000000000ull | i) == imap_getval(tree, slot));
    }

    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 != slot);
        imap_delval(tree, slot);
        ASSERT(!imap_hasval(tree, slot));
    }
    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 == slot);
    }

    for (unsigned i = 0; N > i; i++)
    {
        tree = imap_ensure(tree, +1);
        ASSERT(0 != tree);
        slot = imap_assign(tree, i);
        ASSERT(0 != slot);
        imap_setval(tree, slot, 0x8000000000000000ull | i);
    }
    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 != slot);
        ASSERT(imap_hasval(tree, slot));
        ASSERT((0x8000000000000000ull | i) == imap_getval(tree, slot));
    }

    imap_free(tree);
}

static void imap_assign_val0_test(void)
{
    const unsigned N = 100;
    imap_node_t *tree = 0;
    imap_slot_t *slot;

    for (unsigned i = 0; N > i; i++)
    {
        tree = imap_ensure0(tree, +1);
        ASSERT(0 != tree);
        slot = imap_assign(tree, i);
        ASSERT(0 != slot);
        imap_setval0(tree, slot, 0x800000ull | i);
    }
    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 != slot);
        ASSERT(imap_hasval(tree, slot));
        ASSERT((0x800000ull | i) == imap_getval0(tree, slot));
    }

    for (unsigned i = 0; N > i; i++)
    {
        tree = imap_ensure0(tree, +1);
        ASSERT(0 != tree);
        slot = imap_assign(tree, i);
        ASSERT(0 != slot);
        imap_setval0(tree, slot, i);
    }
    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 != slot);
        ASSERT(imap_hasval(tree, slot));
        ASSERT(i == imap_getval0(tree, slot));
    }

    for (unsigned i = 0; N > i; i++)
    {
        tree = imap_ensure0(tree, +1);
        ASSERT(0 != tree);
        slot = imap_assign(tree, i);
        ASSERT(0 != slot);
        imap_setval0(tree, slot, 0x800000ull | i);
    }
    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 != slot);
        ASSERT(imap_hasval(tree, slot));
        ASSERT((0x800000ull | i) == imap_getval0(tree, slot));
    }

    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 != slot);
        imap_delval(tree, slot);
        ASSERT(!imap_hasval(tree, slot));
    }
    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 == slot);
    }

    for (unsigned i = 0; N > i; i++)
    {
        tree = imap_ensure0(tree, +1);
        ASSERT(0 != tree);
        slot = imap_assign(tree, i);
        ASSERT(0 != slot);
        imap_setval0(tree, slot, 0x800000ull | i);
    }
    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 != slot);
        ASSERT(imap_hasval(tree, slot));
        ASSERT((0x800000ull | i) == imap_getval0(tree, slot));
    }

    imap_free(tree);
}

static void imap_assign_val64_test(void)
{
    const unsigned N = 100;
    imap_node_t *tree = 0;
    imap_slot_t *slot;

    for (unsigned i = 0; N > i; i++)
    {
        tree = imap_ensure64(tree, +1);
        ASSERT(0 != tree);
        slot = imap_assign(tree, i);
        ASSERT(0 != slot);
        imap_setval64(tree, slot, 0x8000000000000000ull | i);
    }
    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 != slot);
        ASSERT(imap_hasval(tree, slot));
        ASSERT((0x8000000000000000ull | i) == imap_getval64(tree, slot));
    }

    for (unsigned i = 0; N > i; i++)
    {
        tree = imap_ensure64(tree, +1);
        ASSERT(0 != tree);
        slot = imap_assign(tree, i);
        ASSERT(0 != slot);
        imap_setval64(tree, slot, i);
    }
    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 != slot);
        ASSERT(imap_hasval(tree, slot));
        ASSERT(i == imap_getval64(tree, slot));
    }

    for (unsigned i = 0; N > i; i++)
    {
        tree = imap_ensure64(tree, +1);
        ASSERT(0 != tree);
        slot = imap_assign(tree, i);
        ASSERT(0 != slot);
        imap_setval64(tree, slot, 0x8000000000000000ull | i);
    }
    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 != slot);
        ASSERT(imap_hasval(tree, slot));
        ASSERT((0x8000000000000000ull | i) == imap_getval64(tree, slot));
    }

    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 != slot);
        imap_delval(tree, slot);
        ASSERT(!imap_hasval(tree, slot));
    }
    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 == slot);
    }

    for (unsigned i = 0; N > i; i++)
    {
        tree = imap_ensure(tree, +1);
        ASSERT(0 != tree);
        slot = imap_assign(tree, i);
        ASSERT(0 != slot);
        imap_setval64(tree, slot, 0x8000000000000000ull | i);
    }
    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 != slot);
        ASSERT(imap_hasval(tree, slot));
        ASSERT((0x8000000000000000ull | i) == imap_getval64(tree, slot));
    }

    imap_free(tree);
}

static void imap_assign_val128_test(void)
{
    const unsigned N = 100;
    imap_node_t *tree = 0;
    imap_slot_t *slot;
    imap_u128_t val128;

    for (unsigned i = 0; N > i; i++)
    {
        tree = imap_ensure128(tree, +1);
        ASSERT(0 != tree);
        slot = imap_assign(tree, i);
        ASSERT(0 != slot);
        val128.v[0] = 0x8000000000000000ull | i;
        val128.v[1] = 0x9000000000000000ull | i;
        imap_setval128(tree, slot, val128);
    }
    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 != slot);
        ASSERT(imap_hasval(tree, slot));
        ASSERT((0x8000000000000000ull | i) == imap_getval128(tree, slot).v[0]);
        ASSERT((0x9000000000000000ull | i) == imap_getval128(tree, slot).v[1]);
    }

    for (unsigned i = 0; N > i; i++)
    {
        tree = imap_ensure128(tree, +1);
        ASSERT(0 != tree);
        slot = imap_assign(tree, i);
        ASSERT(0 != slot);
        val128.v[0] = i;
        val128.v[1] = i;
        imap_setval128(tree, slot, val128);
    }
    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 != slot);
        ASSERT(imap_hasval(tree, slot));
        ASSERT(i == imap_getval128(tree, slot).v[0]);
        ASSERT(i == imap_getval128(tree, slot).v[1]);
    }

    for (unsigned i = 0; N > i; i++)
    {
        tree = imap_ensure(tree, +1);
        ASSERT(0 != tree);
        slot = imap_assign(tree, i);
        ASSERT(0 != slot);
        val128.v[0] = 0x8000000000000000ull | i;
        val128.v[1] = 0x9000000000000000ull | i;
        imap_setval128(tree, slot, val128);
    }
    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 != slot);
        ASSERT(imap_hasval(tree, slot));
        ASSERT((0x8000000000000000ull | i) == imap_getval128(tree, slot).v[0]);
        ASSERT((0x9000000000000000ull | i) == imap_getval128(tree, slot).v[1]);
    }

    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 != slot);
        imap_delval(tree, slot);
        ASSERT(!imap_hasval(tree, slot));
    }
    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 == slot);
    }

    for (unsigned i = 0; N > i; i++)
    {
        tree = imap_ensure(tree, +1);
        ASSERT(0 != tree);
        slot = imap_assign(tree, i);
        ASSERT(0 != slot);
        val128.v[0] = 0x8000000000000000ull | i;
        val128.v[1] = 0x9000000000000000ull | i;
        imap_setval128(tree, slot, val128);
    }
    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, i);
        ASSERT(0 != slot);
        ASSERT(imap_hasval(tree, slot));
        ASSERT((0x8000000000000000ull | i) == imap_getval128(tree, slot).v[0]);
        ASSERT((0x9000000000000000ull | i) == imap_getval128(tree, slot).v[1]);
    }

    imap_free(tree);
}

static void imap_assign_shuffle_dotest(imap_u64_t seed)
{
    const unsigned N = 10000000;
    imap_u32_t *array;
    imap_node_t *tree = 0;
    imap_slot_t *slot;

    tlib_printf("seed=%llu ", (unsigned long long)seed);
    test_srand(seed);

    array = (imap_u32_t *)malloc(N * sizeof(imap_u32_t));
    ASSERT(0 != array);

    for (unsigned i = 0; N > i; i++)
        array[i] = i;
    for (unsigned i = 0; N > i; i++)
    {
        imap_u32_t r = test_rand() % N;
        imap_u32_t t = array[i];
        array[i] = array[r];
        array[r] = t;
    }

    for (unsigned i = 0; N > i; i++)
    {
        tree = imap_ensure(tree, +1);
        ASSERT(0 != tree);
        slot = imap_assign(tree, array[i]);
        ASSERT(0 != slot);
        imap_setval(tree, slot, i);
    }
    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, array[i]);
        ASSERT(0 != slot);
        ASSERT(i == imap_getval(tree, slot));
    }

    imap_free(tree);

    free(array);
}

static void imap_assign_shuffle_test(void)
{
    imap_assign_shuffle_dotest(time(0));
}

static void imap_remove_test(void)
{
    imap_node_t *tree;
    imap_slot_t *slot;

    tree = 0;
    tree = imap_ensure(tree, +1);
    ASSERT(0 != tree);
    imap_remove(tree, 0);
    imap_free(tree);

    tree = 0;
    tree = imap_ensure(tree, +5);
    ASSERT(0 != tree);
    slot = imap_assign(tree, 0xA0000056);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x56);
    slot = imap_assign(tree, 0xA0000057);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x57);
    slot = imap_assign(tree, 0xA0008009);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x8009);
    slot = imap_assign(tree, 0xA0008059);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x8059);
    slot = imap_assign(tree, 0xA0008069);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x8069);

    slot = imap_lookup(tree, 0xA0000056);
    ASSERT(0 != slot);
    ASSERT(0x56 == imap_getval(tree, slot));
    imap_remove(tree, 0xA0000056);
    slot = imap_lookup(tree, 0xA0000056);
    ASSERT(0 == slot);

    slot = imap_lookup(tree, 0xA0000057);
    ASSERT(0 != slot);
    ASSERT(0x57 == imap_getval(tree, slot));
    imap_remove(tree, 0xA0000057);
    slot = imap_lookup(tree, 0xA0000057);
    ASSERT(0 == slot);

    slot = imap_lookup(tree, 0xA0008009);
    ASSERT(0 != slot);
    ASSERT(0x8009 == imap_getval(tree, slot));
    imap_remove(tree, 0xA0008009);
    slot = imap_lookup(tree, 0xA0008009);
    ASSERT(0 == slot);

    slot = imap_lookup(tree, 0xA0008059);
    ASSERT(0 != slot);
    ASSERT(0x8059 == imap_getval(tree, slot));
    imap_remove(tree, 0xA0008059);
    slot = imap_lookup(tree, 0xA0008059);
    ASSERT(0 == slot);

    slot = imap_lookup(tree, 0xA0008069);
    ASSERT(0 != slot);
    ASSERT(0x8069 == imap_getval(tree, slot));
    imap_remove(tree, 0xA0008069);
    slot = imap_lookup(tree, 0xA0008069);
    ASSERT(0 == slot);

    imap_u64_t mark = tree->vec32[imap__tree_mark__];

    tree = imap_ensure(tree, +5);
    ASSERT(0 != tree);
    slot = imap_assign(tree, 0xA0000056);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x56);
    slot = imap_assign(tree, 0xA0000057);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x57);
    slot = imap_assign(tree, 0xA0008009);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x8009);
    slot = imap_assign(tree, 0xA0008059);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x8059);
    slot = imap_assign(tree, 0xA0008069);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x8069);
    slot = imap_lookup(tree, 0xA0000056);
    ASSERT(0 != slot);
    ASSERT(0x56 == imap_getval(tree, slot));
    slot = imap_lookup(tree, 0xA0000057);
    ASSERT(0 != slot);
    ASSERT(0x57 == imap_getval(tree, slot));
    slot = imap_lookup(tree, 0xA0008009);
    ASSERT(0 != slot);
    ASSERT(0x8009 == imap_getval(tree, slot));
    slot = imap_lookup(tree, 0xA0008059);
    ASSERT(0 != slot);
    ASSERT(0x8059 == imap_getval(tree, slot));
    slot = imap_lookup(tree, 0xA0008069);
    ASSERT(0 != slot);
    ASSERT(0x8069 == imap_getval(tree, slot));

    ASSERT(mark == tree->vec32[imap__tree_mark__]);

    imap_free(tree);
}

static void imap_remove_shuffle_dotest(imap_u64_t seed)
{
    const unsigned N = 10000000;
    imap_u32_t *array;
    imap_node_t *tree = 0;
    imap_slot_t *slot;

    tlib_printf("seed=%llu ", (unsigned long long)seed);
    test_srand(seed);

    array = (imap_u32_t *)malloc(N * sizeof(imap_u32_t));
    ASSERT(0 != array);

    for (unsigned i = 0; N > i; i++)
        array[i] = i;

    for (unsigned i = 0; N > i; i++)
    {
        tree = imap_ensure(tree, +1);
        ASSERT(0 != tree);
        slot = imap_assign(tree, array[i]);
        ASSERT(0 != slot);
        imap_setval(tree, slot, i);
    }
    for (unsigned i = 0; N > i; i++)
    {
        slot = imap_lookup(tree, array[i]);
        ASSERT(0 != slot);
        ASSERT(i == imap_getval(tree, slot));
    }

    for (unsigned i = 0; N > i; i++)
    {
        imap_u32_t r = test_rand() % N;
        imap_u32_t t = array[i];
        array[i] = array[r];
        array[r] = t;
    }

    for (unsigned i = 0; N / 2 > i; i++)
        imap_remove(tree, array[i]);
    for (unsigned i = 0; N / 2 > i; i++)
    {
        slot = imap_lookup(tree, array[i]);
        ASSERT(0 == slot);
    }
    for (unsigned i = N / 2; N > i; i++)
    {
        slot = imap_lookup(tree, array[i]);
        ASSERT(0 != slot);
        ASSERT(array[i] == imap_getval(tree, slot));
    }
    for (unsigned i = N / 2; N > i; i++)
        imap_remove(tree, array[i]);
    for (unsigned i = N / 2; N > i; i++)
    {
        slot = imap_lookup(tree, array[i]);
        ASSERT(0 == slot);
    }

    ASSERT(0 == tree->vec32[0]);

    imap_free(tree);

    free(array);
}

static void imap_remove_shuffle_test(void)
{
    imap_remove_shuffle_dotest(time(0));
}

static void imap_iterate_test(void)
{
    imap_node_t *tree;
    imap_slot_t *slot;
    imap_iter_t iter;
    imap_pair_t pair;

    tree = 0;
    tree = imap_ensure(tree, +1);
    ASSERT(0 != tree);
    pair = imap_iterate(tree, &iter, 1);
    ASSERT(0 == pair.x && 0 == pair.slot);
    imap_free(tree);

    tree = 0;
    tree = imap_ensure(tree, +5);
    ASSERT(0 != tree);
    slot = imap_assign(tree, 0xA0000056);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x56);
    slot = imap_assign(tree, 0xA0000057);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x57);
    slot = imap_assign(tree, 0xA0008009);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x8009);
    slot = imap_assign(tree, 0xA0008059);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x8059);
    slot = imap_assign(tree, 0xA0008069);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x8069);
    //
    pair = imap_iterate(tree, &iter, 1);
    ASSERT(0xA0000056 == pair.x && 0 != pair.slot);
    ASSERT(0x56 == imap_getval(tree, pair.slot));
    pair = imap_iterate(tree, &iter, 0);
    ASSERT(0xA0000057 == pair.x && 0 != pair.slot);
    ASSERT(0x57 == imap_getval(tree, pair.slot));
    pair = imap_iterate(tree, &iter, 0);
    ASSERT(0xA0008009 == pair.x && 0 != pair.slot);
    ASSERT(0x8009 == imap_getval(tree, pair.slot));
    pair = imap_iterate(tree, &iter, 0);
    ASSERT(0xA0008059 == pair.x && 0 != pair.slot);
    ASSERT(0x8059 == imap_getval(tree, pair.slot));
    pair = imap_iterate(tree, &iter, 0);
    ASSERT(0xA0008069 == pair.x && 0 != pair.slot);
    ASSERT(0x8069 == imap_getval(tree, pair.slot));
    pair = imap_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x && 0 == pair.slot);
    //
    slot = imap_lookup(tree, 0xA0000056);
    ASSERT(0 != slot);
    ASSERT(0x56 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0000057);
    ASSERT(0 != slot);
    ASSERT(0x57 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0008009);
    ASSERT(0 != slot);
    ASSERT(0x8009 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0008059);
    ASSERT(0 != slot);
    ASSERT(0x8059 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0008069);
    ASSERT(0 != slot);
    ASSERT(0x8069 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0000056);
    ASSERT(0 == slot);
    slot = imap_lookup(tree, 0xA0000057);
    ASSERT(0 == slot);
    slot = imap_lookup(tree, 0xA0008009);
    ASSERT(0 == slot);
    slot = imap_lookup(tree, 0xA0008059);
    ASSERT(0 == slot);
    slot = imap_lookup(tree, 0xA0008069);
    ASSERT(0 == slot);
    imap_free(tree);

    const unsigned N = 257;
    tree = 0;
    for (unsigned i = 0; N > i; i++)
    {
        tree = imap_ensure(tree, +1);
        ASSERT(0 != tree);
        slot = imap_assign(tree, i);
        ASSERT(0 != slot);
        imap_setval(tree, slot, i);
    }
    pair = imap_iterate(tree, &iter, 1);
    for (unsigned i = 0; N > i; i++)
    {
        ASSERT(i == pair.x);
        ASSERT(0 != pair.slot);
        ASSERT(i == imap_getval(tree, pair.slot));
        pair = imap_iterate(tree, &iter, 0);
    }
    ASSERT(0 == pair.x && 0 == pair.slot);
    imap_free(tree);
}

static int u32cmp(const void *x, const void *y)
{
    return (int)(*(imap_u32_t *)x - *(imap_u32_t *)y);
}

static void imap_iterate_shuffle_dotest(imap_u64_t seed)
{
    const unsigned N = 10000000;
    imap_u32_t *array;
    imap_node_t *tree = 0;
    imap_slot_t *slot;
    imap_iter_t iter;
    imap_pair_t pair;

    tlib_printf("seed=%llu ", (unsigned long long)seed);
    test_srand(seed);

    array = (imap_u32_t *)malloc(N * sizeof(imap_u32_t));
    ASSERT(0 != array);

    for (unsigned i = 0; N > i; i++)
        array[i] = i;

    for (unsigned i = 0; N > i; i++)
    {
        tree = imap_ensure(tree, +1);
        ASSERT(0 != tree);
        slot = imap_assign(tree, array[i]);
        ASSERT(0 != slot);
        imap_setval(tree, slot, array[i]);
    }
    pair = imap_iterate(tree, &iter, 1);
    for (unsigned i = 0; N > i; i++)
    {
        ASSERT(array[i] == pair.x);
        ASSERT(0 != pair.slot);
        ASSERT(array[i] == imap_getval(tree, pair.slot));
        pair = imap_iterate(tree, &iter, 0);
    }
    ASSERT(0 == pair.x && 0 == pair.slot);

    for (unsigned i = 0; N > i; i++)
    {
        imap_u32_t r = test_rand() % N;
        imap_u32_t t = array[i];
        array[i] = array[r];
        array[r] = t;
    }

    for (unsigned i = 0; N / 2 > i; i++)
        imap_remove(tree, array[i]);

    qsort(array + N / 2, N / 2, sizeof array[0], u32cmp);

    pair = imap_iterate(tree, &iter, 1);
    for (unsigned i = N / 2; N > i; i++)
    {
        ASSERT(array[i] == pair.x);
        ASSERT(0 != pair.slot);
        ASSERT(array[i] == imap_getval(tree, pair.slot));
        pair = imap_iterate(tree, &iter, 0);
    }
    ASSERT(0 == pair.x && 0 == pair.slot);

    imap_free(tree);

    free(array);
}

static void imap_iterate_shuffle_test(void)
{
    imap_iterate_shuffle_dotest(time(0));
}

static void imap_locate_test(void)
{
    imap_node_t *tree;
    imap_slot_t *slot;
    imap_iter_t iter;
    imap_pair_t pair;

    tree = 0;
    tree = imap_ensure(tree, +1);
    ASSERT(0 != tree);
    pair = imap_locate(tree, &iter, 0);
    ASSERT(0 == pair.x && 0 == pair.slot);
    imap_free(tree);

    tree = 0;
    tree = imap_ensure(tree, +1);
    ASSERT(0 != tree);
    slot = imap_assign(tree, 1200);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 1100);
    pair = imap_locate(tree, &iter, 1100);
    ASSERT(1200 == pair.x && 0 != pair.slot && 1100 == imap_getval(tree, slot));
    pair = imap_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x && 0 == pair.slot);
    imap_free(tree);

    tree = 0;
    tree = imap_ensure(tree, +1);
    ASSERT(0 != tree);
    slot = imap_assign(tree, 1200);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 1100);
    tree = imap_ensure(tree, +1);
    ASSERT(0 != tree);
    slot = imap_assign(tree, 1100);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 1000);
    pair = imap_locate(tree, &iter, 1300);
    ASSERT(0 == pair.x && 0 == pair.slot);
    imap_free(tree);

    tree = 0;
    tree = imap_ensure(tree, +5);
    ASSERT(0 != tree);
    slot = imap_assign(tree, 0xA0000056);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x56);
    slot = imap_assign(tree, 0xA0000057);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x57);
    slot = imap_assign(tree, 0xA0008009);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x8009);
    slot = imap_assign(tree, 0xA0008059);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x8059);
    slot = imap_assign(tree, 0xA0008069);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x8069);
    //
    pair = imap_locate(tree, &iter, 1100);
    ASSERT(0xA0000056 == pair.x && 0 != pair.slot);
    ASSERT(0x56 == imap_getval(tree, pair.slot));
    pair = imap_iterate(tree, &iter, 0);
    ASSERT(0xA0000057 == pair.x && 0 != pair.slot);
    ASSERT(0x57 == imap_getval(tree, pair.slot));
    pair = imap_iterate(tree, &iter, 0);
    ASSERT(0xA0008009 == pair.x && 0 != pair.slot);
    ASSERT(0x8009 == imap_getval(tree, pair.slot));
    pair = imap_iterate(tree, &iter, 0);
    ASSERT(0xA0008059 == pair.x && 0 != pair.slot);
    ASSERT(0x8059 == imap_getval(tree, pair.slot));
    pair = imap_iterate(tree, &iter, 0);
    ASSERT(0xA0008069 == pair.x && 0 != pair.slot);
    ASSERT(0x8069 == imap_getval(tree, pair.slot));
    pair = imap_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x && 0 == pair.slot);
    //
    //
    pair = imap_locate(tree, &iter, 0xA0000057);
    ASSERT(0xA0000057 == pair.x && 0 != pair.slot);
    ASSERT(0x57 == imap_getval(tree, pair.slot));
    pair = imap_iterate(tree, &iter, 0);
    ASSERT(0xA0008009 == pair.x && 0 != pair.slot);
    ASSERT(0x8009 == imap_getval(tree, pair.slot));
    pair = imap_iterate(tree, &iter, 0);
    ASSERT(0xA0008059 == pair.x && 0 != pair.slot);
    ASSERT(0x8059 == imap_getval(tree, pair.slot));
    pair = imap_iterate(tree, &iter, 0);
    ASSERT(0xA0008069 == pair.x && 0 != pair.slot);
    ASSERT(0x8069 == imap_getval(tree, pair.slot));
    pair = imap_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x && 0 == pair.slot);
    //
    //
    pair = imap_locate(tree, &iter, 0xA0007000);
    ASSERT(0xA0008009 == pair.x && 0 != pair.slot);
    ASSERT(0x8009 == imap_getval(tree, pair.slot));
    pair = imap_iterate(tree, &iter, 0);
    ASSERT(0xA0008059 == pair.x && 0 != pair.slot);
    ASSERT(0x8059 == imap_getval(tree, pair.slot));
    pair = imap_iterate(tree, &iter, 0);
    ASSERT(0xA0008069 == pair.x && 0 != pair.slot);
    ASSERT(0x8069 == imap_getval(tree, pair.slot));
    pair = imap_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x && 0 == pair.slot);
    //
    slot = imap_lookup(tree, 0xA0000056);
    ASSERT(0 != slot);
    ASSERT(0x56 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0000057);
    ASSERT(0 != slot);
    ASSERT(0x57 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0008009);
    ASSERT(0 != slot);
    ASSERT(0x8009 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0008059);
    ASSERT(0 != slot);
    ASSERT(0x8059 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0008069);
    ASSERT(0 != slot);
    ASSERT(0x8069 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0000056);
    ASSERT(0 == slot);
    slot = imap_lookup(tree, 0xA0000057);
    ASSERT(0 == slot);
    slot = imap_lookup(tree, 0xA0008009);
    ASSERT(0 == slot);
    slot = imap_lookup(tree, 0xA0008059);
    ASSERT(0 == slot);
    slot = imap_lookup(tree, 0xA0008069);
    ASSERT(0 == slot);
    imap_free(tree);
}

static void imap_locate_random_dotest(imap_u64_t seed)
{
    const unsigned N = 10000000;
    const unsigned M = 1000000;
    imap_u32_t *array;
    imap_node_t *tree = 0;
    imap_slot_t *slot;
    imap_iter_t iter;
    imap_pair_t pair;
    imap_u32_t r, *p;

    tlib_printf("seed=%llu ", (unsigned long long)seed);
    test_srand(seed);

    array = (imap_u32_t *)malloc(N * sizeof(imap_u32_t));
    ASSERT(0 != array);

    for (unsigned i = 0; N > i; i++)
        array[i] = 0x1000000 | (test_rand() & 0x3ffffff);

    for (unsigned i = 0; N > i; i++)
    {
        tree = imap_ensure(tree, +1);
        ASSERT(0 != tree);
        slot = imap_assign(tree, array[i]);
        ASSERT(0 != slot);
        imap_setval(tree, slot, array[i]);
    }

    qsort(array, N, sizeof array[0], u32cmp);

    for (unsigned i = 0; M > i; i++)
    {
        r = test_rand() & 0x3ffffff;
        pair = imap_locate(tree, &iter, r);
        p = test_bsearch(r, array, N);
        if (array + N > p)
        {
            ASSERT(*p == pair.x);
            ASSERT(0 != pair.slot);
            ASSERT(*p == imap_getval(tree, pair.slot));
            pair = imap_iterate(tree, &iter, 0);
            p++;
            if (array + N > p)
            {
                ASSERT(*p == pair.x);
                ASSERT(0 != pair.slot);
                ASSERT(*p == imap_getval(tree, pair.slot));
            }
            else
            {
                ASSERT(0 == pair.x);
                ASSERT(0 == pair.slot);
            }
        }
        else
        {
            ASSERT(0 == pair.x);
            ASSERT(0 == pair.slot);
        }
    }

    for (unsigned i = 0; M > i; i++)
    {
        r = test_rand() % N;
        pair = imap_locate(tree, &iter, array[r]);
        p = test_bsearch(array[r], array, N);
        if (array + N > p)
        {
            ASSERT(*p == pair.x);
            ASSERT(0 != pair.slot);
            ASSERT(*p == imap_getval(tree, pair.slot));
            pair = imap_iterate(tree, &iter, 0);
            p++;
            if (array + N > p)
            {
                ASSERT(*p == pair.x);
                ASSERT(0 != pair.slot);
                ASSERT(*p == imap_getval(tree, pair.slot));
            }
            else
            {
                ASSERT(0 == pair.x);
                ASSERT(0 == pair.slot);
            }
        }
        else
        {
            ASSERT(0 == pair.x);
            ASSERT(0 == pair.slot);
        }
    }

    imap_free(tree);

    free(array);
}

static void imap_locate_random_test(void)
{
    imap_locate_random_dotest(time(0));
}

static void imap_dump_test(void)
{
    imap_node_t *tree;
    imap_slot_t *slot;
    char *dump;

    tree = 0;
    tree = imap_ensure(tree, +1);
    ASSERT(0 != tree);
    dump = 0;
    imap_dump(tree, test_concat_sprintf, &dump);
    ASSERT(0 == dump);
    free(dump);
    imap_free(tree);

    tree = 0;
    tree = imap_ensure(tree, +5);
    ASSERT(0 != tree);
    slot = imap_assign(tree, 0xA0000056);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x56);
    slot = imap_assign(tree, 0xA0000057);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x57);
    slot = imap_assign(tree, 0xA0008009);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x8009);
    slot = imap_assign(tree, 0xA0008059);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x8059);
    slot = imap_assign(tree, 0xA0008069);
    ASSERT(0 != slot);
    imap_setval(tree, slot, 0x8069);
    //
    dump = 0;
    imap_dump(tree, test_concat_sprintf, &dump);
    ASSERT(0 == strcmp(dump, ""
        "00000080: 00000000a0000000/3 0->*40 8->*100\n"
        "00000040: 00000000a0000050/0 6->56 7->57\n"
        "00000100: 00000000a0008000/1 0->*c0 5->*140 6->*180\n"
        "000000c0: 00000000a0008000/0 9->8009\n"
        "00000140: 00000000a0008050/0 9->8059\n"
        "00000180: 00000000a0008060/0 9->8069\n"
        ""));
    free(dump);
    //
    slot = imap_lookup(tree, 0xA0000056);
    ASSERT(0 != slot);
    ASSERT(0x56 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0000057);
    ASSERT(0 != slot);
    ASSERT(0x57 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0008009);
    ASSERT(0 != slot);
    ASSERT(0x8009 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0008059);
    ASSERT(0 != slot);
    ASSERT(0x8059 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0008069);
    ASSERT(0 != slot);
    ASSERT(0x8069 == imap_getval(tree, slot));
    imap_delval(tree, slot);
    slot = imap_lookup(tree, 0xA0000056);
    ASSERT(0 == slot);
    slot = imap_lookup(tree, 0xA0000057);
    ASSERT(0 == slot);
    slot = imap_lookup(tree, 0xA0008009);
    ASSERT(0 == slot);
    slot = imap_lookup(tree, 0xA0008059);
    ASSERT(0 == slot);
    slot = imap_lookup(tree, 0xA0008069);
    ASSERT(0 == slot);
    imap_free(tree);
}

void imap_tests(void)
{
    TEST(imap_primitives_test);
    TEST(imap_ensure_test);
    TEST(imap_assign_test);
    TEST(imap_assign_bigval_test);
    TEST(imap_assign_val0_test);
    TEST(imap_assign_val64_test);
    TEST(imap_assign_val128_test);
    TEST(imap_assign_shuffle_test);
    TEST(imap_remove_test);
    TEST(imap_remove_shuffle_test);
    TEST(imap_iterate_test);
    TEST(imap_iterate_shuffle_test);
    TEST(imap_locate_test);
    TEST(imap_locate_random_test);
    TEST(imap_dump_test);
}

static void iset_assign_test(void)
{
    iset_node_t *tree;

    tree = 0;
    tree = iset_ensure(tree, +1);
    ASSERT(0 != tree);
    ASSERT(!iset_lookup(tree, 0));
    iset_assign(tree, 0);
    ASSERT(iset_lookup(tree, 0));
    ASSERT(!iset_lookup(tree, 1));
    tree = iset_ensure(tree, +1);
    iset_assign(tree, 1);
    ASSERT(iset_lookup(tree, 1));
    ASSERT(!iset_lookup(tree, 1000));
    tree = iset_ensure(tree, +1);
    iset_assign(tree, 1000);
    ASSERT(iset_lookup(tree, 1000));
    iset_free(tree);
}

static void iset_remove_test(void)
{
    iset_node_t *tree;

    tree = 0;
    tree = iset_ensure(tree, +3);
    ASSERT(0 != tree);
    ASSERT(!iset_lookup(tree, 0));
    ASSERT(!iset_lookup(tree, 1));
    ASSERT(!iset_lookup(tree, 2));
    ASSERT(!iset_lookup(tree, 1000));
    ASSERT(!iset_lookup(tree, 2000));
    iset_assign(tree, 0);
    iset_assign(tree, 1);
    iset_assign(tree, 1000);
    ASSERT(iset_lookup(tree, 0));
    ASSERT(iset_lookup(tree, 1));
    ASSERT(!iset_lookup(tree, 2));
    ASSERT(iset_lookup(tree, 1000));
    ASSERT(!iset_lookup(tree, 2000));
    iset_remove(tree, 2);
    ASSERT(iset_lookup(tree, 0));
    ASSERT(iset_lookup(tree, 1));
    ASSERT(!iset_lookup(tree, 2));
    ASSERT(iset_lookup(tree, 1000));
    ASSERT(!iset_lookup(tree, 2000));
    iset_remove(tree, 2000);
    ASSERT(iset_lookup(tree, 0));
    ASSERT(iset_lookup(tree, 1));
    ASSERT(!iset_lookup(tree, 2));
    ASSERT(iset_lookup(tree, 1000));
    ASSERT(!iset_lookup(tree, 2000));
    iset_remove(tree, 0);
    ASSERT(!iset_lookup(tree, 0));
    ASSERT(iset_lookup(tree, 1));
    ASSERT(!iset_lookup(tree, 2));
    ASSERT(iset_lookup(tree, 1000));
    ASSERT(!iset_lookup(tree, 2000));
    iset_remove(tree, 1);
    ASSERT(!iset_lookup(tree, 0));
    ASSERT(!iset_lookup(tree, 1));
    ASSERT(!iset_lookup(tree, 2));
    ASSERT(iset_lookup(tree, 1000));
    ASSERT(!iset_lookup(tree, 2000));
    iset_remove(tree, 1000);
    ASSERT(!iset_lookup(tree, 0));
    ASSERT(!iset_lookup(tree, 1));
    ASSERT(!iset_lookup(tree, 2));
    ASSERT(!iset_lookup(tree, 1000));
    ASSERT(!iset_lookup(tree, 2000));
    iset_free(tree);
}

static void iset_locate_test(void)
{
    iset_node_t *tree;
    iset_iter_t iter;
    iset_pair_t pair;

    tree = 0;
    tree = iset_ensure(tree, +5);
    ASSERT(0 != tree);
    pair = iset_locate(tree, &iter, 0);
    ASSERT(0 == pair.x && !pair.elemof);
    pair = iset_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x && !pair.elemof);
    iset_free(tree);

    tree = 0;
    tree = iset_ensure(tree, +5);
    ASSERT(0 != tree);
    iset_assign(tree, 0);
    iset_assign(tree, 1);
    iset_assign(tree, 10);
    iset_assign(tree, 1000);
    iset_assign(tree, 1002);
    pair = iset_locate(tree, &iter, 0);
    ASSERT(0 == pair.x && pair.elemof);
    pair = iset_iterate(tree, &iter, 0);
    ASSERT(1 == pair.x && pair.elemof);
    pair = iset_locate(tree, &iter, 1);
    ASSERT(1 == pair.x && pair.elemof);
    pair = iset_iterate(tree, &iter, 0);
    ASSERT(10 == pair.x && pair.elemof);
    pair = iset_locate(tree, &iter, 2);
    ASSERT(10 == pair.x && pair.elemof);
    pair = iset_iterate(tree, &iter, 0);
    ASSERT(1000 == pair.x && pair.elemof);
    pair = iset_locate(tree, &iter, 999);
    ASSERT(1000 == pair.x && pair.elemof);
    pair = iset_iterate(tree, &iter, 0);
    ASSERT(1002 == pair.x && pair.elemof);
    pair = iset_locate(tree, &iter, 1000);
    ASSERT(1000 == pair.x && pair.elemof);
    pair = iset_iterate(tree, &iter, 0);
    ASSERT(1002 == pair.x && pair.elemof);
    pair = iset_locate(tree, &iter, 1001);
    ASSERT(1002 == pair.x && pair.elemof);
    pair = iset_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x && !pair.elemof);
    pair = iset_locate(tree, &iter, 1002);
    ASSERT(1002 == pair.x && pair.elemof);
    pair = iset_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x && !pair.elemof);
    pair = iset_locate(tree, &iter, 1003);
    ASSERT(0 == pair.x && !pair.elemof);
    iset_free(tree);
}

static void iset_iterate_test(void)
{
    iset_node_t *tree;
    iset_iter_t iter;
    iset_pair_t pair;

    tree = 0;
    tree = iset_ensure(tree, +5);
    ASSERT(0 != tree);
    pair = iset_iterate(tree, &iter, 1);
    ASSERT(0 == pair.x && !pair.elemof);
    pair = iset_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x && !pair.elemof);
    iset_free(tree);

    tree = 0;
    tree = iset_ensure(tree, +5);
    ASSERT(0 != tree);
    iset_assign(tree, 0);
    iset_assign(tree, 1);
    iset_assign(tree, 10);
    iset_assign(tree, 1000);
    iset_assign(tree, 1002);
    pair = iset_iterate(tree, &iter, 1);
    ASSERT(0 == pair.x && pair.elemof);
    pair = iset_iterate(tree, &iter, 0);
    ASSERT(1 == pair.x && pair.elemof);
    pair = iset_iterate(tree, &iter, 0);
    ASSERT(10 == pair.x && pair.elemof);
    pair = iset_iterate(tree, &iter, 0);
    ASSERT(1000 == pair.x && pair.elemof);
    pair = iset_iterate(tree, &iter, 0);
    ASSERT(1002 == pair.x && pair.elemof);
    pair = iset_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x && !pair.elemof);
    pair = iset_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x && !pair.elemof);
    iset_free(tree);
}

void iset_tests(void)
{
    TEST(iset_assign_test);
    TEST(iset_remove_test);
    TEST(iset_locate_test);
    TEST(iset_iterate_test);
}

static void ivmap_insert_test(void)
{
    ivmap_node_t *tree;
    ivmap_u64_t *y;

    tree = 0;
    tree = ivmap_ensure(tree, +1);
    ASSERT(0 != tree);
    y = ivmap_insert(tree, 1100, 1200);
    ASSERT(0 != y);
    *y = 101100;
    y = ivmap_lookup(tree, 1099);
    ASSERT(0 == y);
    y = ivmap_lookup(tree, 1100);
    ASSERT(0 != y && 101100 == *y);
    y = ivmap_lookup(tree, 1150);
    ASSERT(0 != y && 101100 == *y);
    y = ivmap_lookup(tree, 1199);
    ASSERT(0 != y && 101100 == *y);
    y = ivmap_lookup(tree, 1200);
    ASSERT(0 == y);
    y = ivmap_insert(tree, 1100, 1200);
    ASSERT(0 == y);
    y = ivmap_insert(tree, 1199, 1200);
    ASSERT(0 == y);
    y = ivmap_insert(tree, 1100, 1101);
    ASSERT(0 == y);
    y = ivmap_insert(tree, 1000, 1101);
    ASSERT(0 == y);
    y = ivmap_insert(tree, 1000, 1200);
    ASSERT(0 == y);
    y = ivmap_insert(tree, 1000, 1300);
    ASSERT(0 == y);
    y = ivmap_insert(tree, 1199, 1300);
    ASSERT(0 == y);
    tree = ivmap_ensure(tree, +1);
    ASSERT(0 != tree);
    y = ivmap_insert(tree, 1000, 1100);
    ASSERT(0 != y);
    *y = 101000;
    y = ivmap_lookup(tree, 999);
    ASSERT(0 == y);
    y = ivmap_lookup(tree, 1000);
    ASSERT(0 != y && 101000 == *y);
    y = ivmap_lookup(tree, 1050);
    ASSERT(0 != y && 101000 == *y);
    y = ivmap_lookup(tree, 1099);
    ASSERT(0 != y && 101000 == *y);
    y = ivmap_lookup(tree, 1100);
    ASSERT(0 != y && 101100 == *y);
    y = ivmap_lookup(tree, 1200);
    ASSERT(0 == y);
    tree = ivmap_ensure(tree, +1);
    ASSERT(0 != tree);
    y = ivmap_insert(tree, 1200, 1300);
    ASSERT(0 != y);
    *y = 101200;
    y = ivmap_lookup(tree, 999);
    ASSERT(0 == y);
    y = ivmap_lookup(tree, 1000);
    ASSERT(0 != y && 101000 == *y);
    y = ivmap_lookup(tree, 1100);
    ASSERT(0 != y && 101100 == *y);
    y = ivmap_lookup(tree, 1200);
    ASSERT(0 != y && 101200 == *y);
    y = ivmap_lookup(tree, 1299);
    ASSERT(0 != y && 101200 == *y);
    y = ivmap_lookup(tree, 1300);
    ASSERT(0 == y);
    y = ivmap_insert(tree, 1050, 1150);
    ASSERT(0 == y);
    y = ivmap_insert(tree, 1150, 1250);
    ASSERT(0 == y);
    ivmap_free(tree);

    tree = 0;
    tree = ivmap_ensure(tree, +1);
    ASSERT(0 != tree);
    y = ivmap_insert(tree, 1100, 1200);
    ASSERT(0 != y);
    *y = 101100;
    tree = ivmap_ensure(tree, +1);
    ASSERT(0 != tree);
    y = ivmap_insert(tree, 1300, 1400);
    ASSERT(0 != y);
    *y = 101300;
    y = ivmap_insert(tree, 1000, 1500);
    ASSERT(0 == y);
    y = ivmap_insert(tree, 1000, 1350);
    ASSERT(0 == y);
    y = ivmap_insert(tree, 1350, 1500);
    ASSERT(0 == y);
    y = ivmap_lookup(tree, 1099);
    ASSERT(0 == y);
    y = ivmap_lookup(tree, 1100);
    ASSERT(0 != y && 101100 == *y);
    y = ivmap_lookup(tree, 1199);
    ASSERT(0 != y && 101100 == *y);
    y = ivmap_lookup(tree, 1200);
    ASSERT(0 == y);
    y = ivmap_lookup(tree, 1299);
    ASSERT(0 == y);
    y = ivmap_lookup(tree, 1300);
    ASSERT(0 != y && 101300 == *y);
    y = ivmap_lookup(tree, 1399);
    ASSERT(0 != y && 101300 == *y);
    y = ivmap_lookup(tree, 1400);
    ASSERT(0 == y);
    ivmap_free(tree);
}

static void ivmap_remove_test(void)
{
    ivmap_node_t *tree;
    ivmap_u64_t *y;

    tree = 0;
    tree = ivmap_ensure(tree, +1);
    ASSERT(0 != tree);
    y = ivmap_insert(tree, 1100, 1200);
    ASSERT(0 != y);
    *y = 101100;
    y = ivmap_lookup(tree, 1100);
    ASSERT(0 != y && 101100 == *y);
    ivmap_remove(tree, 1100);
    y = ivmap_lookup(tree, 1100);
    ASSERT(0 == y);
    ivmap_free(tree);

    tree = 0;
    tree = ivmap_ensure(tree, +1);
    ASSERT(0 != tree);
    y = ivmap_insert(tree, 1100, 1200);
    ASSERT(0 != y);
    *y = 101100;
    y = ivmap_insert(tree, 1200, 1300);
    ASSERT(0 != y);
    *y = 101200;
    y = ivmap_lookup(tree, 1100);
    ASSERT(0 != y && 101100 == *y);
    y = ivmap_lookup(tree, 1200);
    ASSERT(0 != y && 101200 == *y);
    ivmap_remove(tree, 1100);
    y = ivmap_lookup(tree, 1100);
    ASSERT(0 == y);
    y = ivmap_lookup(tree, 1200);
    ASSERT(0 != y && 101200 == *y);
    ivmap_remove(tree, 1200);
    y = ivmap_lookup(tree, 1200);
    ASSERT(0 == y);
    ivmap_free(tree);

    tree = 0;
    tree = ivmap_ensure(tree, +1);
    ASSERT(0 != tree);
    y = ivmap_insert(tree, 1100, 1200);
    ASSERT(0 != y);
    *y = 101100;
    y = ivmap_insert(tree, 1300, 1400);
    ASSERT(0 != y);
    *y = 101300;
    y = ivmap_lookup(tree, 1100);
    ASSERT(0 != y && 101100 == *y);
    y = ivmap_lookup(tree, 1300);
    ASSERT(0 != y && 101300 == *y);
    ivmap_remove(tree, 1100);
    y = ivmap_lookup(tree, 1100);
    ASSERT(0 == y);
    y = ivmap_lookup(tree, 1300);
    ASSERT(0 != y && 101300 == *y);
    ivmap_remove(tree, 1300);
    y = ivmap_lookup(tree, 1300);
    ASSERT(0 == y);
    ivmap_free(tree);
}

static void ivmap_locate_test(void)
{
    ivmap_node_t *tree;
    ivmap_u64_t *y;
    ivmap_iter_t iter;
    ivmap_pair_t pair;

    tree = 0;
    tree = ivmap_ensure(tree, +1);
    ASSERT(0 != tree);
    pair = ivmap_locate(tree, &iter, 0);
    ASSERT(0 == pair.x0 && 0 == pair.x1 && 0 == pair.y);
    ivmap_free(tree);

    tree = 0;
    tree = ivmap_ensure(tree, +1);
    ASSERT(0 != tree);
    y = ivmap_insert(tree, 1100, 1200);
    ASSERT(0 != y);
    *y = 101100;
    pair = ivmap_locate(tree, &iter, 0);
    ASSERT(1100 == pair.x0 && 1200 == pair.x1 && 0 != pair.y && 101100 == *pair.y);
    pair = ivmap_locate(tree, &iter, 1099);
    ASSERT(1100 == pair.x0 && 1200 == pair.x1 && 0 != pair.y && 101100 == *pair.y);
    pair = ivmap_locate(tree, &iter, 1100);
    ASSERT(1100 == pair.x0 && 1200 == pair.x1 && 0 != pair.y && 101100 == *pair.y);
    pair = ivmap_locate(tree, &iter, 1199);
    ASSERT(1100 == pair.x0 && 1200 == pair.x1 && 0 != pair.y && 101100 == *pair.y);
    pair = ivmap_locate(tree, &iter, 1200);
    ASSERT(0 == pair.x0 && 0 == pair.x1 && 0 == pair.y);
    ivmap_free(tree);

    tree = 0;
    tree = ivmap_ensure(tree, +1);
    ASSERT(0 != tree);
    y = ivmap_insert(tree, 1100, 1200);
    ASSERT(0 != y);
    *y = 101100;
    tree = ivmap_ensure(tree, +1);
    ASSERT(0 != tree);
    y = ivmap_insert(tree, 1200, 1300);
    ASSERT(0 != y);
    *y = 101200;
    pair = ivmap_locate(tree, &iter, 1100);
    ASSERT(1100 == pair.x0 && 1200 == pair.x1 && 0 != pair.y && 101100 == *pair.y);
    pair = ivmap_locate(tree, &iter, 1200);
    ASSERT(1200 == pair.x0 && 1300 == pair.x1 && 0 != pair.y && 101200 == *pair.y);
    pair = ivmap_locate(tree, &iter, 1300);
    ASSERT(0 == pair.x0 && 0 == pair.x1 && 0 == pair.y);
    ivmap_free(tree);

    tree = 0;
    tree = ivmap_ensure(tree, +1);
    ASSERT(0 != tree);
    y = ivmap_insert(tree, 1100, 1200);
    ASSERT(0 != y);
    *y = 101100;
    tree = ivmap_ensure(tree, +1);
    ASSERT(0 != tree);
    y = ivmap_insert(tree, 1300, 1400);
    ASSERT(0 != y);
    *y = 101300;
    pair = ivmap_locate(tree, &iter, 1100);
    ASSERT(1100 == pair.x0 && 1200 == pair.x1 && 0 != pair.y && 101100 == *pair.y);
    pair = ivmap_locate(tree, &iter, 1200);
    ASSERT(1300 == pair.x0 && 1400 == pair.x1 && 0 != pair.y && 101300 == *pair.y);
    pair = ivmap_locate(tree, &iter, 1300);
    ASSERT(1300 == pair.x0 && 1400 == pair.x1 && 0 != pair.y && 101300 == *pair.y);
    pair = ivmap_locate(tree, &iter, 1400);
    ASSERT(0 == pair.x0 && 0 == pair.x1 && 0 == pair.y);
    ivmap_free(tree);
}

static void ivmap_iterate_test(void)
{
    ivmap_node_t *tree;
    ivmap_u64_t *y;
    ivmap_iter_t iter;
    ivmap_pair_t pair;

    tree = 0;
    tree = ivmap_ensure(tree, +1);
    ASSERT(0 != tree);
    pair = ivmap_iterate(tree, &iter, 1);
    ASSERT(0 == pair.x0 && 0 == pair.x1 && 0 == pair.y);
    ivmap_free(tree);

    tree = 0;
    tree = ivmap_ensure(tree, +1);
    ASSERT(0 != tree);
    y = ivmap_insert(tree, 1100, 1200);
    ASSERT(0 != y);
    *y = 101100;
    pair = ivmap_iterate(tree, &iter, 1);
    ASSERT(1100 == pair.x0 && 1200 == pair.x1 && 0 != pair.y && 101100 == *pair.y);
    pair = ivmap_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x0 && 0 == pair.x1 && 0 == pair.y);
    ivmap_free(tree);

    tree = 0;
    tree = ivmap_ensure(tree, +1);
    ASSERT(0 != tree);
    y = ivmap_insert(tree, 1100, 1200);
    ASSERT(0 != y);
    *y = 101100;
    tree = ivmap_ensure(tree, +1);
    ASSERT(0 != tree);
    y = ivmap_insert(tree, 1200, 1300);
    ASSERT(0 != y);
    *y = 101200;
    pair = ivmap_iterate(tree, &iter, 1);
    ASSERT(1100 == pair.x0 && 1200 == pair.x1 && 0 != pair.y && 101100 == *pair.y);
    pair = ivmap_iterate(tree, &iter, 0);
    ASSERT(1200 == pair.x0 && 1300 == pair.x1 && 0 != pair.y && 101200 == *pair.y);
    pair = ivmap_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x0 && 0 == pair.x1 && 0 == pair.y);
    ivmap_free(tree);

    tree = 0;
    tree = ivmap_ensure(tree, +1);
    ASSERT(0 != tree);
    y = ivmap_insert(tree, 1100, 1200);
    ASSERT(0 != y);
    *y = 101100;
    pair = ivmap_locate(tree, &iter, 0);
    ASSERT(1100 == pair.x0 && 1200 == pair.x1 && 0 != pair.y && 101100 == *pair.y);
    pair = ivmap_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x0 && 0 == pair.x1 && 0 == pair.y);
    pair = ivmap_locate(tree, &iter, 1099);
    ASSERT(1100 == pair.x0 && 1200 == pair.x1 && 0 != pair.y && 101100 == *pair.y);
    pair = ivmap_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x0 && 0 == pair.x1 && 0 == pair.y);
    pair = ivmap_locate(tree, &iter, 1100);
    ASSERT(1100 == pair.x0 && 1200 == pair.x1 && 0 != pair.y && 101100 == *pair.y);
    pair = ivmap_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x0 && 0 == pair.x1 && 0 == pair.y);
    pair = ivmap_locate(tree, &iter, 1199);
    ASSERT(1100 == pair.x0 && 1200 == pair.x1 && 0 != pair.y && 101100 == *pair.y);
    pair = ivmap_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x0 && 0 == pair.x1 && 0 == pair.y);
    pair = ivmap_locate(tree, &iter, 1200);
    ASSERT(0 == pair.x0 && 0 == pair.x1 && 0 == pair.y);
    pair = ivmap_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x0 && 0 == pair.x1 && 0 == pair.y);
    ivmap_free(tree);

    tree = 0;
    tree = ivmap_ensure(tree, +1);
    ASSERT(0 != tree);
    y = ivmap_insert(tree, 1100, 1200);
    ASSERT(0 != y);
    *y = 101100;
    tree = ivmap_ensure(tree, +1);
    ASSERT(0 != tree);
    y = ivmap_insert(tree, 1200, 1300);
    ASSERT(0 != y);
    *y = 101200;
    pair = ivmap_locate(tree, &iter, 1100);
    ASSERT(1100 == pair.x0 && 1200 == pair.x1 && 0 != pair.y && 101100 == *pair.y);
    pair = ivmap_iterate(tree, &iter, 0);
    ASSERT(1200 == pair.x0 && 1300 == pair.x1 && 0 != pair.y && 101200 == *pair.y);
    pair = ivmap_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x0 && 0 == pair.x1 && 0 == pair.y);
    pair = ivmap_locate(tree, &iter, 1200);
    ASSERT(1200 == pair.x0 && 1300 == pair.x1 && 0 != pair.y && 101200 == *pair.y);
    pair = ivmap_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x0 && 0 == pair.x1 && 0 == pair.y);
    pair = ivmap_locate(tree, &iter, 1300);
    ASSERT(0 == pair.x0 && 0 == pair.x1 && 0 == pair.y);
    pair = ivmap_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x0 && 0 == pair.x1 && 0 == pair.y);
    ivmap_free(tree);

    tree = 0;
    tree = ivmap_ensure(tree, +1);
    ASSERT(0 != tree);
    y = ivmap_insert(tree, 1100, 1200);
    ASSERT(0 != y);
    *y = 101100;
    tree = ivmap_ensure(tree, +1);
    ASSERT(0 != tree);
    y = ivmap_insert(tree, 1300, 1400);
    ASSERT(0 != y);
    *y = 101300;
    pair = ivmap_locate(tree, &iter, 1100);
    ASSERT(1100 == pair.x0 && 1200 == pair.x1 && 0 != pair.y && 101100 == *pair.y);
    pair = ivmap_iterate(tree, &iter, 0);
    ASSERT(1300 == pair.x0 && 1400 == pair.x1 && 0 != pair.y && 101300 == *pair.y);
    pair = ivmap_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x0 && 0 == pair.x1 && 0 == pair.y);
    pair = ivmap_locate(tree, &iter, 1200);
    ASSERT(1300 == pair.x0 && 1400 == pair.x1 && 0 != pair.y && 101300 == *pair.y);
    pair = ivmap_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x0 && 0 == pair.x1 && 0 == pair.y);
    pair = ivmap_locate(tree, &iter, 1300);
    ASSERT(1300 == pair.x0 && 1400 == pair.x1 && 0 != pair.y && 101300 == *pair.y);
    pair = ivmap_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x0 && 0 == pair.x1 && 0 == pair.y);
    pair = ivmap_locate(tree, &iter, 1400);
    ASSERT(0 == pair.x0 && 0 == pair.x1 && 0 == pair.y);
    pair = ivmap_iterate(tree, &iter, 0);
    ASSERT(0 == pair.x0 && 0 == pair.x1 && 0 == pair.y);

    ivmap_free(tree);
}

void ivmap_tests(void)
{
    TEST(ivmap_insert_test);
    TEST(ivmap_remove_test);
    TEST(ivmap_locate_test);
    TEST(ivmap_iterate_test);
}

int main(int argc, char **argv)
{
    TESTSUITE(imap_tests);
    TESTSUITE(iset_tests);
    TESTSUITE(ivmap_tests);

    tlib_run_tests(argc, argv);
    return 0;
}
