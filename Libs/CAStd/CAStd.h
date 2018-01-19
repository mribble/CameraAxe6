////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dreaming Robots - Copyright 2017, 2018
// 
// Does logging controls and other macro setup for the Camera Axe software
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __CASTD_H__
#define __CASTD_H__

// Don't define these to force off logging or asserts
#define CA_DEBUG_LOG
#define CA_DEBUG_ASSERT

// Change these to 0 to force off this type of logging
#define CA_INFO                 1
#define CA_ERROR                1
#define CA_SAM_IN_PACKETS       1
#define CA_ESP_IN_PACKETS       1
#define CA_ESP_SPIFFS           1
#define CA_JS_URI               1
// These modify the log output again change to 0 to force off this extra info
#define CA_TIMESTAMP            0
#define CA_PREFIX_CHIP          0

#if defined(__SAM3X8E__)
    #include <Arduino.h>
    #define SerialIO SerialUSB
    #define CHECK_SERIAL SerialIO.dtr()
#elif defined (ESP8266)
    #include <Arduino.h>
    #define SerialIO Serial
    #define CHECK_SERIAL 1
#else
    #error Need a supported microchip
#endif

// SerialIO must be defined to be a hw serial device
// CA_LOG(fmt, ....)            -- Output important info (such as a test passed or failed)
// CA_ASSERT(cond, str)         -- Assert on errors

#ifdef CA_DEBUG_LOG
    // This should not be called directly (only call through macros below)
    void CALog(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

    inline void CALog(const char* fmt, ...) {
        if (CHECK_SERIAL) {
            // Serial.printf would be easier, but not supported on sam3x
            char buf[128]; // resulting string limited to 128 chars
            buf[127] = 0;
            va_list args;
            va_start (args, fmt );
            vsnprintf(buf, 127, fmt, args);
            va_end (args);
           // On atmega we might want to use F() (or variants) to save ram, but that is a no-op on arm architectures
           //vsnprintf_P(buf, maxSize, fmt, args);

            if (CA_PREFIX_CHIP) {
#if defined(__SAM3X8E__)
                SerialIO.print("SAM-");
#elif defined (ESP8266)
                SerialIO.print("ESP-");
#endif
            }
            if (CA_TIMESTAMP) {
                SerialIO.print(millis());
                SerialIO.print("-");
            }
            SerialIO.print(buf);
        }
    }

    #define CA_LOG(flag, fmt, ...) ({if(flag) {CALog(fmt, ##__VA_ARGS__);}})
#else
   #define CA_LOG(...)
#endif // CA_DEBUG_LOG

#ifdef CA_DEBUG_ASSERT
   #define CA_ASSERT(cond, str)                 \
   ({                                           \
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
   })
#else
   #define CA_ASSERT(...)
#endif

#endif //__CASTD_H__