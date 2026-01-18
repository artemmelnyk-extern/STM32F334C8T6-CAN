# NMEA 2000 PGN Selection for UWB Distance Measurement

## Overview

NMEA 2000 doesn't have a specific PGN for **UWB (Ultra-Wideband) distance measurement**. This document evaluates existing NMEA 2000 PGNs that could be adapted for UWB ranging applications, along with considerations for creating a proprietary PGN.

---

## Existing NMEA 2000 Distance PGNs

### **PGN 128267 - Water Depth** (0x1F503) ⭐ RECOMMENDED

Most commonly used for distance measurements in NMEA 2000 networks.

**Extended CAN ID Format:**
```
Extended 29-bit CAN ID: 0x19F50301

Bit Structure:
  Bits 26-28: Priority = 6 (low priority)
  Bit 25:     Reserved = 0
  Bit 24:     Data Page = 1
  Bits 16-23: PDU Format = 0xF5
  Bits 8-15:  PDU Specific = 0x03
  Bits 0-7:   Source Address = 0x01
```

**Data Payload (8 bytes):**
```
Byte 0:    SID (Sequence ID) - Increments with each message
Byte 1-4:  Depth (uint32, little-endian) - 0.01m resolution
           Range: 0 to 42,949,672.95 meters
Byte 5-6:  Offset (int16, little-endian) - 0.001m resolution
           Range: -32.768m to +32.767m
Byte 7:    Max Range (uint8) - 1m resolution
           Range: 0 to 255 meters
```

**Example Message:**
```
CAN ID: 19F50301  [8]  15 E8 03 00 00 00 00 64

Decoding:
  SID: 0x15 (21)
  Depth: 0x000003E8 = 1000 → 1000 × 0.01m = 10.00m
  Offset: 0x0000 = 0 → 0m
  Max Range: 0x64 = 100 → 100m
```

**Advantages for UWB:**
- ✅ 0.01m (1cm) resolution - perfect for UWB precision
- ✅ Simple 8-byte format
- ✅ Well-supported by NMEA 2000 devices and displays
- ✅ Standard PGN - interoperable with existing systems
- ✅ Appropriate update rate (typically 1-10 Hz)

**Implementation for UWB:**
- Replace "depth" with "distance to target/anchor"
- Use offset field for measurement uncertainty/error
- Use max range for UWB module's maximum operating range

---

### **PGN 129029 - GNSS Position Data** (0x1F805)

Full positional data with accuracy metrics, but much more complex.

**Extended CAN ID:** `0x19F80501`

**Data Payload (43 bytes):** *Requires multi-frame transmission*
```
Byte 0:     SID
Byte 1-2:   Date (days since 1970-01-01)
Byte 3-6:   Time (seconds since midnight, 0.0001s resolution)
Byte 7-14:  Latitude (int64, 1e-16 degree resolution)
Byte 15-22: Longitude (int64, 1e-16 degree resolution)
Byte 23-30: Altitude (int64, 1e-6m resolution)
Byte 31:    GNSS Type
Byte 32:    Method
Byte 33:    Integrity
Byte 34:    Number of satellites
Byte 35-36: HDOP (horizontal dilution)
Byte 37-38: PDOP (positional dilution)
Byte 39-40: Geoidal Separation
Byte 41:    Reference stations
Byte 42:    Reserved
```

**Disadvantages for UWB:**
- ❌ Overly complex for simple distance measurement
- ❌ Requires Fast Packet protocol (multi-frame)
- ❌ Wastes bandwidth for non-positional ranging
- ❌ Difficult to parse on simple displays

---

### **PGN 130577 - Direction Data** (0x1FF01)

Navigation-oriented with distance-to-waypoint.

**Extended CAN ID:** `0x19FF0101`

**Data Payload (14 bytes):** *Requires multi-frame transmission*
```
Byte 0:    SID
Byte 1:    Data Mode
Byte 2:    COG Reference
Byte 3-4:  COG (Course Over Ground)
Byte 5-6:  SOG (Speed Over Ground)
Byte 7-8:  Heading
Byte 9-10: Speed Through Water
Byte 11:   Set (current direction)
Byte 12:   Drift (current speed)
Byte 13:   Reserved
```

