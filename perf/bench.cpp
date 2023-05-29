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
#include <memory>
#include <map>
#include <unordered_map>
#include "imap.h"

void test_srand(imap_u64_t s);
imap_u64_t test_rand(void);
void test_imap_insert(imap_node_t *&tree, imap_u64_t x, imap_u64_t y);
void test_imap_assign(imap_node_t *&tree, imap_u64_t x, imap_u64_t y);
void test_imap_remove(imap_node_t *&tree, imap_u64_t x);
imap_u64_t test_imap_lookup(imap_node_t *&tree, imap_u64_t x);
void test_stdu_insert(std::unordered_map<imap_u64_t, imap_u64_t> &stdu, imap_u64_t x, imap_u64_t y);
void test_stdu_assign(std::unordered_map<imap_u64_t, imap_u64_t> &stdu, imap_u64_t x, imap_u64_t y);
void test_stdu_remove(std::unordered_map<imap_u64_t, imap_u64_t> &stdu, imap_u64_t x);
imap_u64_t test_stdu_lookup(std::unordered_map<imap_u64_t, imap_u64_t> &stdu, imap_u64_t x);
void test_stdm_insert(std::map<imap_u64_t, imap_u64_t> &stdm, imap_u64_t x, imap_u64_t y);
void test_stdm_assign(std::map<imap_u64_t, imap_u64_t> &stdm, imap_u64_t x, imap_u64_t y);
void test_stdm_remove(std::map<imap_u64_t, imap_u64_t> &stdm, imap_u64_t x);
imap_u64_t test_stdm_lookup(std::map<imap_u64_t, imap_u64_t> &stdm, imap_u64_t x);

static const unsigned N = 10000000;
static imap_node_t *tree = imap_ensure(0, +1);
static std::unordered_map<imap_u64_t, imap_u64_t> stdu;
static std::map<imap_u64_t, imap_u64_t> stdm;

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
        test_imap_insert(tree, i, i);
}

static void imap_seq_assign_test(void)
{
    for (unsigned i = 0; N > i; i++)
        test_imap_assign(tree, i, i);
}

static void imap_seq_lookup_test(void)
{
    for (unsigned i = 0; N > i; i++)
        test_imap_lookup(tree, i);
}

static void imap_seq_remove_test(void)
{
    for (unsigned i = 0; N > i; i++)
        test_imap_remove(tree, i);
}

static void imap_rnd_insert_test(void)
{
    for (unsigned i = 0; N > i; i++)
        test_imap_insert(tree, test_array[i], test_array[i]);
}

static void imap_rnd_assign_test(void)
{
    for (unsigned i = 0; N > i; i++)
        test_imap_assign(tree, test_array[i], test_array[i]);
}

static void imap_rnd_lookup_test(void)
{
    for (unsigned i = 0; N > i; i++)
        test_imap_lookup(tree, test_array[i]);
}

static void imap_rnd_remove_test(void)
{
    for (unsigned i = 0; N > i; i++)
        test_imap_remove(tree, test_array[i]);
}

static void imap_shortseq_test(void)
{
    for (unsigned i = 0; N / 100 > i; i++)
    {
        for (unsigned j = 0; 100 > j; j++)
            test_imap_insert(tree, j, j);
        for (unsigned j = 0; 100 > j; j++)
            test_imap_assign(tree, j, j);
        for (unsigned j = 0; 100 > j; j++)
            test_imap_lookup(tree, j);
        for (unsigned j = 0; 100 > j; j++)
            test_imap_remove(tree, j);
    }
}

static void stdu_seq_insert_test(void)
{
    for (unsigned i = 0; N > i; i++)
        test_stdu_insert(stdu, i, i);
}

static void stdu_seq_assign_test(void)
{
    for (unsigned i = 0; N > i; i++)
        test_stdu_assign(stdu, i, i);
}

static void stdu_seq_lookup_test(void)
{
    for (unsigned i = 0; N > i; i++)
        test_stdu_lookup(stdu, i);
}

static void stdu_seq_remove_test(void)
{
    for (unsigned i = 0; N > i; i++)
        test_stdu_remove(stdu, i);
}

static void stdu_rnd_insert_test(void)
{
    for (unsigned i = 0; N > i; i++)
        test_stdu_insert(stdu, test_array[i], test_array[i]);
}

static void stdu_rnd_assign_test(void)
{
    for (unsigned i = 0; N > i; i++)
        test_stdu_assign(stdu, test_array[i], test_array[i]);
}

static void stdu_rnd_lookup_test(void)
{
    for (unsigned i = 0; N > i; i++)
        test_stdu_lookup(stdu, test_array[i]);
}

