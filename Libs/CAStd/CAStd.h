#ifndef __CASTD_H__
#define __CASTD_H__


#if defined(__SAM3X8E__)
    #include <Arduino.h>
    #define CA_DEBUG_LOG
    #define CA_DEBUG_ASSERT
    #define CA_DEBUG_INFO
    #define CA_DEBUG_ERROR
    #define SerialIO SerialUSB
#elif defined (ESP8266)
    #define CA_DEBUG_LOG
    #define CA_DEBUG_ASSERT
    #define CA_DEBUG_INFO
    #define CA_DEBUG_ERROR
    #define SerialIO Serial
    //#define SerialIO Serial1               // UART on GPIO2 - use a pullup 
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
void CALog(PGM_P fmt, ...) __attribute__((format(printf, 1, 2)));

inline void CALog(PGM_P fmt, ...) {
   const uint8_t maxSize = 128;
   char          buf[maxSize];
   va_list       args;

   va_start (args, fmt);
#ifdef __SAM3X8E__
   char fmt2[maxSize];                  // SAM3X has no vsnprintf_P so copy into a char array first
   strncpy_P(fmt2, fmt, maxSize);
   vsnprintf(buf, maxSize, fmt2, args);
#elif ESP8266
   vsnprintf_P(buf, maxSize, fmt, args);
#endif
   va_end(args);
   SerialIO.print(buf);
}

// For the CA_LOG function, you *must* use the PSTR() macro for the format string
#define CA_LOG(fmt, ...)           CALog(fmt, ##__VA_ARGS__)
#else 
   #define CA_LOG(...)
#endif // CA_DEBUG_LOG

#ifdef CA_DEBUG_ASSERT
   #define CA_ASSERT(cond, str)            \
   {                                       \
      if(!(cond)) {                        \
         SerialIO.print(__FILE__);         \
         SerialIO.print(F(" line("));      \
         SerialIO.print(__LINE__);         \
         SerialIO.print(F(") -- cond=(")); \
         SerialIO.print(#cond);            \
         SerialIO.print(F(") -- "));       \
         SerialIO.println(str);            \
      }                                    \
   }
#else
   #define CA_ASSERT(...)
#endif

#if defined(CA_DEBUG_INFO) || defined(CA_DEBUG_ERROR)
// This should not be called directly (only call through macros below)
#define _CA_MSG(header, msg, value)    \
   {                                   \
      SerialIO.print(header);          \
      SerialIO.print(msg);             \
      SerialIO.print(F(": "));         \
      SerialIO.println(value);         \
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