---
title: "Implementing NMEA 2000 Temperature Telemetry on STM32F334"
date: "2026-01-18"
description: "Building a dual-message CAN bus system with NMEA 2000 PGN 130312 compliance for industrial temperature monitoring on STM32 microcontroller."
coverImage: "https://images.unsplash.com/photo-1518770660439-4636190af475?w=800"
authorImage: "/avatars/Artem-Melnyk.jpeg"
authorName: "Artem Melnyk"
---

# Implementing NMEA 2000 Temperature Telemetry on STM32F334

This article documents the development of a **dual-message CAN bus system** implementing NMEA 2000 compliance for industrial temperature monitoring on an STM32F334C8T6 microcontroller.

## Project Evolution: v0.0.1 → v0.0.3

### v0.0.1: Initial CAN Implementation
Started with basic CAN transmission at 250 kbps, sending a simple DEADBEEF test pattern to validate hardware and toolchain.

### v0.0.2: Temperature Sensor Integration
Integrated STM32's internal ADC temperature sensor with factory calibration, combining DEADBEEF pattern and temperature data in a single message.

### v0.0.3: NMEA 2000 Compliance
Separated messages into two distinct protocols:
- **A1 Message**: High-frequency heartbeat (30 Hz)
- **T1 Message**: NMEA 2000 compliant temperature telemetry (1 Hz)

---

## NMEA 2000 Protocol Implementation

### Why NMEA 2000?
NMEA 2000 is the industry standard for marine and industrial sensor networks, providing:
- **Interoperability**: Works with standard displays and logging equipment
- **Structured data**: Well-defined Parameter Group Numbers (PGNs)
- **Robust networking**: Based on CAN bus with 29-bit Extended IDs

### PGN 130312: Temperature Standard

The T1 message implements **PGN 130312 - Temperature**, a standardized format for temperature telemetry:

```
Extended 29-bit CAN ID: 0x19FD0801

Bit Structure:
  Bits 26-28: Priority = 6 (low priority for slow-changing data)
  Bit 25:     Reserved = 0
  Bit 24:     Data Page = 1
  Bits 16-23: PDU Format = 0xFD
  Bits 8-15:  PDU Specific = 0x08
  Bits 0-7:   Source Address = 0x01 (device address)
  
PGN Calculation:
  PGN = (Data Page << 16) | (PDU Format << 8) | PDU Specific
  PGN = (1 << 16) | (0xFD << 8) | 0x08 = 130312
```

### Message Format

**T1 Data Payload (8 bytes):**
```
Byte 0: SID (Sequence ID) - Increments with each message
Byte 1: Temperature Instance (0 = single sensor)
Byte 2: Temperature Source (1 = Inside Temperature)
Byte 3-4: Temperature in Kelvin × 100 (uint16, little-endian)
Byte 5-7: Reserved (0xFF)
```

**Example CAN Message:**
```
slcan0  19FD0801  [8]  09 00 01 3B 72 FF FF FF

Decoding:
  SID: 0x09 (9)
  Instance: 0x00 (single sensor)
  Source: 0x01 (inside temperature)
  Temperature: 0x723B = 29243
    → Kelvin = 29243 / 100 = 292.43K
    → Celsius = 292.43 - 273.15 = 19.28°C
```

---

## Message Architecture

### A1 Message: High-Frequency Heartbeat
- **CAN ID**: 0x0A1 (Standard 11-bit)
- **Rate**: 30 Hz (33ms period)
- **Payload**: `DE AD BE EF 00 00 00 00`
- **Purpose**: Fast status indication and bus presence

### T1 Message: Temperature Telemetry
- **CAN ID**: 0x19FD0801 (Extended 29-bit)
- **Rate**: 1 Hz
- **Payload**: NMEA 2000 PGN 130312 format
- **Purpose**: Industrial-standard temperature reporting

---

## Implementation Details

### Temperature Sensor
STM32F334's internal temperature sensor (ADC1_IN16) with factory calibration:
```c
// Factory calibration values at 30°C and 110°C
uint16_t *TS_CAL1 = (uint16_t*)0x1FFFF7B8;
uint16_t *TS_CAL2 = (uint16_t*)0x1FFFF7C2;

// Linear interpolation
float tempCelsius = 30.0f + (rawADC - *TS_CAL1) * 
                    (110.0f - 30.0f) / (*TS_CAL2 - *TS_CAL1);
```

