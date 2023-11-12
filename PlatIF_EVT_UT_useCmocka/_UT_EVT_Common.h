//include nesscery system .H files before cmocka.h

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#ifdef __APPLE__    //[MacOS::/opt/homebrew/include/cmocka.h]
#include </opt/homebrew/include/cmocka.h>
#else
#include <cmocka.h>
#endif

#include "../PlatIF_EVT/PlatIF_Event.h"

#ifndef __UT_EVT_COMMON_H__
#define __UT_EVT_COMMON_H__

#endif // __UT_EVT_COMMON_H__