**Disadvantages for UWB:**
- ❌ Oriented toward navigation/waypoint tracking
- ❌ Multi-frame transmission required
- ❌ Doesn't directly represent simple ranging data
- ❌ Mixed use case (speed, heading, distance)

---

## Proprietary PGN Option

### **PGN 65280 - Custom UWB Distance Measurement** (0xFF00)

Create a custom PGN in the proprietary range (65280-65535 / 0xFF00-0xFFFF).

**Why Millimeter Resolution?**
- UWB achieves 10-30cm accuracy typically, but can reach ±10mm in optimal conditions
- Future-proof for improved UWB modules
- Eliminates rounding errors in calculations
- Better for filtering and averaging algorithms

---

### **Main Attributes & Design Considerations**

#### **1. Sequence Counter (SID)** ⭐ ESSENTIAL
```
Byte 0: Sequence ID (uint8) - Rolls over 0-255
```
**Purpose:**
- Detect lost messages on the CAN bus
- Synchronize data streams
- Identify message age/freshness
- Debug timing issues

**Implementation:**
```c
static uint8_t uwb_sid = 0;
uwbData[0] = uwb_sid++;  // Auto-increment
```

#### **2. Distance Measurement** ⭐ ESSENTIAL
```
Byte 1-2: Distance (uint16, little-endian) - 1mm resolution
          Range: 0 to 65,535mm (0 to 65.535m)
```
**Or Extended Range:**
```
Byte 1-4: Distance (uint32, little-endian) - 1mm resolution
          Range: 0 to 4,294,967,295mm (0 to 4,294km)
```

**Recommendation:** Use uint16 for most applications (65m is sufficient)

#### **3. Signal Quality / Link Quality** ⭐ RECOMMENDED
```
Byte 3: Signal Quality (uint8)
        0 = No signal / Invalid
        1-100 = Quality percentage
        255 = Unknown/Not available
```
**Purpose:**
- Filter unreliable measurements
- Adaptive algorithms (ignore poor quality data)
- Diagnostic/debugging
- Multi-path detection indicator

#### **4. Measurement Uncertainty / Accuracy** ⭐ RECOMMENDED
```
Byte 4-5: Range Accuracy (uint16) - 1mm resolution
          Estimated error: ±value in mm
```
**Or Variance/Confidence:**
```
Byte 4: Standard Deviation (uint8) - in cm (0-255cm)
Byte 5: Confidence Level (uint8) - 0-100%
```

**Purpose:**
- Kalman filter input
- Data fusion with other sensors
- Quality thresholding

#### **5. Target/Anchor Identifier** - OPTIONAL
```
Byte 6-7: Target/Anchor ID (uint16)
          0x0000 = Unknown/Single target
          0x0001-0xFFFE = Specific anchor/tag ID
          0xFFFF = Broadcast/Multiple
```
**Purpose:**
- Multi-anchor systems (trilateration)
- Track multiple tags simultaneously
- Distinguish between different UWB nodes

#### **6. Status Flags** - OPTIONAL
```
Byte 6 or 7: Status Flags (uint8 bitfield)
  Bit 0: Valid Measurement (1=valid, 0=invalid)
  Bit 1: Line of Sight (1=LOS, 0=NLOS)
  Bit 2: Moving Target (1=moving, 0=static)
  Bit 3: Calibration Status (1=calibrated, 0=uncalibrated)
  Bit 4: Low Battery Warning
  Bit 5: Temperature Compensation Active
  Bit 6-7: Reserved
```

**Purpose:**
- NLOS detection (critical for accuracy)
- Filter invalid data
- System health monitoring

#### **7. Update Rate Counter** - OPTIONAL
```
Byte 7: Update Rate (uint8) - Hz
        Actual measurement frequency
```
**Purpose:**
- Adaptive sampling
- Detect communication delays
- Diagnostic information

---

### **Recommended Payload Designs**

