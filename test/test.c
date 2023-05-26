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
    imap__unpacklo4__(vec32, val64);
    val64 = imap__packlo4__(vec32);
    ASSERT(0xFEDCBA9876543210ull == val64);

    memset(vec32, 0, sizeof vec32);
    ASSERT(0 == imap__popcnthi28__(vec32, &val32));
    memset(vec32, 0, sizeof vec32);
    vec32[0] = 0xff;
    ASSERT(1 == imap__popcnthi28__(vec32, &val32) && 0xff == val32);
    memset(vec32, 0, sizeof vec32);
    vec32[1] = 0xef, vec32[3] = 0xd0;
    ASSERT(2 == imap__popcnthi28__(vec32, &val32));
    memset(vec32, 0, sizeof vec32);
    vec32[3] = 0xd0;
    ASSERT(1 == imap__popcnthi28__(vec32, &val32) && 0xd0 == val32);
}

static void imap_ensure_test(void)
{
    imap_node_t *tree, *tree2;

    tree = 0;
    tree = imap_ensure(tree, 0);
    ASSERT(0 == tree);
    imap_free(tree);

    tree = 0;
    tree = imap_ensure(tree, -1);
    ASSERT(0 != tree);
    imap_free(tree);

    tree = 0;
    tree = imap_ensure(tree, +1);
    ASSERT(0 != tree);
    imap_free(tree);

    tree = 0;
    tree = imap_ensure(tree, -1);
    ASSERT(0 != tree);
    tree2 = imap_ensure(tree, -1);
    ASSERT(tree == tree2);
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
    tree = imap_ensure(tree, -1);
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
    tree = imap_ensure(tree, -2);
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
    tree = imap_ensure(tree, -3);
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
    tree = imap_ensure(tree, -4);
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
    tree = imap_ensure(tree, -5);
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
    tree = imap_ensure(tree, -5);
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

    tree = imap_ensure(tree, -5);
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
    tree = imap_ensure(tree, -5);
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
    tree = imap_ensure(tree, -5);
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
    tree = imap_ensure(tree, -5);
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
    TEST(imap_assign_shuffle_test);
    TEST(imap_remove_test);
    TEST(imap_remove_shuffle_test);
    TEST(imap_iterate_test);
    TEST(imap_locate_test);
    TEST(imap_dump_test);
}

int main(int argc, char **argv)
{
    TESTSUITE(imap_tests);

    tlib_run_tests(argc, argv);
    return 0;
}
