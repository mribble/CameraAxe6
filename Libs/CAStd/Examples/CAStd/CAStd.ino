// examples for for CAStd macros

#ifdef ESP8266
#define SerialIO Serial
#elif __SAM3X8E__
#define SerialIO Serial
#endif

#define CA_DEBUG_ERROR
#define CA_DEBUG_INFO
#define CA_DEBUG_LOG
#define CA_DEBUG_ASSERT

#include <CAStd.h>

void setup (void) {
	Serial.begin(74880);
}

void loop(void) {
	CA_ERROR(F("ERROR"), F("Message"));
	CA_INFO(F("INFO"), F("Message"));
	CA_ASSERT(1 == 0, F("failed"));
	CA_LOG(PSTR("CA_LOG: message: %c %s %d\n"), '1', "string", 77);          // cannot use F() macro for char string
	CA_LOG(PSTR("CA_LOG: format string only\n\n"));
	delay(5000);
}