#### **Option A: Full-Featured (8 bytes)**
```
Byte 0:    SID (Sequence ID)
Byte 1-2:  Distance (uint16, mm) - 0 to 65.535m
Byte 3:    Signal Quality (0-100%)
Byte 4-5:  Accuracy/Uncertainty (uint16, mm)
Byte 6-7:  Target ID (uint16)
```

#### **Option B: Extended Range (8 bytes)**
```
Byte 0:    SID (Sequence ID)
Byte 1-4:  Distance (uint32, mm) - 0 to 4294km
Byte 5:    Signal Quality (0-100%)
Byte 6:    Status Flags (bitfield)
Byte 7:    Update Rate (Hz)
```

#### **Option C: Minimal (8 bytes)**
```
Byte 0:    SID (Sequence ID)
Byte 1-2:  Distance (uint16, mm)
Byte 3:    Signal Quality (0-100%)
Byte 4:    Status Flags (bitfield)
Byte 5-7:  Reserved (0xFF) - Future expansion
```

#### **Option D: Single-Anchor Optimized (8 bytes)** ⭐ YOUR CONFIGURATION
```
Byte 0:    SID (Sequence ID) - Essential for lost message detection
Byte 1-2:  Distance (uint16, mm) - 0 to 65,535mm (0 to 65.535m)
Byte 3:    Signal Quality (0-100%, 255=Not Available) - Future use
Byte 4:    Status Flags (bitfield) - Future expansion
           Bit 0: Valid Measurement
           Bit 1: Line of Sight (LOS)
           Bit 2-7: Reserved
Byte 5:    Update Rate (Hz) - Actual measurement frequency
Byte 6-7:  Reserved (0xFF) - Future expansion
```

**Design Philosophy:**
- ✅ One UWB device per CAN node (no multi-anchor complexity)
- ✅ 65m range sufficient for most applications
- ✅ Signal Quality placeholder (set to 0xFF until implemented)
- ✅ Status Flags ready for future features
- ✅ Update Rate for diagnostics
- ✅ 2 bytes reserved for future needs

---

### **Extended CAN ID Format**
```
Extended 29-bit CAN ID: 0x19FF0001

Bit Structure:
  Bits 26-28: Priority = 6 (low priority)
  Bit 25:     Reserved = 0
  Bit 24:     Data Page = 1
  Bits 16-23: PDU Format = 0xFF (proprietary)
  Bits 8-15:  PDU Specific = 0x00 (sub-type)
  Bits 0-7:   Source Address = 0x01 (device address)
```

**Priority Options:**
- Priority 2 (0x02): High priority - safety-critical ranging
- Priority 4 (0x04): Medium priority - control systems
- Priority 6 (0x06): Low priority - monitoring/logging

---

### **Implementation Example: Option A (Full-Featured)**

