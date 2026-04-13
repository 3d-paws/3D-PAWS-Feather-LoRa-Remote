# GPS Module
[←Top](../README.md)<BR>
## Obtaining Time
A GPS module can be added but not necessary. If partnered with a Feather M0 LoRa board, The GPS will be used to set the RTC clock at power on Which means you do not need to enter a date and time on the the serial console to set the RTC.

Every 8 hous from power on, the RTC clock is updated from GPS.

Information (INFO) messages will contain geolocation information from the GPS.

## Use the below GPS module
- [Adafruit Mini GPS PA1010D - UART and I2C - STEMMA QT](https://www.adafruit.com/product/4415)

## Wiring
The GPS module connects to the i2c buss. A command will put the GPS to sleep. A pulse to the GPS's Reset  pin is required to wake the GPS up.  Connect the GPS Reset pin to the Feather D5 pin.