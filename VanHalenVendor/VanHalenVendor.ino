#include <UIPEthernet.h>
#include <MQTT.h>
#include <Servo.h>


#define INTERVAL        3000 // 3 sec delay between publishing

// Servo settings
const int redServoPin     = 3;
const int orangeServoPin  = 4;
const int yellowServoPin  = 5;
const int greenServoPin   = 6;
const int purpleServoPin  = 7;

Servo red;
Servo orange;
Servo yellow;
Servo green;
Servo purple;

// Servo wiring
// Orange Signal
// Red    5V
// Brown  GND

//MQTT settings
const char channelName[] = "VanHalen/Vending";
const char CLIENT_ID[] = "VanHalenVendor";
uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};
EthernetClient ethClient;
MQTTClient mqttClient;
const char mqttServer[] = "192.168.1.15";
const int mqttServerPort = 1883;

void setup() {
  // setup serial communication
  Serial.begin(115200);

  // set servo pins for each color
  red.attach(redServoPin);
  orange.attach(orangeServoPin);
  yellow.attach(yellowServoPin);
  green.attach(greenServoPin);
  purple.attach(purpleServoPin);
  
  // setup ethernet communication using DHCP
  if(Ethernet.begin(mac) == 0) {
    Serial.println(F("Ethernet configuration using DHCP failed"));
    for(;;);
  }

  mqttClient.begin(mqttServer, mqttServerPort, ethClient);
  mqttClient.onMessage(messageReceived);
  
  connect();
}

void loop() {
  mqttClient.loop();

  if (!mqttClient.connected()) {
    connect();
  }
  delay(1000);
}

// callback function to use incoming MQTT messages
void callback(char* topic, byte* payload, unsigned int length) {

  int rAmount = int((char)payload[0]) - 48;
  int oAmount = int((char)payload[1]) - 48;
  int yAmount = int((char)payload[2]) - 48;
  int gAmount = int((char)payload[3]) - 48;
  int pAmount = int((char)payload[4]) - 48;
 
  Serial.println("Dropping Skittles");
  Serial.print("Red: ");
  Serial.println(rAmount);
  Serial.print("Orange: ");
  Serial.println(oAmount);
  Serial.print("Yellow: ");
  Serial.println(yAmount);
  Serial.print("Green: ");
  Serial.println(gAmount);
  Serial.print("Purple: ");
  Serial.println(pAmount);
  Serial.println("-----------------------------");
  
  dropSkittles(red,     rAmount);
  dropSkittles(orange,  oAmount);
  dropSkittles(yellow,  yAmount);
  dropSkittles(green,   gAmount);
  dropSkittles(purple,  pAmount);

  Serial.println("DONE");
  Serial.println("-----------------------------");
}

void connect() {
  
  Serial.println("Trying to connect to broker...");
  while (!mqttClient.connect(CLIENT_ID)) {
    Serial.print(".");
    delay(1000);
  }
  
  Serial.println("Connected to MQTT broker!");

  mqttClient.subscribe("VanHalen/Vending");
  Serial.println("Connected to " + String(channelName));
  
}

void dropSkittles(Servo color, int amount){

  for (int i=0;i<amount;i++){
  delay(500);
  color.write(80);
  delay(500);
  color.write(90);  
  }
  }

void messageReceived(String &topic, String &payload) {
  Serial.println("test");
  Serial.println(topic);
  Serial.println(payload);
}
  
