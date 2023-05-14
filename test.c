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
#include <time.h>

//#define IMAP_USE_SIMD
#define IMAP_ASSERT(expr)               ASSERT(expr)
#define IMAP_DUMPFN(...)                tlib_printf(__VA_ARGS__)
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

void imap_tests(void)
{
    TEST(imap_ensure_test);
    TEST(imap_assign_test);
    TEST(imap_assign_bigval_test);
    TEST(imap_assign_shuffle_test);
}

int main(int argc, char **argv)
{
    TESTSUITE(imap_tests);

    tlib_run_tests(argc, argv);
    return 0;
}
