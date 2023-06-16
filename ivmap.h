/*
 * ivmap.h
 *
 * Copyright 2023 Bill Zissimopoulos
 */
/*
 * This file is part of imap.
 *
 * It is licensed under the MIT license. The full license text can be found
 * in the License.txt file at the root of this project.
 */

#ifndef IVMAP__GUARD__
#define IVMAP__GUARD__

#include <imap.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef imap_u32_t ivmap_u32_t;
typedef imap_u64_t ivmap_u64_t;
typedef imap_node_t ivmap_node_t;
typedef imap_iter_t ivmap_iter_t;
typedef struct ivmap_pair ivmap_pair_t;

struct ivmap_pair
{
    ivmap_u64_t x0, x1;
    ivmap_u64_t *y;
};

static inline
ivmap_node_t *ivmap_ensure(ivmap_node_t *tree, ivmap_u32_t n)
{
    return imap_ensure128(tree, n);
}

static inline
void ivmap_free(ivmap_node_t *tree)
{
    imap_free(tree);
}

static inline
ivmap_u64_t *ivmap_lookup(ivmap_node_t *tree, ivmap_u64_t x)
{
    imap_u128_t *p128;
    imap_pair_t pair = imap_succ(tree, x);
    return pair.slot && (p128 = imap_addrof128(tree, pair.slot), p128->v[0] <= x) ? &p128->v[1] : 0;
}

static inline
ivmap_u64_t *ivmap_insert(ivmap_node_t *tree, ivmap_u64_t x0, ivmap_u64_t x1)
{
    imap_u128_t *p128;
    imap_pair_t pair0 = imap_succ(tree, x0);
    imap_pair_t pair1 = imap_succ(tree, x1);
    if (pair0.slot != pair1.slot || (
        pair0.slot && (p128 = imap_addrof128(tree, pair0.slot), p128->v[0] < x1)))
        return 0;
    imap_slot_t *slot = imap_assign(tree, x1);
    imap_u128_t v128 = { { x0, 0 } };
    imap_setval128(tree, slot, v128);
    p128 = imap_addrof128(tree, slot);
    return &p128->v[1];
}

static inline
void ivmap_remove(ivmap_node_t *tree, ivmap_u64_t x)
{
    imap_u128_t *p128;
    imap_pair_t pair = imap_succ(tree, x);
    if (pair.slot && (p128 = imap_addrof128(tree, pair.slot), p128->v[0] <= x))
        imap_remove(tree, pair.x);
}

static inline
ivmap_pair_t ivmap_locate(ivmap_node_t *tree, ivmap_iter_t *iter, ivmap_u64_t x)
{
    ivmap_pair_t result = { 0 };
    imap_u128_t *p128;
    imap_pair_t pair = imap_locate(tree, iter, x + 1);
    if (pair.slot)
    {
        p128 = imap_addrof128(tree, pair.slot);
        result.x0 = p128->v[0];
        result.x1 = pair.x;
        result.y = &p128->v[1];
    }
    return result;
}

static inline
ivmap_pair_t ivmap_iterate(ivmap_node_t *tree, ivmap_iter_t *iter, int restart)
{
    ivmap_pair_t result = { 0 };
    imap_u128_t *p128;
    imap_pair_t pair = imap_iterate(tree, iter, restart);
    if (pair.slot)
    {
        p128 = imap_addrof128(tree, pair.slot);
        result.x0 = p128->v[0];
        result.x1 = pair.x;
        result.y = &p128->v[1];
    }
    return result;
}

#ifdef __cplusplus
}
#endif

#endif // IVMAP__GUARD__
