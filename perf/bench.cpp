/*
 * bench.cpp
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
#include <map>
#include <unordered_map>
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

static const unsigned N = 10000000;
static imap_node_t *tree = imap_ensure(0, +1);
static std::unordered_map<imap_u64_t, imap_u64_t> stdu;
static std::map<imap_u64_t, imap_u64_t> stdm;
static volatile imap_u64_t value;

static imap_u32_t *init_random_array()
{
    test_srand(time(0));
    imap_u32_t *array = (imap_u32_t *)malloc(N * sizeof(imap_u32_t));
    for (unsigned i = 0; N > i; i++)
        array[i] = i;
    for (unsigned i = 0; N > i; i++)
    {
        imap_u32_t r = test_rand() % N;
        imap_u32_t t = array[i];
        array[i] = array[r];
        array[r] = t;
    }
    return array;
}
static imap_u32_t *test_array = init_random_array();

static void imap_seq_insert_test(void)
{
    for (unsigned i = 0; N > i; i++)
    {
        tree = imap_ensure(tree, +1);
        auto slot = imap_assign(tree, i);
        imap_setval(tree, slot, i);
    }
}

static void imap_seq_assign_test(void)
{
    for (unsigned i = 0; N > i; i++)
    {
        auto slot = imap_assign(tree, i);
        imap_setval(tree, slot, i);
    }
}

static void imap_seq_lookup_test(void)
{
    for (unsigned i = 0; N > i; i++)
    {
        auto slot = imap_lookup(tree, i);
        value = imap_getval(tree, slot);
    }
}

static void imap_seq_remove_test(void)
{
    for (unsigned i = 0; N > i; i++)
        imap_remove(tree, i);
}

static void imap_rnd_insert_test(void)
{
    for (unsigned i = 0; N > i; i++)
    {
        tree = imap_ensure(tree, +1);
        auto slot = imap_assign(tree, test_array[i]);
        imap_setval(tree, slot, test_array[i]);
    }
}

static void imap_rnd_assign_test(void)
{
    for (unsigned i = 0; N > i; i++)
    {
        auto slot = imap_assign(tree, test_array[i]);
        imap_setval(tree, slot, test_array[i]);
    }
}

static void imap_rnd_lookup_test(void)
{
    for (unsigned i = 0; N > i; i++)
    {
        auto slot = imap_lookup(tree, test_array[i]);
        value = imap_getval(tree, slot);
    }
}

static void imap_rnd_remove_test(void)
{
    for (unsigned i = 0; N > i; i++)
        imap_remove(tree, test_array[i]);
}

static void imap_shortseq_test(void)
{
    for (unsigned i = 0; N / 100 > i; i++)
    {
        for (unsigned j = 0; 100 > j; j++)
        {
            tree = imap_ensure(tree, +1);
            auto slot = imap_assign(tree, j);
            imap_setval(tree, slot, j);
        }
        for (unsigned j = 0; 100 > j; j++)
        {
            auto slot = imap_assign(tree, j);
            imap_setval(tree, slot, j);
        }
        for (unsigned j = 0; 100 > j; j++)
        {
            auto slot = imap_lookup(tree, j);
            value = imap_getval(tree, slot);
        }
        for (unsigned j = 0; 100 > j; j++)
            imap_remove(tree, j);
    }
}

static void stdu_seq_insert_test(void)
{
    for (unsigned i = 0; N > i; i++)
        stdu.emplace(i, i);
}

static void stdu_seq_assign_test(void)
{
    for (unsigned i = 0; N > i; i++)
        stdu.insert_or_assign(i, i);
}

static void stdu_seq_lookup_test(void)
{
    for (unsigned i = 0; N > i; i++)
        value = stdu.at(i);
}

static void stdu_seq_remove_test(void)
{
    for (unsigned i = 0; N > i; i++)
        stdu.erase(i);
}

static void stdu_rnd_insert_test(void)
{
    for (unsigned i = 0; N > i; i++)
        stdu.emplace(test_array[i], test_array[i]);
}

static void stdu_rnd_assign_test(void)
{
    for (unsigned i = 0; N > i; i++)
        stdu.insert_or_assign(test_array[i], test_array[i]);
}

static void stdu_rnd_lookup_test(void)
{
    for (unsigned i = 0; N > i; i++)
        value = stdu.at(test_array[i]);
}

static void stdu_rnd_remove_test(void)
{
    for (unsigned i = 0; N > i; i++)
        stdu.erase(test_array[i]);
}

static void stdu_shortseq_test(void)
{
    for (unsigned i = 0; N / 100 > i; i++)
    {
        for (unsigned j = 0; 100 > j; j++)
            stdu.emplace(j, j);
        for (unsigned j = 0; 100 > j; j++)
            stdu.insert_or_assign(j, j);
        for (unsigned j = 0; 100 > j; j++)
            value = stdu.at(j);
        for (unsigned j = 0; 100 > j; j++)
            stdu.erase(j);
    }
}

static void stdm_seq_insert_test(void)
{
    for (unsigned i = 0; N > i; i++)
        stdm.emplace(i, i);
}

static void stdm_seq_assign_test(void)
{
    for (unsigned i = 0; N > i; i++)
        stdm.insert_or_assign(i, i);
}

static void stdm_seq_lookup_test(void)
{
    for (unsigned i = 0; N > i; i++)
        value = stdm.at(i);
}

static void stdm_seq_remove_test(void)
{
    for (unsigned i = 0; N > i; i++)
        stdm.erase(i);
}

static void stdm_rnd_insert_test(void)
{
    for (unsigned i = 0; N > i; i++)
        stdm.emplace(test_array[i], test_array[i]);
}

static void stdm_rnd_assign_test(void)
{
    for (unsigned i = 0; N > i; i++)
        stdm.insert_or_assign(test_array[i], test_array[i]);
}

static void stdm_rnd_lookup_test(void)
{
    for (unsigned i = 0; N > i; i++)
        value = stdm.at(test_array[i]);
}

static void stdm_rnd_remove_test(void)
{
    for (unsigned i = 0; N > i; i++)
        stdm.erase(test_array[i]);
}

static void stdm_shortseq_test(void)
{
    for (unsigned i = 0; N / 100 > i; i++)
    {
        for (unsigned j = 0; 100 > j; j++)
            stdm.emplace(j, j);
        for (unsigned j = 0; 100 > j; j++)
            stdm.insert_or_assign(j, j);
        for (unsigned j = 0; 100 > j; j++)
            value = stdm.at(j);
        for (unsigned j = 0; 100 > j; j++)
            stdm.erase(j);
    }
}

void perf_tests(void)
{
    TEST(imap_seq_insert_test);
    TEST(stdu_seq_insert_test);
    TEST_OPT(stdm_seq_insert_test);
    TEST(imap_seq_assign_test);
    TEST(stdu_seq_assign_test);
    TEST_OPT(stdm_seq_assign_test);
    TEST(imap_seq_lookup_test);
    TEST(stdu_seq_lookup_test);
    TEST_OPT(stdm_seq_lookup_test);
    TEST(imap_seq_remove_test);
    TEST(stdu_seq_remove_test);
    TEST_OPT(stdm_seq_remove_test);
    TEST(imap_rnd_insert_test);
    TEST(stdu_rnd_insert_test);
    TEST_OPT(stdm_rnd_insert_test);
    TEST(imap_rnd_assign_test);
    TEST(stdu_rnd_assign_test);
    TEST_OPT(stdm_rnd_assign_test);
    TEST(imap_rnd_lookup_test);
    TEST(stdu_rnd_lookup_test);
    TEST_OPT(stdm_rnd_lookup_test);
    TEST(imap_rnd_remove_test);
    TEST(stdu_rnd_remove_test);
    TEST_OPT(stdm_rnd_remove_test);
    TEST(imap_shortseq_test);
    TEST(stdu_shortseq_test);
    TEST_OPT(stdm_shortseq_test);
}

int main(int argc, char **argv)
{
    TESTSUITE(perf_tests);

    tlib_run_tests(argc, argv);
    return 0;
}
