# PCMDI - Physically Connected Multi Device Interface

PCMDI is a custom MCU communication protocol, implementing software device addressing with up to 255 devices and following the Master/Slave methodology.

#### Addressing

The address `0x00` is reserved for the master device.
The remaining 254 addresses can be used for slave devices.

#### Signal Wires

| Signal | Description |
|--|--|
| CLK (Clock) | Used for synchronisation of the devices |
| TSE (Transmit Start/End) | Determines when the data transfer should start or end |
| DAT (Data) | A signal representing the data which is to be sent/received |
| GND (Ground) | A common ground connection between the devices |
