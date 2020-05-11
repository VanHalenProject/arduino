# VanHalen Vending

### Purpose

>The VanHalen Vending code is used to implement the vending part of the VanHalen sorting machine. 
>The Vending code is made for the Arduino Uno board.
>
>Required hardware:
> - Arduino Uno board
> - Ethernet Module ENC28J60
> - SG90 Mini Servo x 5

### Quick start

The vending machine uses an Arduino Uno board to run the code, it uses an Ethernet Module ENC28J60 to communicate over ethernet using a standard utp cable and uses 5 SG90 Mini Servos to be able to vend each individual Skittle color.

The vending machine requires a working DHCP server to optain a valid ip address and a working internet connection in order to communcate with the MQTT broker.

To adjust the MQTT topic and the MQTT broker server the following variables can be altered:

	const char* channelName = "VanHalen/Vending";
	const char* server = "broker.hivemq.com";

### Pinning

The pinning for the Arduino Uno board is as follows:

 - Digital 3	- orange servo 1 wire (Red)
 - Digital 4	- orange servo 2 wire (Orange)
 - Digital 5	- orange servo 3 wire (Yellow)
 - Digital 6	- orange servo 4 wire (Green)
 - Digital 7	- orange servo 5 wire (Purple)
 - Digital 10	- cs Ethernet
 - Digital 11	- s1 Ethernet
 - Digital 12	- s0 Ethernet
 - Digital 13	- sck Ethernet
 - 3.3v		- vcc Ethernet
 - 5v		- all red servo wires
 - GND		- all brown servo wires, gnd Ethernet

### Controlling the Vending machine

The vending machine uses MQTT messages received on the given topic (channelName variable). The message received contains an amount of each color to be vendored sepperated by a ";".

The format to be used is as follows:
"Red amount; Orange amount; Yellow amount; Green amount, Purple amount;"

For instance, say we want 2 red, 3 orange, 10 yellow, 5 green and 0 purple Skittles the message will be: "2;3;10;5;0;".
  
