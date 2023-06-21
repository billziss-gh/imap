fuzz: bin afl ;

ifeq ($(OS),Windows_NT)

else

FUZZ=./fuzz.out
bin: fuzz.out
fuzz.out: ../imap.h fuzz.cpp
	afl-clang++ -I.. -Wall -Wstrict-aliasing=1 -O3 fuzz.cpp -lstdc++ -o $@

endif

afl: bin
	afl-fuzz -i t -o o -x fuzz.dict $(FUZZ)
