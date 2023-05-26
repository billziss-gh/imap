ifeq ($(OS),Windows_NT)

bench: bench.exe
	.\bench.exe $(BENCH_CMDLINE)
bench.exe: ../imap.h bench.cpp
	cl /I.. /D_CRT_SECURE_NO_WARNINGS /W3 /GS- /sdl- /O2 /Oi /MT bench.cpp ../tlib/testsuite.c /Fe$@

else

bench: bench.out
	./bench.out $(BENCH_CMDLINE)
bench.out: ../imap.h bench.cpp
	g++ -I.. -Wall -Wstrict-aliasing=1 -O3 -std=c++17 -x c++ bench.cpp -x c ../tlib/testsuite.c -o $@

endif
