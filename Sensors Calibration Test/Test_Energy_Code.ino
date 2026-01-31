// ==================================================
// FINAL SMART ENERGY METER
// ESP32 + ACS712 + ZMPT101B + Relay + Switch
// ==================================================

#define CURRENT_PIN   34
#define VOLTAGE_PIN   35
#define RELAY_PIN     5
#define SWITCH_PIN    18   // Switch → GND (INPUT_PULLUP)

#define SAMPLE_WINDOW_MS 100   // ~5 AC cycles @ 50Hz

// -------- CALIBRATED VALUES (LOCKED) --------
float ACS_SENSITIVITY = 0.34;     // ACS712 (your calibrated value)
float ACS_OFFSET      = 0;        // Auto-calibrated

float ZMPT_OFFSET     = 2738.90;  // Your measured offset
float VOLTAGE_CAL     = 800.0;    // Your calibrated voltage factor

// -------- FILTERS --------
#define CURRENT_NOISE_CUT  0.03    // Below = treat as zero

// -------- COST --------
#define COST_PER_KWH 10.0          // BDT (change if needed)

// -------- ENERGY --------
float energy_kWh = 0.0;
unsigned long lastEnergyMillis = 0;

// ==================================================
void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);

  digitalWrite(RELAY_PIN, HIGH); // Relay OFF (active LOW)

  analogReadResolution(12);
  analogSetPinAttenuation(CURRENT_PIN, ADC_11db);
  analogSetPinAttenuation(VOLTAGE_PIN, ADC_11db);

  delay(2000);

  // -------- CURRENT OFFSET CALIBRATION --------
  // Relay ON, NO LOAD
  digitalWrite(RELAY_PIN, LOW);
  delay(1000);

  long sum = 0;
  for (int i = 0; i < 3000; i++) {
    sum += analogRead(CURRENT_PIN);
    delayMicroseconds(200);
  }
  ACS_OFFSET = sum / 3000.0;

  digitalWrite(RELAY_PIN, HIGH); // Relay OFF

  lastEnergyMillis = millis();

  Serial.println("===== SYSTEM READY =====");
  Serial.print("ACS OFFSET = ");
  Serial.println(ACS_OFFSET);
  Serial.println("========================");
}

// ==================================================
void loop() {

  bool switchOn = (digitalRead(SWITCH_PIN) == LOW);

  // -------- RELAY OFF --------
  if (!switchOn) {
    digitalWrite(RELAY_PIN, HIGH);
    Serial.println("Relay OFF | V=0  I=0  P=0W  Energy=0 kWh");
    delay(1000);
    return;
  }

  // -------- RELAY ON --------
  digitalWrite(RELAY_PIN, LOW);

  // ===== CURRENT RMS =====
  unsigned long start = millis();
  float sumSqI = 0;
  int countI = 0;

  while (millis() - start < SAMPLE_WINDOW_MS) {
    float adc = analogRead(CURRENT_PIN);
    float v = (adc - ACS_OFFSET) * (3.3 / 4095.0);
    sumSqI += v * v;
    countI++;
  }

  float Irms = sqrt(sumSqI / countI) / ACS_SENSITIVITY;
  if (Irms < CURRENT_NOISE_CUT) Irms = 0;

  // ===== VOLTAGE RMS =====
  start = millis();
  float sumSqV = 0;
  int countV = 0;

  while (millis() - start < SAMPLE_WINDOW_MS) {
    float adc = analogRead(VOLTAGE_PIN);
    float v = adc - ZMPT_OFFSET;
    sumSqV += v * v;
    countV++;
  }

  float vrms_adc = sqrt(sumSqV / countV);
  float Vrms = (vrms_adc * (3.3 / 4095.0)) * VOLTAGE_CAL;

  // ===== POWER =====
  float powerW = Vrms * Irms;

  // ===== ENERGY =====
  unsigned long now = millis();
  float hours = (now - lastEnergyMillis) / 3600000.0;
  energy_kWh += (powerW * hours) / 1000.0;
  lastEnergyMillis = now;

  // ===== COST =====
  float cost = energy_kWh * COST_PER_KWH;

  // ===== PRINT =====
  Serial.println("--------------------------------");
  Serial.print("Voltage : "); Serial.print(Vrms, 1); Serial.println(" V");
  Serial.print("Current : "); Serial.print(Irms, 3); Serial.println(" A");
  Serial.print("Power   : "); Serial.print(powerW, 2); Serial.println(" W");
  Serial.print("Energy  : "); Serial.print(energy_kWh, 4); Serial.println(" kWh");
  Serial.print("Cost    : "); Serial.print(cost, 2); Serial.println(" BDT");

  delay(1000);
}
