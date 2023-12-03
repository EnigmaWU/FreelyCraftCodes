//## Select UTFWK of Cmocka or Criterion or GoogleTest

//Use Cmocka as unit test framework
#ifdef CONFIG_BUILD_WITH_UTFWK_CMOCKA
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#endif//CONFIG_BUILD_WITH_UTFWK_CMOCKA

//Use Criterion as unit test framework
#ifdef CONFIG_BUILD_WITH_UTFWK_CRITERION
#include <criterion/criterion.h>
#endif//CONFIG_BUILD_WITH_UTFWK_CRITERION

//Use GoogleTest as unit test framework
#ifdef CONFIG_BUILD_WITH_UTFWK_GTEST
#include <gtest/gtest.h>
#endif//CONFIG_BUILD_WITH_UTFWK_GTEST

#include <unistd.h>
#include <stdint.h>
#include <semaphore.h>

#include "../PlatIF_EVT/PlatIF_Event.h"

#ifndef __UT_EVT_COMMON_H__
#define __UT_EVT_COMMON_H__

#endif // __UT_EVT_COMMON_H__
