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
const char* server = "192.168.1.15";
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
  mqttClient.setServer(server,1883);
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

  if (length > 0){
  String value = "";
  for (int i=0;i<length;i++) {
      value += (char)payload[i];
  }

  int amount[5], r=0, t=0;
  
  for (int i=0; i < value.length(); i++)
  { 
   if(value.charAt(i) == ';') 
    { 
     amount[t] = value.substring(r, i).toInt(); 
     r=(i+1); 
      t++; 
    }
  }
  

  int rAmount = amount[0];
  int oAmount = amount[1];
  int yAmount = amount[2];
  int gAmount = amount[3];
  int pAmount = amount[4];
 
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
