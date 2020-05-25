#include <Arduino.h>
#include <WiFi101.h>
#include <MQTT.h>
#include <math.h>
//#include <SPI.h>
#include <Servo.h>

//TCS230 pins wiring to Arduino
#define S0 4
#define S1 5
#define S2 6
#define S3 7
#define sensorOut 8

//SERVO setup
Servo servo1;
Servo servo2;
String startMessage = "start";

//Setting up MQTT/WIFI connection
const char WIFI_SSID[] = "PlonaceSkarpetki";
const char WIFI_PASS[] = "NieDlaPsaKielbasa1983";
const char mqttServer[] = "mqtt.eclipse.org";
const int mqttServerPort = 1883;
const char key[] = "key";
const char secret[] = "secret";
const char device[] = "MKR1000";
const char topic[] = "VanHalen/PreSort";

unsigned long lastMillis = 0;

enum ColorName{RED, ORANGE, YELLOW, GREEN, PURPLE};

const char* getColorName(ColorName color){
  switch(color){
    case RED: return "red";
    case ORANGE: return "orange";
    case YELLOW: return "yellow";
    case GREEN: return "green";
    case PURPLE: return "purple";  
  }  
}

/////////////////////////////////////////////////////////////////////////////////////

class MeasuredRGB
{
  private:
   int r,g,b;
   
  public:
   MeasuredRGB(int R, int G, int B){
      r = R;
      g = G;
      b = B;
   }

    int getR(){
      return r;  
    }

    int getG(){
      return g;  
    }

    int getB(){
      return b;  
    }
   
};

///////////////////////////////////////////////////////////////////////////
class ColorRange
{
  private:
    int rMin;
    int rMax;
    int gMin;
    int gMax;
    int bMin;
    int bMax;
    ColorName colorName;
    
  public:
    ColorRange(int RMin, int RMax, int GMin, int GMax, int BMin, int BMax, ColorName newColorName){
      rMin = RMin;
      rMax = RMax;
      gMin = GMin;
      gMax = GMax;
      bMin = BMin;
      bMax = BMax;
      colorName = newColorName;
    }

    int getName(){
      return colorName;  
    }
    
    int getRMin(){
      return rMin;
    }
    
    int getRMax(){
      return rMax;
    }    
    
    int getGMin(){
      return gMin;
    }    
    
    int getGMax(){
      return gMax;
    }    
    
    int getBMin(){
      return bMin;
    }    
    
    int getBMax(){
      return bMax;
    }
};

class RGBController
{
  private:
    int R;
    int G;
    int B;
  
  public:
    MeasuredRGB getMeasuredRGB(){
      R = 0;
      G = 0;
      B = 0;

      for(int i = 0; i < 5; i++){
        //RED = low, low
        digitalWrite(S2,LOW);
        digitalWrite(S3,LOW);
        R += pulseIn(sensorOut, LOW);
        //GREEN = high, high
        digitalWrite(S2,HIGH);
        digitalWrite(S3,HIGH);
        G += pulseIn(sensorOut, LOW);
        //BLUE = low, high
        digitalWrite(S2,LOW);
        digitalWrite(S3,HIGH);
        B += pulseIn(sensorOut, LOW);
      }
      R = round(R/5);
      G = round(G/5);
      B = round(B/5);
      MeasuredRGB rgb(R,G,B);
        Serial.print("R: ");
        Serial.print(rgb.getR());
        Serial.print("    G: ");
        Serial.print(rgb.getG());
        Serial.print("    B: ");
        Serial.print(rgb.getB());
        Serial.print("\n");
      return rgb;
    }

    int discernColor(MeasuredRGB rgb, ColorRange colors[]){
      int i = 0;   
      while (i <= 4){
        if((inRange(colors[i].getRMin(), colors[i].getRMax(), rgb.getR())) && (inRange(colors[i].getGMin(), colors[i].getGMax(), rgb.getG())) && (inRange(colors[i].getBMin(), colors[i].getBMax(), rgb.getB()))){
          return colors[i].getName();
        }else {
          i++;
        }
      }
    }

    bool inRange(int min, int max, int value)
    {
        return ((value-max)*(value-min) <= 0);
    }

};

class Container
{
  private:
    int quantity;
    const int quantityMax = 50;
    int contPos;
    ColorName contColor;
    
