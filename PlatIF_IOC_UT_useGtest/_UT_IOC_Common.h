// This is a common header file for all UTs of PlatIF_IOC from API user's perspective,
//   which means the UTs focus on IOC's behavior in user view but its internal implementation.
// We design UT from following aspects/cagegory:
//   FreeDrafts, Typical, Demo, Boundary, State, Performance, Concurrency, Robust, Fault, Misuse, Others.
//      align to IMPROVE VALUE、AVOID LOST、BALANCE SKILL vs COST.

//[FreeDrafts]: 
//[Typical]:
//[Demo]:
//[Boundary]:
//[State]:
//[Performance]:
//[Concurrency]:
//[Robust]: Repeatly reach IOC's max capacity, check IOC's behavior still correct.
//[Fault]:
//[Misuse]:
//[Others]:

//---------------------------------------------------------------------------------------------------------------------
// Use GoogleTest as unit test framework
#ifdef CONFIG_BUILD_WITH_UTFWK_GTEST
    #define GTEST_HAS_PTHREAD 1
    #include <gtest/gtest.h>
#endif//CONFIG_BUILD_WITH_UTFWK_GTEST

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>

#include "../PlatIF_IOC/PlatIF_IOC.h"

#ifndef __UT_IOC_COMMON_H__
#define __UT_IOC_COMMON_H__

#endif // __UT_EVT_COMMON_H__
