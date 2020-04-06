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
const char WIFI_SSID[] = "XXX";
const char WIFI_PASS[] = "XXX";
const char mqttServer[] = "192.168.2.3";
const int mqttServerPort = 1883;
const char key[] = "key";
const char secret[] = "secret";
const char device[] = "mkr1000";
int status = WL_IDLE_STATUS;
WiFiClient net;
MQTTClient client;
unsigned long lastMillis = 0;

//define colors
enum Colors{NONE, RED, GREEN, PURPLE, YELLOW, ORANGE};
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
  //MAIN PROGRAM
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    Serial.println("R: "+String(redSmoothOutFrequency())+"  G: "+String(greenSmoothOutFrequency())+"  B: "+String(blueSmoothOutFrequency()));
    //THIS IS WHERE THE COLOR IS ACTUALLY DETECTED - whatColor() method; EACH CHANNEL IS SMOOTHED OUT (AV. OF 5 CONSECUTIVE MEASUREMENTS) FOR MORE STABLE RESULTS
	client.publish("/color", whatColor(redSmoothOutFrequency(), greenSmoothOutFrequency(), blueSmoothOutFrequency()));
    lastMillis = millis();
  }

};

///////////////////////////////////////////////////////////////////////////////////
const char* whatColor(int redFrequency, int greenFrequency, int blueFrequency){
  Colors tempColor = NONE;

  if((isInRange(56, 60, redFrequency)) && (isInRange(60, 63, greenFrequency)) && (isInRange(17, 20, blueFrequency))){
    tempColor = YELLOW;
  }
  
  if((isInRange(67, 71, redFrequency)) && (isInRange(83, 86, greenFrequency)) && (isInRange(21, 25, blueFrequency))){
    tempColor = RED;
  }
 
  if((isInRange(84, 88, redFrequency)) && (isInRange(84, 88, greenFrequency)) && (isInRange(22, 25, blueFrequency))){
    tempColor = PURPLE;
  }
 
  if((isInRange(75, 80, redFrequency)) && (isInRange(67, 72, greenFrequency)) && (isInRange(20, 23, blueFrequency))){
    tempColor = GREEN;
  }

    if((isInRange(58, 62, redFrequency)) && (isInRange(75, 79, greenFrequency)) && (isInRange(19, 22, blueFrequency))){
    tempColor = ORANGE;
  }
  return getColorName(tempColor);
};

/////////////////TAKING AN AVERAGE FROM 5 MEASUREMENTS TO SMOOTH THE FREQ RESPONSE OUT
int redSmoothOutFrequency(){
  int freq1 = 0;
  int freq2 = 0;
  int freq3 = 0;
  int freq4 = 0;
  int freq5 = 0;
  for(int i = 0; i < 5; i++){
  //READING FROM RED CHANNEL
    digitalWrite(S2,LOW);
    digitalWrite(S3,LOW);
    int currentFrequency = pulseIn(sensorOut, LOW);
    switch(i){
      case 1: freq1 = currentFrequency;
      break;
      case 2: freq2 = currentFrequency;
      break;
      case 3: freq3 = currentFrequency;
      break;
      case 4: freq4 = currentFrequency;
      break;
      case 5: freq5 = currentFrequency;
      break;
    }
  }
  return round((freq1 + freq2 + freq3 + freq4 + freq5)/5);  
}

int greenSmoothOutFrequency(){
  int freq1 = 0;
  int freq2 = 0;
  int freq3 = 0;
  int freq4 = 0;
  int freq5 = 0;
  for(int i = 0; i < 5; i++){
  //READING FROM GREEN CHANNEL
    digitalWrite(S2,HIGH);
    digitalWrite(S3,HIGH);
    int currentFrequency = pulseIn(sensorOut, LOW);
    switch(i){
      case 1: freq1 = currentFrequency;
      break;
      case 2: freq2 = currentFrequency;
      break;
      case 3: freq3 = currentFrequency;
      break;
      case 4: freq4 = currentFrequency;
      break;
      case 5: freq5 = currentFrequency;
      break;
    }
  }
  return round((freq1 + freq2 + freq3 + freq4 + freq5)/5);  
}


int blueSmoothOutFrequency(){
  int freq1 = 0;
  int freq2 = 0;
  int freq3 = 0;
  int freq4 = 0;
  int freq5 = 0;
  for(int i = 0; i < 5; i++){
  //READING FROM BLUE CHANNEL
    digitalWrite(S2,LOW);
    digitalWrite(S3,HIGH);
    int currentFrequency = pulseIn(sensorOut, LOW);
    switch(i){
      case 1: freq1 = currentFrequency;
      break;
      case 2: freq2 = currentFrequency;
      break;
      case 3: freq3 = currentFrequency;
      break;
      case 4: freq4 = currentFrequency;
      break;
      case 5: freq5 = currentFrequency;
      break;
    }
  }
  return round((freq1 + freq2 + freq3 + freq4 + freq5)/5);  
}

//////////////////////////////////////////////////////////////////////////////////////
bool isInRange(int lowValue, int highValue, int targetValue){ 
  boolean inRange = false;
  if(targetValue <= highValue && targetValue >= lowValue){
    inRange = true;
  }
  return inRange;
};

//////////////////////////////////GETTING THE COLOR NAMES OUT OF ENUM
const char* getColorName(enum Colors color) {
   switch (color) 
   {
      case NONE: return "none";
      case RED: return "Red";
      case GREEN: return "Green";
      case PURPLE: return "Purple";
      case YELLOW: return "Yellow";
      case ORANGE: return "Orange";
    
   }
};

/////////////////////////////////MQTT/WIFI CONNECTIONS
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

///////////////////////////////FEEDBACK MESSAGE
void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
}
