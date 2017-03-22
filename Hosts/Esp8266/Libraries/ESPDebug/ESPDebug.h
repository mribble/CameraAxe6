#ifndef __ESPDEBUG_H__
#define __ESPDEBUG_H__

/*
 Common debug macros for ESP8266 code
 CA_DEBUG symbol is managed externally (before including this header file, of course)
 */

#ifdef CA_DEBUG

//#define SERIAL_DEBUG             // enable separate Serial1 console for debug output as the hw serial (UART) is connected to the SAM3x - comment out to use Serial

#ifdef SERIAL_DEBUG

#define SerialIO      Serial1    // UART on GPIO2 - use a pullup (this is necessary for boot anyway)

#else // SERIAL_DEBUG

#define SerialIO      Serial

#endif // SERIAL_DEBUG

#define CA_MSG(header, msg, value)     \
   do {                                \
      SerialIO.print(header);          \
      SerialIO.print(__FILE__);        \
      SerialIO.print(F(" line("));     \
      SerialIO.print(__LINE__);        \
      SerialIO.print(F(") "));         \
      SerialIO.print(msg);             \
      SerialIO.print(F(": "));         \
      SerialIO.println(value);         \
   } while ( false )
#define CA_INFO(msg, value)        CA_MSG(F("INFO> "), msg, value)
#define CA_ERROR(msg, value)       CA_MSG(F("ERROR> "), msg, value)
#define CA_ASSERT(cond, str)              \
    do                                    \
    {                                     \
      if(!(cond))                         \
      {                                   \
        SerialIO.print(__FILE__);         \
        SerialIO.print(F(" line("));      \
        SerialIO.print(__LINE__);         \
        SerialIO.print(F(") -- cond=(")); \
        SerialIO.print(#cond);            \
        SerialIO.print(F(") -- "));       \
        SerialIO.println(str);            \
      }                                   \
    } while ( false )

#else // CA_DEBUG

#define SerialIO         Serial
#define CA_INFO(...) 
#define CA_ERROR(...) 
#define CA_ASSERT(...)

#endif // CA_DEBUG

#endif //ESPDEBUG_H