```c
/* UWB Proprietary PGN Configuration */
#define UWB_PGN_PROPRIETARY  65280UL  // 0xFF00
#define UWB_PRIORITY         6UL
#define UWB_SOURCE_ADDRESS   0x01

/* UWB Data Structure */
typedef struct {
  uint8_t  sid;           // Sequence ID
  uint16_t distance_mm;   // Distance in millimeters
  uint8_t  quality;       // Signal quality 0-100%
  uint16_t accuracy_mm;   // Measurement accuracy ±mm
  uint16_t target_id;     // Target/Anchor identifier
} UWB_Data_t;

/* Global Variables */
static uint8_t uwb_sid = 0;
CAN_TxHeaderTypeDef txHeaderUWB;

/* Setup Function */
void UWB_CAN_Init(void)
{
  txHeaderUWB.DLC = 8;
  txHeaderUWB.IDE = CAN_ID_EXT;
  txHeaderUWB.RTR = CAN_RTR_DATA;
  txHeaderUWB.ExtId = (UWB_PRIORITY << 26) | 
                      (1UL << 24) |                    // Data Page = 1
                      (UWB_PGN_PROPRIETARY << 8) | 
                      UWB_SOURCE_ADDRESS;
  txHeaderUWB.TransmitGlobalTime = DISABLE;
}

/* Send UWB Distance Message */
HAL_StatusTypeDef UWB_SendDistance(UWB_Data_t *uwbData)
{
  uint8_t canData[8];
  
  // Pack data into CAN frame
  canData[0] = uwb_sid++;  // Auto-increment sequence
  canData[1] = (uint8_t)(uwbData->distance_mm & 0xFF);
  canData[2] = (uint8_t)(uwbData->distance_mm >> 8);
  canData[3] = uwbData->quality;
  canData[4] = (uint8_t)(uwbData->accuracy_mm & 0xFF);
  canData[5] = (uint8_t)(uwbData->accuracy_mm >> 8);
  canData[6] = (uint8_t)(uwbData->target_id & 0xFF);
  canData[7] = (uint8_t)(uwbData->target_id >> 8);
  
  // Send CAN message
  uint32_t mailbox;
  return HAL_CAN_AddTxMessage(&hcan, &txHeaderUWB, canData, &mailbox);
}

/* Usage Example */
void main_loop(void)
{
  UWB_Data_t uwbData;
  
  // Get UWB measurement from sensor
  uwbData.distance_mm = 2450;   // 2.450m
  uwbData.quality = 85;          // 85% signal quality
  uwbData.accuracy_mm = 15;      // ±15mm accuracy
  uwbData.target_id = 0x0001;    // Anchor #1
  
  // Send via CAN
  if (UWB_SendDistance(&uwbData) != HAL_OK)
  {
    // Handle error
  }
}
```

**Example CAN Message:**
```
CAN ID: 19FF0001  [8]  0A 92 09 55 0F 00 01 00

Decoding:
  SID: 0x0A (10)
  Distance: 0x0992 = 2450mm → 2.450m
  Quality: 0x55 = 85%
  Accuracy: 0x000F = 15mm → ±15mm
  Target ID: 0x0001 = Anchor #1
```

---

### **Implementation Example: Option D (Single-Anchor)** ⭐ RECOMMENDED FOR YOUR USE CASE

