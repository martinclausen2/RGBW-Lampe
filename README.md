# RGBW-Lampe

The "RGBW-Lampe" projects allows to create full color lighting appliances.

## Features

- Four independent LED driver channels
  - High dynamic dimming range using PT4115 LED driver
- Status LED to indicate power on, channel, standby, and network status
- Brightness measurement to adjust brightness at power on
- IR remote control (receiver and sender)
  -	Option to network devices
- Single rotational encoder and switch needed to control
- Embedded terminal to control settings, clock, and alarms available via WLAN
  - Connect to home automation via MQTT
  - Adjust settings via exposed terminal
- Protected electronic power switch

## Technical Features
- ARM Cortex M3 based STM32L151 microcontroller
  - EEPROM for permanent storage of settings
- ESP8266 based WLAN connectivity
- RC5 based communication (sender and receiver)
- Four PT4115 step-down LED driver
  - PWM input
  - LED driving current up to 1A
- High dynamic brightness measurement with low cost phototransistor SFH320
- PCB prepared (not used in current software) for 
  - Acceleration sensor
  - Pin headers for extension with additional I2C devices, LCD, or LED drivers
  - EEPROM from 24CXXX series with I2C connection - currently the MCU's internal EEPROM is utilized, the EEPROM can be populated if other STM32 MCU are to be used which do not contain a EEPROM

# Design description

## Hardware

### WLAN
The low cost WLAN module ESP-12 based on the ESP8266 is used to connect to home automation, expose an interface to configure the appliance, and to obtain date and time via NTP.

### MCU
The STM32L1 series features for this application relevant features like a large number of timers, and an embedded EEPROM.

The MCU can be programmed and debugged via SWD accessable through a custom pin header, that also exposes the serial port.

### LED Driver
Collor mixing requires a large dimming range, which was realized via PWM dimming. The LED driver PT4115 has a short rise time allowing to create also very short current pulses to drive the LEDs at low levels.

### Logic Power Supply
A step down voltage regulator generates 3.3 V logic supply. It needs to be capable to provide good load regulation and sufficient current for the WLAN module as well as the IR-LED.

### RC5 receiver and sender

A IR emitting diode is driven by a small MOSFET from the PWM output of a timer in the MCU to send RC5 commands.

To receive RC5 commands the output of the IR receiver is sampled at a rate of about 4.5kHz by the MCU. The power supply of the IR receiver is cut in standby mode to prevent the battery to be drained prematurely.

### Brightness Sensor

The ambient brightness is sensed via an in expensive a broadly available phototransistor SFA 320. To enhance the dynamic range, the MCU can switch the bias current between two different levels.
The precision is fully sufficient to adjust the brightness of the power LEDs at power on and dim the status LED in standby.

## Software

### WLAN Module

The time is fetched on start and about once per day later from the NTP server.

The terminal is exposed via port 22.

## Known Limitations
- The WLAN module has fixed settings. Recompilation and flashing of the ESP8266 is needed to change them.
  - MQTT (device?.h)
  - WLAN (credentials.h)
  - NTP server (device?.h)
- The WLAN module exposes the ports of terminal (22) and MQTT without any protection.
- Commands received via MQTT and updates from the NTP server are injected into the terminal communication. Commands from the terminal might be interruped by MQTT or time updates.
- OTA updates of the STM32 are not implemented yet.

# User Manual

The user interface is rather simplistic.

Turning the encoder increases or decreases the brightness.
The integrated push button is used to select functionality and change between on and off.
Detailed settings can be adjusted via the serial terminal.

Coming from standby a short button press switches the light on. A long button press activates the networked mode, where other compatible lighting appliances receive commands to follow on, off, and brightness settings.

When the appliance in on the short button press selects between the four individual LED strings and a mode where the brightness is modulated between the channels.

When the button is pressed, the status led previews a for a short time the next selected functionality.


Standby can be reached via a long button pressed from power on mode.

