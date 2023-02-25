# Overview
Ateneo ventilator (A-vent) project is a low-cost and intelligent mechanical ventilator was developed under [Ateneo Innovation Center](https://www.ateneoinnovation.org/) (AIC) at [Ateneo de Manila University](https://www.ateneo.edu/) started during the peak of COVID-19 pandemic. The project development consist of 3 phases, the following are:
* **Phase 1** Design and development of intelligent A-vent unit
* **Phase 2** Modular alarm: real-time detection of PVA (patient-ventilator asynchrony) and machine problems
* **Phase 3** Wireless real-time monitoring: applied IoT (Internet of Things), digital twin of A-vent

## Phase 1
COVID-19 pandemic is a respiratory diseas resulted in high demand for mechanical ventilator. During the peak of pandemic, the healthcare provider around the world overwhelmed the healthcare system and lacks with mechanical ventilator unit for critically ill COVID-19 patients. The alternative solution for the ventilator demand is to use an open-source based ventilator machine was first introduced by [MIT emergency ventilator](https://emergency-vent.mit.edu/). The pandemic opens a new opportunity for institutions, and organizations worldwide to developed a low-cost ventilator machine, and the AIC was one of the respondent to that situation.

<img src="https://github.com/TronixLab/A-Vent-Project/blob/main/media/avent%201.1.jpg" width="18%"></img>  <img 
src="https://github.com/TronixLab/A-Vent-Project/blob/main/media/avent%201.2.jpg" width="18%"></img>  <img 
src="https://github.com/TronixLab/A-Vent-Project/blob/main/media/avent%201.3.jpg" width="18%"></img>  <img 
src="https://github.com/TronixLab/A-Vent-Project/blob/main/media/avent%201.4.jpg" width="18%"></img>  <img 
src="https://github.com/TronixLab/A-Vent-Project/blob/main/media/avent%201.5.jpg" width="18%"></img>

A-vent was designed and development through the leadership of Engr. [Paul M. Cabacungan](https://2012.ateneo.edu/ls/sose/ateneo-innovation-center/faculty/paul-m-cabacungan) - AIC Operation Manager together with Mr. Reymond Cao and research assistant. The initial developement was made of easily available food-grade and low-cost materials, and enough to demonstrated a minimum viable prototype. Upon with the 1st development phase, we published a paper titled [*Design and Development of A-vent: A Low-Cost Ventilator with Cost-Effective Mobile Cloud Caching and Embedded Machine Learning*](https://ieeexplore.ieee.org/document/9550920). This paper demonstrate the A-vent minimum viable prototype and integration of AIC technologies such Mobile/Near Cloud Computing, and Machine Learning that predicts the type of PVA as proof of concept.

## Phase 2
The A-vent unit was modified and made it smaller for transportation ease of the unit and designed with touch screen HMI (human machine interface). This development phase emphasized the application of [embedded machine learning (TinyML)](https://www.tinyml.org/) to detect the PVA and machine problems to generate a real-time alarms for intelligent monitoring of machine-and-patient interaction.

<img src="https://github.com/TronixLab/A-Vent-Project/blob/main/media/avent%202.1.jpg" width="15%"></img> <img src="https://github.com/TronixLab/A-Vent-Project/blob/main/media/avent%202.2.jpg" width="15%"></img> <img src="https://github.com/TronixLab/A-Vent-Project/blob/main/media/avent%202.3.jpg" width="15%"></img> <img src="https://github.com/TronixLab/A-Vent-Project/blob/main/media/avent%202.4.jpg" width="15%"></img> <img src="https://github.com/TronixLab/A-Vent-Project/blob/main/media/avent%202.5.jpg" width="15%"></img> <img src="https://github.com/TronixLab/A-Vent-Project/blob/main/media/avent%202.6.jpg" width="15%"></img> 

The idea of the A-vent development phase was shift to the creation a modular alarm device can be suitable for any ventilator machines. The modular alarm device for open-source ventilator was first demonstrated by Illinois University, the project called [Illinois RapidAlarm](https://rapidvent.grainger.illinois.edu/rapidalarm). The Illinois RapidAlarm[[1](https://rapidalarm.github.io/#!index.md)] is a sensor and alarm module for use with emergency pressure-cycled ventilators such as the Illinois RapidVent that do not have built-in monitoring systems. The module connects to a ventilator circuit and monitors the pressure delivered to the patient airway. It produces an audible alarm when it detects a problem with the ventilator, such as a disconnection or obstruction, and also displays information about airway pressure and breathing rate. They published IEEE paper, [*Low-Complexity System and Algorithm for an Emergency Ventilator Sensor and Alarm*,](https://ieeexplore.ieee.org/document/9184284) reports the team’s work to build a low-cost, easy-to-produce electronic sensor and alarm system for pressure-cycled ventilators. The devices estimate clinically useful metrics, such as pressure and respiratory rate, and sound an alarm when the ventilator malfunctions.

The same algorithm was applied to A-vent alarm system, not only for pressure cycle, and flow rate, also applied for tidal volume alarm. Moreover, we added the applied Artificial Intelligence called TinyML for the recognition of PVA and machine problems with anomaly detection that will generate alarm in such event. This intelligent monitoring of mechanical ventilator will help hospital to monitor the critically-ill patients in real-time and assist healthcare workers that may reduce their workload. We presented a paper titled *Waveform Tracker Alarm for Automatic Patient-Ventilator Asynchrony (PVA) and Mechanical State Recognition for Mechanical Ventilators Using Embedded Deep Learning* at 11th [ICMCE (International Conference on Mechatronics and Control Engineering)](http://www.icmce.org/) 2023. In this paper, we demonstrated the detection of various type of PVA and machine problems, the event was emulated and generate an alarm signal in near real-time due to possitive alarm sequencing that avoid triggering of false alarm.

<img src="https://github.com/TronixLab/A-Vent-Project/blob/main/media/PVA%20alarm.png" width="45%"></img> <img src="https://github.com/TronixLab/A-Vent-Project/blob/main/media/Machine%20alarm.png" width="45%"></img>

## Phase 3
In this development phase is the application of wireless sensor network, and IoT combined with the intelligent alarm device. It demonstrate a digital replica of the system through a remote dashboard called Digital Twin. It stored the data on the local server database and display the ventilator waveform and diagnosis on the web dashboard that can be access through a WLAN (wireless local area network).

<img src="https://github.com/TronixLab/A-Vent-Project/blob/main/media/avent%203.1.jpg" width="45%"></img> <img src="https://github.com/TronixLab/A-Vent-Project/blob/main/media/dashboard.png" width="45%"></img>

The dashboard consist of place holder images that represent the physical system. This this way, the data on the dashboard can be easily comprehended and to interpret. TinyML approach for ventilator machine alarm system demonstrated an decentralized processing or edge computing of the data, ensure the real-time operation for time-constrained applications are critical for medical devices.

# Main Components (Phase 2/3)
* [ESP32 DevKit Board](https://circuit.rocks/esp32-wifi-ble.html)
* [MPX5010DP Differential Pressure Sensor](https://www.digikey.com/en/products/detail/nxp-usa-inc/MPX5010DP/464054)
* [SFM3300 Sensirion Flow Meter](https://www.digikey.ph/en/products/detail/sensirion-ag/SFM3300-250-D/9857673)
* [4-Channel 16-bit Analog-to-Digital Converter Module](https://circuit.rocks/adc-16-bit-4-channel-ads1115-with-programmable-gain-amplifier)
* [4-Channel Logic Level Bidirectional Converter (3.3V/5V)](https://circuit.rocks/logic-converter-4-chanel-5v-3v)

# Schematic Diagram
![Schematic Diagram](https://github.com/TronixLab/A-Vent-Project/blob/main/media/Schematic%20Diagram.png)

# Dependencies
## Documentation
Prior to the replication of the prototype, it is important to read and study the following documentation for your reference.
* [Getting Started with ESP32](https://github.com/TronixLab/DOIT_ESP32_DevKit-v1_30P)
* [ESP32 Real-Time Operating System (FreeRTOS)](https://github.com/TronixLab/introduction-to-rtos)
* [MPX2010DP](https://github.com/TronixLab/MPX2010DP)
* [SFM3300](https://github.com/TronixLab/SFM3300)
* [BLE Fingertip Pulse Oximeter](https://github.com/TronixLab/BLE-Fingertip-Pulse-Oximeter-Jumper-Medical-)

## Arduino Libraries
See Arduino Documentation for [Installing Libraries](https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries). Add the following libraries to your Arduino IDE.
* [PubSubClient](https://github.com/knolleary/pubsubclient)
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
* [Adafruit_ADS1X15](https://github.com/adafruit/Adafruit_ADS1X15)
* [MovingAverageFloat](https://github.com/pilotak/MovingAverageFloat)
* [A-Vent_PVA_Detection_inferencing](https://github.com/TronixLab/A-Vent-Project/blob/main/libraries/ei-a-vent-pva-detection-arduino-1.0.6.zip)

## Software
The following software needed to install are:
* [Arduino IDE](https://docs.arduino.cc/software/ide-v1)
* [Eclipse Mosquitto MQTT Broker](https://mosquitto.org/)
* [Node-RED](https://nodered.org/)
* [InfluxDB](https://www.influxdata.com/)
* [PostgreSQL](https://www.postgresql.org/)

# Software Installation
### Installing Mosquitto MQTT Broker on Window
To install the MQTT Broker on a Windows computer, please follow the steps below.
1. Download the MQTT Broker, version 1.6.x, and earlier are recommended for ease of use. [Download link](https://mosquitto.org/download/).
2. After downloading, double-click on the downloaded .exe file and follow the installation process.
3. Open Command Prompt as Administrator. Go to the folder/Path where you installed your broker. During the installation, please refer to the installation directory. In the Command Prompt run the following command according to your Mosquitto installation folder directory.
```
cd C:\Program Files\mosquitto
```
4. Execute the following commands to install Broker and start Mosquitto Service for Windows.
 ```
mosquitto install
net start mosquitto
```
This will automatically install the Mosquitto service on Windows. The Mosquitto can be tested by executing the following command.
```
mosquitto -h
mosquitto -v
```
5. Mosquitto service runs on port 1883 by design. In Windows, make sure port 1883 is available. To do so, open a command prompt and type the following command.
```
netstat -a
```
This will show all of your Windows computer's active service ports. If 1883 is shown, the port has been successfully opened for communication.
6. Open another Command Prompt as Administrator for publisher and subscriber. Navigate to the Mosquitto installed folder or path as stated in step 4. Execute the following Command to Subscribe to topic test_topic.
```
mosquitto_sub -t test_topic -h localhost
```
You can not receive data because there is no publisher at this time. To receive data, perform the following publisher steps in another command prompt and then search the subscriber terminal for received data. Execute the following command to Publish the message “Hello World”.
```
mosquitto_pub -t test_topic -h localhost -m "Hello World"
```
Now check your subscriber Command Prompt, you should get `Hello World`.
#### Mosquitto CLI Commands and flags
**Common Commands**
* `mosquitto` - running the Mosquitto broker.
* `mosquitto_pub` - command-line utility for publishing messages to a broker.
* `mosquitto_sub` - command-line utility for subscribing to topics on a broker.
**Common Flags**
* `-h` (help) - Display usage information.
* `-v` (verbose) - Use verbose logging (network status debugging).
* `-h` (host) - Specify the host to connect to. Defaults to localhost.
* `-p` (port) - Connect to the port specified. If not given, the default of 1883 for plain MQTT or 8883 for MQTT over TLS will be used.
* `-m` (message) - Send a single message from the command line.
* `-t` (topic) - The MQTT topic on which to publish the message.

### Installing Node-RED on Windows
Node-RED can be installed locally on Windows and Linux, as well as the Raspberry Pi. Node-RED was used to subscribe the data to the Mosquitto MQTT broker and parse it to write metrics on the database. This guide details how to install Node-RED in a Windows environment. The steps are only applicable to Windows 10.
1. From the latest 14.x version of Node.js from the official NodeJS download page, get the new LTS recommended version of the [NodeJS Windows installer](https://nodejs.org/en/download/). To ensure that NodeJS and npm are correctly installed, open a Command Prompt and execute the following command.
```
node --version && npm –version
```
2. Installing Node-RED as a global module adds the command node-red to your system path. At the command prompt, enter the following:
```
npm install -g --unsafe-perm node-red
```
3. The command node-red is added to your system path after installing Node-RED as a global module. At the command prompt, enter the following:
```
node-red
```
The Node-RED log will be output to the terminal as a result. To keep Node-RED going, you must leave the terminal open.
4. Open the editor in a web browser while Node-RED is running. You can access it using the URL:
```
http://localhost:1880
```
If you are using a browser on the same computer that is running Node-RED. If you're using a browser on another device, you'll need to enter the IP address of the Node-RED server:
```
http://<ip-address>:1880
```
For more details, read the Node-RED documentation [here](https://nodered.org/docs/user-guide/editor/). You may upload the [Node-RED flow](https://github.com/TronixLab/A-Vent-Project/blob/main/files/AVENT_IOT-NODE-RED.json) for project A-vent from your workspace.

**Note**: *Install the following Node-RED palletes `-influxdb`, and `-postgresql`.

### Installing InfluxDB on Windows
This guide covers all the required steps for a clean InfluxDB installation on a Windows computer. Here you'll find all of the [downloads](https://portal.influxdata.com/downloads/) Telegraf, InfluxDB, Chronograf, and Kapacitor. Each of these tools serves a distinct purpose, such as collecting metrics, storing data, visualizing time series, or performing post-processing functions on your data. This guide will only cover the InfluxDB time-series database component.
1. Install InfluxDB v1.x Windows Binaries (64-bit) by downloading the installation file. Simply copy and paste the offered link into your browser, and the installation file will be downloaded automatically. For example `https://dl.influxdata.com/influxdb/releases/influxdb-1.8.5_windows_amd64.zip`
2. Create a folder let’s name it InfluxDB where you save the Influxdb application file in the Program Files folder or Documents. To avoid Administrator constraints when accessing the files, it is suggested that you move your InfluxDB installation folder to Documents. Inside your folder, create another folder let's name it Influxdb, and extract the installation content files into it.
3. The system requirements for Influxdb for Windows OS are Win10, 64-bit AMD architecture, and Powershell. You will use Powershell to execute influx and influxd commands. Let’s say that your InfluxDB application file is extracted on 'C:\Program Files\InfluxData\influxdb'. In Powershell, navigate into 'C:\Program Files\InfluxData\influxdb' and start InfluxDB by running the influxd daemon:
```
./influxd
```
When starting InfluxDB for the first time, Windows Defender will appear with the following message: `Windows Defender Firewall has blocked some features of this app`. Select Private networks, such as my home or work network, then click Allow access.
Let’s run a quick-dry test to see if everything is okay. Now that your InfluxDB server has started, start a new Command Prompt and execute the following command.
```
curl -sl -I http://localhost:8086/ping
```
The /ping endpoint is used to check if your server is running or not. If you are getting a 204 No Content HTTP response, it seems that everything is configured correctly.
4. Run InfluxDB as a service. To run InfluxDB as a Windows service the easiest way is to use nssm (Non-Sucking Service Manager) tool on Windows. To download NSSM, head over to [https://nssm.cc/download](https://nssm.cc/download). Extract it in the root InfluxDB directory you created earlier. From there, in the current NSSM folder (win64), open Command Prompt as Administrator and run the following command. Prompt as Administrator and run the following command.
```
nssm install
```
Alternatively, you can also use PowerShell and run the following command.
```
.\nssm.exe install
```
You will be prompted with the NSSM window. On the Application tab enter the following example:
* Application Path: `C:\Users\userName\Documents\InfluxData\influxdb-1.8.1-1\influxd.exe`
* Startup Directory: `C:\Users\userName\Documents\InfluxData\influxdb-1.8.1-1\`
* Service Name: `influxdb`
Once done, click `Install Service`. In the Windows search menu, type `Services` and open the `Services window`. You should now be able to start the influxdb service.
5. Set up InfluxDB through the UI. With InfluxDB running, visit localhost:8086. This should direct you to a page as shown in the figure below. Click Get Started. Set up your initial user by entering a `Username` for your initial user. Set a `Password` and Confirm Password for your user. Enter your `Organization Name`. Enter your initial `Bucket Name`. Click `Continue`.
6. Create your time-series database by creating a new bucket where all the data are stored. Navigation on the 'Data Explorer', at the left bottom of the page we can see the list of the buckets. Create a new one by clicking on `+ Create Bucket`. Name it anythinh you want, and note it for project reference. Back to [Node-RED flow](https://github.com/TronixLab/A-Vent-Project/blob/main/files/AVENT_IOT-NODE-RED.json), configure the influxdb node according to your InfluxDB Bucket name.

### Installing PostgreSQL on Windows
PostgreSQL or Postgres is the most commonly used open-source relational database. It offers features like robustness, reliability, cost-free, etc. PostgreSQL serves as a data warehouse for multiple applications like web apps, mobile apps, etc. It enables us to store enormous and sophisticated data securely. However, to achieve any of its functionalities or features, firstly, we have to download and install PostgreSQL. It used in this project to store string data type, however Influxdb is used over PostgreSQL to store numerical value for real-time data query and efficiency.
1. Download the postgreSQL from this link [https://www.postgresql.org/download/](https://www.postgresql.org/download/). Select the operating system on which you want to download the PostgreSQL. In our case, its Windows operating system. Click on the 'Download the installer' option to download the interactive installer. Choose the latest PostgreSQL version for your respective operating system and click on the download button.
2. Once the downloading is completed, open the respective `.exe` file. Specify or browse the directory where you want to install the PostgreSQL. After specifying, click on the `Next` button to proceed further.
3. Select a `data directory` to store your data and click on the `Next` button.
4. Set a password (must remember it for later use) for the Postgres superuser and click on the `Next` button. Whether set the port number or go with the default port number `5432` to connect to a database. After specifying, click on the `Next` button to proceed further.
5. Once the PostgreSQL setup wizard is completed, click on the `Finish` button to close the installation windows. Click the windows button and find the `postgreSQL 14`. Once you find it click on it and select the `pgAdmin 4`. Clicking on the `pgAdmin 4` will ask you to enter the password. That was all the necessary information about downloading, installing, and launching PostgreSQL on Windows 10 operating system. You may now create your own PostgreSQL and configure your [Node-RED flow](https://github.com/TronixLab/A-Vent-Project/blob/main/files/AVENT_IOT-NODE-RED.json) according to your PostgreSQL database.

### Installing Grafana on Windows
1. Download the Windows installer package, and navigate to [Download Grafana](https://grafana.com/grafana/download).
2. Select a Grafana version you want to install. The most recent Grafana version is selected by default.
3. Select an Edition, you can choose either of two editions.
a. Enterprise - Recommended download. Functionally identical to the open-source version, but includes features you can unlock with a license if you so choose.
b. Open Source - Functionally identical to the enterprise version, but you will need to download the enterprise version if you want enterprise features.
4. Click Windows. Then click Download the installer. This automatically downloads the installation setup. Open and run the installer. To run Grafana, open your browser and go to the Grafana port `http://localhost:3000`. The default HTTP port that Grafana listens to is `3000` unless you have configured a different port.
5. To sign in to Grafana for the first time. Enter `admin` for username field and `password` for password field. Click Sign in. If successful, you will see a prompt to change the password. Click `OK` on the prompt and change your password. It is recommended that you change the default administrator password.
6. Add InfluxDB as a datasource on Grafana. In the left menu, click on the `Settings` > `Data sources` section. In the `Add data source` selection panel, at Time series databases choose InfluxDB as a data source.
Here are the configuration settings you have to match to configure InfluxDB on Grafana:
* Name: InfluxDB
* Query Language: Flux
* URL: http://localhost:8086
* Organization: ADMU (your custom organization on InfluxDB)
* Token: XXXX (your user token on InfluxDB)
* Default Bucket: NODE_RED (your custom database name on InfluxDB)

Click on `Save and Test`, and make sure that you are not getting any errors. If you get a `502 Bad Gateway error`? Check that your URL field is set to HTTPS, or consider other fields should configure properly. If everything tests well, it's time to construct the Grafana dashboard.

8. You may load the Grafana dashboard as digital twin system for A-vent project, download the json file [here](https://github.com/TronixLab/A-Vent-Project/blob/main/files/A-VENT%20Digital%20Twin%20Dashboard-Grafana.json), and load it to your Grafana.

# Demonstration
[![GitHub Logo](https://github.com/TronixLab/A-Vent-Project/blob/main/media/youtube.png)](https://www.youtube.com/watch?v=Z4yzic3626o)