  public:
    Container(int pos, ColorName newColor){
      contPos = pos;
      contColor = newColor;
      quantity = 0;
    }

    bool isEmpty(){
      if(quantity == 0){
        return true;
      }else{
        return false;
      }
    }

    bool isFull(){
      if(quantity >= quantityMax){
        return true;
      }else {
        return false;
      }
    }

    int upQty(){
        if(!isFull()){
          quantity++;
          return getQty();
        }else{
          return getQty();
        }
    }

    int getQty(){
      return quantity;
    }

    void resetQty(){
      quantity = 0;
    }

    int getPosition(){
      return contPos;
    }

    const char* getName(){
      return getColorName(contColor);  
    }

};

class ServoController
{
  private:
    Servo myServo;
    const int homePos = 0;
    int currentPos = 0;
    int runPos = 90;
    int moveDelay = 2000;

  public:
    ServoController(Servo newServo){
      myServo = newServo;  
    }
    
    bool runTo(Container container){
      myServo.write(container.getPosition());
      delay(moveDelay);
      if(myServo.read() == container.getPosition()){
        container.upQty();
        return true;
      }else{
        return false;
      }
    }

    bool runHome(){
      myServo.write(homePos);
      delay(moveDelay);
      if(myServo.read() == homePos){
        return true;
      }else{
        return false;  
      }
    }

    bool succeded(){
      if(myServo.read() == runPos){
        return true;
      }else{
        return false;
      }
    }

};
int status = WL_IDLE_STATUS;
WiFiClient net;
MQTTClient client;
String startPayload; 
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

  client.subscribe(topic);
}
//Feedback message
void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
}
////////////////////////////////////////////////////////////////////////////////
ColorRange colors[5] {
  {94,98, 111,116, 29,33, RED},
  {82,86, 104,110, 27,30, ORANGE},
  {72,78, 75,82, 22,26, YELLOW},
  {95,100, 86,90, 25,30, GREEN},
  {102,120, 104,120, 28,36, PURPLE}
};

Container container[5]{
  {1, RED},
  {2, ORANGE},
  {3, YELLOW},
  {4, GREEN},
  {5, PURPLE} 
};

bool initialized = false;
bool sort = false;
RGBController RGBcontr = RGBController();
ServoController servo1contr(servo1);
ServoController servo2contr(servo2);
////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);
  //network.connect();

  //servo1.attach(18); // pin a3(pwm) = 18 on mkr1000
  
  //servo2.attach();

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

void loop() {
  if(!initialized){
    if((servo1contr.runHome()) && (servo2contr.runHome())){
      initialized = true;
    }   
  }
  if (!net.connected()) {
    connect();
  }
  client.loop();

  if(startPayload.equals(startMessage)){
    Serial.print("Start message received!");
    sort = true;
  }
  
//if START command received and servos INITIALIZED do the presorting
  if(sort && initialized){
    Serial.print("Start sorting operation...");
  char message[10];
    switch(RGBcontr.discernColor(RGBcontr.getMeasuredRGB(), colors)){
      
      case RED:
        servo1contr.runTo(container[RED]);    
        sprintf(message, "%s = %i", container[RED].getName(), container[RED].getQty());
        client.publish(topic, message);
        break;
        
      case ORANGE:
        servo1contr.runTo(container[ORANGE]);
        sprintf(message, "%s = %i", container[RED].getName(), container[RED].getQty());
        client.publish(topic, message);
        break;
        
      case YELLOW:
        servo1contr.runTo(container[YELLOW]);
        sprintf(message, "%s = %i", container[ORANGE].getName(), container[ORANGE].getQty());
        client.publish(topic, message);
        break;
        
      case GREEN:
        servo1contr.runTo(container[GREEN]);
        sprintf(message, "%s = %i", container[GREEN].getName(), container[GREEN].getQty());
        client.publish(topic, message);
        break;
        
      case PURPLE:
        servo1contr.runTo(container[PURPLE]);
        sprintf(message, "%s = %i", container[PURPLE].getName(), container[PURPLE].getQty());
        client.publish(topic, message);
        break;
        
      default:
        Serial.print("ERROR(switch): color not recognized.");
    }
  }
  
};