| LED Color 	| Condition | Status   | 
| :------------ | :------ | :------- |
| red			|         | standby  |
| white			|         | power on |
| blue flashing | 1 flash, pause | WLAN module booting |
| blue flashing | 3 flash, pause | No time from NTP server obtained |
| green flash	| while adjusting brightness | maximum brightness reached |
| red flash		| while adjusting brightness | minimum brightness reached |
| white flash	| pressing the button short | selected white LED channel  |
| red flash		| pressing the button short | selected red LED channel |
| blue flash	| pressing the button short | selected blue LED channel |
| green flash	| pressing the button short | selected green LED channel |
| pink flash	| pressing the button short | selected mood light mode |
| green flash	| pressing the button very long | cancel button pressed |

The white flash is actually not visible, since the LED is already shining white.

# Configuration Manual

The serial port uses the following parameter 115200 Baud rate, data site 8, parity none, stop bits 1.

This terminal is exposed via the WLAN module to the user on port 22.

The commands can be listed by sending the command `help`.

## WLAN

WLAN credentials must be set at compile time in the file `credentials.h`.

## MQTT

Broker and node name must be defined at compile time in the file `device?.h`:

	#define MQTTbroker "192.168.178.123"
	#define nodename "RGBW-Lampe1"

Topics might be freely defined at compile time in the file `device?.h`. The following section contains some examples.

The appliance publises to different topics its status:

	#define publishtopicstatus "RGBW-Lampe/1/status"
	#define publishtopicNTP "RGBW-Lampe/1/NTP"
	#define publishtopicswitch "RGBW-Lampe/1/switch"

The appliance subscribes to two topics defined at compile time in the file `device?.h`. The names are here choosen to be independent of the device id.

### Switch

	#define subscribetopicswitch "RGBW-Lampe/switch"
The commands are `on` and `off`

	#define subscribetopic "RGBW-Lampe"
All data is forwarded to the terminal of the MCU.

## Commands Terminal

### Set Brightness

	bright [type] <-c channel_no> <-b brightness_value>

set brightness values

| Type | Decription |
| :--- | :--------- |
| 0 | Brightness last used before lights were switched off. |
| 1 | Minimum brightness when switched on |
| 2 | Maximum brightness any time |
| 3 | Maximum brightness when an alarm is triggered |
| 4 | PWM offset for brightness |
| 5 | Current brightness |

### Get External Brightness

	getextbright

Get the external brightness from the phototransistor

### Infrared Remote Control

	remote <-a address> <-rm receiver mode> <-sm sender mode> <-sa send address> <-sd send data>

set infrared remote parameters

| Mode | Decription |
| :--- | :--------- |
| 0 | Off |
| 1 | Alarm |
| 2 | Condional |
| 3 | All |

### Set Date and Time

	time [hour] [minute] [second]

set time of RTC

	date [2 digit year] [month] [day] [w] with w weekday

set date of RTC

	timestamp

get date and time from RTC

### Alarm

	alarmschedule [alarm no] <-w weekday> <-h hour> <-m minute>

set alarm schedule

	alarmsetting <-f time to fade-in light> <-si time to signal> <-sn snooze time>

set alarm parameters
	
	alarm <-a 1 | 0> <-s alarm skip count>

trigger, reset, set alarm skip count

### Beep

	beep <-s 1 | 0> volume - <-v volume> volume level - <-vl 1 | 0>

Call without an parameters to hear a beep. Using the `-s` parameter allows to turn the acustic signal on and off. Volume allows for a continues selection of the volume between 0 and 255. The volume level sets the range in which the volume is adjustable.

### Power

	power <-l 1 | 0>

switch light.

### Reset Settings

	reset

reset settings to factory defaults

### Status LED

	statusled [flash count]

flash status led

### Mood light

	fadelight <-f 1 | 0> time - <-t time> <-b brightness> <-mb maximum brightness>

switch and parameterize on mood light

### DebugCmd

	debug

Dummy command to absorb debug messages from host.

### Reset WLAN Module

	resetesp

Reset network controller ESP8266.

### Version

	version

show STM32 software version