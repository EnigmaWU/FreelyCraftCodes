#include <stdio.h>

#ifndef __TOS_BASE_HELPERS_H__
#define __TOS_BASE_HELPERS_H__

#define TOS_calcArrayElmtCnt(array) (sizeof(array)/sizeof(array[0]))

/**
 * @brief New added code always add this macro to indicate that it's not tested yet.
 *
 * RefRst: TOS_RESULT_NOT_TESTED_BUG
 */
#define TOS_abortNotTested()                                                           \
  do {                                                                                 \
    printf("$^NOT-TESTED-CODE^$ FUNCNAME=%s @ %s:%d\n", __func__, __FILE__, __LINE__); \
    abort();                                                                           \
  } while (0)

#endif//__TOS_BASE_HELPERS_H__
