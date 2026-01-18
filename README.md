# STM32 CAN Bus Communication Project

## Overview
This project implements CAN (Controller Area Network) bus communication on an STM32F334C8T6 microcontroller. The firmware transmits two separate CAN messages:
- **A1 Message**: Test pattern (0xDEADBEEF) at 30 Hz
- **T1 Message**: Temperature data in NMEA 2000 PGN 130312 format at 1 Hz

A GPIO pin toggles for visual feedback at approximately 30 Hz.

## Hardware Specifications
- **Microcontroller**: STM32F334C8T6
- **CAN Interface**: 
  - RX: PA11
  - TX: PA12
- **LED Output**: PA2 (toggles at 10 Hz)
- **Temperature Sensor**: Internal ADC1_IN16 (factory calibrated)
- **Clock**: External HSE with PLL (32 MHz system clock)

## CAN Configuration
- **Baud Rate**: 250 kbps (configurable via prescaler and bit timing)
- **Mode**: Normal mode
- **Message IDs**: 
  - **A1**: 0x0A1 (Standard 11-bit identifier) - Test pattern at 30 Hz
  - **T1**: 0x19FD0801 (Extended 29-bit identifier) - NMEA 2000 Temperature at 1 Hz
- **Data Length**: 8 bytes
- **Filter**: Accept all messages (IDMASK mode with all zeros)
- **FIFO**: RX FIFO0

### CAN Timing Parameters
- Prescaler: 4
- Sync Jump Width: 1 TQ
- Time Segment 1: 3 TQ
- Time Segment 2: 4 TQ

### ADC Configuration
- **Resolution**: 12-bit (0-4095)
- **Channel**: ADC1_IN16 (internal temperature sensor)
- **Sampling Time**: 601.5 cycles (for accuracy)
- **Calibration**: Factory calibrated at 30°C and 110°C
- **Temperature Range**: -40°C to +125°C (typical operation)
- **Accuracy**: ±2°C (with factory calibration)

## Project Structure
```
uwb-stm-can/
├── Core/
│   ├── Inc/                    # Header files
│   │   ├── main.h
│   │   ├── can.h
│   │   ├── gpio.h
│   │   ├── adc.h
│   │   └── temperature.h
│   └── Src/                    # Source files
│       ├── main.c              # Main application logic
│       ├── can.c               # CAN peripheral configuration
│       ├── gpio.c              # GPIO configuration
│       ├── adc.c               # ADC configuration
│       ├── temperature.c       # Temperature sensor driver
│       └── system_stm32f3xx.c  # System initialization
├── Drivers/                    # STM32 HAL drivers
│   ├── CMSIS/                  # ARM CMSIS libraries
│   └── STM32F3xx_HAL_Driver/   # STM32F3 HAL driver
├── STM32CubeIDE/              # IDE project files
└── simplecan.ioc              # STM32CubeMX configuration
```

## Features
- ✅ CAN bus initialization and configuration
- ✅ Periodic message transmission (1 Hz)
- ✅ GPIO toggle for visual status indication
- ✅ Configurable CAN filter
- ✅ Transmit mailbox status checking
- ✅ Internal temperature sensor reading with factory calibration
- ✅ Temperature data transmission via CAN (1 Hz)

## Functionality
The main application loop performs the following operations:
1. **LED Toggle**: Toggles PA2 every ~33ms (approximately 30 Hz)
2. **A1 Message Transmission**: Sends DEADBEEF test pattern at 30 Hz
3. **Temperature Reading**: Reads internal temperature sensor every 1 second
4. **T1 Message Transmission**: Sends temperature data in NMEA 2000 format at 1 Hz

### A1 Message Format (ID 0x0A1) - 30 Hz
```
Standard 11-bit CAN ID: 0x0A1
Data: DE AD BE EF 00 00 00 00

Byte 0-3: 0xDE AD BE EF - Fixed test pattern
Byte 4-7: 0x00 - Reserved

Example:
  slcan0  0A1  [8]  DE AD BE EF 00 00 00 00
```

### T1 Message Format (ID 0x19FD0801) - 1 Hz
```
Extended 29-bit CAN ID: 0x19FD0801
  Priority: 6 (bits 26-28) - Low priority for slow-changing data
  Reserved: 0 (bit 25)
  Data Page: 1 (bit 24)
  PDU Format: 0xFD (bits 16-23)
  PDU Specific: 0x08 (bits 8-15)
  PGN: 130312 (0x1FD08) - NMEA 2000 Temperature
  Source: 0x01 (bits 0-7) - Device address

NMEA 2000 PGN 130312 Format:
Byte 0: SID (Sequence ID) - Increments with each message
Byte 1: Temperature Instance (0 = single sensor)
Byte 2: Temperature Source (1 = Inside Temperature)
Byte 3-4: Temperature in Kelvin × 100 (uint16, little-endian)
Byte 5-7: Reserved (0xFF)

Example: slcan0  19FD0801  [8]  28 00 01 65 74 FF FF FF
  Byte 0: 0x28 (40) = SID
  Byte 1: 0x00 = Instance 0
  Byte 2: 0x01 = Inside Temperature
  Bytes 3-4: 0x7465 = 29797 → 297.97K → 24.82°C
  Bytes 5-7: 0xFF FF FF = Reserved

Temperature Calculation:
  Kelvin = (bytes[4] << 8) | bytes[3]
  Celsius = (Kelvin / 100.0) - 273.15
  Example: 29797 / 100 - 273.15 = 24.82°C
```

## Building and Flashing
This project is designed for STM32CubeIDE:

1. Open the project in STM32CubeIDE
2. Build the project: `Project → Build Project`
3. Connect your STM32 board via ST-Link
4. Flash the firmware: `Run → Debug` or `Run → Run`

### Using Command Line (Optional)
```bash
cd STM32CubeIDE/Debug
make clean
make -j4
# Flash using st-flash or openocd
```

## Dependencies
- STM32F3xx HAL Driver v1.5.x
- CMSIS Core v5.x
- STM32CubeIDE or compatible ARM GCC toolchain

## CAN Bus Setup
To test this firmware:
1. Connect a CAN transceiver (e.g., TJA1050, MCP2551) to PA11 (RX) and PA12 (TX)
2. Connect CAN_H and CAN_L to your CAN bus network
3. Ensure proper bus termination (120Ω resistors)
4. Use a CAN analyzer or another CAN node to monitor messages

## Monitoring CAN Messages
You can observe the transmitted messages using:
- USB-to-CAN adapters (e.g., CANable, PCAN-USB)
- Software tools: `candump`, `cansniffer` (Linux), or CANalyzer (Windows)

Example using SocketCAN on Linux:
```bash
candump slcan0
# Expected output:
# A1 messages at 30 Hz:
# slcan0  0A1  [8]  DE AD BE EF 00 00 00 00
# slcan0  0A1  [8]  DE AD BE EF 00 00 00 00
# ...
# T1 message at 1 Hz:
# slcan0  19FD0801  [8]  28 00 01 65 74 FF FF FF  (temp: 24.82°C)
```

## Troubleshooting
- **No CAN messages**: Check CAN transceiver connections and bus termination
- **Build errors**: Ensure all HAL drivers are properly included in the project
- **LED not blinking**: Verify PA2 GPIO configuration and hardware connections

## License
Copyright (c) 2026 STMicroelectronics. Licensed under the terms found in the LICENSE file.

## Author
Project generated using STM32CubeMX and configured for CAN bus communication testing.
