/*
 * iset.h
 *
 * Copyright 2023 Bill Zissimopoulos
 */
/*
 * This file is part of imap.
 *
 * It is licensed under the MIT license. The full license text can be found
 * in the License.txt file at the root of this project.
 */

#ifndef ISET__GUARD__
#define ISET__GUARD__

#include <imap.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef imap_u32_t iset_u32_t;
typedef imap_u64_t iset_u64_t;
typedef imap_node_t iset_node_t;
typedef struct iset_iter iset_iter_t;
typedef struct iset_pair iset_pair_t;

struct iset_iter
{
    imap_iter_t iter;
    iset_u32_t y;
    iset_u64_t x;
};
struct iset_pair
{
    iset_u64_t x;
    iset_u32_t elemof;
};

#if defined(_MSC_VER)
static inline
iset_u32_t iset__bsf__(iset_u32_t x)
{
    return _BitScanForward((unsigned long *)&x, x), (unsigned long)x;
}
#elif defined(__GNUC__)
static inline
iset_u32_t iset__bsf__(iset_u32_t x)
{
    return __builtin_ffs(x) - 1;
}
#endif

static inline
iset_node_t *iset_ensure(iset_node_t *tree, iset_u32_t n)
{
    return imap_ensure0(tree, n);
}

static inline
void iset_free(iset_node_t *tree)
{
    imap_free(tree);
}

static inline
iset_u32_t iset_lookup(iset_node_t *tree, iset_u64_t x)
{
    imap_u64_t q = x / imap__slot_sbits__;
    imap_u32_t r = x % imap__slot_sbits__;
    imap_slot_t *slot = imap_lookup(tree, q);
    return slot ? (imap_getval0(tree, slot) & (1 << r)) : 0;
}

static inline
void iset_assign(iset_node_t *tree, iset_u64_t x)
{
    imap_u64_t q = x / imap__slot_sbits__;
    imap_u32_t r = x % imap__slot_sbits__;
    imap_slot_t *slot = imap_assign(tree, q);
    imap_setval0(tree, slot, imap_getval0(tree, slot) | (1 << r));
}

static inline
void iset_remove(iset_node_t *tree, iset_u64_t x)
{
    imap_u64_t q = x / imap__slot_sbits__;
    imap_u32_t r = x % imap__slot_sbits__;
    imap_slot_t *slot = imap_lookup(tree, q);
    if (slot)
    {
        imap_u32_t v0 = imap_getval0(tree, slot) & ~(1 << r);
        if (v0)
            imap_setval0(tree, slot, v0);
        else
            imap_remove(tree, q);
    }
}

static inline
iset_pair_t iset_locate(iset_node_t *tree, iset_iter_t *iter, iset_u64_t x)
{
    iset_pair_t result = { 0 };
    imap_u64_t q = x / imap__slot_sbits__;
    imap_u32_t r = x % imap__slot_sbits__;
    imap_pair_t pair;
    imap_u32_t dirn;
    iter->x = iter->y = 0;
    pair = imap_locate(tree, &iter->iter, q);
    if (pair.slot)
    {
        iter->y = imap_getval0(tree, pair.slot) & ~((1 << r) - 1);
        if (iter->y)
        {
            iter->x = q * imap__slot_sbits__;
            dirn = iset__bsf__(iter->y);
            iter->y &= ~(1 << dirn);
            result.x = iter->x + dirn;
            result.elemof = 1;
        }
    }
    return result;
}

static inline
iset_pair_t iset_iterate(iset_node_t *tree, iset_iter_t *iter, int restart)
{
    iset_pair_t result = { 0 };
    imap_pair_t pair;
    imap_u32_t dirn;
    if (restart)
        iter->x = iter->y = 0;
    else if (iter->y)
        goto fill;
    pair = imap_iterate(tree, &iter->iter, restart);
    if (pair.slot)
    {
        iter->y = imap_getval0(tree, pair.slot);
        iter->x = pair.x * imap__slot_sbits__;
    fill:
        dirn = iset__bsf__(iter->y);
        iter->y &= ~(1 << dirn);
        result.x = iter->x + dirn;
        result.elemof = 1;
    }
    return result;
}

#ifdef __cplusplus
}
#endif

#endif // ISET__GUARD__
