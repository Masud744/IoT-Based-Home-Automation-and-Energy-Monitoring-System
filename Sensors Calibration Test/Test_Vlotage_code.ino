#define VOLTAGE_PIN 35

#define SAMPLE_WINDOW_MS 100   // ~5 AC cycles @ 50Hz

float ZMPT_OFFSET = 2738.90;   // 🔒 your measured offset
float VOLTAGE_CAL = 800.0;   // approx, we will fine tune

void setup() {
  Serial.begin(115200);
  delay(3000);

  analogReadResolution(12);
  analogSetPinAttenuation(VOLTAGE_PIN, ADC_11db);

  Serial.println("ZMPT AC RMS Voltage Test Started");
}

void loop() {

  unsigned long start = millis();
  float sumSq = 0;
  int count = 0;

  while (millis() - start < SAMPLE_WINDOW_MS) {
    float adc = analogRead(VOLTAGE_PIN);
    float v = (adc - ZMPT_OFFSET);   // centered waveform
    sumSq += v * v;
    count++;
  }

  float vrms_adc = sqrt(sumSq / count);

  // Convert ADC RMS → AC Voltage
  float voltage = vrms_adc * (3.3 / 4095.0) * VOLTAGE_CAL;

  Serial.print("Voltage = ");
  Serial.print(voltage, 1);
  Serial.println(" V");

  delay(1000);
}
