ifeq ($(OS),Windows_NT)

bench: bench.exe
	.\bench.exe $(BENCH_CMDLINE)
bench.exe: ../imap.h bench.cpp wrap.cpp
	cl /I.. /DIMAP_USE_SIMD /D_CRT_SECURE_NO_WARNINGS /W3 /GS- /sdl- /O2 /Oi /MT /GL- bench.cpp wrap.cpp ../tlib/testsuite.c /Fe$@

else

bench: bench.out
	./bench.out $(BENCH_CMDLINE)
bench.out: ../imap.h bench.cpp wrap.cpp
	g++ -I.. -DIMAP_USE_SIMD -march=native -Wall -Wstrict-aliasing=1 -O3 -flto=none -std=c++17 -x c++ bench.cpp wrap.cpp -x c ../tlib/testsuite.c -o $@

endif
