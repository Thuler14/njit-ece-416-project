// ====================================================
// M1: Temperature Read Test (DS18B20)
// Purpose: Verify wiring, addresses, and sensor readings in °F
// Board: ESP32 (Control Module)
// Bus: GPIO 4 with 4.7kΩ pull-up to 3V3
// ====================================================

#include <DallasTemperature.h>
#include <OneWire.h>

#include "../../control/config.h"  // Centralized constants

OneWire oneWire(TEMP_PIN_ONEWIRE);
DallasTemperature sensors(&oneWire);

float readF(const DeviceAddress addr) {
  float c = sensors.getTempC(addr);
  if (c == DEVICE_DISCONNECTED_C) return NAN;
  return DallasTemperature::toFahrenheit(c);
}

void printAddr(const DeviceAddress addr) {
  for (uint8_t i = 0; i < 8; i++) {
    if (addr[i] < 16) Serial.print("0");
    Serial.print(addr[i], HEX);
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[ShowerCtrl - DS18B20 Reader]");

  sensors.begin();
  sensors.setResolution(TEMP_HOT_ADDR, 12);
  sensors.setResolution(TEMP_COLD_ADDR, 12);
  sensors.setResolution(TEMP_OUT_ADDR, 12);

  Serial.println("Detected sensor addresses:");
  Serial.print("HOT  = 0x");
  printAddr(TEMP_HOT_ADDR);
  Serial.println();
  Serial.print("COLD = 0x");
  printAddr(TEMP_COLD_ADDR);
  Serial.println();
  Serial.print("OUT  = 0x");
  printAddr(TEMP_OUT_ADDR);
  Serial.println();
  Serial.println();
}

void loop() {
  sensors.requestTemperatures();

  float Th = readF(TEMP_HOT_ADDR);
  float Tc = readF(TEMP_COLD_ADDR);
  float To = readF(TEMP_OUT_ADDR);

  if (!isnan(Th) && !isnan(Tc) && !isnan(To)) {
    Serial.printf("T_hot: %.1f °F,  T_cold: %.1f °F,  T_out: %.1f °F\n", Th, Tc, To);
  } else {
    Serial.printf("Status → HOT:%s  COLD:%s  OUT:%s\n",
                  isnan(Th) ? "ERR" : "OK",
                  isnan(Tc) ? "ERR" : "OK",
                  isnan(To) ? "ERR" : "OK");
  }

  delay(1000);
}
