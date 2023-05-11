/*
 * tlib/testsuite.c
 *
 * Copyright 2023 Bill Zissimopoulos
 */
/*
 * This file is part of imap.
 *
 * It is licensed under the MIT license. The full license text can be found
 * in the License.txt file at the root of this project.
 */

#include <tlib/testsuite.h>
#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct test
{
    char name[64];
    void (*fn)(void);
    int optional;
    struct test *next;
};
static struct test test_suite_sentinel = { .next = &test_suite_sentinel };
static struct test *test_suite_tail = &test_suite_sentinel;
static struct test test_sentinel = { .next = &test_sentinel };
static struct test *test_tail = &test_sentinel;
static void add_test_to_list(const char *name, void (*fn)(void), int optional, struct test **tail)
{
    struct test *test = calloc(1, sizeof *test);
    strncpy(test->name, name, sizeof test->name - 1);
    test->name[sizeof test->name - 1] = '\0';
    test->fn = fn;
    test->optional = optional;
    test->next = (*tail)->next;
    (*tail)->next = test;
    (*tail) = test;
}
void tlib_add_test_suite(const char *name, void (*fn)(void))
{
    add_test_to_list(name, fn, 0, &test_suite_tail);
}
void tlib_add_test(const char *name, void (*fn)(void))
{
    add_test_to_list(name, fn, 0, &test_tail);
}
void tlib_add_test_opt(const char *name, void (*fn)(void))
{
    add_test_to_list(name, fn, 1, &test_tail);
}

struct hook
{
    void (*fn)(const char *name, void (*fn)(void), int v);
    struct hook *next;
};
static struct hook hook_sentinel = { .next = &hook_sentinel };
static struct hook *hook_tail = &hook_sentinel;
static void add_hook_to_list(void (*fn)(const char *name, void (*fn)(void), int v), struct hook **tail)
{
    struct hook *hook = calloc(1, sizeof *hook);
    hook->fn = fn;
    hook->next = (*tail)->next;
    (*tail)->next = hook;
    (*tail) = hook;
}
void tlib_add_hook(void (*fn)(const char *name, void (*fn)(void), int v))
{
    add_hook_to_list(fn, &hook_tail);
}

