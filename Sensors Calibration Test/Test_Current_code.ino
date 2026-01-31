// ==================================================
// FINAL ACS712 CURRENT MEASUREMENT (SMART FILTER)
// ESP32 + SWITCH CONTROLLED RELAY
// Supports SMALL + HEAVY + MIXED LOAD
// ==================================================

#define CURRENT_PIN 34
#define RELAY_PIN   5
#define SWITCH_PIN  18   // Switch to GND (INPUT_PULLUP)

#define SAMPLE_WINDOW_MS 100   // ~5 AC cycles @ 50Hz

// -------- FILTER LIMITS (LOCKED) --------
#define NOISE_CUT          0.02    // Below this = noise only
#define SMALL_LOAD_LIMIT   0.10    // Bulb / LED range

float ACS_SENSITIVITY = 0.34;      // 🔒 Calibrated
float ACS_OFFSET = 0;

// --------------------------------------------------
void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);

  digitalWrite(RELAY_PIN, HIGH);   // Relay OFF (active LOW)

  analogReadResolution(12);
  analogSetPinAttenuation(CURRENT_PIN, ADC_11db);

  delay(2000);

  // -------- OFFSET CALIBRATION --------
  // Relay ON + NO LOAD (IMPORTANT)
  digitalWrite(RELAY_PIN, LOW);
  delay(1000);

  long sum = 0;
  for (int i = 0; i < 3000; i++) {
    sum += analogRead(CURRENT_PIN);
    delayMicroseconds(200);
  }
  ACS_OFFSET = sum / 3000.0;

  Serial.println("Offset calibrated (Relay ON, no load)");
  Serial.println("--------------------------------");
}

// --------------------------------------------------
void loop() {

  bool switchOn = (digitalRead(SWITCH_PIN) == LOW);

  // -------- RELAY OFF --------
  if (!switchOn) {
    digitalWrite(RELAY_PIN, HIGH);
    Serial.println("Current = 0.000 A (RELAY OFF)");
    delay(1000);
    return;
  }

  // -------- RELAY ON --------
  digitalWrite(RELAY_PIN, LOW);

  unsigned long start = millis();
  float sumSq = 0;
  int count = 0;

  while (millis() - start < SAMPLE_WINDOW_MS) {
    float adc = analogRead(CURRENT_PIN);
    float v = (adc - ACS_OFFSET) * (3.3 / 4095.0);
    sumSq += v * v;
    count++;
  }

  float vrms = sqrt(sumSq / count);
  float Irms = vrms / ACS_SENSITIVITY;

  // -------- SMART FILTER --------
  if (Irms < NOISE_CUT) {
    Irms = 0;                   // Pure noise / open-line
  }
  // else if between NOISE_CUT and SMALL_LOAD_LIMIT
  // → small real load (bulb, LED) → allowed
  // else heavy load → allowed

  Serial.print("Current = ");
  Serial.print(Irms, 3);
  Serial.println(" A");

  delay(1000);
}
