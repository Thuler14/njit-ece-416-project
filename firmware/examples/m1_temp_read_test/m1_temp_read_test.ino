// ====================================================
// M1: Temperature Read Test (DS18B20)
// Purpose: Verify wiring, addresses, and sensor readings in °F
// Board: ESP32 (Control Module)
// Bus: GPIO 4 with 4.7kΩ pull-up to 3V3
// ====================================================

#include <DallasTemperature.h>
#include <OneWire.h>

#include "../../control/config.h"  // Centralized constants

OneWire oneWire(PIN_TEMP_ONEWIRE);
DallasTemperature sensors(&oneWire);

DeviceAddress T_HOT = ADDR_T_HOT;
DeviceAddress T_COLD = ADDR_T_COLD;
DeviceAddress T_OUT = ADDR_T_OUT;

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
  sensors.setResolution(T_HOT, 12);
  sensors.setResolution(T_COLD, 12);
  sensors.setResolution(T_OUT, 12);

  Serial.println("Detected sensor addresses:");
  Serial.print("HOT  = 0x");
  printAddr(T_HOT);
  Serial.println();
  Serial.print("COLD = 0x");
  printAddr(T_COLD);
  Serial.println();
  Serial.print("OUT  = 0x");
  printAddr(T_OUT);
  Serial.println();
  Serial.println();
}

void loop() {
  sensors.requestTemperatures();

  float Th = readF(T_HOT);
  float Tc = readF(T_COLD);
  float To = readF(T_OUT);

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
