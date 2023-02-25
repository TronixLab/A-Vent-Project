#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#include <A-Vent_PVA_Detection_inferencing.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <MovingAverageFloat.h>
#include "SFM3300.h"

#define FREQUENCY_HZ        50
#define INTERVAL_MS         (1000 / (FREQUENCY_HZ + 1))
#define resetDPin            33
#define SFM3300_addr         0x40
#define ADS1115_addr         0x48
#define SFM3300_offSet       32768.00D
#define SFM3300_scaleFactor  120.00D
#define SDA_ADS              4
#define SCL_ADS              5
#define I2C_SPEED            100000
#define voltageFactor        0.0625F
#define Voff                 200.00F
#define Sensitivity          4.413F
#define mmH2O_cmH2O          10.00F

#define PLOT 0
#if PLOT == 1
#define plot(x) Serial.print(x)
#define plotln(x) Serial.println(x)
#else
#define plot(x)
#define plotln(x)
#endif

#define EI_PRINTF 1
#if EI_PRINTF == 1
#define ei_printf(x,...) printf(x,##__VA_ARGS__)
#else
#define ei_printf(x,...)
#endif

SFM3300 sensirionFlow(SFM3300_addr);

Adafruit_ADS1115 ads;

MovingAverageFloat <10> flowFilter;
MovingAverageFloat <10> pressureFilter;

double airFlow = 0.00;
float tidalVolume = 0.00;
float pressure = 0.00;
static volatile float filteredFlow = 0.00;
static volatile float scaledVol = 0.00;
static volatile float filteredPres = 0.00;
static unsigned long previousMillis = 0;
int zeroCount = 0;

void TaskBlink( void *pvParameters );
void TaskPlot( void *pvParameters );

// To classify 1 frame of data you need EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE values
static float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
// Keep track of where we are in the feature array
size_t feature_ix = 0;
static unsigned long last_interval_ms = 0;
static int falseAlarm = 0;
int alarmSignal = 0;
float anomalyScore = 0;

String predictedLabel = "";

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

  // Create tasks
  xTaskCreatePinnedToCore(
    TaskBlink
    ,  "TaskBlink"
    ,  1024
    ,  NULL
    ,  1
    ,  NULL
    ,  ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskPlot
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

  // This is a structure that smoothens the output result
  // With the default settings 70% of readings should be the same before classifying.
  ei_classifier_smooth_t smooth;
  ei_classifier_smooth_init(&smooth, 70 /* no. of readings */, 20 /* min. readings the same */, 0.8 /* min. confidence */, 0.3 /* max anomaly */);

  // Scaled and filtered the waveform for plotting
  filteredFlow = flowFilter.get() - 0.00;
  scaledVol = tidalVolume / 25;
  filteredPres = pressureFilter.get() - 1.02F;

  //INTERVAL_MS    EI_CLASSIFIER_INTERVAL_MS
  if (millis() > last_interval_ms + INTERVAL_MS) {
    last_interval_ms = millis();

    // Fill the features buffer
    features[feature_ix++] = filteredFlow;
    features[feature_ix++] = scaledVol;
    features[feature_ix++] = filteredPres;
  }

  // Features buffer full? Then classify!
  if (feature_ix == EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
    // Create signal from features frame
    signal_t signal;
    int err = numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    if (err != 0) {
      ei_printf("Failed to create signal from buffer (%d)\n", err);
      return;
    }

    // Run classifier
    ei_impulse_result_t result = { 0 };

    err = run_classifier(&signal, &result, false);
    if (err != EI_IMPULSE_OK) {
      ei_printf("ERR: Failed to run classifier (%d)\n", err);
      return;
    }

    // Print predictions
    ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
              result.timing.dsp, result.timing.classification, result.timing.anomaly);

    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
      ei_printf("%s:\t%.5f\n", result.classification[ix].label, result.classification[ix].value);
    }

#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ei_printf("Anomaly:\t %.3f\n", result.anomaly);
    anomalyScore = result.anomaly;
#endif

    // Reset features frame
    feature_ix = 0;

    // Alarm Algorithm
    if (result.classification[6].value >= 0.8 && result.anomaly < 0.3) {
      // Reset the alarm variables
      falseAlarm = 0;
      alarmSignal = 0;

      // Assigned normal prediction
      predictedLabel = "Normal Waveform     ";
    }
    else {
      falseAlarm ++;
      if (falseAlarm > 5) {
        // Trigger Alarm Signal
        alarmSignal = 12;

        // Identify Alarms
        if (result.classification[0].value >= 0.8) {
          //DCA
          predictedLabel = "Delay Cycling       ";
        }
        else if (result.classification[1].value >= 0.8) {
          //DPPV
          predictedLabel = "Detach Pressure Port";
        }
        else if (result.classification[2].value >= 0.8) {
          //DTV
          predictedLabel = "Disconnected Tube  ";
        }
        else if (result.classification[3].value >= 0.8) {
          //DTA
          predictedLabel = "Double Triggering  ";
        }
        else if (result.classification[4].value >= 0.8) {
          //IEA
          predictedLabel = "Ineffective Effort ";
        }
        else if (result.classification[5].value >= 0.8) {
          //MFV
          predictedLabel = "Machine Failure    ";
        }
        else if (result.classification[7].value >= 0.8) {
          //RTA
          predictedLabel = "Reverse Triggering ";
        }
      }
    }
  }
  // Free some memory
  ei_classifier_smooth_free(&smooth);
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

void TaskPlot(void *pvParameters)
{
  (void) pvParameters;
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = EI_CLASSIFIER_FREQUENCY;
  xLastWakeTime = xTaskGetTickCount ();
  for (;;)
  {
    // Plot the normilized waveform; use serial monitor
    plot(filteredFlow);
    plot(",");
    plot(scaledVol);
    plot(",");
    plot(filteredPres);
    plot(",");
    plot(alarmSignal);
    plot(",");
    plot(anomalyScore);
    plotln();
    //vTaskDelayUntil( &xLastWakeTime, xFrequency );
    vTaskDelay(EI_CLASSIFIER_INTERVAL_MS / portTICK_PERIOD_MS);
  }
}