### Main Loop Timing
```c
while (1) {
  HAL_Delay(33U);  // 30Hz base rate
  
  // A1: Send every cycle (30 Hz)
  uint8_t a1Data[8] = {0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00, 0x00, 0x00};
  HAL_CAN_AddTxMessage(&hcan, &txHeaderA1, a1Data, &canMailbox);
  
  // T1: Send every 30 cycles (1 Hz)
  if (++t1Counter >= 30) {
    t1Counter = 0;
    uint16_t tempKelvin = (uint16_t)((tempCelsius + 273.15f) * 100.0f);
    uint8_t t1Data[8] = {
      nmea2000_sid++,         // SID
      0x00,                   // Instance
      0x01,                   // Source (Inside)
      (uint8_t)(tempKelvin),  // Temp low byte
      (uint8_t)(tempKelvin >> 8), // Temp high byte
      0xFF, 0xFF, 0xFF        // Reserved
    };
    HAL_CAN_AddTxMessage(&hcan, &txHeaderT1, t1Data, &canMailbox);
  }
}
```

---

## Hardware Validation

Using SocketCAN on Linux with SLCAN adapter:

```bash
candump slcan0

# Output showing both messages:
slcan0  0A1   [8]  DE AD BE EF 00 00 00 00  # A1 @ 30Hz
slcan0  0A1   [8]  DE AD BE EF 00 00 00 00
...
slcan0  19FD0801  [8]  09 00 01 3B 72 FF FF FF  # T1 @ 1Hz (19.28°C)
```

---

## Design Decisions

### Priority Level 6
Chose **priority 6 (low)** for temperature data because:
- Temperature changes slowly (not time-critical)
- 1 Hz update rate is sufficient
- Higher priority reserved for critical control messages

### Source Address 0x01
Used **source address 0x01** (instead of 0x00) because:
- 0x00 can cause conflicts in multi-device networks
- Each device should have a unique address (0-251 range)
- Enables proper device identification on NMEA 2000 networks

### Temperature Source = 1 (Inside Temperature)
Best match for MCU internal temperature among NMEA 2000 options:
- Represents the internal environment of the device
- Alternative could be Source 3 (Engine Room) for electronics enclosures

---

## Toolchain

- **Microcontroller**: STM32F334C8T6 (ARM Cortex-M4, 64KB Flash)
- **Compiler**: arm-none-eabi-gcc 13.2.1
- **Debugger/Programmer**: st-flash 1.8.0 (STLink v2)
- **CAN Interface**: SLCAN adapter for testing
- **Build System**: GNU Make
- **Version Control**: Git with semantic versioning

---

## Results

Successfully implemented a production-ready dual-message CAN system:

✅ **A1 heartbeat** transmitting at 30 Hz  
✅ **T1 NMEA 2000** temperature at 1 Hz  
✅ Temperature accuracy: ±1°C (factory calibrated)  
✅ Binary size: 8,672 bytes (13% of 64KB flash)  
✅ NMEA 2000 PGN 130312 compliant  
✅ Hardware validated on CAN bus

---

## Future Enhancements

- Implement CAN receive for bidirectional communication
- Add additional NMEA 2000 PGNs (voltage, current, etc.)
- Implement address claiming for dynamic addressing
- Add error handling for CAN bus-off states
- Power consumption optimization (sleep modes)

---

## Repository

Full source code available on GitHub:  
[STM32F334C8T6-CAN](https://github.com/artemmelnyk-extern/STM32F334C8T6-CAN)

**Release**: v0.0.3 (2026-01-18)

---

## Conclusion

This project demonstrates successful integration of NMEA 2000 standards into embedded systems, providing a foundation for industrial-grade sensor telemetry. The dual-message architecture balances fast status updates with standardized data reporting, making it suitable for both diagnostic purposes and integration with commercial NMEA 2000 displays and logging equipment.

The separation of messages allows flexible system architecture where high-frequency heartbeats ensure system health monitoring while low-frequency telemetry conserves bus bandwidth and follows industry best practices.