static void stdu_rnd_remove_test(void)
{
    for (unsigned i = 0; N > i; i++)
        test_stdu_remove(stdu, test_array[i]);
}

static void stdu_shortseq_test(void)
{
    for (unsigned i = 0; N / 100 > i; i++)
    {
        for (unsigned j = 0; 100 > j; j++)
            test_stdu_insert(stdu, j, j);
        for (unsigned j = 0; 100 > j; j++)
            test_stdu_assign(stdu, j, j);
        for (unsigned j = 0; 100 > j; j++)
            test_stdu_lookup(stdu, j);
        for (unsigned j = 0; 100 > j; j++)
            test_stdu_remove(stdu, j);
    }
}

static void stdm_seq_insert_test(void)
{
    for (unsigned i = 0; N > i; i++)
        test_stdm_insert(stdm, i, i);
}

static void stdm_seq_assign_test(void)
{
    for (unsigned i = 0; N > i; i++)
        test_stdm_assign(stdm, i, i);
}

static void stdm_seq_lookup_test(void)
{
    for (unsigned i = 0; N > i; i++)
        test_stdm_lookup(stdm, i);
}

static void stdm_seq_remove_test(void)
{
    for (unsigned i = 0; N > i; i++)
        test_stdm_remove(stdm, i);
}

static void stdm_rnd_insert_test(void)
{
    for (unsigned i = 0; N > i; i++)
        test_stdm_insert(stdm, test_array[i], test_array[i]);
}

static void stdm_rnd_assign_test(void)
{
    for (unsigned i = 0; N > i; i++)
        test_stdm_assign(stdm, test_array[i], test_array[i]);
}

static void stdm_rnd_lookup_test(void)
{
    for (unsigned i = 0; N > i; i++)
        test_stdm_lookup(stdm, test_array[i]);
}

static void stdm_rnd_remove_test(void)
{
    for (unsigned i = 0; N > i; i++)
        test_stdm_remove(stdm, test_array[i]);
}

static void stdm_shortseq_test(void)
{
    for (unsigned i = 0; N / 100 > i; i++)
    {
        for (unsigned j = 0; 100 > j; j++)
            test_stdm_insert(stdm, j, j);
        for (unsigned j = 0; 100 > j; j++)
            test_stdm_assign(stdm, j, j);
        for (unsigned j = 0; 100 > j; j++)
            test_stdm_lookup(stdm, j);
        for (unsigned j = 0; 100 > j; j++)
            test_stdm_remove(stdm, j);
    }
}

static size_t memtrack_total = 0;
template <typename T>
class memtrack_allocator
{
public:
    using value_type = T;
    memtrack_allocator()
    {
    }
    template <typename U>
    memtrack_allocator(const memtrack_allocator<U> &other)
    {
    }
    T *allocate(std::size_t size)
    {
        memtrack_total += sizeof(T) * size;
        return (T *)malloc(sizeof(T) * size);
    }
    void deallocate(T *p, std::size_t size)
    {
        memtrack_total -= sizeof(T) * size;
        free(p);
    }
};

static void imap_memtrack_test(void)
{
    imap_node_t *t = imap_ensure(0, +1);

    for (unsigned i = 0; N > i; i++)
        test_imap_insert(t, i, i);

    tlib_printf("%u/%u ", t->vec32[imap__tree_mark__], t->vec32[imap__tree_size__]);

    imap_free(t);
}

static void stdu_memtrack_test(void)
{
    IMAP_ASSERT(memtrack_total == 0);

    std::unordered_map<
        imap_u64_t,
        imap_u64_t,
        std::hash<uint64_t>,
        std::equal_to<uint64_t>,
        memtrack_allocator<std::pair<const imap_u64_t, imap_u64_t>>> u;

    for (unsigned i = 0; N > i; i++)
        u.emplace(i, i);

    tlib_printf("%llu ", (unsigned long long)memtrack_total);
}

static void stdm_memtrack_test(void)
{
    IMAP_ASSERT(memtrack_total == 0);

    std::map<
        imap_u64_t,
        imap_u64_t,
        std::less<uint64_t>,
        memtrack_allocator<std::pair<const imap_u64_t, imap_u64_t>>> m;

    for (unsigned i = 0; N > i; i++)
        m.emplace(i, i);

    tlib_printf("%llu ", (unsigned long long)memtrack_total);
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
    TEST(imap_memtrack_test);
    TEST(stdu_memtrack_test);
    TEST_OPT(stdm_memtrack_test);
}

int main(int argc, char **argv)
{
    TESTSUITE(perf_tests);

    tlib_run_tests(argc, argv);
    return 0;
}
