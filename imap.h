/*
 * imap.h
 *
 * Copyright 2023 Bill Zissimopoulos
 */
/*
 * This file is part of imap.
 *
 * It is licensed under the MIT license. The full license text can be found
 * in the License.txt file at the root of this project.
 */

#ifndef IMAP__GUARD__
#define IMAP__GUARD__

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(IMAP_INTERFACE) && !defined(IMAP_IMPLEMENTATION)
#define IMAP_INTERFACE
#define IMAP_IMPLEMENTATION
#define IMAP_DECLFUNC                   static inline
#define IMAP_DEFNFUNC                   static inline
#elif !defined(IMAP_INTERFACE) && defined(IMAP_IMPLEMENTATION)
#define IMAP_INTERFACE
#endif

#if !defined(IMAP_DECLFUNC)
#define IMAP_DECLFUNC
#endif
#if !defined(IMAP_DEFNFUNC)
#define IMAP_DEFNFUNC
#endif

#if defined(IMAP_INTERFACE)

    #if defined(_MSC_VER)
    typedef unsigned __int8 imap_u8_t;
    typedef unsigned __int16 imap_u16_t;
    typedef unsigned __int32 imap_u32_t;
    typedef unsigned __int64 imap_u64_t;
    #elif defined(__GNUC__)
    typedef __UINT8_TYPE__ imap_u8_t;
    typedef __UINT16_TYPE__ imap_u16_t;
    typedef __UINT32_TYPE__ imap_u32_t;
    typedef __UINT64_TYPE__ imap_u64_t;
    #endif

    typedef struct imap_node imap_node_t;
    typedef imap_u32_t imap_slot_t;
    typedef struct imap_iter imap_iter_t;
    typedef struct imap_pair imap_pair_t;
    typedef int imap_dumpfn_t(void *ctx, const char *fmt, ...);

    struct imap_node
    {
        /* 64 bytes */
        union
        {
            imap_u32_t vec32[16];
            imap_u64_t vec64[8];
        };
    };
    struct imap_iter
    {
        imap_u32_t stack[16];
        imap_u32_t stackp;
    };
    struct imap_pair
    {
        imap_u64_t x;
        imap_slot_t *slot;
    };

    IMAP_DECLFUNC
    void imap_free(imap_node_t *tree);
    IMAP_DECLFUNC
    imap_node_t *imap_ensure(imap_node_t *tree, int n);
    IMAP_DECLFUNC
    imap_slot_t *imap_lookup(imap_node_t *tree, imap_u64_t x);
    IMAP_DECLFUNC
    imap_slot_t *imap_assign(imap_node_t *tree, imap_u64_t x);
    IMAP_DECLFUNC
    int imap_hasval(imap_node_t *tree, imap_slot_t *slot);
    IMAP_DECLFUNC
    imap_u64_t imap_getval(imap_node_t *tree, imap_slot_t *slot);
    IMAP_DECLFUNC
    void imap_setval(imap_node_t *tree, imap_slot_t *slot, imap_u64_t y);
    IMAP_DECLFUNC
    void imap_delval(imap_node_t *tree, imap_slot_t *slot);
    IMAP_DECLFUNC
    void imap_remove(imap_node_t *tree, imap_u64_t x);
    IMAP_DECLFUNC
    imap_pair_t imap_locate(imap_node_t *tree, imap_iter_t *iter, imap_u64_t x);
    IMAP_DECLFUNC
    imap_pair_t imap_iterate(imap_node_t *tree, imap_iter_t *iter, int restart);
    IMAP_DECLFUNC
    void imap_dump(imap_node_t *tree, imap_dumpfn_t *dumpfn, void *ctx);

#endif

