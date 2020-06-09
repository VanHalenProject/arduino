# VanHalen Sorting

### Purpose

>This is the pre-sorting part of the VanHalen Sorting Machine project. It detects color and sorts the candy accordingly
>(each color gets a separate container)

>Required hardware:
> - Arduino MKR1000 board
> - TCS230/3200 color detection sensor
> - SG90 Servo x 2

### Quick start

The pre-sorting uses an Arduino MKR1000 board in conjunction with the TCS230/3200 color detection sensor and two SG90 Digital Servos, 
to serve detecting and sorting duties, and transmits its state over a WiFi connection using MQTT protocol. 
It requires a DHCP server to establish a network connection.

      const char topic[] = "VanHalen/PreSort";
	    const char mqttServer[] = "broker.hivemq.com";


### Pinning

MKR1000 pinout:

- MKR1000  -----  TCS230
5v --------------- VCC
GND -------------- GND
8 ---------------- OUT
7 ----------------- S2
~6 ---------------- S3
~5 ---------------- S0
4 ----------------- S1

- MKR1000 ---- DIGITAL SERVO (1 & 2)
5V(pwr supply) ---------------- RED
GND(common) ----------------  BROWN
D0/D1 ---------------------- ORANGE



### Controlling the Sorting machine

The sorting machine will start sorting colors as soon as it receives the "start" command via MQTT on selected topic.
As soon as a given container runs full and there's another candy of that particular color detected, the sorting process stops and 
a summary of amount of sorted candies per color will be transmitted via MQTT.
The message follows this format: 
“red_23; orange_34; yellow_26; green_39; purple_35” 
