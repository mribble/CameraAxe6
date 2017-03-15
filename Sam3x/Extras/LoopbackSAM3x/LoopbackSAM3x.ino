/*
SAM3X side of the serial I/O loopback test
This verifies correct communiction from the ESPs hw UART to the Serial1 (19,18) ports on the Due/SAM3x
*/

#define BAUD_RATE      74880
#define SerialPort     Serial1

#define ESP_RST        20
#define ESP_GPIO0      21

void setup(void)
{
   Serial.begin(74880);
   //Serial.println("SAM3x listen and echo STARTED...");

   SerialPort.begin(BAUD_RATE);
   // set the control lines for the ESP assuming they are connected
   pinMode(ESP_RST, OUTPUT);
   pinMode(ESP_GPIO0, OUTPUT);
   digitalWrite(ESP_RST, HIGH);
   digitalWrite(ESP_GPIO0, HIGH);
}

void loop(void)
{
   while ( SerialPort.available() ) {
      SerialPort.write(SerialPort.read());
   }
}
