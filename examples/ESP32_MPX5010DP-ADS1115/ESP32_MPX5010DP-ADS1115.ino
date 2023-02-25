#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <MovingAverageFloat.h>

#define FREQUENCY_HZ        50
#define INTERVAL_MS         (1000 / (FREQUENCY_HZ + 1))
#define ADS1115 0x48        // ADS1115 I2C Address
#define SDA_ADS 4           // SFM3300 DATA Pin at GPIO 21
#define SCL_ADS 5           // SFM3300 SCK Pin at GPIO 22
#define I2C_SPEED 100000    // I2C Speed at 400kHz (high speed), or 100kHz (standard speed)

Adafruit_ADS1115 ads;            // ADC object at I2C address 0x48 for addr pin = GND
#define voltageFactor 0.03125F    // 2/3x gain +/- 6.144V = 0.1875mV/step
#define Voff          200.00F    // offset voltage in mV can be use for calibration *see datasheet
#define Sensitivity   4.413F     // sensitivity in mV/mmH2O *see datasheet
#define mmH2O_cmH2O   10.00F     // divide by this figure to convert mmH2O to cmH2O

TwoWire ADS_Wire = TwoWire(0);

MovingAverageFloat <10> filter;

void setup() {
  Serial.begin(115200);
  ADS_Wire.begin(SDA_ADS, SCL_ADS, I2C_SPEED);

  if (!ads.begin(ADS1115, &ADS_Wire)) {
    Serial.println("Could not find a valid ADS1115 sensor, check wiring!");
    while (true);
  }
  else {
    //                                                           ADS1015            ADS1115
    //                                                        --------------   ------------------
    // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
    // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
    // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
    // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
    // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
    // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
    ads.setGain(GAIN_FOUR);
  }
}

void loop() {
  // Voltage reading of ADS1115 results from MPX5010DP sensor:
  int16_t adc0 = ads.readADC_SingleEnded(0);
  float Vout = float(adc0 * voltageFactor);

  // Calculate pressue value in cmH2O:
  float PcmH2O = ((Vout - Voff) / Sensitivity / mmH2O_cmH2O);
  filter.add(PcmH2O);
  Serial.println(filter.get());
  delay(INTERVAL_MS);
}
