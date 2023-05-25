# IMAP - Integer Map

![](doc/demo.svg)

This project presents a new data structure for storing ordered integer maps. An ordered integer map contains _x->y_ mappings, where _x_ and _y_ are integers and where the _lookup_, _assign_ (insert / replace), _remove_ and _iterate_ (in natural numeric order) operations are efficient.

The data structure is a cache-friendly, radix tree that attempts to minimize the number of memory accesses required to compute a result or alter the data structure.

## Definition

An **imap** is a data structure that represents an ordered 64-bit integer map. The data structure is a tree that consists of nodes, both internal and extenal. Internal nodes store the hierarchical structure of the tree and the _x_ values. External nodes store the _y_ values.

### Internal Nodes

We first observe that a 64-bit integer written in base-16 (hexadecimal) contains 16 digits. We also observe that each digit can take one of 16 values. Thus a 64-bit integer can be written as _h<sub>F</sub>...h<sub>1</sub>h<sub>0</sub>_ where _h<sub>i</sub>_ is the hexadecimal digit at position _i=0,1,...,F_.

An internal node consists of:

- A prefix, which is a 64-bit integer.
- A position, which is a 4-bit integer.
- 16 pointers that point to children nodes.

The prefix together with the position describe which subset from the set of 64-bit integers is contained under a particular internal node. For example, the prefix _00000000a0008000_ together with position _1_ is written _00000000a0008000 / 1_ and describes the set of all 64-bit integers _x_ such that _00000000a00080**00** <= x <= 00000000a00080**ff**_. In this example, position _1_ denotes the highlighted digit _00000000a00080**0**0_.

In the following graphs we will use the following symbol to denote an internal node:

![Internal Node](doc/node.svg)

The size of an internal node is exactly 64-bytes, which happens to be the most common cache-line size. To accomplish this an internal node is stored as an array of 16 32-bit integers ("slots"). The high 28 bits of each slot are used to store pointers to other nodes (internal and external); it can also be used to store the _y_ value directly without using external node storage if the _y_ value can "fit" in the slot. The low 4 bits of each slot are used to encode one of the hexadecimal digits of the prefix. Because the lowest hexadecimal digit of every possible prefix (_h<sub>0</sub>_) is always 0, we use the low 4 bits of slot 0 to store the node position.

![Internal Node Structure](doc/nodestru.svg)

### External Nodes

An external node consists of 8 64-bit values. Its purpose is to act as storage for _y_ values (that cannot fit in internal node slots). The size of an external node is exactly 64-bytes.

In the following graphs we do not use an explicit symbol to denote external nodes. Rather we use the following symbol to denote a single _y_ value:

![Value](doc/value.svg)

### Lookup Algorithm

### Assign Algorithm

### Remove Algorithm

### Iterate Algorithm

## Implementation

## License

MIT
