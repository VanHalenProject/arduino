#include <WiFi101.h>
#include <MQTT.h>
#include <math.h>

//TCS230 pins wiring to Arduino
#define S0 4
#define S1 5
#define S2 6
#define S3 7
#define sensorOut 8
//Setting up MQTT/WIFI connection
const char WIFI_SSID[] = "Your SSID";
const char WIFI_PASS[] = "Your Pass";
const char mqttServer[] = "192.168.2.3";
const int mqttServerPort = 1883;
const char key[] = "key";
const char secret[] = "secret";
const char device[] = "mkr1000";

int status = WL_IDLE_STATUS;
WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

//Stores frequency read by the photodiodes
int redFrequency = 0;
int greenFrequency = 0;
int blueFrequency = 0;

//define colors
enum Colors{WHITE, RED, GREEN, BLUE, YELLOW, ORANGE, BROWN, VIOLET};
////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);
  connect();
  
  // Setting the outputs
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  
  // Setting the sensorOut as an input
  pinMode(sensorOut, INPUT);
  
  //Frequency scaling to 2%
  digitalWrite(S0,LOW);
  digitalWrite(S1,HIGH);
  
};
////////////////////////////////////////////////////////////////////////////////
void loop() {
  client.loop();

  if (!net.connected()) {
    connect();
  }

  //red sensor
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  redFrequency = pulseIn(sensorOut, LOW);

  //green sensor
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  greenFrequency = pulseIn(sensorOut, LOW);

  //blue sensor
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  blueFrequency = pulseIn(sensorOut, LOW);

  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    client.publish("/color", whatColor(redFrequency, greenFrequency, blueFrequency));
    lastMillis = millis();
  }

};

///////////////////////////////////////////////////////////////////////////////////
const char* whatColor(int redFrequency, int greenFrequency, int blueFrequency){
  Colors tempColor = WHITE;

  //YELLOW observed sensor values R:14-16 G:23-24 B:5-7
  if((isInRange(10, 17, redFrequency)) && (isInRange(14, 25, greenFrequency)) && (isInRange(3, 8, blueFrequency))){
    tempColor = YELLOW;
  }
  //RED observed sensor values R:19-22 G:81-83 B:12-14
  if((isInRange(18, 24, redFrequency)) && (isInRange(70, 90, greenFrequency)) && (isInRange(12, 19, blueFrequency))){
    tempColor = RED;
  }
  //BLUE observed sensor values R:81-84 G:37-39 B:9-11
  if((isInRange(70, 90, redFrequency)) && (isInRange(35, 45, greenFrequency)) && (isInRange(8, 13, blueFrequency))){
    tempColor = BLUE;
  }
  //GREEN observed sensor values R:26-29 G:23-26 B:9-10
  if((isInRange(25, 36, redFrequency)) && (isInRange(26, 34, greenFrequency)) && (isInRange(9, 12, blueFrequency))){
    tempColor = GREEN;
  }
  return getColorName(tempColor);
};

//GETTING THE COLOR NAMES OUT OF ENUM
const char* getColorName(enum Colors color) {
   switch (color) 
   {
      case WHITE: return "White";
      case RED: return "Red";
      case GREEN: return "Green";
      case BLUE: return "Blue";
      case YELLOW: return "Yellow";
      case ORANGE: return "Orange";
      case BROWN: return "Brown";
      case VIOLET: return "Violet";     
   }
};
//////////////////////////////////////////////////////////////////////////////////////
bool isInRange(int lowValue, int highValue, int targetValue){ 
  boolean inRange = false;
  if(targetValue <= highValue && targetValue >= lowValue){
    inRange = true;
  }
  return inRange;
};

//TAKING AN AVERAGE FROM 5 MEASUREMENTS TO SMOOTH THE FREQ RESPONSE OUT
int averageRedFrequency(){
  //red sensor
  int freq1 = 0;
  int freq2 = 0;
  int freq3 = 0;
  int freq4 = 0;
  int freq5 = 0;
  for(int i = 0; i < 5; i++){
    digitalWrite(S2,LOW);
    digitalWrite(S3,LOW);
    int currentRedFrequency = pulseIn(sensorOut, LOW);
    switch(i){
      case 1: freq1 = currentRedFrequency;
      break;
      case 2: freq2 = currentRedFrequency;
      break;
      case 3: freq3 = currentRedFrequency;
      break;
      case 4: freq4 = currentRedFrequency;
      break;
      case 5: freq5 = currentRedFrequency;
      break;
    }
  }
  return round((freq1 + freq2 + freq3 + freq4 + freq5)/5);  
}

//MQTT/WIFI FUNCTIONS
void connect() {
  Serial.print("Trying to connect to WiFi...");
  while ( status != WL_CONNECTED) {
    status = WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnected to WiFi!\n");

  client.begin(mqttServer, mqttServerPort, net);

  Serial.println("Trying to connect to broker...");
  while (!client.connect(device, key, secret)) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("Connected to MQTT broker!");

  client.onMessage(messageReceived);

  client.subscribe("/color");
}
//Feedback message
void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
}
