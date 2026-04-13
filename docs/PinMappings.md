# Pin Mappings
[←Top](../README.md)<BR>

## Pin Definitions Table

| Board Label | Arduino | Info & Usage                      | Grove Shield Connector         |
|-------------|---------|-----------------------------------|--------------------------------|
| RST         |         |                                   |                                |
| 3V          | 3v3     | Power                             |                                |
| ARef        |         |                                   |                                |
| GND         |         |                                   |                                |
| A0          | A0      | Option Pin 3                      | Grove A0                       |
| A1          | A1      | Option Pin 4                      | Grove A0                       |
| A2          | A2      | Interrupt for Anemometer          | Grove A2                       |
| A3          | A3      | Interrupt for Rain Gauge 1        | Grove A2                       |
| A4          | A4      | Option Pin1 (op1)                 | Grove A4                       |
| A5          | A5      | Option Pin2 (op2)                 | Grove A4                       |
| SCK         | SCK     | SPI0 Clock - SD/LoRa              | Not on Grove                   |
| MOS         | MOSI    | Used by SD Card/LoRa              | Not on Grove                   |
| MIS         | MISO    | Used by SD Card/LoRa              | Not on Grove                   |
| RX0         | D0      |                                   | Grove UART                     |
| TX1         | D1      |                                   | Grove UART                     |
| io1         | DIO1    | Do not use                        | Not on Grove (Particle Pin D9) |
| BAT         | VBAT    | Power                             |                                |
| En          |         | Connect to GND to disable 3.3V regulator |                                |
| USB         | VBUS    | Power                             |                                |
| 13          | D13     | LED                               | Not on Grove                   |
| 12          | D12     | Serial Console Enable             | Not on Grove                   |
| 11          | D11     | Unused                            | Not on Grove                   |
| 10          | D10     | Used by SD Card as CS             | Grove D4 (Particle Pin D5)     |
| 9           | D9/A7   | Voltage Battery Pin               | Grove D4 (Particle Pin D4)     |
| 6           | D6      | to AQS Set Pin                    | Grove D2 (Particle Pin D3)     |
| 5           | D5      | to GPS Rst Pin                    | Grove D2 (Particle Pin D2)     |
| SCL         | D3      | I2C Clock                         | Grove I2C_1                    |
| SDA         | D2      | I2C Data                          | Grove I2C_1                    |

## Additional Pin Notes

**Not exposed on headers:**
- D8 = LoRa NSS (Chip Select CS)
- D4 = LoRa Reset  
- D3 = LoRa DIO