# Smart Energy Meter – Calibration & Test READMEs

This document contains separate README sections for each calibration and test sketch used in the Smart Energy Meter project. Each section is written independently so it can be copied into its own README.md file if needed.

---

## README 1: ACS712 Current Sensor Calibration & Test

### Purpose
This sketch is used to test and calibrate the ACS712-30A current sensor before integrating it into the main energy meter system. The goal is to determine:
- Zero-current offset (no-load condition)
- Practical sensor response under load
- A reference value for empirical sensitivity calibration

This step ensures reliable current measurement when used with the ESP32 ADC.

### Hardware Used
- ESP32 Dev Module
- ACS712-30A Current Sensor
- Known resistive AC load (e.g., 15 W or 30 W bulb)

### Pin Connections

| Component | Pin Name | ESP32 Pin | Description |
|---------|----------|-----------|-------------|
| ACS712  | OUT      | GPIO 34   | Analog output from current sensor |
| ACS712  | VCC      | 5V        | Sensor power supply |
| ACS712  | GND      | GND       | Common ground |

### Calibration Method
1. Power the ESP32 and ACS712 with no load connected.
2. The code samples the ADC multiple times to calculate the average offset.
3. This offset represents the zero-current reference and is stored in software.
4. A known load is then connected to observe sensor response.
5. RMS voltage at the sensor output is measured to help derive an empirical scale factor.

### Output Observations
- With no load, the calculated signal should be close to zero.
- With a known load, the ADC values should fluctuate consistently.
- These observations confirm correct sensor operation.

### Outcome
The offset value obtained here is later used in the main project. The observed RMS signal under known load conditions is used to derive a practical sensitivity factor for accurate current calculation.

---

## README 2: ZMPT101B Voltage Sensor Calibration & Test

### Purpose
This sketch is used to calibrate and verify the ZMPT101B AC voltage sensor. The calibration focuses on:
- Determining the ADC offset
- Measuring ADC-based RMS voltage
- Deriving a scaling factor to convert ADC values to real mains voltage

### Hardware Used
- ESP32 Dev Module
- ZMPT101B Voltage Sensor Module
- AC mains supply
- Reference multimeter

### Pin Connections

| Component | Pin Name | ESP32 Pin | Description |
|---------|----------|-----------|-------------|
| ZMPT101B | OUT     | GPIO 35   | Analog voltage output |
| ZMPT101B | VCC     | 5V        | Sensor power supply |
| ZMPT101B | GND     | GND       | Common ground |

### Calibration Method
1. Power the ESP32 and ZMPT101B module.
2. The code measures the average ADC value with no AC signal variation to find the offset.
3. RMS ADC values are calculated over a sampling window.
4. A known mains voltage (measured using a multimeter) is used as a reference.
5. A scaling factor is adjusted so that calculated voltage matches the reference value.

### Output Observations
- Offset value remains stable over time.
- RMS ADC value increases proportionally with AC voltage.
- After scaling, displayed voltage closely matches multimeter readings.

### Outcome
The final offset and voltage calibration factor are used directly in the main energy meter code to compute accurate RMS voltage.

---

## README 3: Energy Calculation Test Code

### Purpose
This sketch validates the energy calculation logic independently before full system integration. It focuses on:
- Power computation using calibrated voltage and current values
- Time-based energy accumulation
- Verifying kWh calculation logic

### Hardware Used
- ESP32 Dev Module
- Calibrated ACS712 current sensor
- Calibrated ZMPT101B voltage sensor
- Known AC load

### Pin Connections

| Component | Signal | ESP32 Pin | Description |
|---------|--------|-----------|-------------|
| ACS712 (Load 1) | OUT | GPIO 34 | Current measurement input |
| ZMPT101B | OUT | GPIO 35 | Voltage measurement input |
| Relay | IN | GPIO 5 | Load control |

### Test Method
1. Apply a known load and record voltage and current readings.
2. Calculate instantaneous power using P = V × I.
3. Integrate power over time to compute energy in watt-hours and kilowatt-hours.
4. Compare calculated energy with expected theoretical values.

### Output Observations
- Power values remain stable for a fixed load.
- Energy increases linearly with time.
- Computed energy values match expected calculations within acceptable tolerance.

### Outcome
This test confirms that the energy integration logic is correct and suitable for use in the final smart energy meter system.

---

## Notes for Final Integration
- All calibration constants obtained from these test sketches must be copied into the main project.
- Calibration should be repeated if hardware, power supply, or sensor modules are changed.
- These calibration steps ensure accuracy, stability, and professional-grade behavior of the final system.

