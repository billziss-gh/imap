/*
 * wrap.cpp
 *
 * Copyright 2023 Bill Zissimopoulos
 */
/*
 * This file is part of imap.
 *
 * It is licensed under the MIT license. The full license text can be found
 * in the License.txt file at the root of this project.
 */

#include <map>
#include <unordered_map>
#include "imap.h"

static imap_u64_t seed = 0;
void test_srand(imap_u64_t s)
{
    seed = s;
}
imap_u64_t test_rand(void)
{
    /* constants used by Knuth and MUSL */
    seed = seed * 6364136223846793005ULL + 1;
    return seed;
}

void test_imap_insert(imap_node_t *&tree, imap_u64_t x, imap_u64_t y)
{
    tree = imap_ensure(tree, +1);
    auto slot = imap_assign(tree, x);
    imap_setval(tree, slot, y);
}

void test_imap_assign(imap_node_t *&tree, imap_u64_t x, imap_u64_t y)
{
    auto slot = imap_assign(tree, x);
    imap_setval(tree, slot, y);
}

void test_imap_remove(imap_node_t *&tree, imap_u64_t x)
{
    imap_remove(tree, x);
}

imap_u64_t test_imap_lookup(imap_node_t *&tree, imap_u64_t x)
{
    auto slot = imap_lookup(tree, x);
    return imap_getval(tree, slot);
}

void test_stdu_insert(std::unordered_map<imap_u64_t, imap_u64_t> &stdu, imap_u64_t x, imap_u64_t y)
{
    stdu.emplace(x, y);
}

void test_stdu_assign(std::unordered_map<imap_u64_t, imap_u64_t> &stdu, imap_u64_t x, imap_u64_t y)
{
    stdu.insert_or_assign(x, y);
}

void test_stdu_remove(std::unordered_map<imap_u64_t, imap_u64_t> &stdu, imap_u64_t x)
{
    stdu.erase(x);
}

imap_u64_t test_stdu_lookup(std::unordered_map<imap_u64_t, imap_u64_t> &stdu, imap_u64_t x)
{
    return stdu.at(x);
}

void test_stdm_insert(std::map<imap_u64_t, imap_u64_t> &stdm, imap_u64_t x, imap_u64_t y)
{
    stdm.emplace(x, y);
}

void test_stdm_assign(std::map<imap_u64_t, imap_u64_t> &stdm, imap_u64_t x, imap_u64_t y)
{
    stdm.insert_or_assign(x, y);
}

void test_stdm_remove(std::map<imap_u64_t, imap_u64_t> &stdm, imap_u64_t x)
{
    stdm.erase(x);
}

imap_u64_t test_stdm_lookup(std::map<imap_u64_t, imap_u64_t> &stdm, imap_u64_t x)
{
    return stdm.at(x);
}
