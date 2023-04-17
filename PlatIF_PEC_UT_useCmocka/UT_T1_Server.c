#include "_UT_Common.h"



int main(void)
{
    const struct CMUnitTest tests[] = {
        //cmocka_unit_test(),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}