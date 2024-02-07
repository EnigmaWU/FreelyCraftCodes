#ifndef __TOS_BASE_HELPERS_H__
#define __TOS_BASE_HELPERS_H__

#define TOS_calcArrayElmtCnt(array) (sizeof(array)/sizeof(array[0]))

#define TOS_abortNotTested()                                     \
  do {                                                           \
    printf("Not tested %s@%s:%d\n"__func__, __FILE__, __LINE__); \
    abort();                                                     \
  } while (0)

#endif//__TOS_BASE_HELPERS_H__
