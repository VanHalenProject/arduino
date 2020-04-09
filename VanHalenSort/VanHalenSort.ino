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
const char WIFI_SSID[] = "xxx";               //YOUR WIFI SSID NAME HERE
const char WIFI_PASS[] = "xxx";               //YOUR WIFI PASSWORD HERE
const char mqttServer[] = "mqtt.eclipse.org"; //test.mosquitto.org///mqtt.eclipse.org
const int mqttServerPort = 1883;
const char key[] = "key";
const char secret[] = "secret";
const char device[] = "mkr1000";
int status = WL_IDLE_STATUS;
WiFiClient net;
MQTTClient client;
unsigned long lastMillis = 0;

//define colors
int yellowRange[3][2] = {
    {56, 60},
    {60, 63},
    {17, 20}};

int redRange[3][2] = {
    {67, 71},
    {83, 86},
    {21, 25}};

int purpleRange[3][2] = {
    {84, 88},
    {84, 88},
    {22, 25}};

int greenRange[3][2] = {
    {75, 80},
    {67, 72},
    {20, 23}};

int orangeRange = {
    {58, 62},
    {75, 79},
    {19, 22}};
////////////////////////////////////////////////////////////////////////////////////
void setup()
{
  Serial.begin(9600);
  connect();

  // Setting the outputs
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  // Setting the sensorOut as an input
  pinMode(sensorOut, INPUT);

  //Frequency scaling to 2% - 2% gives good results with Arduino
  digitalWrite(S0, LOW);
  digitalWrite(S1, HIGH);
};
////////////////////////////////////////////////////////////////////////////////
void loop()
{
  client.loop();

  if (!net.connected())
  {
    connect();
  }
  //MAIN PROGRAM
  if (millis() - lastMillis > 1000)
  {
    lastMillis = millis();
    //Serial.println("R: "+String(smoothOutFrequency(low,low))+"  G: "+String(smoothOutFrequency(high,high))+"  B: "+String(smoothOutFrequency(low, high)));
    //THIS IS WHERE THE COLOR IS ACTUALLY DETECTED, EACH CHANNEL IS SMOOTHED OUT (AV. OF 5 CONSECUTIVE MEASUREMENTS) FOR MORE STABLE RESULTS
    client.publish("/VanHalen/color", discernColor(smoothOutFrequency(low, low), smoothOutFrequency(high, high), smoothOutFrequency(low, high)));
    lastMillis = millis();
  }
};

///////////////////////////////////////////////////////////////////////////////////
const char *discernColor(int[] avFreqRGB){};

/////////////////TAKING AN AVERAGE FROM 5 MEASUREMENTS TO SMOOTH THE FREQ RESPONSE OUT
int[] averageFreqRGB()
{
  int R = 0;
  int G = 0;
  int B = 0;

  for (int i = 0; i < 5; i++)
  {
    //RED = low, low
    digitalWrite(S2, LOW);
    digitalWrite(S3, LOW);
    R = R + pulseIn(sensorOut, LOW);
    //GREEN = high, high
    digitalWrite(S2, HIGH);
    digitalWrite(S3, HIGH);
    G = G + pulseIn(sensorOut, LOW);
    //BLUE = low, high
    digitalWrite(S2, LOW);
    digitalWrite(S3, HIGH);
    B = B + pulseIn(sensorOut, LOW);
  };
  R = round(R / 5);
  G = round(G / 5);
  B = round(B / 5);
  return int sumRGB[3] = {R, G, B};
};

/////////////////////////////////MQTT/WIFI CONNECTIONS
void connect()
{
  Serial.print("Trying to connect to WiFi...");
  while (status != WL_CONNECTED)
  {
    status = WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnected to WiFi!\n");

  client.begin(mqttServer, mqttServerPort, net);

  Serial.println("Trying to connect to broker...");
  while (!client.connect(device, key, secret))
  {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("Connected to MQTT broker!");

  client.onMessage(messageReceived);

  client.subscribe("/VanHalen/color");
}

///////////////////////////////FEEDBACK MESSAGE
void messageReceived(String &topic, String &payload)
{
  Serial.println("incoming: " + topic + " - " + payload);
}