#if defined(IMAP_IMPLEMENTATION)

    #if !defined(IMAP_ASSERT)
    #define IMAP_ASSERT(expr)           ((void)0)
    #endif

    #if !defined(IMAP_MALLOC) && !defined(IMAP_FREE)
    #include <stdlib.h>
    #define IMAP_MALLOC(s)              (malloc(s))
    #define IMAP_FREE(p)                (free(p))
    #endif

    #if !defined(IMAP_MEMCPY)
    #include <string.h>
    #define IMAP_MEMCPY(dst, src, siz)  (memcpy(dst, src, siz))
    #endif

    #if !defined(IMAP_ALIGNED_ALLOC) && !defined(IMAP_ALIGNED_FREE)

    static inline
    void *imap__aligned_alloc__(imap_u64_t alignment, imap_u64_t size)
    {
        void *p = IMAP_MALLOC(size + sizeof(void *) + alignment - 1);
        if (!p)
            return p;
        void **ap = (void **)(((imap_u64_t)p + sizeof(void *) + alignment - 1) & ~(alignment - 1));
        ap[-1] = p;
        return ap;
    }

    static inline
    void imap__aligned_free__(void *p)
    {
        if (0 != p)
            IMAP_FREE(((void **)p)[-1]);
    }

    #define IMAP_ALIGNED_ALLOC(a, s)    (imap__aligned_alloc__(a, s))
    #define IMAP_ALIGNED_FREE(p)        (imap__aligned_free__(p))

    #endif

    #if !defined(IMAP_DUMP_NODE)
    #define IMAP_DUMP_NODE(...)         (imap_dump_node(__VA_ARGS__))
    #endif

    #if defined(_MSC_VER)

    static inline
    imap_u32_t imap__bsr__(imap_u64_t x)
    {
        return _BitScanReverse64((unsigned long *)&x, x | 1), (unsigned long)x;
    }

    #elif defined(__GNUC__)

    static inline
    imap_u32_t imap__bsr__(imap_u64_t x)
    {
        return 63 - __builtin_clzll(x | 1);
    }

    #endif

    static inline
    imap_u64_t imap__ceilpow2__(imap_u64_t x)
    {
        return 1ull << (imap__bsr__(x - 1) + 1);
    }

    #if defined(IMAP_USE_SIMD)

    #include <immintrin.h>

    static inline
    imap_u64_t imap__packlo4_simd__(imap_u32_t vec32[16])
    {
        // aligned load node index into 512-bit vector
        __m512i input = _mm512_load_epi32(vec32);

        // set index to bits 0,1,2,3,32,33,34,35; corresponds to the lower nibbles of 2 u32_t's
        __m512i index = _mm512_set1_epi64(0x2322212003020100);

        // bitshuffle according to the index and return the result
        return _mm512_bitshuffle_epi64_mask(input, index);
    }

    static inline
    void imap__unpacklo4_simd__(imap_u32_t vec32[16], imap_u64_t value)
    {
        // aligned load node index into 512-bit vector
        __m512i input = _mm512_load_epi32(vec32);

        // prepare the lower and upper nibbles of the 64-bit value
        __m128i valu0 = _mm_set1_epi64x(value & 0x0f0f0f0f0f0f0f0full);
        __m128i valu1 = _mm_set1_epi64x((value >> 4) & 0x0f0f0f0f0f0f0f0full);

        // unpack 64-bit value into the lower nibbles of the 32-bit lanes of a 512-bit vector
        __m512i nibb0 = _mm512_cvtepu8_epi64(valu0);
        __m512i nibb1 = _mm512_slli_epi64(_mm512_cvtepu8_epi64(valu1), 32);

        // clear the lower nibbles of the input vector
        input = _mm512_and_epi32(_mm512_set1_epi32(0xfffffff0), input);

        // update the input vector with the result of OR'ing the input and the nibbles
        input = _mm512_ternarylogic_epi32(input, nibb0, nibb1, 0xfe/* OR ALL */);

        // aligned store the updated vector back into the node index
        _mm512_store_epi32(vec32, input);
    }

    #define imap__packlo4__             imap__packlo4_simd__
    #define imap__unpacklo4__           imap__unpacklo4_simd__

    #else

    static inline
    imap_u64_t imap__packlo4_port__(imap_u32_t vec32[16])
    {
        union
        {
            imap_u32_t *vec32;
            imap_u64_t *vec64;
        } u;
        u.vec32 = vec32;
        return
            ((u.vec64[0] & 0xf0000000full)) |
            ((u.vec64[1] & 0xf0000000full) << 4) |
            ((u.vec64[2] & 0xf0000000full) << 8) |
            ((u.vec64[3] & 0xf0000000full) << 12) |
            ((u.vec64[4] & 0xf0000000full) << 16) |
            ((u.vec64[5] & 0xf0000000full) << 20) |
            ((u.vec64[6] & 0xf0000000full) << 24) |
            ((u.vec64[7] & 0xf0000000full) << 28);
    }

    static inline
    void imap__unpacklo4_port__(imap_u32_t vec32[16], imap_u64_t value)
    {
        union
        {
            imap_u32_t *vec32;
            imap_u64_t *vec64;
        } u;
        u.vec32 = vec32;
        u.vec64[0] = (u.vec64[0] & ~0xf0000000full) | ((value) & 0xf0000000full);
        u.vec64[1] = (u.vec64[1] & ~0xf0000000full) | ((value >> 4) & 0xf0000000full);
        u.vec64[2] = (u.vec64[2] & ~0xf0000000full) | ((value >> 8) & 0xf0000000full);
        u.vec64[3] = (u.vec64[3] & ~0xf0000000full) | ((value >> 12) & 0xf0000000full);
        u.vec64[4] = (u.vec64[4] & ~0xf0000000full) | ((value >> 16) & 0xf0000000full);
        u.vec64[5] = (u.vec64[5] & ~0xf0000000full) | ((value >> 20) & 0xf0000000full);
        u.vec64[6] = (u.vec64[6] & ~0xf0000000full) | ((value >> 24) & 0xf0000000full);
        u.vec64[7] = (u.vec64[7] & ~0xf0000000full) | ((value >> 28) & 0xf0000000full);
    }

    #define imap__packlo4__             imap__packlo4_port__
    #define imap__unpacklo4__           imap__unpacklo4_port__

    #endif

    #define imap__tree_root__           0
    #define imap__tree_resv__           1
    #define imap__tree_mark__           2
    #define imap__tree_size__           3
    #define imap__tree_nfre__           4
    #define imap__tree_vfre__           5

    #ifdef __cplusplus
    #define imap__node_zero__           (imap_node_t{ 0 })
    #define imap__pair_zero__           (imap_pair_t{ 0 })
    #define imap__pair__(x, slot)       (imap_pair_t{ (x), (slot) })
    #else
    #define imap__node_zero__           ((imap_node_t){ 0 })
    #define imap__pair_zero__           ((imap_pair_t){ 0 })
    #define imap__pair__(x, slot)       ((imap_pair_t){ (x), (slot) })
    #endif

    static inline
    imap_u32_t imap__alloc_node__(imap_node_t *tree)
    {
        imap_u32_t mark = tree->vec32[imap__tree_nfre__];
        if (mark)
            tree->vec32[imap__tree_nfre__] = *(imap_u32_t *)((imap_u8_t *)tree + mark);
        else
        {
            mark = tree->vec32[imap__tree_mark__];
            IMAP_ASSERT(mark + sizeof(imap_node_t) <= tree->vec32[imap__tree_size__]);
            tree->vec32[imap__tree_mark__] = mark + sizeof(imap_node_t);
        }
        return mark;
    }

    static inline
    void imap__free_node__(imap_node_t *tree, imap_u32_t mark)
    {
        *(imap_u32_t *)((imap_u8_t *)tree + mark) = tree->vec32[imap__tree_nfre__];
        tree->vec32[imap__tree_nfre__] = mark;
    }

    static inline
    imap_node_t *imap__node__(imap_node_t *tree, imap_u32_t val)
    {
        return (imap_node_t *)((imap_u8_t *)tree + val);
    }

    static inline
    imap_u64_t imap__node_prefix__(imap_node_t *node)
    {
        return imap__packlo4__(node->vec32);
    }

    static inline
    void imap__node_setprefix__(imap_node_t *node, imap_u64_t prefix)
    {
        imap__unpacklo4__(node->vec32, prefix);
    }

    static inline
    imap_u32_t imap__node_pos__(imap_node_t *node)
    {
        return node->vec32[0] & 0xf;
    }

    static inline
    imap_u32_t imap__node_popcnt__(imap_node_t *node, imap_u32_t *p)
    {
        imap_u32_t pcnt = 0, sval, dirn;
        *p = 0;
        for (dirn = 0; 16 > dirn; dirn++)
        {
            sval = node->vec32[dirn];
            if (sval & ~0xf)
            {
                *p = sval;
                pcnt++;
            }
        }
        return pcnt;
    }

    static inline
    imap_u32_t imap__xpfx__(imap_u64_t x, imap_u32_t pos)
    {
        return x & (~0xfull << (pos << 2));
    }

    static inline
    imap_u32_t imap__xpos__(imap_u64_t x)
    {
        return imap__bsr__(x) >> 2;
    }

    static inline
    imap_u32_t imap__xdir__(imap_u64_t x, imap_u32_t pos)
    {
        return (x >> (pos << 2)) & 0xf;
    }

    static inline
    imap_u32_t imap__alloc_values__(imap_node_t *tree)
    {
        imap_u32_t mark = imap__alloc_node__(tree);
        imap_node_t *node = imap__node__(tree, mark);
        mark <<= 3;
        tree->vec32[imap__tree_vfre__] = mark;
        node->vec64[0] = mark + (1 << 6);
        node->vec64[1] = mark + (2 << 6);
        node->vec64[2] = mark + (3 << 6);
        node->vec64[3] = mark + (4 << 6);
        node->vec64[4] = mark + (5 << 6);
        node->vec64[5] = mark + (6 << 6);
        node->vec64[6] = mark + (7 << 6);
        node->vec64[7] = 0;
        return mark;
    }

    #define imap__prefix_pos__          0xf
    #define imap__slot_pmask__          0x0000000f
    #define imap__slot_node__           0x00000010
    #define imap__slot_scalar__         0x00000020
    #define imap__slot_value__          0xffffffe0

    IMAP_DEFNFUNC
    void imap_free(imap_node_t *tree)
    {
        IMAP_ALIGNED_FREE(tree);
    }

    IMAP_DEFNFUNC
    imap_node_t *imap_ensure(imap_node_t *tree, int n)
    {
        imap_node_t *newtree;
        imap_u32_t hasnfre, hasvfre, newmark, oldsize, newsize;
        imap_u64_t newsize64;
        if (0 == n)
            return tree;
        if (0 == tree)
        {
            hasnfre = 0;
            hasvfre = 1;
            newmark = sizeof(imap_node_t);
            oldsize = 0;
        }
        else
        {
            hasnfre = !!tree->vec32[imap__tree_nfre__];
            hasvfre = !!tree->vec32[imap__tree_vfre__];
            newmark = tree->vec32[imap__tree_mark__];
            oldsize = tree->vec32[imap__tree_size__];
        }
        newmark += 0 > n ?
            (-n * 2 - hasnfre) * sizeof(imap_node_t) :
            (+n * 2 - hasnfre) * sizeof(imap_node_t) + (n - hasvfre) * sizeof(imap_u64_t);
        if (newmark <= oldsize)
            return tree;
        newsize64 = imap__ceilpow2__(newmark);
        if (0x80000000 < newsize64)
            return 0;
        newsize = (imap_u32_t)newsize64;
        newtree = (imap_node_t *)IMAP_ALIGNED_ALLOC(sizeof(imap_node_t), newsize);
        if (!newtree)
            return newtree;
        if (0 == tree)
        {
            newtree->vec32[imap__tree_root__] = 0;
            newtree->vec32[imap__tree_resv__] = 0;
            newtree->vec32[imap__tree_mark__] = sizeof(imap_node_t);
            newtree->vec32[imap__tree_size__] = newsize;
            newtree->vec32[imap__tree_nfre__] = 0;
            newtree->vec32[imap__tree_vfre__] = 3 << 6;
            newtree->vec64[3] = 4 << 6;
            newtree->vec64[4] = 5 << 6;
            newtree->vec64[5] = 6 << 6;
            newtree->vec64[6] = 7 << 6;
            newtree->vec64[7] = 0;
        }
        else
        {
            IMAP_MEMCPY(newtree, tree, tree->vec32[imap__tree_mark__]);
            IMAP_ALIGNED_FREE(tree);
            newtree->vec32[imap__tree_size__] = newsize;
        }
        return newtree;
    }

    IMAP_DEFNFUNC
    imap_slot_t *imap_lookup(imap_node_t *tree, imap_u64_t x)
    {
        imap_node_t *node = tree;
        imap_slot_t *slot;
        imap_u32_t sval, posn = 16, dirn = 0;
        for (;;)
        {
            slot = &node->vec32[dirn];
            sval = *slot;
            if (!(sval & imap__slot_node__))
            {
                if (!(sval & imap__slot_value__) || 0 != posn ||
                    imap__node_prefix__(node) != (x & ~0xfull))
                    return 0;
                return slot;
            }
            node = imap__node__(tree, sval & imap__slot_value__);
            posn = imap__node_pos__(node);
            dirn = imap__xdir__(x, posn);
        }
    }

    IMAP_DEFNFUNC
    imap_slot_t *imap_assign(imap_node_t *tree, imap_u64_t x)
    {
        imap_node_t *newnode, *node = tree;
        imap_slot_t *slot;
        imap_u32_t newmark, sval, diff, posn = 16, dirn = 0;
        imap_u64_t prfx;
        for (;;)
        {
            slot = &node->vec32[dirn];
            sval = *slot;
            if (!(sval & imap__slot_node__))
            {
                if (0 == posn)
                {
                    IMAP_ASSERT(imap__node_prefix__(node) == (x & ~0xfull));
                    return slot;
                }
                newmark = imap__alloc_node__(tree);
                *slot = (*slot & imap__slot_pmask__) | imap__slot_node__ | newmark;
                break;
            }
            node = imap__node__(tree, sval & imap__slot_value__);
            prfx = imap__node_prefix__(node);
            posn = prfx & imap__prefix_pos__;
            diff = imap__xpos__(prfx ^ x);
            if (diff > posn)
            {
                newmark = imap__alloc_node__(tree);
                *slot = (*slot & imap__slot_pmask__) | imap__slot_node__ | newmark;
                newnode = imap__node__(tree, newmark);
                newmark = imap__alloc_node__(tree);
                *newnode = imap__node_zero__;
                newnode->vec32[imap__xdir__(prfx, diff)] = sval;
                newnode->vec32[imap__xdir__(x, diff)] = imap__slot_node__ | newmark;
                imap__node_setprefix__(newnode, imap__xpfx__(prfx, diff) | diff);
                break;
            }
            dirn = imap__xdir__(x, posn);
        }
        newnode = imap__node__(tree, newmark);
        *newnode = imap__node_zero__;
        imap__node_setprefix__(newnode, x & ~0xfull);
        return &newnode->vec32[x & 0xfull];
    }

    IMAP_DEFNFUNC
    int imap_hasval(imap_node_t *tree, imap_slot_t *slot)
    {
        IMAP_ASSERT(!(*slot & imap__slot_node__));
        imap_u32_t sval = *slot;
        return sval & imap__slot_value__;
    }

    IMAP_DEFNFUNC
    imap_u64_t imap_getval(imap_node_t *tree, imap_slot_t *slot)
    {
        IMAP_ASSERT(!(*slot & imap__slot_node__));
        imap_u32_t sval = *slot;
        if (sval & imap__slot_scalar__)
            return sval >> 6;
        else
            return (sval >> 6) ? tree->vec64[sval >> 6] : 0;
    }

    IMAP_DEFNFUNC
    void imap_setval(imap_node_t *tree, imap_slot_t *slot, imap_u64_t y)
    {
        IMAP_ASSERT(!(*slot & imap__slot_node__));
        imap_u32_t sval = *slot;
        if (y < (1 << 26))
        {
            if (!(sval & imap__slot_scalar__) && (sval >> 6))
            {
                tree->vec64[sval >> 6] = tree->vec32[imap__tree_vfre__];
                tree->vec32[imap__tree_vfre__] = sval & imap__slot_value__;
            }
            *slot = (*slot & imap__slot_pmask__) | imap__slot_scalar__ | (imap_u32_t)(y << 6);
        }
        else
        {
            if ((sval & imap__slot_scalar__) || !(sval >> 6))
            {
                sval = tree->vec32[imap__tree_vfre__];
                if (!sval)
                    sval = imap__alloc_values__(tree);
                IMAP_ASSERT(sval >> 6);
                tree->vec32[imap__tree_vfre__] = (imap_u32_t)tree->vec64[sval >> 6];
            }
            IMAP_ASSERT(!(sval & imap__slot_node__));
            IMAP_ASSERT(!(sval & imap__slot_scalar__) && (sval >> 6));
            *slot = (*slot & imap__slot_pmask__) | sval;
            tree->vec64[sval >> 6] = y;
        }
    }

    IMAP_DEFNFUNC
    void imap_delval(imap_node_t *tree, imap_slot_t *slot)
    {
        IMAP_ASSERT(!(*slot & imap__slot_node__));
        imap_u32_t sval = *slot;
        if (!(sval & imap__slot_scalar__) && (sval >> 6))
        {
            tree->vec64[sval >> 6] = tree->vec32[imap__tree_vfre__];
            tree->vec32[imap__tree_vfre__] = sval & imap__slot_value__;
        }
        *slot &= imap__slot_pmask__;
    }

    IMAP_DEFNFUNC
    void imap_remove(imap_node_t *tree, imap_u64_t x)
    {
        imap_iter_t iterdata, *iter = &iterdata;
        imap_node_t *node = tree;
        imap_slot_t *slot;
        imap_u32_t sval, pval, posn = 16, dirn = 0;
        iter->stackp = 0;
        for (;;)
        {
            slot = &node->vec32[dirn];
            sval = *slot;
            if (!(sval & imap__slot_node__))
            {
                if (!(sval & imap__slot_value__) || 0 != posn ||
                    imap__node_prefix__(node) != (x & ~0xfull))
                    ;
                else
                    imap_delval(tree, slot);
                break;
            }
            node = imap__node__(tree, sval & imap__slot_value__);
            posn = imap__node_pos__(node);
            dirn = imap__xdir__(x, posn);
            iter->stack[iter->stackp++] = (sval & imap__slot_value__) | dirn;
        }
        while (iter->stackp)
        {
            sval = iter->stack[--iter->stackp];
            node = imap__node__(tree, sval & imap__slot_value__);
            posn = !!imap__node_pos__(node);
            if (posn != imap__node_popcnt__(node, &pval))
                break;
            imap__free_node__(tree, sval & imap__slot_value__);
            if (!iter->stackp)
                slot = &tree->vec32[0];
            else
            {
                sval = iter->stack[iter->stackp - 1];
                dirn = sval & 31;
                node = imap__node__(tree, sval & imap__slot_value__);
                slot = &node->vec32[dirn];
            }
            *slot = (*slot & imap__slot_pmask__) | (pval & ~imap__slot_pmask__);
        }
    }

    IMAP_DEFNFUNC
    imap_pair_t imap_locate(imap_node_t *tree, imap_iter_t *iter, imap_u64_t x)
    {
        imap_node_t *node = tree;
        imap_slot_t *slot;
        imap_u32_t sval, posn = 16, dirn = 0;
        iter->stackp = 0;
        for (;;)
        {
            slot = &node->vec32[dirn];
            sval = *slot;
            if (!(sval & imap__slot_node__))
            {
                if (!(sval & imap__slot_value__) || 0 != posn ||
                    imap__node_prefix__(node) != (x & ~0xfull))
                    return imap_iterate(tree, iter, 0);
                return imap__pair__(imap__node_prefix__(node) | dirn, slot);
            }
            node = imap__node__(tree, sval & imap__slot_value__);
            posn = imap__node_pos__(node);
            dirn = imap__xdir__(x, posn);
            iter->stack[iter->stackp++] = ((sval & imap__slot_value__) | dirn) + 1;
        }
    }

    IMAP_DEFNFUNC
    imap_pair_t imap_iterate(imap_node_t *tree, imap_iter_t *iter, int restart)
    {
        imap_node_t *node;
        imap_slot_t *slot;
        imap_u32_t sval, dirn;
        if (restart)
        {
            iter->stackp = 0;
            sval = dirn = 0;
            goto enter;
        }
        // loop while stack is not empty
        while (iter->stackp)
        {
            // get slot value and increment direction
            sval = iter->stack[iter->stackp - 1]++;
            dirn = sval & 31;
            if (15 < dirn)
            {
                // if directions 0-15 have been examined, pop node from stack
                iter->stackp--;
                continue;
            }
        enter:
            node = imap__node__(tree, sval & imap__slot_value__);
            slot = &node->vec32[dirn];
            sval = *slot;
            if (sval & imap__slot_node__)
                // push node into stack
                iter->stack[iter->stackp++] = sval & imap__slot_value__;
            else if (sval & imap__slot_value__)
                return imap__pair__(imap__node_prefix__(node) | dirn, slot);
        }
        return imap__pair_zero__;
    }

    static inline
    int imap_dump_node(imap_node_t *tree, imap_u32_t mark, imap_dumpfn_t *dumpfn, void *ctx)
    {
        imap_node_t *node;
        imap_slot_t *slot;
        imap_u32_t sval, posn, dirn;
        imap_u64_t prfx;
        node = imap__node__(tree, mark);
        prfx = imap__node_prefix__(node);
        posn = prfx & imap__prefix_pos__;
        dumpfn(ctx, "%08x: %016llx/%x",
            mark, (unsigned long long)(prfx & ~imap__prefix_pos__), posn);
        for (dirn = 0; 16 > dirn; dirn++)
        {
            slot = &node->vec32[dirn];
            sval = *slot;
            if (sval & imap__slot_node__)
                dumpfn(ctx, " %x->*%x", dirn, sval & imap__slot_value__);
            else if (sval & imap__slot_value__)
                dumpfn(ctx, " %x->%llx", dirn, (unsigned long long)imap_getval(tree, slot));
        }
        dumpfn(ctx, "\n");
        return posn;
    }

    static inline
    int imap_dump_node_gv(imap_node_t *tree, imap_u32_t mark, imap_dumpfn_t *dumpfn, void *ctx)
    {
        imap_node_t *node;
        imap_slot_t *slot;
        imap_u32_t sval, posn, dirn;
        imap_u64_t prfx;
        node = imap__node__(tree, mark);
        prfx = imap__node_prefix__(node);
        posn = prfx & imap__prefix_pos__;
        dumpfn(ctx, "\"%x\" [shape=record label=\"{%016llx / %x|"
            "{<0>0|<1>1|<2>2|<3>3|<4>4|<5>5|<6>6|<7>7|<8>8|<9>9|<A>A|<B>B|<C>C|<D>D|<E>E|<F>F}"
            "}\"]\n",
            mark, (unsigned long long)(prfx & ~imap__prefix_pos__), posn);
        for (dirn = 0; 16 > dirn; dirn++)
        {
            slot = &node->vec32[dirn];
            sval = *slot;
            if (sval & imap__slot_node__)
                dumpfn(ctx, "\"%x\":\"%x\":s->\"%x\":n\n", mark, dirn, sval & imap__slot_value__);
            else if (sval & imap__slot_value__)
                dumpfn(ctx, "\"%x\":\"%x\":s->\"%llx\":n\n", mark, dirn, (unsigned long long)imap_getval(tree, slot));
        }
        return posn;
    }

    IMAP_DEFNFUNC
    void imap_dump(imap_node_t *tree, imap_dumpfn_t *dumpfn, void *ctx)
    {
        imap_iter_t iterdata, *iter = &iterdata;
        imap_node_t *node;
        imap_u32_t sval, dirn;
        iter->stackp = 0;
        sval = dirn = 0;
        goto enter;
        // loop while stack is not empty
        while (iter->stackp)
        {
            // get slot value and increment direction
            sval = iter->stack[iter->stackp - 1]++;
            dirn = sval & 31;
            if (15 < dirn)
            {
                // if directions 0-15 have been examined, pop node from stack
                iter->stackp--;
                continue;
            }
        enter:
            node = imap__node__(tree, sval & imap__slot_value__);
            sval = node->vec32[dirn];
            if (sval & imap__slot_node__ &&
                IMAP_DUMP_NODE(tree, sval & imap__slot_value__, dumpfn, ctx))
                // push node into stack, if node pos != 0
                iter->stack[iter->stackp++] = sval & imap__slot_value__;
        }
    }

#endif

#ifdef __cplusplus
}
#endif

#endif // IMAP__GUARD__
