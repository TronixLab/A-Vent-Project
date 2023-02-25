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
#include "SFM3300.h"

#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#define resetDPin            33
#define SFM3300_addr         0x40
#define ADS1115_addr         0x48
#define SFM3300_offSet       32768.00D
#define SFM3300_scaleFactor  120.00D
#define FREQUENCY_HZ         100
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

double airFlow = 0.00;
float tidalVolume = 0.00;
float PcmH2O = 0.00;
unsigned long previousMillis = 0;
int zeroCount = 0;
float alarm_value = 0;
int trueAlarm = 0;

void TaskBlink( void *pvParameters );
void TaskPrint( void *pvParameters );

#define SAMPLE_RATE 50
#define ATTACK_TIME 0.1F
#define RELEASE_TIME 5.0F
#define SMOOTH_COEFF 0.5F
#define THRESH_RATIO 1.5F;
#define THRESH_DIFF 3.0F;

float ATTACK_COEFF = 0.00F;
float RELEASE_COEFF = 0.00F;

float v_high = 12.00F;
float v_low = 0.00F;
int t_high = 0;
int t_low = 0;
float v_max = 12.00F;
float v_min = 0.00F;
int t_peak = 0;

bool inhalation = false;
bool lastBreath = false;

float pip = 12;
float peep = 0;
int respiration_rate = 0;
unsigned long int respiratoryPeriod = 0;
unsigned long int lastPeriod = 0;

int THRESH_NC = 10;  // s
int THRESH_LP = 10;  // cm H20
int THRESH_HP = 30;  // cm H20
int THRESH_LR = 15;  // breaths/min
int THRESH_HR = 40;  // breaths/min
int THRESH_LV = 120; // mL
int THRESH_HV = 230; // mL

void setup() {
  Serial.begin(115200);
  sensirionFlow.hardReset(resetDPin);
  sensirionFlow.begin();

  Wire1.begin(SDA_ADS, SCL_ADS, I2C_SPEED);

  ATTACK_COEFF = pow(M_E, -1 / (SAMPLE_RATE * ATTACK_TIME));
  RELEASE_COEFF = pow(M_E, -1 / (SAMPLE_RATE * RELEASE_TIME));

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
  PcmH2O = ((Vout - Voff) / Sensitivity / mmH2O_cmH2O);

  static unsigned long timer = millis();
  if (millis() - timer >= (1000 / SAMPLE_RATE)) {
    // Calculate the tidal volume in mL @ every 50ms:
    tidalVolume = tidalVolume + ((airFlow / 60) * (millis() - previousMillis));
    previousMillis = millis();

    if (tidalVolume < 0.00D) {
      tidalVolume = 0.00;
    }

    if (airFlow > 0) {
      inhalation = true;
    }
    else if (airFlow < 0) {
      inhalation = false;
    }

    if (inhalation != lastBreath) {
      if (inhalation == true) {
        tidalVolume = 0.00;
        respiratoryPeriod = millis() - lastPeriod;
        lastPeriod = millis();

        if (abs(floor(airFlow)) < 1.00D) {
          zeroCount ++;
          if (zeroCount > 2) {
            respiration_rate = 0.00;
          }
        }
        else {
          zeroCount = 0;
          respiration_rate = (60000 / respiratoryPeriod);
        }
      }
    }
    lastBreath = inhalation;

    timer = millis();
    float p = airFlow;

    t_peak++;
    // if current sample is PIP attack
    if (p > v_high) {
      v_high = ATTACK_COEFF * v_high + (1 - ATTACK_COEFF) * p;
      v_max = p;
      t_high = 0;

      if (!inhalation) {

        // smooth peep display
        peep = SMOOTH_COEFF * peep + (1 - SMOOTH_COEFF) * v_min;
      }
    }
    // if current sample is PIP release
    else {
      v_high = RELEASE_COEFF * v_high + (1 - RELEASE_COEFF) * p;
      t_high++;
    }
    // if current sample is PEEP attack
    if (p < v_low) {
      v_low = ATTACK_COEFF * v_low + (1 - ATTACK_COEFF) * p;
      v_min = p;
      t_low = 0;

      if (inhalation && (t_peak > 1)) {

        // smooth pip display
        pip = SMOOTH_COEFF * pip + (1 - SMOOTH_COEFF) * v_max;

        // calculate breathing rate
        t_peak = t_high;
      }
    }
    // if current sample is PEEP release
    else {
      v_low = RELEASE_COEFF * v_low + (1 - RELEASE_COEFF) * p;
      t_low++;
    }
  }
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
    if (respiration_rate == 0) {
      alarm_value = 30;
    }
    else {
      alarm_value = 0;
    }

    Serial.print(airFlow);
    Serial.print(",");
    Serial.print(respiration_rate);
    Serial.print(",");
    Serial.print(alarm_value);
    Serial.println();
    vTaskDelay(INTERVAL_MS / portTICK_PERIOD_MS);
  }
}
