#ifndef _TEST_
#define _TEST_

static const char *_current_failing_test = NULL;
static int _num_tests = 0;
static int _num_asserts = 0;
static int _num_failed_tests = 0;
static int _num_failed_asserts = 0;

static void _start(const char* name)
{
    _current_failing_test = NULL;
    _num_tests++;
}

static void _assert(const char* name, int line, int ok)
{
    _num_asserts++;
    if (!ok){
        if (!_current_failing_test){
            _current_failing_test = name;
            _num_failed_tests++;
        }
        _num_failed_asserts++;
        printf("Assert failed for %s at line %d\n", name, line);
    }
}

static int _report(const char* filename)
{
    if (_num_failed_tests > 0){
        printf("%s: FAILED: %d/%d tests, %d/%d asserts\n", filename, _num_failed_tests, _num_tests, _num_failed_asserts, _num_asserts);
        return _num_failed_tests;
    }
    else{
        printf("%s: OK: %d/%d tests, %d/%d asserts\n", filename, _num_tests, _num_tests, _num_asserts, _num_asserts);
        return 0;
    }
}

#define START() _start(__func__)
#define ASSERT(ok) _assert(__func__, __LINE__, ok)
#define END() _end(__func__)
#define REPORT() _report(__FILE__)
#endif
