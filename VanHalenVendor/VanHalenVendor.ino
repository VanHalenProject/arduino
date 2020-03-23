#include <UIPEthernet.h>
#include <PubSubClient.h>
#include <Servo.h>

#define CLIENT_ID       "UnoMQTT"
#define INTERVAL        3000 // 3 sec delay between publishing

// Servo settings
int redServoPin     = 3;
int orangeServoPin  = 4;
int yellowServoPin  = 5;
int greenServoPin   = 6;
int purpleServoPin  = 7;

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
const char* channelName = "VanHalen/Vending";
uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};
EthernetClient ethClient;
PubSubClient mqttClient;
long lastReconnectAttempt = 0;

void setup() {
  // setup serial communication
  Serial.begin(9600);

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
  // setup mqtt client
  mqttClient.setClient(ethClient);
  mqttClient.setServer("192.168.1.15",1883);
  mqttClient.setCallback(callback);
  Serial.println(F("MQTT client configured"));

  lastReconnectAttempt = 0;
}

void loop() {
  if (!mqttClient.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) { // Try to reconnect.
      lastReconnectAttempt = now;
      if (reconnect()) { // Attempt to reconnect.
        lastReconnectAttempt = 0;
      }
    }
  } else { // Connected.
    mqttClient.loop();
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

boolean reconnect() {
  if (mqttClient.connect(CLIENT_ID)) {
    mqttClient.subscribe(channelName); // Subscribe to channel.
  }
  return mqttClient.connected();
}

void dropSkittles(Servo color, int amount){

  for (int i=0;i<amount;i++){
  delay(500);
  color.write(80);
  delay(500);
  color.write(90);  
  }
  }
