// DEFINE BASIC PRIMITIVE TYPES FOR THINKOS EASY CODING
#include <stdint.h>

#ifndef __TOS_BASETYPES_H__
#define __TOS_BASETYPES_H__

// IF NORMAL VARIABLE, USE FOLLOWING TYPES IN PREFER
typedef long LONG_T;
typedef long* LONG_pT;

typedef unsigned long ULONG_T;
typedef unsigned long* ULONG_pT;

///////////////////////////////////////////////////////////////////////////////
// IF SIZE CARED STRUCT OR VARIABLE, USE FOLLOWING TYPES IN PREFER
typedef uint8_t U8_T;
typedef uint8_t* U8_pT;

typedef int8_t S8_T;
typedef int8_t* S8_pT;

typedef uint16_t U16_T;
typedef uint16_t* U16_pT;

typedef int16_t S16_T;
typedef int16_t* S16_pT;

typedef uint32_t U32_T;
typedef uint32_t* U32_pT;

typedef int32_t S32_T;
typedef int32_t* S32_pT;

typedef uint64_t U64_T;
typedef uint64_t* U64_pT;

typedef int64_t S64_T;
typedef int64_t* S64_pT;

#endif  //__TOS_BASETYPES_H__