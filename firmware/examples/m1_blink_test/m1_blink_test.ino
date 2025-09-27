// Blink Test for ESP32
// Blinks onboard LED (GPIO 2) at 1 Hz and prints "Hello World!" to Serial

#define LED_PIN 2  // Onboard LED pin

void setup() {
  Serial.begin(115200);      // Start Serial at 115200 baud
  pinMode(LED_PIN, OUTPUT);  // Configure LED pin as output
}

void loop() {
  Serial.println("Hello World!");  // Print to Serial
  digitalWrite(LED_PIN, HIGH);     // LED ON
  delay(500);                      // Wait 500 ms
  digitalWrite(LED_PIN, LOW);      // LED OFF
  delay(500);                      // Wait 500 ms
}
