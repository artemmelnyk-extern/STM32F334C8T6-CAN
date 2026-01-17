# STM32 CAN Bus Communication Project

## Overview
This project implements CAN (Controller Area Network) bus communication on an STM32F334C8T6 microcontroller. The firmware transmits a test pattern (0xDEADBEEF) over the CAN bus at 1 Hz while toggling a GPIO pin for visual feedback. It also reads the internal temperature sensor and transmits temperature data via CAN every second.

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
  - 0x0A1 (Standard 11-bit identifier) - Test pattern
  - 0x0A2 (Standard 11-bit identifier) - Temperature data
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
1. **LED Toggle**: Toggles PA2 every 100ms (10 Hz)
2. **CAN Transmission**: Sends 8-byte message containing `{0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00, 0x00, 0x00}` on ID 0x0A1 every 100ms
3. **Temperature Reading**: Reads internal temperature sensor every 1 second
4. **Temperature Transmission**: Sends temperature data on CAN ID 0x0A2 every 1 second

### Temperature CAN Message Format (ID 0x0A2)
```
Byte 0: 0x54 ('T') - Message identifier
Byte 1: Temperature high byte (signed, °C × 10)
Byte 2: Temperature low byte (signed, °C × 10)
Byte 3: Raw ADC value high byte
Byte 4: Raw ADC value low byte
Byte 5-7: Reserved (0x00)

Example: 0x54 00 FA 06 2C 00 00 00
  → Temperature: 0x00FA = 250 = 25.0°C
  → Raw ADC: 0x062C = 1580
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
candump can0
# Expected output:
# can0  0A1  [8]  DE AD BE EF 00 00 00 00  (test pattern)
# can0  0A2  [8]  54 00 FA 06 2C 00 00 00  (temperature: 25.0°C)
```

## Troubleshooting
- **No CAN messages**: Check CAN transceiver connections and bus termination
- **Build errors**: Ensure all HAL drivers are properly included in the project
- **LED not blinking**: Verify PA2 GPIO configuration and hardware connections

## License
Copyright (c) 2026 STMicroelectronics. Licensed under the terms found in the LICENSE file.

## Author
Project generated using STM32CubeMX and configured for CAN bus communication testing.
