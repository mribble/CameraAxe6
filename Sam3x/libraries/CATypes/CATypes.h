#ifndef __CATYPES_H__
#define __CATYPES_H__

#include <stdint.h>
#include <stdarg.h>
#include <arduino.h>

#define CA_DEBUG
#define CA_LOGGING

typedef int8_t  int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;
typedef float float32;
typedef double float64;

#define CA_FALSE 0
#define CA_TRUE  1

// Turn off debugging to improve performance and reduce code size
#ifdef CA_DEBUG
  #ifdef __SAM3X8E__                  
    #define CA_ASSERT(cond, str)        \
    do                                  \
    {                                   \
      if(!(cond))                       \
      {                                 \
        SerialUSB.print(__FILE__);      \
        SerialUSB.print(" line(");      \
        SerialUSB.print(__LINE__);      \
        SerialUSB.print(") -- cond=("); \
        SerialUSB.print(#cond);         \
        SerialUSB.print(") -- ");       \
        SerialUSB.println(str);         \
      }                                 \
    } while(0)
  #elif __RFduino__
    #define CA_ASSERT(cond, str)        \
    do                                  \
    {                                   \
      if(!(cond))                       \
      {                                 \
        Serial.print(__FILE__);         \
        Serial.print(" line(");         \
        Serial.print(__LINE__);         \
        Serial.print(") -- cond=(");    \
        Serial.print(#cond);            \
        Serial.print(") -- ");          \
        Serial.println(str);            \
      }                                 \
    } while(0)
  #endif // __RFduino__
#else
#define CA_ASSERT(val, str)
#endif

#endif //__CATYPES_H__
