#ifndef __CASTD_H__
#define __CASTD_H__

/*
 Common macros for SAM3x and ESP8266 code
 
 CA_DEBUG and ESP_ALT_CONSOLE symbols must be defined (enable debug output) or not defined (no debug output)
 before including this header file
 */

#ifdef CA_DEBUG

#ifdef ESP8266
#ifdef ESP_ALT_CONSOLE
#define SerialIO      Serial1        // UART on GPIO2 - use a pullup (this is necessary for boot anyway)
#else
#define SerialIO      Serial
#endif // ESP_ALT_CONSOLE
#elif __SAM3X8E__
#define SerialIO      SerialUSB      // native USB port
#endif

#define CA_MSG(header, msg, value)     \
   do {                                \
      SerialIO.print(header);          \
      SerialIO.print(msg);             \
      SerialIO.print(F(": "));         \
      SerialIO.println(value);         \
   } while ( false )
#define CA_INFO(msg, value)        CA_MSG(F("INFO> "), msg, value)
#define CA_ERROR(msg, value)       CA_MSG(F("ERROR> "), msg, value)

#define CA_ASSERT(cond, str)               \
   do {                                    \
      if(!(cond)) {                        \
         SerialIO.print(__FILE__);         \
         SerialIO.print(F(" line("));      \
         SerialIO.print(__LINE__);         \
         SerialIO.print(F(") -- cond=(")); \
         SerialIO.print(#cond);            \
         SerialIO.print(F(") -- "));       \
         SerialIO.println(str);            \
      }                                    \
   } while ( false )

#else // CA_DEBUG

#define SerialIO         Serial      // for ESP8266, this will send (non-debug) connection messages to Serial
#define CA_INFO(...) 
#define CA_ERROR(...) 
#define CA_ASSERT(...)

#endif // CA_DEBUG

#endif //__CASTD_H__