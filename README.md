# STM32 CAN Bus Communication Project

## Overview
This project implements CAN (Controller Area Network) bus communication on an STM32F334C8T6 microcontroller. The firmware transmits a test pattern (0xDEADBEEF) over the CAN bus at 1 Hz while toggling a GPIO pin for visual feedback.

## Hardware Specifications
- **Microcontroller**: STM32F334C8T6
- **CAN Interface**: 
  - RX: PA11
  - TX: PA12
- **LED Output**: PA2 (toggles at 10 Hz)
- **Clock**: External HSE with PLL (32 MHz system clock)

## CAN Configuration
- **Baud Rate**: 250 kbps (configurable via prescaler and bit timing)
- **Mode**: Normal mode
- **Message ID**: 0x0A1 (Standard 11-bit identifier)
- **Data Length**: 8 bytes
- **Filter**: Accept all messages (IDMASK mode with all zeros)
- **FIFO**: RX FIFO0

### CAN Timing Parameters
- Prescaler: 4
- Sync Jump Width: 1 TQ
- Time Segment 1: 3 TQ
- Time Segment 2: 4 TQ

## Project Structure
```
uwb-stm-can/
├── Core/
│   ├── Inc/                    # Header files
│   │   ├── main.h
│   │   ├── can.h
│   │   └── gpio.h
│   └── Src/                    # Source files
│       ├── main.c              # Main application logic
│       ├── can.c               # CAN peripheral configuration
│       ├── gpio.c              # GPIO configuration
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

## Functionality
The main application loop performs the following operations:
1. **LED Toggle**: Toggles PA2 every 100ms (10 Hz)
2. **CAN Transmission**: Sends 8-byte message containing `{0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00, 0x00, 0x00}`
3. **Status Check**: Verifies if the CAN message is pending in the transmit mailbox

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
# can0  0A1  [8]  DE AD BE EF 00 00 00 00
```

## Troubleshooting
- **No CAN messages**: Check CAN transceiver connections and bus termination
- **Build errors**: Ensure all HAL drivers are properly included in the project
- **LED not blinking**: Verify PA2 GPIO configuration and hardware connections

## License
Copyright (c) 2026 STMicroelectronics. Licensed under the terms found in the LICENSE file.

## Author
Project generated using STM32CubeMX and configured for CAN bus communication testing.
