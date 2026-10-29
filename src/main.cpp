//Author: Saint Ezru
// Date: 2026-10-29
// Description: This program reads temperature, humidity, and pressure data from AHT20 and BMP280 sensors connected to an ESP32 microcontroller via I2C. It outputs the readings in CSV format over the serial connection every 2 seconds. The program also calculates the estimated altitude based on the pressure reading from the BMP280 sensor.
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <esp_timer.h>
#include <math.h>

// Wiring: breakout SDA -> GPIO 16, SCL -> GPIO 4, VCC -> 3V3, GND -> GND.
constexpr int SDA_PIN = 16;
constexpr int SCL_PIN = 4;
constexpr uint32_t I2C_FREQUENCY_HZ = 100000;
constexpr uint32_t READ_INTERVAL_MS = 2000;

// Standard sea-level pressure. Replace with your local sea-level pressure
// (QNH, in hPa) for a better altitude estimate. Altitude is calculated,
// not directly measured, and changes with the weather.
constexpr float SEA_LEVEL_PRESSURE_HPA = 1013.25f;

Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp(&Wire);

bool ahtReady = false;
bool bmpReady = false;

// A blank field represents an unavailable/invalid value, never a fake zero.
void printCsvValue(float value) {
  Serial.print(',');
  if (isfinite(value)) {
    Serial.print(value, 2);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Keep the application serial stream strictly CSV. This header is sent
  // once per boot, even if the I2C bus or sensors cannot be initialized.
  Serial.println("uptime_ms,aht20_temperature_c,aht20_humidity_pct,"
                 "bmp280_temperature_c,bmp280_pressure_pa,estimated_altitude_m");

  // ESP32 Wire.begin() takes SDA first, then SCL.
  if (!Wire.begin(SDA_PIN, SCL_PIN, I2C_FREQUENCY_HZ)) {
    return;
  }
  Wire.setTimeOut(50);

  ahtReady = aht.begin(&Wire);

  bmpReady = bmp.begin(0x76);
  if (!bmpReady) {
    bmpReady = bmp.begin(0x77);
  }

  if (bmpReady) {
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                    Adafruit_BMP280::SAMPLING_X2,   // Temperature oversampling
                    Adafruit_BMP280::SAMPLING_X16,  // Pressure oversampling
                    Adafruit_BMP280::FILTER_X16,
                    Adafruit_BMP280::STANDBY_MS_500);
    delay(100);  // Allow the first BMP280 conversion to finish.
  }
}

void loop() {
  // 64-bit milliseconds since boot, recorded when this sample starts.
  // The Python receiver should add a UTC timestamp for CSV/database storage.
  const uint64_t uptimeMs = static_cast<uint64_t>(esp_timer_get_time()) / 1000;
  float ahtTemperatureC = NAN;
  float ahtHumidityPct = NAN;
  float bmpTemperatureC = NAN;
  float bmpPressurePa = NAN;
  float estimatedAltitudeM = NAN;

  if (ahtReady) {
    sensors_event_t humidity = {};
    sensors_event_t temperature = {};

    // Argument order matters: humidity first, then temperature.
    if (aht.getEvent(&humidity, &temperature)) {
      ahtTemperatureC = temperature.temperature;
      ahtHumidityPct = humidity.relative_humidity;
    }
  }

  if (bmpReady) {
    bmpTemperatureC = bmp.readTemperature();
    const float pressurePa = bmp.readPressure();

    if (isfinite(pressurePa) && pressurePa > 0.0f) {
      bmpPressurePa = pressurePa;
      // Same barometric formula used by Adafruit's readAltitude(), using
      // the pressure sample included in this CSV row.
      estimatedAltitudeM = 44330.0f *
          (1.0f - powf((pressurePa / 100.0f) / SEA_LEVEL_PRESSURE_HPA, 0.1903f));
    }
  }

  // Always write exactly six fields, in the same order as the header.
  Serial.printf("%llu", static_cast<unsigned long long>(uptimeMs));
  printCsvValue(ahtTemperatureC);
  printCsvValue(ahtHumidityPct);
  printCsvValue(bmpTemperatureC);
  printCsvValue(bmpPressurePa);
  printCsvValue(estimatedAltitudeM);
  Serial.println();

  delay(READ_INTERVAL_MS);
}
