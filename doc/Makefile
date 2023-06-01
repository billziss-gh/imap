graph: bin svg ;

ifeq ($(OS),Windows_NT)

GRAPH=.\graph.exe
bin: graph.exe
graph.exe: ../imap.h graph.c
	cl /I.. /D_CRT_SECURE_NO_WARNINGS /W3 /O2 /std:c11 /permissive- graph.c /Fe$@

else

GRAPH=./graph.out
bin: graph.out
graph.out: ../imap.h graph.c
	gcc -I.. -Wall -Wstrict-aliasing=1 -O3 graph.c -o $@

endif

svg: demo.svg node.svg nodestru.svg value.svg mapping0.svg mapping1.svg mapping2.svg lookup.svg assign0.svg assign1.svg assign2.svg remove0.svg remove1.svg remove2.svg

%.svg: %.g
	$(GRAPH) < $< | dot -Tsvg -Granksep=1.0 -Gnodesep=0.5 -Nfontname=monospace -Elabelfontname=monospace -o $@
%.svg: %.dot
	dot -Tsvg -Granksep=1.0 -Gnodesep=0.5 -Nfontname=monospace -Elabelfontname=monospace $< -o $@
