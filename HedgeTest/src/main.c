#include "tmp_tests_depends.h"
#include "hedgelib/hl_memory.h"
#include "hedgelib/hl_blob.h"
#include "hedgelib/hl_list.h"
#include "hedgelib/io/hl_path.h"
#include "hedgelib/io/hl_file.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define TEST(name, code) static HlResult name(void) code
#include "tests.h"
#undef TEST

typedef HlResult(*testFunc)(void);
typedef struct test
{
    const char* name;
    testFunc func;
}
test;

static test tests[] =
{
    /*
       First test is a dummy test so we can avoid trailing
       comma issues and not have to subtract one from i
       each time within the test loop.
    */
    { "Dummy Test", 0 }

#define TEST(name, code) ,{ #name, name }
#include "tests.h"
};

#define testCount (sizeof(tests) / sizeof(test))
static const size_t lastTestIndex = (testCount - 1);

int main(int argc, char* argv[])
{
    HlResult results[testCount];
    size_t successfulTestCount = 0;
    size_t i;

    /* Run all tests and print their results. */
    for (i = 1; i < testCount; ++i)
    {
        HlResult result;
        printf("Running test %u/%u (\"%s\")... ",
            (unsigned int)i, (unsigned int)lastTestIndex,
            tests[i].name);

        /* Run test. */
        result = tests[i].func();

        /* Print result. */
        if (HL_OK(result))
        {
            printf("SUCCEEDED\n");
            ++successfulTestCount;
        }
        else
        {
            printf("FAILED\n");
        }

        /* Store result. */
        results[i] = result;
    }

    /* Print end results and return. */
    if (successfulTestCount == lastTestIndex)
    {
        printf("\nAll tests succeeded!\n");
    }
    else
    {
        size_t failedTestCount = (lastTestIndex - successfulTestCount);
        printf("\n** ERROR: %d test(s) failed:\n",
            (unsigned int)failedTestCount);

        for (i = 1; i < testCount; ++i)
        {
            if (HL_FAILED(results[i]))
            {
                printf("   - Test #%d (\"%s\")\n",
                    (unsigned int)i, tests[i].name);
            }
        }
    }

    return EXIT_SUCCESS;
}
