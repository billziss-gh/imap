/*
 * graph.c
 *
 * Copyright 2023 Bill Zissimopoulos
 */
/*
 * This file is part of imap.
 *
 * It is licensed under the MIT license. The full license text can be found
 * in the License.txt file at the root of this project.
 */

#include <stdio.h>

#define IMAP_DUMP_NODE(...)             (imap_dump_node_gv(__VA_ARGS__))
#include "imap.h"

int main(int argc, char **argv)
{
    imap_node_t *tree;
    imap_slot_t *slot;
    unsigned long long x, y;
    char line[80];
    char cmd;

    tree = imap_ensure(0, +1);

    while (fgets(line, sizeof line, stdin))
    {
        if (2 == sscanf(line, "%llx = %llx", &x, &y))
        {
            /* assign */
            tree = imap_ensure(tree, +1);
            slot = imap_assign(tree, (imap_u64_t)x);
            imap_setval(tree, slot, (imap_u64_t)y);
        }
        else if (2 == sscanf(line, "%llx %c", &x, &cmd) && 'r' == cmd)
        {
            /* remove */
            imap_remove(tree, (imap_u64_t)x);
        }
        else if (1 == sscanf(line, "%c", &cmd) && 'd' == cmd)
        {
            /* dump */
            fprintf(stdout, "digraph {\n");
            imap_dump(tree, (imap_dumpfn_t *)fprintf, stdout);
            fprintf(stdout, "}\n");
        }
    }

    return 0;
}
