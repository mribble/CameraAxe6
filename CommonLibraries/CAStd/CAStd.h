#ifndef __CASTD_H__
#define __CASTD_H__

/*
 Common macros for SAM3x and ESP8266 code
 
 The symbols: CA_DEBUG_ERROR, CA_DEBUG_INFO, CA_DEBUG_LOG, CA_DEBUG_ASSERT 
 must be defined (enable output) or not defined (no output)
 before including this header file
 
 Likewise, for the ESP8266, SerialIO must also be defined first (Serial or Serial1)
 */

#if defined(__SAM3X8E__) && not defined(SerialIO)
#define SerialIO  SerialUSB
#endif


#ifdef CA_DEBUG_LOG
/* 
  Base function to print debug messages
  Requires the format string to use the PSTR() macro
   
  We need this for SAM3X becuase there is no Serial.printf function available
  While there is a Serial.print available for the ESP8266, it is best to maintain consistency with the use of PSTR()
  
  This function should not be called directly - only through the macro below
 */
void CALog(PGM_P fmt, ...) __attribute__((format(printf, 1, 2)));

void CALog(PGM_P fmt, ...) {
   const uint8_t maxSize = 128;                        // Max resulting string size
   char          buf[maxSize]; 
   va_list       args;
	 
	va_start (args, fmt);
#ifdef __SAM3X8E__
	char format[maxSize];                               // SAM3X has no vsnprintf_P so copy into a char array first
	strncpy_P(format, fmt, maxSize);
   vsnprintf(buf, maxSize, format, args);
#elif ESP8266
   vsnprintf_P(buf, maxSize, fmt, args);
#endif
   va_end(args);
   SerialIO.print(buf);
}

// for the CA_LOG function, you *must* use the PSTR() macro for the format string
#define CA_LOG(fmt, ...)           CALog(fmt, ##__VA_ARGS__)

#else 
	#define CA_LOG(...)
#endif // LOG

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
	
#define _CA_MSG(header, msg, value)    \
   {                                   \
      SerialIO.print(header);          \
      SerialIO.print(msg);             \
      SerialIO.print(F(": "));         \
      SerialIO.println(value);         \
   } 
	
// functions for simple messages using Serial.print. Use the F() macro for strings
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