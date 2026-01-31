// ==================================================
// SMART ENERGY METER (TOTAL + SESSION + MONTHLY)
// ESP32 + 2x ACS712-30A + ZMPT101B + 2x Relay
// ==================================================

// ---------- PINS ----------
#define VOLTAGE_PIN   35
#define CURRENT1_PIN  34
#define CURRENT2_PIN  33

#define RELAY1_PIN    5
#define RELAY2_PIN    17

#define SWITCH1_PIN   18
#define SWITCH2_PIN   19

// ---------- CONSTANTS ----------
#define SAMPLE_WINDOW_MS 300
#define COST_PER_KWH     10.0   // BDT

// ---------- ACS712 (Empirically calibrated) ----------
float ACS_SCALE_FACTOR = 5.5;   // Calibrated for ESP32 + ACS712-30A
#define CURRENT_DEADZONE 0.02

float ACS1_OFFSET = 0;
float ACS2_OFFSET = 0;

// ---------- ZMPT ----------
float ZMPT_OFFSET = 2738.90;
float VOLTAGE_CAL = 800.0;

// ---------- ENERGY ----------
float totalEnergy_kWh   = 0.0;   // 🔴 Meter reading (never reset)
float sessionEnergy_kWh = 0.0;   // 🟢 Current session
float monthStartEnergy  = 0.0;   // 🔵 Snapshot at month start

unsigned long lastMillis = 0;

// ==================================================
void calibrateACS(int pin, float &offset) {
  long sum = 0;
  for (int i = 0; i < 3000; i++) {
    sum += analogRead(pin);
    delayMicroseconds(200);
  }
  offset = sum / 3000.0;
}

// ==================================================
float readCurrentRMS(int pin, float offset) {
  unsigned long start = millis();
  float sumSq = 0;
  int count = 0;

  while (millis() - start < SAMPLE_WINDOW_MS) {
    float adc = analogRead(pin);
    float v = (adc - offset) * (3.3 / 4095.0);
    sumSq += v * v;
    count++;
  }

  float Irms = sqrt(sumSq / count) / ACS_SCALE_FACTOR;
  if (Irms < CURRENT_DEADZONE) Irms = 0;
  return Irms;
}

// ==================================================
float readVoltageRMS() {
  unsigned long start = millis();
  float sumSq = 0;
  int count = 0;

  while (millis() - start < SAMPLE_WINDOW_MS) {
    float adc = analogRead(VOLTAGE_PIN);
    float v = adc - ZMPT_OFFSET;
    sumSq += v * v;
    count++;
  }

  float vrms_adc = sqrt(sumSq / count);
  float Vrms = (vrms_adc * (3.3 / 4095.0)) * VOLTAGE_CAL;

  if (Vrms < 180 || Vrms > 260) Vrms = 230.0;
  return Vrms;
}

// ==================================================
void setup() {
  Serial.begin(115200);

  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(SWITCH1_PIN, INPUT_PULLUP);
  pinMode(SWITCH2_PIN, INPUT_PULLUP);

  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);

  analogReadResolution(12);
  analogSetPinAttenuation(VOLTAGE_PIN, ADC_11db);
  analogSetPinAttenuation(CURRENT1_PIN, ADC_11db);
  analogSetPinAttenuation(CURRENT2_PIN, ADC_11db);

  delay(2000);

  Serial.println("Calibrating ACS712 (NO LOAD)...");
  calibrateACS(CURRENT1_PIN, ACS1_OFFSET);
  calibrateACS(CURRENT2_PIN, ACS2_OFFSET);

  lastMillis = millis();

  Serial.println("===== SYSTEM READY =====");
}

// ==================================================
void loop() {

  bool sw1 = (digitalRead(SWITCH1_PIN) == LOW);
  bool sw2 = (digitalRead(SWITCH2_PIN) == LOW);

  digitalWrite(RELAY1_PIN, sw1 ? LOW : HIGH);
  digitalWrite(RELAY2_PIN, sw2 ? LOW : HIGH);

  float Vrms = readVoltageRMS();

  float I1 = sw1 ? readCurrentRMS(CURRENT1_PIN, ACS1_OFFSET) : 0;
  float I2 = sw2 ? readCurrentRMS(CURRENT2_PIN, ACS2_OFFSET) : 0;

  float P1 = Vrms * I1;
  float P2 = Vrms * I2;
  float totalPower = P1 + P2;

  unsigned long now = millis();
  float hours = (now - lastMillis) / 3600000.0;
  lastMillis = now;

  float deltaEnergy = (totalPower * hours) / 1000.0;

  // -------- ENERGY UPDATE --------
  totalEnergy_kWh   += deltaEnergy;
  sessionEnergy_kWh += deltaEnergy;

  float monthlyEnergy = totalEnergy_kWh - monthStartEnergy;
  float monthlyCost   = monthlyEnergy * COST_PER_KWH;

  // -------- OUTPUT --------
  Serial.println("--------------------------------");
  Serial.print("Voltage : "); Serial.print(Vrms, 1); Serial.println(" V");

  Serial.print("Load-1  : "); Serial.print(P1, 1); Serial.println(" W");
  Serial.print("Load-2  : "); Serial.print(P2, 1); Serial.println(" W");

  Serial.print("TOTAL POWER     : "); Serial.print(totalPower, 1); Serial.println(" W");
  Serial.print("TOTAL ENERGY    : "); Serial.print(totalEnergy_kWh, 4); Serial.println(" kWh");
  Serial.print("SESSION ENERGY  : "); Serial.print(sessionEnergy_kWh, 4); Serial.println(" kWh");
  Serial.print("MONTHLY ENERGY  : "); Serial.print(monthlyEnergy, 4); Serial.println(" kWh");
  Serial.print("MONTHLY BILL    : "); Serial.print(monthlyCost, 2); Serial.println(" BDT");

  delay(3000);
}
