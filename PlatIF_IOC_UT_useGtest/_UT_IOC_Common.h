//Use GoogleTest as unit test framework
#ifdef CONFIG_BUILD_WITH_UTFWK_GTEST
    #define GTEST_HAS_PTHREAD 1
    #include <gtest/gtest.h>
#endif//CONFIG_BUILD_WITH_UTFWK_GTEST

#include <unistd.h>
#include <stdint.h>
#include <semaphore.h>

#include "../PlatIF_IOC/PlatIF_IOC.h"

#ifndef __UT_IOC_COMMON_H__
#define __UT_IOC_COMMON_H__

#endif // __UT_EVT_COMMON_H__
