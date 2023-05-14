ifeq ($(OS),Windows_NT)

test: test.exe
	.\test.exe
testcxx: testcxx.exe
	.\testcxx.exe
test.exe: imap.h test.c
	cl /I. /D_CRT_SECURE_NO_WARNINGS /W3 /O2 /Tc test.c tlib/testsuite.c /Fe$@
testcxx.exe: imap.h test.c
	cl /I. /D_CRT_SECURE_NO_WARNINGS /W3 /O2 /Tp test.c tlib/testsuite.c /Fe$@

else

test: test.out
	./test.out
testcxx: testcxx.out
	./testcxx.out
test.out: imap.h test.c
	gcc -I. -Wall -O3 -fno-strict-aliasing -x c test.c -x c tlib/testsuite.c -o $@
testcxx.out: imap.h test.c
	g++ -I. -Wall -O3 -fno-strict-aliasing -x c++ test.c -x c tlib/testsuite.c -o $@

endif
