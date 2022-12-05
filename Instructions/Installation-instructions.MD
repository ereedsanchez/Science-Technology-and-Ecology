1. Install Arduino 1.8
2. Install CH210 driver
https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads
CP210x Windows universal Driver
CP210x VCP Mac OSX Driver
see below instructions for installation*

3. In Arduino Install esp32 libraries
Documentation / Instructions: https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html

3a. Copy: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
and paste in Arduino/File/Prefrences/ Additional Board URL.

see image: https://docs.espressif.com/projects/arduino-esp32/en/latest/_images/install_guide_preferences.png

3b. Go to Tools/Board/Boards Manager 
Search for esp32
Install esp32 by expressif systems, v 2.0.5
This may take a while. 

4. Install Libraries
- PubSubClient by Nick O'Leary, 2.8.0
- DHT sensor library by Adafruit, 1.4.4
- - Install All - Adafruit Unified Sensor, 1.1.6
- DallasTemerature by Miles Burton, 3.9.0
- - Install All - OneWire, 2.3.7
- Adafruit SHT31 by Adafruit, 2.2.0
- - Install All - Adafruit BusIO, 1.14.1

5. Use Firmware
https://github.com/ereedsanchez/Science-Technology-and-Ecology/blob/main/Firmware/All-Feather32-WIP-V2.ino
 
5a. Edit Wifi credentials
const char* ssid = "MyAltice e46924";         
const char* password = "2805-indigo-37";          

5b. Edit Device IDs
const char* DHT22TEMP = "90001";
const char* DHT22HUM = "90002";
const char* SHT30TEMP = "90003";
const char* SHT30HUM = "90004";
const char* DS18B20TEMP = "90005";
const char* VOLTAGE = "90006";
** use refernce list

6. Add Sensor (Device IDs) to Grafana
6a. Goto http://165.22.191.125:3000/ Login (YAYB / OuterSpace)
6b. Add sensors to YAYB Dashboard
    Create (+) Dashboard
    Name it: YAYB+YourNames
6c. Add new panel
    Title Panel(s): 
	YAYB-0x DHT Temperature

6d.FROM sensordata Time column created_at Metric column device_id
SELECT Column: value
WHERE Macro: $__timeFilter Expr: device_id = '90002'

change device id.

6e. Repeat 6c, and 6d for the following. Refer to device IDs dosument. 
	YAYB-0x DHT Humidity
	YAYB-0x SHT Temperature
	YAYB-0x SHT Humidity
	YAYB-0x DS Temperature
	YAYB-0x Voltage


----------------------------------------------------------------------------------------
*CP210 Driver
INSTALLING
----------
The VCP driver is automatically installed by Windows Update for CP210x with
alternate PID values of 0xEA63, 0xEA7A, and 0xEA7B.  These are not default
values.

Otherwise, unzip the .zip file and you will have a directory containing the
driver installer files.

### Prompted install ###

If Windows prompts you to install a driver for a CP210x device
(for example, if you plug a CP210x into your computer):

1. Use the dialog to browse for the driver location
2. Locate the driver folder (that you previously unzipped)
3. Follow the instructions

### Manual install ###

1. Using Windows File Explorer, locate the driver folder
   (that you previously unzipped)
2. Right click on the silabser.inf file and select Install
3. Follow the instructions

-----------------------------------------
**Device IDs Reference List -  
YAYB-01-DHT-Temp- 90001
YAYB-01-DHT-HUM - 90002
YAYB-01-SHT-Temp- 90003
YAYB-01-SHT-HUM - 90004
YAYB-01-DS-Temp - 90005
YAYB-01-Voltage  - 90006
