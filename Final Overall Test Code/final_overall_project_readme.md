# Smart Energy Meter – Final Integrated System

## Project Overview
This project implements a complete smart energy metering system using ESP32. The system measures AC voltage and current, calculates real-time power and energy consumption, applies monthly billing logic similar to real electricity meters, and uploads selected data to the ThingSpeak cloud platform for remote monitoring.

The project integrates all previously tested and calibrated modules (current sensing, voltage sensing, energy calculation, and cloud communication) into a single final application.

---

## System Features

- Dual-load energy monitoring
- Individual current measurement for each load
- Shared voltage measurement
- Real-time power calculation
- Continuous total energy (kWh) accumulation
- Automatic monthly energy and bill calculation
- Time synchronization using NTP
- Cloud data upload to ThingSpeak

---

## Hardware Components

- ESP32 Dev Module
- ACS712-30A Current Sensor (2 units)
- ZMPT101B Voltage Sensor (1 unit)
- Relay Module (2-channel or equivalent)
- Manual switches for load control
- AC loads (bulbs or resistive loads)
- AC mains supply

---

## Pin Configuration

### ESP32 Pin Mapping

| Component | Signal | ESP32 Pin | Description |
|---------|--------|-----------|-------------|
| ZMPT101B | OUT | GPIO 35 | Analog voltage input |
| ACS712 Load 1 | OUT | GPIO 34 | Current sensing for Load 1 |
| ACS712 Load 2 | OUT | GPIO 33 | Current sensing for Load 2 |
| Relay 1 | IN | GPIO 5 | Load 1 control |
| Relay 2 | IN | GPIO 17 | Load 2 control |
| Switch 1 | INPUT | GPIO 18 | Manual control for Load 1 |
| Switch 2 | INPUT | GPIO 19 | Manual control for Load 2 |

All sensor grounds and ESP32 ground must be common.

---

## Sensor Calibration Summary

### Current Sensor (ACS712-30A)

- Offset calibration performed at no-load condition during startup
- Empirical scale factor applied to compensate for ESP32 ADC non-linearity
- Each load has an independent calibration offset

Calibration parameters used in code:
- Offset values calculated at runtime
- Scale factor defined as a constant based on experimental results

### Voltage Sensor (ZMPT101B)

- Offset determined experimentally
- RMS voltage calculated over a sampling window
- Scaling factor adjusted using known mains voltage measured by a multimeter

These calibration values ensure stable and realistic measurements.

---

## Energy and Billing Logic

### Total Energy

- Represents lifetime meter reading
- Accumulates continuously in kilowatt-hours
- Never resets automatically

### Monthly Energy

- Calculated as the difference between current total energy and the stored energy at the start of the month
- Automatically resets at the beginning of a new month using NTP time

### Monthly Bill

- Calculated using:
  Monthly Bill = Monthly Energy × Cost per kWh
- Cost rate is configurable in software

This logic closely follows real electricity billing systems.

---

## Cloud Integration (ThingSpeak)

### Uploaded Parameters

| ThingSpeak Field | Data Uploaded |
|------------------|---------------|
| Field 6 | Total Energy (kWh) |
| Field 7 | RMS Voltage (V) |
| Field 8 | Monthly Bill (BDT) |

- Data upload interval: 20 seconds
- Uses HTTP GET method
- Compatible with ThingSpeak free plan limits

---

## Software Structure

- Sensor calibration functions
- RMS measurement functions for voltage and current
- Energy integration based on elapsed time
- Monthly change detection using NTP
- Cloud communication handled in a separate function

All logic is contained in a single Arduino sketch for simplicity and clarity.

---

## Safety and Testing Notes

- Use low-power resistive loads during testing
- Ensure proper insulation and secure connections for AC wiring
- Calibration should be repeated if hardware components are changed

---

## Conclusion

This final integrated system demonstrates a complete and practical implementation of a smart energy meter. It combines accurate sensing, real-world billing logic, and cloud-based monitoring in a professional and extensible design suitable for academic projects and demonstrations.

