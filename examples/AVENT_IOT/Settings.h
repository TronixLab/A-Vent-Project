#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#define EI_PRINTF 1
#if EI_PRINTF == 1
#define ei_printf(x,...) printf(x,##__VA_ARGS__)
#else
#define ei_printf(x,...)
#endif

#define PLOT 0
#if PLOT == 1
#define plot(x) Serial.print(x)
#define plotln(x) Serial.println(x)
#else
#define plot(x)
#define plotln(x)
#endif

#define resetDPin            33
#define SFM3300_addr         0x40
#define ADS1115_addr         0x48
#define SFM3300_offSet       32768.00D
#define SFM3300_scaleFactor  120.00D
#define FREQUENCY_HZ         50
#define INTERVAL_MS          (1000 / (FREQUENCY_HZ + 1))
#define SDA_ADS              4
#define SCL_ADS              5
#define I2C_SPEED            100000
#define voltageFactor        0.0625F
#define Voff                 200.00F
#define Sensitivity          4.413F
#define mmH2O_cmH2O          10.00F

// Wi-Fi credentials 
const char* SSID = "Mipau";       // "REPLACE_WITH_WIFI_SSID" PLDTHOMEFIBRTjp7K Mipau AteneoInnovation
const char* PASSWORD = "phoneWifi777";  // "REPLACE_WITH_WIFI_PASSWORD" prSantiago57! phoneWifi777 admuinnov

// Mosquito MQTT Broker Credentials (ThingClient, IP, Port, Topic)
#define THINGNAME "ESP32 NODE"
#define mqtt_server "192.168.43.241"  // 192.168.1.8 192.168.43.241 192.168.100.179
#define mqtt_port 1883
#define MQTT_TOPIC_1 "AVENT/ESPNODE/WAVEFORM"
#define MQTT_TOPIC_2 "AVENT/ESPNODE/THRESHOLD"
#define MQTT_TOPIC_3 "AVENT/ESPNODE/OXIMETER"
#define MQTT_TOPIC_4 "AVENT/ESPNODE/PEAKCURVE"
#define MQTT_TOPIC_5 "AVENT/ESPNODE/ALARMS"
#define capacity JSON_OBJECT_SIZE(200)
