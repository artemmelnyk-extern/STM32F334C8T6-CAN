# Temperature Sensor Implementation

## Overview
Added internal temperature sensor support to the STM32F334C8T6 CAN project. The temperature is read from the microcontroller's built-in sensor (ADC1_IN16) and transmitted via CAN bus every 1 second.

## Implementation Details

### Hardware
- **Sensor**: STM32F334 internal temperature sensor
- **ADC Channel**: ADC1_IN16 (Channel 16)
- **Resolution**: 12-bit (0-4095)
- **Calibration**: Factory calibrated at 30°C and 110°C
- **Accuracy**: ±2°C (with factory calibration)

### Files Added
1. **Core/Inc/adc.h** - ADC peripheral header
2. **Core/Src/adc.c** - ADC initialization and MSP functions
3. **Core/Inc/temperature.h** - Temperature sensor driver header
4. **Core/Src/temperature.c** - Temperature reading and calculation functions

### Calibration Data
The STM32F334 stores factory calibration values in system memory:
- `TEMP30_CAL_ADDR`: 0x1FFFF7B8 - ADC reading at 30°C
- `TEMP110_CAL_ADDR`: 0x1FFFF7C2 - ADC reading at 110°C

These values are unique per chip and compensate for manufacturing variations.

### Temperature Calculation Formula
```c
Temperature (°C) = ((110 - 30) / (CAL_110 - CAL_30)) × (ADC_reading - CAL_30) + 30
```

This two-point calibration provides linear interpolation between the factory-measured points.

### CAN Message Format

**Message ID**: 0x0A2 (Standard 11-bit)
**Data Length**: 8 bytes
**Transmission Rate**: 1 Hz

| Byte | Description | Format |
|------|-------------|--------|
| 0 | Message ID | 0x54 ('T' for Temperature) |
| 1 | Temperature High | Signed int16 MSB |
| 2 | Temperature Low | Signed int16 LSB |
| 3 | Raw ADC High | uint16 MSB |
| 4 | Raw ADC Low | uint16 LSB |
| 5 | Reserved | 0x00 |
| 6 | Reserved | 0x00 |
| 7 | Reserved | 0x00 |

**Temperature Format**: Signed integer × 10 (e.g., 250 = 25.0°C)
**Range**: -3276.8°C to +3276.7°C (0.1°C resolution)

### Example CAN Message
```
ID: 0x0A2
Data: 54 00 FA 06 2C 00 00 00

Interpretation:
- Byte 0: 0x54 = 'T' (temperature message)
- Bytes 1-2: 0x00FA = 250 → 25.0°C
- Bytes 3-4: 0x062C = 1580 → Raw ADC value
- Bytes 5-7: Reserved
```

## API Functions

### `Temperature_ReadADC()`
```c
uint16_t Temperature_ReadADC(void);
```
Reads raw 12-bit ADC value from temperature sensor.
- Returns: ADC value (0-4095)

### `Temperature_GetCelsius()`
```c
float Temperature_GetCelsius(void);
```
Calculates temperature in Celsius using factory calibration.
- Returns: Temperature in °C (float)

### `Temperature_GetCelsiusInt()`
```c
int16_t Temperature_GetCelsiusInt(void);
```
Returns temperature as integer × 10 for CAN transmission.
- Returns: Temperature × 10 (e.g., 255 = 25.5°C)

## Usage Example

### Reading Temperature
```c
// Read temperature as float
float temp = Temperature_GetCelsius();
// Example result: 25.3°C

// Read temperature for CAN transmission
int16_t tempInt = Temperature_GetCelsiusInt();
// Example result: 253 (represents 25.3°C)

// Read raw ADC value
uint16_t raw = Temperature_ReadADC();
// Example result: 1580
```

### Monitoring with candump
```bash
candump slcan0

# Expected output:
slcan0  0A1  [8]  DE AD BE EF 00 00 00 00  # Test pattern (10 Hz)
slcan0  0A2  [8]  54 00 FA 06 2C 00 00 00  # Temperature: 25.0°C (1 Hz)
```

### Decoding Temperature
```python
# Python example to decode temperature from CAN message
def decode_temperature(data):
    temp_raw = (data[1] << 8) | data[2]  # Bytes 1-2
    temp_celsius = temp_raw / 10.0
    
    adc_raw = (data[3] << 8) | data[4]   # Bytes 3-4
    
    return temp_celsius, adc_raw

# Example data: [0x54, 0x00, 0xFA, 0x06, 0x2C, 0x00, 0x00, 0x00]
temp, adc = decode_temperature([0x54, 0x00, 0xFA, 0x06, 0x2C, 0x00, 0x00, 0x00])
print(f"Temperature: {temp}°C, ADC: {adc}")
# Output: Temperature: 25.0°C, ADC: 1580
```

## Build Information

### Memory Usage
- **Text**: 8,636 bytes (vs 11,828 bytes before - reduced due to removed test code)
- **Data**: 12 bytes
- **BSS**: 1,788 bytes (248 + 1540 stack/heap)
- **Flash Usage**: ~8.6 KB / 64 KB (13.4%)
- **RAM Usage**: ~1.8 KB / 12 KB (15%)

### Configuration Changes
1. Enabled `HAL_ADC_MODULE_ENABLED` in [stm32f3xx_hal_conf.h](Core/Inc/stm32f3xx_hal_conf.h)
2. Added ADC HAL drivers to [Makefile](Makefile):
   - `stm32f3xx_hal_adc.c`
   - `stm32f3xx_hal_adc_ex.c`

## Testing

### Flash Firmware
```bash
make clean
make -j4
make flash
```

### Monitor CAN Bus
```bash
# Using slcan0 interface
candump slcan0

# Expected output every second:
# Test pattern (10 Hz):
slcan0  0A1  [8]  DE AD BE EF 00 00 00 00

# Temperature data (1 Hz):
slcan0  0A2  [8]  54 00 FA 06 2C 00 00 00
```

### Verify Temperature Reading
1. Monitor CAN messages with `candump slcan0`
2. Temperature should be around room temperature (20-30°C)
3. Try heating the microcontroller gently (e.g., with finger) - temperature should increase
4. Temperature value should stabilize when at constant ambient

## Troubleshooting

### Temperature Seems Incorrect
- Check calibration data is being read correctly
- Verify ADC is properly initialized
- Ensure sufficient ADC sampling time (601.5 cycles)
- Factory calibration assumes 3.3V ADC reference

### No Temperature Messages
- Verify CAN ID 0x0A2 is not filtered
- Check ADC initialization succeeded
- Ensure ADC calibration completed at startup
- Verify HAL_ADC_MODULE_ENABLED is defined

### ADC Returns 0 or 4095
- ADC may not be calibrated (call `HAL_ADCEx_Calibration_Start`)
- Check ADC clock is enabled
- Verify temperature sensor channel is configured

## Future Enhancements
1. **Averaging**: Implement moving average filter for stability
2. **Oversampling**: Use ADC oversampling for higher resolution
3. **Alarms**: Add temperature threshold warnings
4. **Logging**: Store temperature history for trend analysis
5. **External Sensor**: Add support for external I2C/SPI temperature sensors
6. **Multi-sensor**: Support multiple temperature measurement points

## References
- STM32F334 Reference Manual (RM0364) - Section 15: Analog-to-digital converter
- STM32F334 Datasheet - Temperature sensor characteristics
- AN3964: Temperature sensor implementation on STM32 MCUs
- Factory calibration addresses from STM32F334 system memory map

## Author
Implementation completed: January 17, 2026
Branch: dev/issue-001-temperature
