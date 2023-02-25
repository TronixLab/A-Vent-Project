#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <MovingAverageFloat.h>
#include "SFM3300.h"

#define resetDPin            33
#define SFM3300_addr         0x40
#define ADS1115_addr         0x48
#define SFM3300_offSet       32768.00D
#define SFM3300_scaleFactor  120.00D
#define FREQUENCY_HZ         50
#define INTERVAL_MS         (1000 / (FREQUENCY_HZ + 1))
#define SDA_ADS              4
#define SCL_ADS              5
#define I2C_SPEED            100000
#define voltageFactor        0.0625F
#define Voff                 200.00F
#define Sensitivity          4.413F
#define mmH2O_cmH2O          10.00F

SFM3300 sensirionFlow(SFM3300_addr);

Adafruit_ADS1115 ads;

MovingAverageFloat <10> flowFilter;
MovingAverageFloat <10> pressureFilter;

double airFlow = 0.00;
float tidalVolume = 0.00;
float pressure = 0.00;
unsigned long previousMillis = 0;
int zeroCount = 0;

void TaskBlink( void *pvParameters );
void TaskPrint( void *pvParameters );

void setup() {
  Serial.begin(115200);
  sensirionFlow.hardReset(resetDPin);
  sensirionFlow.begin();

  Wire1.begin(SDA_ADS, SCL_ADS, I2C_SPEED);

  if (!ads.begin(ADS1115_addr, &Wire1)) {
    Serial.println("Could not find a valid ADS1115 sensor, check wiring!");
    while (true);
  }
  else {
    //                                                                ADS1115
    //                                                        --------------------------
    // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 0.1875mV (default)
    // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 0.125mV
    // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 0.0625mV
    // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.03125mV
    // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.015625mV
    // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.0078125mV
    ads.setGain(GAIN_TWO);
  }

  xTaskCreatePinnedToCore(
    TaskBlink
    ,  "TaskBlink"
    ,  1024
    ,  NULL
    ,  1
    ,  NULL
    ,  ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskPrint
    ,  "Print Data"
    ,  1024
    ,  NULL
    ,  2
    ,  NULL
    ,  ARDUINO_RUNNING_CORE);
}

void loop() {
  unsigned int rawData = sensirionFlow.getValue();
  // Calculate air flow rate in Standard Litter per minute (SLM)
  airFlow = ((double(rawData) - SFM3300_offSet) / SFM3300_scaleFactor);

  //Check if the sensor get stuck
  if (floor(airFlow) == -273 || airFlow == 57.65D) {
    // Software reset the microcontroller unit
    ESP.restart();
  }

  // Voltage reading of ADS1115 results from MPX5010DP sensor:
  int16_t adc0 = ads.readADC_SingleEnded(0);
  float Vout = float(adc0 * voltageFactor);

  // Calculate pressue value in cmH2O:
  float PcmH2O = ((Vout - Voff) / Sensitivity / mmH2O_cmH2O);

  // Filter signal using moving average algorithm
  flowFilter.add(airFlow);
  pressureFilter.add(PcmH2O);

  // Check the flow triggers (inspiration and expiration):
  if (abs(floor(airFlow)) < 0.50D) {
    zeroCount ++;
    if (zeroCount > 2) {
      tidalVolume = 0.00;
    }
  }
  else {
    // Calculate the tidal volume in mL @ every 50ms:
    tidalVolume = tidalVolume + ((airFlow / 60) * (millis() - previousMillis));
    previousMillis = millis();
    zeroCount = 0;

    if (tidalVolume < 0.00D) {
      tidalVolume = 0.00;
    }
  }
  delay(INTERVAL_MS);
}

void TaskBlink(void *pvParameters)
{
  (void) pvParameters;
  pinMode(LED_BUILTIN, OUTPUT);
  bool state = false;
  for (;;)
  {
    state = !(state);
    digitalWrite(LED_BUILTIN, state);
    vTaskDelay(250 / portTICK_PERIOD_MS);
  }
}

void TaskPrint(void *pvParameters)
{
  (void) pvParameters;
  for (;;)
  {
    // Scale the waveform for plotting
    float normFlow = flowFilter.get() - 0.00F;
    float normVol = tidalVolume / 25.0F;
    //float normVol = tidalVolume;
    float normPres = pressureFilter.get() - 1.02F;

    // Plot the normilized waveform; use serial monitor
    Serial.print(normFlow);
    Serial.print(",");
    Serial.print(normVol);
    Serial.print(",");
    Serial.print(normPres);
    Serial.println();
    vTaskDelay(INTERVAL_MS / portTICK_PERIOD_MS);
  }
}