```c
/* UWB Proprietary PGN Configuration */
#define UWB_PGN_PROPRIETARY  65280UL  // 0xFF00
#define UWB_PRIORITY         6UL
#define UWB_SOURCE_ADDRESS   0x01

/* Status Flag Definitions */
#define UWB_STATUS_VALID     (1 << 0)  // Measurement is valid
#define UWB_STATUS_LOS       (1 << 1)  // Line of Sight detected

/* Signal Quality Special Values */
#define UWB_QUALITY_NOT_AVAILABLE  0xFF  // Feature not implemented yet

/* UWB Data Structure - Single Anchor */
typedef struct {
  uint8_t  sid;           // Sequence ID (auto-increments)
  uint16_t distance_mm;   // Distance in millimeters (0-65535mm)
  uint8_t  quality;       // Signal quality 0-100%, 255=not available
  uint8_t  status_flags;  // Bitfield: validity, LOS, etc.
  uint8_t  update_rate;   // Actual update rate in Hz
} UWB_SingleAnchor_t;

/* Global Variables */
static uint8_t uwb_sid = 0;
CAN_TxHeaderTypeDef txHeaderUWB;

/* Initialize UWB CAN Header */
void UWB_CAN_Init(void)
{
  txHeaderUWB.DLC = 8;
  txHeaderUWB.IDE = CAN_ID_EXT;
  txHeaderUWB.RTR = CAN_RTR_DATA;
  txHeaderUWB.ExtId = (UWB_PRIORITY << 26) | 
                      (1UL << 24) |                    // Data Page = 1
                      (UWB_PGN_PROPRIETARY << 8) | 
                      UWB_SOURCE_ADDRESS;
  txHeaderUWB.TransmitGlobalTime = DISABLE;
}

/* Send UWB Distance Message - Single Anchor */
HAL_StatusTypeDef UWB_SendDistance_SingleAnchor(UWB_SingleAnchor_t *uwbData)
{
  uint8_t canData[8];
  
  // Pack data into CAN frame
  canData[0] = uwb_sid++;                              // Auto-increment SID
  canData[1] = (uint8_t)(uwbData->distance_mm & 0xFF); // Distance low byte
  canData[2] = (uint8_t)(uwbData->distance_mm >> 8);   // Distance high byte
  canData[3] = uwbData->quality;                       // Signal quality
  canData[4] = uwbData->status_flags;                  // Status flags
  canData[5] = uwbData->update_rate;                   // Update rate (Hz)
  canData[6] = 0xFF;                                   // Reserved
  canData[7] = 0xFF;                                   // Reserved
  
  // Send CAN message with error checking
  uint32_t mailbox;
  HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(&hcan, &txHeaderUWB, canData, &mailbox);
  
  return status;
}

/* Usage Example in Main Loop */
void main_loop(void)
{
  UWB_SingleAnchor_t uwbData;
  
  // Read UWB distance from sensor (example values)
  uwbData.distance_mm = 3250;                    // 3.250m from anchor
  uwbData.quality = UWB_QUALITY_NOT_AVAILABLE;   // Not implemented yet
  uwbData.status_flags = UWB_STATUS_VALID;       // Measurement is valid
  uwbData.update_rate = 10;                      // 10 Hz update rate
  
  // Send via CAN bus
  if (UWB_SendDistance_SingleAnchor(&uwbData) != HAL_OK)
  {
    // Handle transmission error
    // Could retry or set error flag
  }
  
  HAL_Delay(100);  // 10 Hz update rate
}

/* Integration with Existing Project */
void UWB_Integration_Example(void)
{
  UWB_SingleAnchor_t uwbData;
  static uint32_t uwb_counter = 0;
  
  // Send UWB message at 10 Hz (every 100ms)
  uwb_counter++;
  if (uwb_counter >= 10)  // Assuming main loop runs at 100Hz
  {
    uwb_counter = 0;
    
    // TODO: Replace with actual UWB sensor reading
    uwbData.distance_mm = 5000;                    // Placeholder: 5.000m
    uwbData.quality = UWB_QUALITY_NOT_AVAILABLE;   // Set to 0xFF for now
    uwbData.status_flags = UWB_STATUS_VALID | UWB_STATUS_LOS;
    uwbData.update_rate = 10;
    
    UWB_SendDistance_SingleAnchor(&uwbData);
  }
}
```

**Example CAN Message:**
```
CAN ID: 19FF0001  [8]  15 B2 0C FF 03 0A FF FF

Decoding:
  SID: 0x15 (21) - Message sequence number
  Distance: 0x0CB2 = 3250mm → 3.250m
  Quality: 0xFF = Not Available (feature not implemented)
  Status: 0x03 = Valid (bit 0) + LOS (bit 1)
  Update Rate: 0x0A = 10 Hz
  Reserved: 0xFF FF (for future use)
```

**Integration Steps:**
1. Add UWB header setup in your initialization code
2. Call `UWB_SendDistance_SingleAnchor()` at your desired rate (1-20 Hz)
3. Set quality to `0xFF` until sensor provides signal quality
4. Use status flags when you need to indicate errors or special conditions
5. Reserved bytes ready for future features (accuracy, battery, etc.)

---

### **Advantages:**
- ✅ Optimized specifically for UWB ranging
- ✅ 1mm resolution for maximum precision
- ✅ Signal quality and accuracy metrics
- ✅ Multi-anchor support
- ✅ Single-frame transmission
- ✅ Sequence counter for lost message detection
- ✅ Expandable with status flags

### **Disadvantages:**
- ❌ Not a standard PGN - no interoperability
- ❌ Won't display on standard NMEA 2000 devices
- ❌ Requires custom software/firmware for interpretation
- ❌ Must be documented for other developers

---

### **Other Considerations**

#### **Multi-Frame Option (For More Data)**
If 8 bytes isn't enough, use Fast Packet Protocol:
```
Frame 1: [SID][Distance 4B][Quality][Accuracy 2B]
Frame 2: [Target ID][RSSI][Temp][Battery][LOS flag]...
```