static FILE *tlib_out, *tlib_err;
static jmp_buf test_jmp_buf, *test_jmp;
static char assert_buf[256];
static void test_printf(const char *fmt, ...);
static inline unsigned long long get_time(void)
{
#if defined(_WIN32)
    #pragma comment(lib, "winmm.lib")
    unsigned long __stdcall timeGetTime(void);
    return timeGetTime();
#elif defined(__linux__)
    int clock_gettime(int, struct timespec *);
    struct timespec ts;
    clock_gettime(7/*CLOCK_BOOTTIME*/, &ts);
    return (unsigned long long)ts.tv_sec * 1000ULL + (unsigned long long)ts.tv_nsec / 1000000ULL;
#else
    return (unsigned long long)time(0) * 1000ULL;
#endif
}
static unsigned long long run_test(struct test *test)
{
    for (struct hook *hook = hook_tail->next->next; 0 != hook->fn; hook = hook->next)
        hook->fn(test->name, test->fn, +1);
    unsigned long long t0 = get_time();
    test->fn();
    unsigned long long t1 = get_time();
    for (struct hook *hook = hook_tail->next->next; 0 != hook->fn; hook = hook->next)
        hook->fn(test->name, test->fn, -1);
    return t1 - t0;
}
static void do_test_default(struct test *test, int testno)
{
    if (0 != test)
    {
        snprintf(assert_buf, sizeof assert_buf, "KO\n    ");
        char dispname[39 + 1];
        size_t displen = strlen(test->name);
        if (displen > sizeof dispname - 1)
            displen = sizeof dispname - 1;
        memcpy(dispname, test->name, displen);
        memset(dispname + displen, '.', sizeof dispname - 1 - displen);
        dispname[sizeof dispname - 1] = '\0';
        test_printf("%s ", dispname);
        unsigned long long d = run_test(test);
        test_printf("OK %u.%02us\n", (unsigned)(d / 1000), (unsigned)((d % 1000) / 10));
    }
    else
        test_printf("--- COMPLETE ---\n");
}
static void do_test_list(struct test *test, int testno)
{
    if (0 != test)
        test_printf("%s\n", test->name);
}
static void do_test_tap(struct test *test, int testno)
{
    if (0 != test)
    {
        snprintf(assert_buf, sizeof assert_buf, "not ok %d %s\n# ", testno + 1, test->name);
        run_test(test);
        test_printf("ok %d %s\n", testno + 1, test->name);
    }
    else
        test_printf("1..%d\n", testno);
}
static void test_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    FILE *f = tlib_out ? tlib_out : stdout;
    vfprintf(f, fmt, ap);
    fflush(f);
    va_end(ap);
}
void tlib_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    FILE *f = tlib_err ? tlib_err : stderr;
    vfprintf(f, fmt, ap);
    fflush(f);
    va_end(ap);
}
void tlib_run_tests(int argc, char *argv[])
{
    argc--; argv++;
    void (*do_test)(struct test *, int) = do_test_default;
    int match_any = 1, no_abort = 0;
    unsigned long repeat = 1;
    for (char **ap = argv, **aendp = ap + argc; aendp > ap; ap++)
    {
        const char *a = *ap;
        if ('-' == a[0])
        {
            if (0 == strcmp("--list", a))
                do_test = do_test_list;
            else if (0 == strcmp("--tap", a))
                do_test = do_test_tap;
            else if (0 == strcmp("--no-abort", a))
                no_abort = 1;
            else if (0 == strcmp("--repeat-forever", a))
                repeat = ULONG_MAX;
            else if ('-' == a[1])
            {
                fprintf(stderr, "tlib_run_tests: unknown option %s\n", a);
                exit(2);
            }
        }
        else
            match_any = 0;
    }
    for (struct test *test = test_suite_tail->next->next; 0 != test->fn; test = test->next)
        test->fn();
    while (repeat--)
    {
        int testno = 0;
        for (struct test *test = test_tail->next->next; 0 != test->fn; test = test->next)
        {
            int match_arg = match_any && !test->optional;
            for (char **ap = argv, **aendp = ap + argc; aendp > ap; aendp--)
            {
                const char *a = aendp[-1];
                int sign = a[0];
                if ('+' == sign)
                    a++;
                else if ('-' == sign)
                {
                    if ('-' == a[1])
                        continue;
                    a++;
                }
                size_t l = strlen(a);
                if (0 == (0 < l && '*' == a[l - 1] ?
                    strncmp(test->name, a, l - 1) : strcmp(test->name, a)))
                {
                    if ('+' == sign)
                        match_arg = 1;
                    else if ('-' == sign)
                        match_arg = 0;
                    else
                        match_arg = !test->optional;
                    break;
                }
            }
            if (!match_arg)
                continue;
            assert_buf[0] = '\0';
            if (no_abort)
            {
                test_jmp = &test_jmp_buf;
                if (0 == setjmp(*test_jmp))
                    do_test(test, testno);
                test_jmp = 0;
            }
            else
                do_test(test, testno);
            testno++;
        }
        do_test(0, testno);
    }
}
void tlib__assert(const char *func, const char *file, int line, const char *expr)
{
#if defined(_WIN32)
    const char *p = strrchr(file, '\\');
#else
    const char *p = strrchr(file, '/');
#endif
    file = 0 != p ? p + 1 : file;
    if (0 == func)
        test_printf("%sASSERT(%s) failed at: %s:%d\n", assert_buf, expr, file, line);
    else
        test_printf("%sASSERT(%s) failed at %s:%d:%s\n", assert_buf, expr, file, line, func);
    if (0 != test_jmp)
        longjmp(*test_jmp, 1);
}
