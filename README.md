# PCMDI - Physically Connected Multi Device Interface

PCMDI is a custom MCU communication protocol that follows the Master/Slave methodology.

### Features

- Can address up to 255 devices
- Can perform device discovery on the bus
- Can send/receive data with an arbitrary length, stored in 32 user definable registers.

## Packet

Each packet starts with a 16 bit header, which contains the following fields:

| Field | Size | Description  |
| --- | --- | --- |
| Address | 8 bits | An address of a slave device |
| Transmission Mode  | 1 bit | Selects between Read(0) / Write(1) mode |
| [IRD](#ird) | 2 bits | Internal Register Descriptor |
| [RID](#rid) | 5 bits | Register Index Descriptor |

#### Registers

Registers are storage blocks that contain internal PCMDI or user data. There are 4 reserved internal registers (see [IRD](#ird)) and 32 user definable registers.

##### Register fields:
| Field | Size | Description |
| --- | --- | --- |
| Index Descriptor | 5 bits | A unique number for each register |
| Size | 16 bits | The size of the data stored in the register |
| Data | n | Pointer to the data stored in the register |

#### Register Bank

A register bank is a container that stores the **user defined registers**. The maximum number of registers that can be stored in the bank is 32, due to the packet header register addressing limit.

##### Register bank fields:
| Field | Size | Description |
| --- | --- |
| Registers | n | An array that stores the user defined registers |
| Size | 8 bits | The number of registers stored in the bank |

#### IRD

The Internal Register Descriptor (IRD) is a 2 bit field in the header of a message. It represents the type of message that we are sending and it can be one of the following:

| Type of message | Value | Usage |
| --- | --- | --- |
| [Discover](#device-discovery-process) | 0x00  | Discovering devices on the bus |
| [Bank size request](#bank-size-request) | 0x01 | Reading the size of the register bank on a slave device |
| [Register size request](#register-size-request) | 0x02 | Reading the size of a [user defined register](#registers) on a slave device |
| [Register data request](#register-data-request) | 0x03 | Reading or writing data in a [user defined register](#registers) on a slave device |

#### RID

The Register Index Descriptor is a 5 bit field in the header of a message. It is used in conjunction with [IRD](#ird) values `0x02` and `0x03` for reading or writing to a [user defined register](#registers) with an index descriptor represented by this field.

### Device discovery process

Set header `IRD` to `PCMDI_IRD_DISCOVER_REQUEST (0x00)`.
Set header transmission mode to `PCMDI_TRANSMISSION_MODE_READ`.
When discovering new devices, `RID` is ignored.


For each of the 254 addresses, do the following steps:
1. Set header address to a new address
2. Set `DAT` pin as `OUTPUT`
3. Set the `TSE` pin
4. Transmit header
5. Set `DAT` pin as `INPUT`
6. Send 8 clock pulses
7. The `DAT` line going high at any point during the last 8 clock pulses means that the device is present on the bus
8. Clear the `TSE` pin

Assuming we are trying to discover if a device with address `0x01` exists on the bus, the header should look like the following: `00000001 000xxxxx`

### Bank size request

Set header address to the address of the device from which you wish to read the bank size from.
Set header `IRD` to `PCMDI_IRD_BANK_SIZE_REQUEST (0x01)`.
Set header transmission mode to `PCMDI_TRANSMISSION_MODE_READ`.

When requesting the bank size, `RID` is ignored.

1. Set `DAT` pin as `OUTPUT`.
2. Set the `TSE` pin.
3. Transmit header.
4. Set `DAT` pin as `INPUT`.
5. Send 8 clock pulses.
6. The received data from the `DAT` line indicates the size of the bank from the requested device.
7. Clear the `TSE` pin.

Assuming we make a request to the device with address `0x01`, the header should look like the following: `00000001 001xxxxx`

### Register size request

Set header address to the address of the device from which you wish to read the bank size from.
Set header `IRD` to `PCMDI_IRD_REGISTER_SIZE_REQUEST (0x02)`.
Set header transmission mode to `PCMDI_TRANSMISSION_MODE_READ`.
Set header `RID` to the index of the register from which we want to get the size of.

1. Set `DAT` pin as `OUTPUT`.
2. Set the `TSE` pin.
3. Transmit header.
4. Set `DAT` pin as `INPUT`.
5. Send 8 clock pulses.
6. The received data from the `DAT` line indicates the size of the register from the requested device.
7. Clear the `TSE` pin.

Assuming we make a request to the device with address `0x01` and we want to read the size of the `0x02` register, the header should look like the following: `00000001 01000010`

### Register data request

Set header address to the address of the device from which you wish to read the bank size from.
Set header `IRD` to `PCMDI_IRD_REGISTER_DATA_REQUEST (0x03)`.
Set header `RID` to the index of the register we wish to read.

#### Reading data

Set header transmission mode to `PCMDI_TRANSMISSION_MODE_READ`.

1. Set `DAT` pin as `OUTPUT`.
2. Set the `TSE` pin.
3. Transmit header.
4. Set `DAT` pin as `INPUT`.
5. Send ***N*** clock pulses, where ***N*** is the size of the requested register. The number of pulses can be obtained by doing a register size request.
6. The received data from the `DAT` line is the contents of the requested register.
7. Clear the `TSE` pin.

Assuming we make a request to the device with address `0x01` and we want to read the data of the `0x02` register, the header should look like the following: `00000001 01100010`

#### Writing data

Set header transmission mode to `PCMDI_TRANSMISSION_MODE_WRITE`

Assuming we make a request to the device with address `0x01` and we want to write data to the `0x02` register, the header should look like the following: `00000001 11100010`

1. Set `DAT` pin as `OUTPUT`.
2. Set the `TSE` pin.
3. Transmit header.
5. Transmit ***N*** bits of data, where ***N*** is the size of the register in which we are writing. This number can be obtained by firstly doing a register size request and then writing the data to that register.
7. Clear the `TSE` pin.

