# 3D-PAWS-Feather-LoRa-Remote

Last Updated: 2025-10-08

## Description

This software is supported on a Adafruit Feather LoRa board. It provides full weather station features. It uses LoRa (not LoRaWAN) to transmit observations to a receiving station for relay. Currently that is a Particle Full Station with Adafruit LoRa RFM95 module. Future plan is to also support for a RaspberryPI as a receiver and relay.

### Normal Main Loop operation
Normally the unit is in low power sleep mode. If a rain tip occurs, we wake up increment the tip count in the interrupt handler. Wakeup from sleep occures on configation settings of (5,6,10,15,20,30 minute) hourly periods. Actually wakeup is 1 minute prior from one of these periods. If Wind, Distance or Air Quality is configured. This allows time for samples to be taken for those observations where it is required. After observations have been made, they are transmitted in one or more LoRa messages. Then time to next observation is determined and we go into low power mode for this duration.

#### Things to Note

- The same sensors the Particle Full Station supports are supported by the Feather LoRaRemote. See [Particle Full Station Documentation](https://github.com/3d-paws/3D-PAWS-Particle-FullStation/blob/master/README.md) for sensors information.
- LoRa transmission are AES128 bit encrypted. The settings must match those on the receiving site.
- When we transmit we are sending to a LoRa gateway id 1. The receiving site must be addressed with the same id.
- Our LoRa id is set to match the Chords site these observations are destined for. The webhook at on the Particle Console will use this id to direct the observation to the logging site.
- Observation data are transmitted in JSON format. In that a unique device is is transmitted with every transmission. Example "devid":"548fa41ef43ee791". This can used if Chords is no longer the end logging site.
- At startup and every 24 hours after a INFO message is sent. These will be multiple LoRa messages. This is do to the LoRa message length constraint. INFO provides information on the station's configuration. File INFO.TXT on the SD card will be maintained with the most current information. Below is an example.

<div style="overflow:auto; white-space:pre; font-family: monospace; font-size: 8px; line-height: 1.5; height: 100px; border: 1px solid black; padding: 10px;">

```
{"at":"2026-04-13T15:58:08","id":2,"devid":"548fa41ef43ee791","mtype":"IF","ver":"FLR-260412","bv":4.28,"hth":1,"obsi":"15m","obsti":"15m","t2nt":"52s","elev":0,"rtro":"0:00","lora":"72,23,915MHz,OK","devs":"eeprom,dsmux,sd,gps","gps":{"lat":40.096000,"lon":-105.088398,"alt":1560.100000,"sat":11,"hdop":1.000000,"on":0}"sensors":"BMX1(BMP390),BMX2(BMP280),MCP1,SHT45(44),SHT31(45),HDC302X(46),BMP581(47),AS5600,WS(A2),HI,WBT,WBGT WO/GLOBE"}
```

</div>

- A I2C mux is supported for multiple Tinovi Soil Moisture sensors.
- Air Quality Sensor PM25AQI must be connected to mux port 7. And feather pin D6 must be connected to PM25AQI pin SET. This turns on and off the PM25AQI fan and laser. The mux is needed to isolate the PM25AQI's I2C channel. The PM25AQI hold the I2C buss when powered down. Interfering with other I2C devices.
- Air Quality Sensor PM25AQI should be powered by the 5V VBUS pin. We have seen issues multiple i2c devices and the AQS.
- SDU Boot is supported after the first code install via USB port is performed. After which to install new code you can add file UPDATE.BIN on the SD card. UPDATE.BIN is removed as part up the update process.

### LoRa Antenna Length
The recommended antenna wire lengths for Adafruit LoRa Feather boards are frequency-dependent quarter-wave whip antennas:
- For 433 MHz, the antenna length should be 6.5 inches (16.5 cm).
- For 868 MHz, the antenna length should be 3.25 inches (8.2 cm).
- For 915 MHz, the antenna length should be 3 inches (7.8 cm).

## READMEs
### [Configuration File Example](docs/Configuration.md)
### [GPS Module](docs/GPSModule.md)
### [Pin Mappings](docs/PinMappings.md)