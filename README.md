# 3D-PAWS-Feather-LoRa-Remote

Last Updated: 2025-10-08

## Description

This software is supported on a Adafruit Feather LoRa board. It provides full weather station features. It uses LoRa (not LoRaWAN) to transmit observations to a receiving station for relay. Currently that is a Particle Full Station with Adafruit LoRa RFM95 module. Future plan is to also support for a RaspberryPI as a receiver and relay. \
 \
See [Particle Full Station Documentation](https://github.com/3d-paws/3D-PAWS-Particle-FullStation/blob/master/README.md) for features documented there and supported in this code.

### Normal Main Loop operation
Normally the unit is in low power sleep mode. If a rain tip occurs, we wake up increment the tip count in the interrupt handler. Wakeup from sleep occures on configation settings of (5,6,10,15,20,30 minute) hourly periods. Actually wakeup is 1 minute prior from one of these periods. If Wind, Distance or Air Quality is configured. THis allow time for samples to be taken for those observations where it is required. After observations have been made, they are transmitted in one or more LoRa messages. Then time to next observation is determined and we go into low power mode for this duration.

### CONFIG.TXT example
<div style="overflow:auto; white-space:pre; font-family: monospace; font-size: 8px; line-height: 1.5; height: 480px; border: 1px solid black; padding: 10px;">

```
#
# CONFIG.TXT
#
# Line Length is limited to 63 characters
#12345678901234567890123456789012345678901234567890123456789012

# Private Key - 128 bits (16 bytes of ASCII characters)
aes_pkey=10FE2D3C4B5A6978

# Initialization Vector must be and always will be 128 bits (16 bytes.)
# The real iv is actually myiv repeated twice
# 1234567 -> 0x12D687 = 0x00 0x12 0xD6 0x87 0x00 0x12 0xD6 0x87 
aes_myiv=1234567

# This unit's LoRa ID for Receiving and Sending Messages
# Also Chords Site instrument_id
lora_unitid=2

# LoRa Address who we are sending to aka LoRa 3D-PAWS
lora_gwid=1

# You can set transmitter power from 5 to 23 dBm, default is 13
lora_txpower=23

# Valid entries are 433, 866, 915
lora_freq=915

#################################################
# General Configurations Settings
#################################################
# No Wind Anemometer pin A2
# 0 = wind data
# 1 = no wind data
nowind=0

# Rain Gauge rg1 pin A3
# Options 0,1
# 0 = false
# 1 = true
rg1_enable=0

# OptionPin 1 - pin A4
# 0 = No sensor
# 1 = raw (op1r - average 5 samples spaced 10ms)
# 2 = 2nd rain gauge (rg2)
# 5 = 5m distance sensor (ds, dsr)
# 10 = 10m distance sensor (ds, dsr)
op1=0

# OptionPin 2 - pin A5
# 0 = No sensor (Pin in use if pm25aqi air quality detected)
# 1 = raw (op2r - average 5 samples spaced 10ms)
# 2 = read Voltaic battery voltage (vbv)
op2=0

# OptionPin 3 - pin A0
# 0 = No sensor
# 1 = raw (op3r - average 5 samples spaced 10ms)

# OptionPin 4 - pin A1
# 0 = No sensor
# 1 = raw (op4r - average 5 samples spaced 10ms)

# Distance sensor baseline. If positive, distance = baseline - ds_median
ds_baseline=0

# elevation used for MSLP
elevation=0

# Rain Total Rollover Offset from 0 UTC
# Set rtro to the UTC hour when your Rain Total Rollover action should occur.
#
# - Find your UTC offset (e.g., Denver MDT = -6, Kenya = +3)
# - Calculate: RTRO = Local_Hour - UTC_Offset
# - Use 0-23 range (if negative, add 24)
#
# Example: Set roll over to be at Midnight local time. 
# - Denver midnight MDT: 0 - (-6) = 6 
# - Kenya 6 AM:          6 - 3 = 3 
# - Sydney 8 AM AEDT:    8 - 11 = -3 + 24 = 21 
# - UTC midnight:        0 - 0 = 0
#
# rtro=H(:MM) - valid values are where H = (0-23) with optional ":" and MM = (00,15,30,45)
rtro=0

#################################################
# System Timing
#################################################

# Valid Observation Period in minutes (5,6,10,15,20,30)
# 15 minute observation period is the default
obs_period=15
```

</div>

#### Things to Note
- cf.h has an example configuration file in comments.
- LoRa transmission are AES128 bit encrypted. The settings must match those on the receiving site.
- When we transmit we are sending to a LoRa gateway id 1. The receiving site must be addressed with the same id.
- Our LoRa id is set to match the Chords site these observations are destined for. The webhook at on the Particle Console will use this id to direct the observation.
- Observations data are trans mitted in JSON format. In that a unique device is is transmitted with every transmission. Example "devid":"330eff6367815b7d93bfbcec". This can used if Chords is no longer the end logging site.
- At startup and every 24 hours after a INFO message is sent. These will be multiple LoRa messages. This is do to the LoRa message length constraint. INFO provides information on the station's configuration. File INFO.TXT on the SD card will be maintained with the most current information. Below is an example.

<div style="overflow:auto; white-space:pre; font-family: monospace; font-size: 8px; line-height: 1.5; height: 100px; border: 1px solid black; padding: 10px;">

```
{"at":"2025-10-07T01:30:15","id":72,"devid":"330eff6367815b7d93bfbcec","mtype":"IF","ver":"FLoRaRemote-250925","bv":4.27,"hth":66026689,"obsi":"5m","obsti":"5m","t2nt":"225s","lora":"72,23,915MHz,OK""devs":"mux,sd","sensors":"PM25AQ(D5),RG1(A3),DIST 5M(A5),SM0(A0),ST0(A1)"}
```

</div>

- The code is maintaining a special soil moinsture and temperatue solution using A0 for analog reading and A1 for a OneWire temperature sensor (SM0, ST0) tags.
- A I2C mux is supported for multiple Tinovi Soil Moisture sensors.
- Air Quality Sensor PM25AQI must be connected to mux port 7. And feather pin D5 must be connected to PM25AQI pin SET. This turns on and off the PM25AQI fan and laser. The mux is needed to isolate the PM25AQI's I2C channel. The PM25AQI hold the I2C buss when powered down. Interfering with other I2C devices.
- SDU Boot is supported after the first code install via USB port is performed. After which to install new code you can add file UPDATE.BIN on the SD card. UPDATE.BIN is removed as part up the update process.
### Pin Mappings

<div style="overflow:auto; white-space:pre; font-family: monospace; font-size: 8px; line-height: 1.5; height: 480px; border: 1px solid black; padding: 10px;">

```
 * Pin Definitions
 * 
 * Board Label   Arduino  Info & Usage                   Grove Shield Connector   
 * ======================================================================================================================
 * RST
 * 3V            3v3 Power
 * ARef
 * GND
 * A0            A0       Option Pin 3                   Grove A0
 * A1            A1       Option Pin 4                   Grove A0
 * A2            A2       Interrupt For Anemometer       Grove A2
 * A3            A3       Interrupt For Rain Gauge 1     Grove A2
 * A4            A4       Option Pin 1                   Grove A4
 * A5            A5       Option Pin 2                   Grove A4
 * SCK           SCK      SPI0 Clock - SD/LoRa           Not on Grove
 * MOS           MOSI     Used by SD Card/LoRa           Not on Grove
 * MIS           MISO     Used by SDCard/LoRa            Not on Grove
 * RX0           D0                                      Grove UART
 * TX1           D1                                      Grove UART 
 * io1           DIO1     to D6 if LoRa WAN              Not on Grove (Particle Pin D9) !!!! Removed for LoRaRemote
   
 * BAT           VBAT Power
 * En            Control - Connect to ground to disable the 3.3v regulator
 * USB           VBUS Power
 * 13            D13      LED                            Not on Grove 
 * 12            D12      Serial Console Enable          Not on Grove
 * 11            D11      Enable Sensors 2n2222/2N3904   Not on Grove
 * 10            D10      Used by SD Card as CS          Grove D4  (Particle Pin D5)
 * 9             D9/A7    Voltage Battery Pin            Grove D4  (Particle Pin D4)
 * 6             D6       to DIO1 if LoRa WAN            Grove D2  (Particle Pin D3)
 * 5             D5       GPS Wake Pin                   Grove D2  (Particle Pin D2)
 * SCL           D3       i2c Clock                      Grove I2C_1
 * SDA           D2       i2c Data                       Grove I2C_1
 * 
 * Not exposed on headers
 * D8 = LoRa NSS aka Chip Select CS
 * D4 = LoRa Reset
 * D3 = LoRa DIO
```

</div>

### LoRa Antenna Length
The recommended antenna wire lengths for Adafruit LoRa Feather boards are frequency-dependent quarter-wave whip antennas:
- For 433 MHz, the antenna length should be 6.5 inches (16.5 cm).
- For 868 MHz, the antenna length should be 3.25 inches (8.2 cm).
- For 915 MHz, the antenna length should be 3 inches (7.8 cm).