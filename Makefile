ifeq ($(OS),Windows_NT)

test: test.exe;
testcxx: testcxx.exe;
test.exe: imap.h test.cpp
	cl /I. /D_CRT_SECURE_NO_WARNINGS /W3 /O2 /Tc test.cpp tlib/testsuite.c /Fe$@
testcxx.exe: imap.h test.cpp
	cl /I. /D_CRT_SECURE_NO_WARNINGS /W3 /O2 /Tp test.cpp tlib/testsuite.c /Fe$@

else

test: imap.h test.cpp
	gcc -I. -Wall -O3 -fno-strict-aliasing -x c test.cpp -x c tlib/testsuite.c -o $@
testcxx: imap.h test.cpp
	g++ -I. -Wall -O3 -fno-strict-aliasing -x c++ test.cpp -x c tlib/testsuite.c -o $@

endif