#### **Timestamp Addition**
If precise timing is critical:
```
Byte 5-7: Timestamp (uint24) - milliseconds since last second
          Provides sub-second timing resolution
```

#### **Multiple Distance Measurements**
For multi-anchor systems, send separate messages per anchor:
```
Source Address 0x01 → Anchor 1 distance
Source Address 0x02 → Anchor 2 distance
Source Address 0x03 → Anchor 3 distance
```

---

## Recommendation Summary

### **Best Choice: PGN 128267 (Water Depth)**

**Use Case:** UWB distance measurement with standard NMEA 2000 compatibility

**Rationale:**
1. **Precision**: 1cm resolution matches UWB capabilities
2. **Simplicity**: Single-frame, 8-byte format
3. **Compatibility**: Displays on any NMEA 2000 device
4. **Semantics**: "Depth/Distance" is a natural mapping
5. **Field Reuse**: Offset = measurement error, Max Range = UWB range

**Implementation Example:**
```c
/* UWB Distance message using PGN 128267 */
CAN_TxHeaderTypeDef txHeaderUWB;
txHeaderUWB.DLC = 8;
txHeaderUWB.IDE = CAN_ID_EXT;
txHeaderUWB.RTR = CAN_RTR_DATA;
txHeaderUWB.ExtId = (6UL << 26) | (128267UL << 8) | 0x01; // 0x19F50301
txHeaderUWB.TransmitGlobalTime = DISABLE;

// Prepare data
uint32_t distanceCm = 250;  // 2.50m
uint32_t distanceValue = distanceCm;  // Already in 0.01m units
int16_t offsetMm = 5;  // ±5mm accuracy
int16_t offsetValue = offsetMm;  // Convert to 0.001m units
uint8_t maxRange = 100;  // 100m UWB max range

uint8_t uwbData[8];
uwbData[0] = uwb_sid++;  // SID
uwbData[1] = (uint8_t)(distanceValue & 0xFF);
uwbData[2] = (uint8_t)((distanceValue >> 8) & 0xFF);
uwbData[3] = (uint8_t)((distanceValue >> 16) & 0xFF);
uwbData[4] = (uint8_t)((distanceValue >> 24) & 0xFF);
uwbData[5] = (uint8_t)(offsetValue & 0xFF);
uwbData[6] = (uint8_t)((offsetValue >> 8) & 0xFF);
uwbData[7] = maxRange;

HAL_CAN_AddTxMessage(&hcan, &txHeaderUWB, uwbData, &canMailbox);
```

---

### **Alternative: Proprietary PGN 65280**

**Use Case:** Custom UWB network with specialized requirements

**When to Use:**
- Need 1mm resolution (instead of 1cm)
- Multi-anchor/target system
- Signal quality metrics required
- No need for standard NMEA 2000 display compatibility
- Building a proprietary system

---

## Implementation Considerations

### Update Rate
- **PGN 128267**: Typically 1-10 Hz (suitable for most applications)
- **UWB Systems**: Can provide 20-100+ Hz update rates
- **Recommendation**: Start with 10 Hz, adjust based on network load

### CAN Bus Bandwidth
At 250 kbps CAN speed:
- Single 8-byte message ≈ 130 bits (including CAN overhead)
- 10 Hz = 1,300 bits/second = 0.52% bus utilization
- Very low impact on overall network

### Error Handling
- Check `HAL_CAN_AddTxMessage()` return value
- Implement timeout detection for stale UWB readings
- Use SID rollover (0-255) to detect lost messages

### Multi-Device Networks
- Assign unique Source Address (bits 0-7) to each UWB device
- Standard practice: 0x01-0xFD for devices, 0xFE-0xFF reserved
- Example: Anchor 1 = 0x01, Anchor 2 = 0x02, Tag = 0x10

---

## References

- NMEA 2000 PGN List: [actisense.com](https://actisense.com/acti_download/nmea-2000-pgn/)
- CAN Protocol Specification: ISO 11898
- NMEA 2000 Standard: IEC 61162-3

---

**Document Version:** 1.0  
**Date:** 2026-01-18  
**Project:** STM32F334 CAN Bus Implementation
