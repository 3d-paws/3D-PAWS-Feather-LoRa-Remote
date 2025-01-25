
## Interfacing from Arduino
**WARNING!!! max 5v voltage levels only, more voltage will damage device**
**SDA and SCL lines requires pull-up resitors to VCC, use 2.2K to 10K resistors. for better performance, smalled resistor should be used**

###wiring to Arduiono:

Arduiono pin #3V3 - sensor red (3.3v)

Arduiono pin #A4 - sensor green (SDA)

Arduiono pin #A5 - sensor white (SCL)

Arduiono pin #GND - sensor black (GND)

pin #GND - shield (GND)


### Get software

This sample software demonstrates how to read data from sensor.

Sensor default I2C address is 101.

Download Arduino library from [there.](https://github.com/tinovi/i2cMultiSoilArduino/tree/master/lib/i2cMultiArd)

Check out our shop [there.](https://tinovi.com/tinovi-shop/)
