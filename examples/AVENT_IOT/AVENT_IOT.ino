#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_ADS1X15.h>
#include <MovingAverageFloat.h>
#include <A-Vent_PVA_Detection_inferencing.h>

#include "SFM3300.h"
#include "Settings.h"
#include "Parameters.h"
#include "RecursiveFilter.h"

SFM3300 sensirionFlow(SFM3300_addr);

Adafruit_ADS1115 ads;

MovingAverageFloat <10> flowFilter;
MovingAverageFloat <10> pressureFilter;

StaticJsonDocument<capacity> alarmdoc;

WiFiClient ESP32_client;
PubSubClient client(ESP32_client);

void TaskBlink( void *pvParameters );
void TaskPlot( void *pvParameters );
void TaskThresholdAlarm( void *pvParameters );
void TaskPublishWaveform( void *pvParameters );
void TaskPublishThreshold( void *pvParameters );
void TaskPublishOximeter( void *pvParameters );
void TaskPublishPeakCurve( void *pvParameters );
void TaskPublishAlarm( void *pvParameters );

// Custom function to reconnect on MQTT broker
boolean reconnect() {
  if (client.connect(THINGNAME)) {
    Serial.println("ESP32 MQTT client connected!");
  }
  return client.connected();
}

void setup() {
  Serial.begin(115200);

  // Set ESP32 WiFi to station mode
  WiFi.mode(WIFI_STA);

  // Initiate the Wi-Fi connection
  WiFi.begin(SSID, PASSWORD);

  Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    // Check timeout
    static unsigned long timer = millis();
    if (millis() - timer > 10000) {
      // Reset ESP32
      ESP.restart();
    }
    Serial.print(".");
    delay(100);
  }

  Serial.print("\nConnected to the WiFi network: ");
  Serial.println(SSID);

  // Connect to the Mosquito MQTT broker to local server
  client.setServer(mqtt_server, mqtt_port);
  client.connect(THINGNAME);

  // Check MQTT broker connection status
  if (!client.loop()) {
    Serial.println("MQTT broker Timeout!");
  }
  else {
    Serial.println("ESP32 MQTT client connected!");
  }
  delay(1000);

  sensirionFlow.hardReset(resetDPin);
  sensirionFlow.begin();

  Wire1.begin(SDA_ADS, SCL_ADS, I2C_SPEED);

  if (!ads.begin(ADS1115_addr, &Wire1)) {
    Serial.println("Could not find a valid ADS1115 sensor, check wiring!");
    while (true);
  }
  else {
    // 2x gain +/- 2.048V  1 bit = 0.0625mV
    ads.setGain(GAIN_TWO);
  }

  ATTACK_COEFF = pow(M_E, -1 / (SAMPLE_RATE * ATTACK_TIME));
  RELEASE_COEFF = pow(M_E, -1 / (SAMPLE_RATE * RELEASE_TIME));

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
    ,  "Plot Data"
    ,  1024
    ,  NULL
    ,  2
    ,  NULL
    ,  ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskThresholdAlarm
    ,  "Threshold Alarm"
    ,  1024
    ,  NULL
    ,  2
    ,  NULL
    ,  ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskPublishThreshold
    , "MQTT Thresholds"
    , 2048
    , NULL
    , 3
    , NULL
    , ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskPublishOximeter
    , "MQTT Oximeter"
    , 2048
    , NULL
    , 3
    , NULL
    , ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskPublishPeakCurve
    , "MQTT Peak Curve"
    , 2048
    , NULL
    , 3
    , NULL
    , ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskPublishWaveform
    , "MQTT Waveform"
    , 2048
    , NULL
    , 4
    , NULL
    , ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskPublishAlarms
    , "MQTT Alarms"
    , 4096
    , NULL
    , 4
    , NULL
    , ARDUINO_RUNNING_CORE);
}

