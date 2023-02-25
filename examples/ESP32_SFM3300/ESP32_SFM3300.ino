#include <Wire.h>

#define POLYNOMIAL 0x31     // P(x)=x^8+x^5+x^4+1 = 100110001
#define SFM3300 0x40        // SFM3300 I2C Address
#define SCL_PIN 22          // SFM3300 SCK Pin at GPIO 22
#define SDA_PIN 21          // SFM3300 DATA Pin at GPIO 21
#define I2C_SPEED 400000    // I2C Speed at 400kHz
#define delta_Time 20

bool crc_error;
double flowRate;
double tidalVol;

uint8_t CRC_prim (uint8_t x, uint8_t crc) {
  crc ^= x;
  for (uint8_t bit = 8; bit > 0; --bit) {
    if (crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
    else crc = (crc << 1);
  }
  return crc;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN, I2C_SPEED);
  //Wire.begin(SDA_PIN, SCL_PIN);

  Wire.beginTransmission(SFM3300);
  Wire.write(0x20);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(100);

  Wire.beginTransmission(SFM3300);
  Wire.write(0x10);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(100);
}

void loop() {
  if (3 == Wire.requestFrom(SFM3300, 3)) {
    uint8_t crc = 0;
    uint16_t a = Wire.read();
    crc = CRC_prim (a, crc);
    uint8_t  b = Wire.read();
    crc = CRC_prim (b, crc);
    uint8_t  c = Wire.read();
    if (crc_error = (crc != c))
      return;
    a = (a << 8) | b;

    flowRate = ((double)a - 32768.00) / 120.00;    // Flow rate in Standard Liter per minute
    tidalVol += ((flowRate / 60.0) * delta_Time);  // Tidal volume in Standard cubic centimeter or milli-liters

    // Print Data
    Serial.print(flowRate * 100);
    Serial.print(",");
    Serial.print(tidalVol);
    Serial.println();
  } else {
    Serial.println("Please check SFM3300 wiring!");
  }
  delay(delta_Time);
}
