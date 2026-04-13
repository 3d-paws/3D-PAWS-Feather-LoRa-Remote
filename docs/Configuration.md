# Configuration File Example
[←Top](../README.md)

On the SD card, create a file named CONFIG.TXT.

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

### At initialization:
- I2C sensors are auto detected.