void loop() {
  // Display mode: 1 - show prediction, 2 - plot waveform tracker
  if (Serial.available() > 0) {
    displayMode = Serial.parseInt();
  }

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
  pressureCMH2O = ((Vout - Voff) / Sensitivity / mmH2O_cmH2O);

  // Filter signal using moving average algorithm
  flowFilter.add(airFlow);
  pressureFilter.add(pressureCMH2O);

  // Try to reconnect ESP32 device to MQTT broker
  if (!client.connected()) {
    Serial.print("Mosquito MQTT broker Timeout! ");
    Serial.println("Attempting MQTT connection..");
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    client.loop();
  }

  // Sampling rate @ 50 samples per seconds
  if (millis() > last_interval_ms + INTERVAL_MS) {
    last_interval_ms = millis();

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

    // Check the inhalation state
    if (airFlow > 0) {
      inhalation = true;
    }
    else if (airFlow < 0) {
      inhalation = false;
    }

    // Calculate the respiratory rate in breath per minute:
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

    // Track peak pressure and volume curve
    pressureCurve = pressureTracker(pressureCMH2O);
    volumeCurve = volumeTracker(tidalVolume);

    // Scaled and filtered the waveform for plotting
    filteredFlow = flowFilter.get() - 0.00;
    scaledVol = tidalVolume / 25;
    filteredPres = pressureFilter.get() - 1.02F;

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

    // Asynchrony Alarm Detection
    if (result.classification[6].value >= 0.8 && result.anomaly < 0.3) {
      // Reset the alarm variables
      asynchronyPositiveAlarm = 0;
      alarmSignal = 0;
      asynchronyAlarm = false;

      // Assigned normal prediction
      machineStatus = "Normal Machine";
      pvaStatus = "Normal Waveform";
      anomalyStatus = "Typical Waveform";
    }
    else {
      asynchronyPositiveAlarm ++;
      if (asynchronyPositiveAlarm > 3) {
        // Trigger Alarm Signal
        alarmSignal = 30;

        // Identify Alarms
        if (result.classification[0].value >= 0.8) {
          // DCA
          pvaStatus = "Delay Cycling";
          machineStatus = "NaN";
          anomalyStatus = "Typical Waveform";
        }
        else if (result.classification[1].value >= 0.8) {
          // DPPV
          machineStatus = "Detach Pressure Port";
          pvaStatus = "NaN";
          anomalyStatus = "Typical Waveform";
        }
        else if (result.classification[2].value >= 0.8) {
          // DTV
          machineStatus = "Disconnected Tube";
          pvaStatus = "NaN";
          anomalyStatus = "Typical Waveform";
        }
        else if (result.classification[3].value >= 0.8) {
          // DTA
          pvaStatus = "Double Triggering";
          machineStatus = "NaN";
          anomalyStatus = "Typical Waveform";
        }
        else if (result.classification[4].value >= 0.8) {
          // IEA
          pvaStatus = "Ineffective Effort";
          machineStatus = "NaN";
          anomalyStatus = "Typical Waveform";
        }
        else if (result.classification[5].value >= 0.8) {
          // MFV
          machineStatus = "Machine Failure";
          pvaStatus = "NaN";
          anomalyStatus = "Typical Waveform";
        }
        else if (result.classification[7].value >= 0.8) {
          // RTA
          pvaStatus = "Reverse Triggering";
          machineStatus = "NaN";
          anomalyStatus = "Typical Waveform";
        }
        else if (result.anomaly >= 0.3) {
          // Anomaly detection
          anomalyStatus = "Anomalous Waveform";
          pvaStatus = "NaN";
          machineStatus = "NaN";
        }
      }
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

void TaskPlot(void *pvParameters)
{
  (void) pvParameters;
  for (;;)
  {
    plot(pressureCMH2O);
    plot(",");
    plot(pressureCurve.min);
    plot(",");
    plot(pressureCurve.max);
    plot(",");
    plot(tidalVolume);
    plot(",");
    plot(volumeCurve.max);
    plot(",");
    plot(airFlow);
    plotln();
    vTaskDelay(INTERVAL_MS / portTICK_PERIOD_MS);
  }
}

void TaskThresholdAlarm(void *pvParameters)
{
  (void) pvParameters;
  for (;;)
  {
    // All threshold are normal ranges
    if (pressureCurve.min > THRESH_LP && pressureCurve.max < THRESH_HP &&
        respiration_rate > THRESH_LR && respiration_rate < THRESH_HR &&
        volumeCurve.max > THRESH_LV && volumeCurve.max < THRESH_HV) {
      // Reset Alarm Signal
      alarmSignal = 0;
    }
    else {
      // Pressure Alarm
      if (pressureCurve.min > THRESH_LP && pressureCurve.max < THRESH_HP) {
        // Reset alarm variables
        pthdPositiveAlarm = 0;
        pressureStatus = "Normal Pressure";
      }
      else {
        pthdPositiveAlarm ++;
        if (pthdPositiveAlarm > 5) {
          // Trigger Alarm Signal
          alarmSignal = 30;
          if (pressureCurve.min < THRESH_LP) {
            pressureStatus = "Critical PEEP";
          }
          else if (pressureCurve.max > THRESH_HP) {
            pressureStatus = "Critical PIP";
          }
        }
      }

      // Respiratory Rate Alarm
      if (respiration_rate > THRESH_LR && respiration_rate < THRESH_HR) {
        fthdPositiveAlarm = 0;
        respirationStatus = "Normal RR";
      }
      else {
        fthdPositiveAlarm ++;
        if (fthdPositiveAlarm > 5) {
          // Trigger Alarm Signal
          alarmSignal = 30;
          if (respiration_rate < THRESH_LR) {
            respirationStatus = "Low RR";
          }
          else if (respiration_rate > THRESH_HR) {
            respirationStatus = "High RR";
          }
        }
      }

      // Tidal Volume Alarm
      if (volumeCurve.max > THRESH_LV && volumeCurve.max < THRESH_HV) {
        vthdPositiveAlarm = 0;
        volumeStatus = "Normal TD";
      }
      else {
        vthdPositiveAlarm ++;
        if (vthdPositiveAlarm > 5) {
          // Trigger Alarm Signal
          alarmSignal = 30;
          if (volumeCurve.max < THRESH_LV) {
            volumeStatus = "Low TD";
          }
          else if (volumeCurve.max > THRESH_HV) {
            volumeStatus = "High TD";
          }
        }
      }
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void TaskPublishWaveform(void *pvParameters)
{
  (void) pvParameters;
  for (;;)
  {
    StaticJsonDocument<200> doc;
    doc["Pressure"] = filteredPres;
    doc["Air Flow"] = filteredFlow;
    doc["Tidal Volume"] = scaledVol;
    doc["Alarm Signal"] = alarmSignal;

    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    client.publish(MQTT_TOPIC_1, buffer, n);
    vTaskDelay(INTERVAL_MS / portTICK_PERIOD_MS);
  }
}

void TaskPublishThreshold(void *pvParameters)
{
  (void) pvParameters;
  for (;;)
  {
    StaticJsonDocument<200> doc;
    doc["Low Pressure"] = THRESH_LP;
    doc["High Pressure"] = THRESH_HP;
    doc["Low Respiration"] = THRESH_LR;
    doc["High Respiration"] = THRESH_HR;
    doc["Low Volume"] = THRESH_LV;
    doc["High Volume"] = THRESH_HV;

    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    client.publish(MQTT_TOPIC_2, buffer, n);
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

void TaskPublishOximeter(void *pvParameters)
{
  (void) pvParameters;
  for (;;)
  {
    StaticJsonDocument<200> doc;
    doc["Pulse Rate"] = random(70, 150);
    doc["SpO2"] = random(93, 100);
    doc["Perfusion Index"] = float(random(0, 30) / 3.33);

    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    client.publish(MQTT_TOPIC_3, buffer, n);
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

void TaskPublishPeakCurve(void *pvParameters)
{
  (void) pvParameters;
  for (;;)
  {
    StaticJsonDocument<200> doc;
    doc["PEEP"] = pressureCurve.min;
    doc["PIP"] = pressureCurve.max;
    doc["Peak Volume"] = volumeCurve.max;
    doc["Respiratory Rate"] = respiration_rate;

    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    client.publish(MQTT_TOPIC_4, buffer, n);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

void TaskPublishAlarms(void *pvParameters)
{
  (void) pvParameters;
  for (;;)
  {
    StaticJsonDocument<200> doc;
    doc["MachineStatus"] = machineStatus;
    doc["AsynchronyStatus"] = pvaStatus;
    doc["PressureStatus"] = pressureStatus;
    doc["RespirationStatus"] = respirationStatus;
    doc["VolumeStatus"] = volumeStatus;
    doc["AnomalyStatus"] = anomalyStatus;

    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    client.publish(MQTT_TOPIC_5, buffer, n);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}
