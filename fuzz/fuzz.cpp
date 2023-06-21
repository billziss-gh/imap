/*
 * fuzz.cpp
 *
 * Copyright 2023 Bill Zissimopoulos
 */
/*
 * This file is part of imap.
 *
 * It is licensed under the MIT license. The full license text can be found
 * in the License.txt file at the root of this project.
 */

#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <imap.h>
#include <map>

int main(int argc, char **argv)
{
    const bool print = false;
    imap_node_t *tree;
    imap_slot_t *slot;
    imap_iter_t iter;
    imap_pair_t pair;
    std::map<imap_u64_t, imap_u64_t> stdm;
    unsigned long long x, y;
    char line[80];
    char cmd;

    tree = imap_ensure(0, +1);

    while (fgets(line, sizeof line, stdin))
    {
        if (2 == sscanf(line, "%llx = %llx", &x, &y))
        {
            /* assign */
            if (print) printf("%llX=%llX\n", x, y);
            tree = imap_ensure(tree, +1);
            slot = imap_assign(tree, (imap_u64_t)x);
            imap_setval(tree, slot, (imap_u64_t)y);
            stdm[x] = y;
        }
        else if (2 == sscanf(line, "%llx %c", &x, &cmd) && 'r' == cmd)
        {
            /* remove */
            if (print) printf("%llXr\n", x);
            imap_remove(tree, (imap_u64_t)x);
            stdm.erase(x);
        }
        else if (2 == sscanf(line, "%llx %c", &x, &cmd) && 'l' == cmd)
        {
            /* locate */
            if (print) printf("%llXl\n", x);
            pair = imap_locate(tree, &iter, x);
            if (pair.slot)
            {
                auto i = stdm.lower_bound(x);
                assert(i != stdm.end());
                assert(pair.x == i->first);
                assert(imap_getval(tree, pair.slot) == i->second);
                pair = imap_iterate(tree, &iter, 0);
                i++;
                if (pair.slot)
                {
                    assert(i != stdm.end());
                    assert(pair.x == i->first);
                    assert(imap_getval(tree, pair.slot) == i->second);
                    pair = imap_iterate(tree, &iter, x);
                }
                else
                    assert(i == stdm.end());
            }
            else
            {
                auto i = stdm.lower_bound(x);
                assert(i == stdm.end());
            }
        }
        else if (1 == sscanf(line, "%llx", &x))
        {
            if (print) printf("%llX\n", x);
            /* lookup */
            slot = imap_lookup(tree, x);
            if (slot)
            {
                assert(imap_getval(tree, slot) == stdm[x]);
            }
            else
            {
                assert(0 == stdm.count(x));
            }
        }
    }

    imap_free(tree);

    return 0;
}
