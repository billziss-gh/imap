ifeq ($(OS),Windows_NT)

test: test.exe
	.\test.exe
testcxx: testcxx.exe
	.\testcxx.exe
test.exe: imap.h test.cpp
	cl /I. /D_CRT_SECURE_NO_WARNINGS /W3 /O2 /Tc test.cpp tlib/testsuite.c /Fe$@
testcxx.exe: imap.h test.cpp
	cl /I. /D_CRT_SECURE_NO_WARNINGS /W3 /O2 /Tp test.cpp tlib/testsuite.c /Fe$@

else

test: test.out
	./test.out
testcxx: testcxx.out
	./testcxx.out
test.out: imap.h test.cpp
	gcc -I. -Wall -O3 -fno-strict-aliasing -x c test.cpp -x c tlib/testsuite.c -o $@
testcxx.out: imap.h test.cpp
	g++ -I. -Wall -O3 -fno-strict-aliasing -x c++ test.cpp -x c tlib/testsuite.c -o $@

endif
