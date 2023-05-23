ifeq ($(OS),Windows_NT)

test: test.exe
	.\test.exe
testcxx: testcxx.exe
	.\testcxx.exe
test.exe: imap.h test.c
	cl /I. /D_CRT_SECURE_NO_WARNINGS /W3 /O2 /std:c11 /permissive- /Tc test.c tlib/testsuite.c /Fe$@
testcxx.exe: imap.h test.c
	cl /I. /D_CRT_SECURE_NO_WARNINGS /W3 /O2 /std:c++14 /permissive- /Tp test.c tlib/testsuite.c /Fe$@

else

test: test.out
	./test.out
testcxx: testcxx.out
	./testcxx.out
test.out: imap.h test.c
	gcc -I. -Wall -Wstrict-aliasing=1 -O3 -x c test.c -x c tlib/testsuite.c -o $@
testcxx.out: imap.h test.c
	g++ -I. -Wall -Wstrict-aliasing=1 -O3 -x c++ test.c -x c tlib/testsuite.c -o $@

endif
