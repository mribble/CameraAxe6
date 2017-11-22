#ifndef __CASTD_H__
#define __CASTD_H__


#if defined(__SAM3X8E__)
    #include <Arduino.h>
    #define CA_DEBUG_LOG
    //#define CA_DEBUG_ASSERT
    #define CA_DEBUG_INFO
    #define CA_DEBUG_ERROR
    #define SerialIO SerialUSB
    #define CHECK_SERIAL SerialIO.dtr()
#elif defined (ESP8266)
    #include <Arduino.h>
    #define CA_DEBUG_LOG
    #define CA_DEBUG_ASSERT
    #define CA_DEBUG_INFO
    #define CA_DEBUG_ERROR
    #define SerialIO Serial
    #define CHECK_SERIAL 1
#else
    #error Need a supported microchip
#endif

// Common macros for embedded microcontrollers
// Each of the following must be defined or or there is no
// output for their respective macro.
//   CA_DEBUG_ERROR, CA_DEBUG_INFO, CA_DEBUG_LOG, CA_DEBUG_ASSERT 

// SerialIO must be defined to be a hw serial device

// CA_LOG(fmt, ....)            -- Output important info (such as a test passed or failed)
// CA_ASSERT(cond, str)         -- Assert on errors
// CA_DEBUG_INFO(str, value)    -- Log developer debug messages
// CA_ERROR(str, value)         -- Log error messages

#ifdef CA_DEBUG_LOG

// Base function to print debug messages
// Requires the format string to use the PSTR() macro
// We need this for SAM3X because there is no Serial.printf function available
//  While there is a Serial.printf available for the ESP8266, it is best to maintain consistency with the use of PSTR()

// This should not be called directly (only call through macros below)
void CALog(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

inline void CALog(const char* fmt, ...) {
    if (CHECK_SERIAL) {
        char buf[128]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(buf, 128, fmt, args);
        va_end (args);
        SerialIO.print(buf);
       // On atmega we might want to use something like this if we are using F() to save ram
       //vsnprintf_P(buf, maxSize, fmt, args);
    }
}

// For the CA_LOG function, you *must* use the PSTR() macro for the format string
#define CA_LOG(fmt, ...)           CALog(fmt, ##__VA_ARGS__)
#else 
   #define CA_LOG(...)
#endif // CA_DEBUG_LOG

#ifdef CA_DEBUG_ASSERT
   #define CA_ASSERT(cond, str)                 \
   {                                            \
        if(!(cond)) {                           \
            if (CHECK_SERIAL) {                 \
                SerialIO.print(__FILE__);       \
                SerialIO.print(" line(");       \
                SerialIO.print(__LINE__);       \
                SerialIO.print(" -- cond=(");   \
                SerialIO.print(#cond);          \
                SerialIO.print(" -- ");         \
                SerialIO.println(str);          \
            }                                   \
        }                                       \
   }
#else
   #define CA_ASSERT(...)
#endif

#if defined(CA_DEBUG_INFO) || defined(CA_DEBUG_ERROR)
// This should not be called directly (only call through macros below)
#define _CA_MSG(header, msg, value) \
{                                   \
    if (CHECK_SERIAL)               \
    {                               \
        SerialIO.print(header);     \
        SerialIO.print(msg);        \
        SerialIO.print(F(": "));    \
        SerialIO.println(value);    \
    }                               \
} 
#endif
   
// Functions for simple messages using Serial.print. Use the F() macro for strings
#ifdef CA_DEBUG_INFO
   #define CA_INFO(msg, value)        _CA_MSG(F("INFO> "), msg, value)
#else
   #define CA_INFO(...)
#endif

#ifdef CA_DEBUG_ERROR
   #define CA_ERROR(msg, value)       _CA_MSG(F("ERROR> "), msg, value)
#else
   #define CA_ERROR(...)
#endif

#endif 