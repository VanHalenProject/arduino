#include <Arduino.h>
#include <WiFi101.h>
#include <MQTT.h>
#include <math.h>
#include <Servo.h>

//TCS230 pins wiring to Arduino
#define S0 4
#define S1 5
#define S2 6
#define S3 7
#define sensorOut 8

//Setting up MQTT/WIFI connection
const char topic[] = "VanHalen/PreSort";

const char WIFI_SSID[] = "XXX";
const char WIFI_PASS[] = "XXX";
const char mqttServer[] = "mqtt.eclipse.org";
const int mqttServerPort = 1883;
const char key[] = "key";
const char secret[] = "secret";
const char device[] = "MKR1000";
int status = WL_IDLE_STATUS;
WiFiClient net;
MQTTClient client;
String startPayload; 

//////MQTT/WIFI functions
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

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
}

//Color ENUM names
enum ColorName{RED, ORANGE, YELLOW, GREEN, PURPLE, EXCESS};
const char* getEnumName(int color){
  switch(color){
    case RED: return "RED";
    case ORANGE: return "ORANGE";
    case YELLOW: return "YELLOW";
    case GREEN: return "GREEN";
    case PURPLE: return "PURPLE";  
  }  
};
///////////////////////////////////////////////////////////////
//CUSTOM CLASSES

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
        Serial.print("  R: ");
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
    const int quantityMax = 12;
    int contPos;
    ColorName contColor;
    
  public:
    Container(int pos, ColorName newColor){
      contPos = pos;
      contColor = newColor;
      quantity = 0;
    }

    bool isEmpty(){
      return quantity == 0;
    }

    bool isFull(){
      return quantity >= quantityMax;   
    }

    void upQty(){
      quantity++;
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
      return getEnumName(contColor);
    }

};

//SERVO setup
Servo servo1;
Servo servo2;

class ServoController
{
  private:
    Servo myServo;
    const int homePos = 0;
    int currentPos = 0;
    int runPos = 90;
    const int moveDelay = 500;
    const int dispensePos = 90;
    const int dispenseDelay = 250;

  public:
    ServoController(Servo newServo){
      myServo = newServo;  
    }

    void runServo(int pos){
      myServo.write(pos); 
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

    bool dispense(){
      myServo.write(dispensePos);
      delay(dispenseDelay);
      if(myServo.read() == dispensePos){
        myServo.write(homePos);
        delay(dispenseDelay);
        if(myServo.read() == homePos){
          return true;
        }else{
          return false;  
        }
      }
    }

};

////////////////////////////////////////////////////////////////////////////////
//Initialize color ranges for detection
ColorRange colors[5] {
  {88,98, 108,116, 29,33, RED},
  {77,86, 99,110, 26,30, ORANGE},
  {72,78, 75,82, 22,26, YELLOW},
  {95,104, 86,94, 25,30, GREEN},
  {102,120, 104,120, 28,36, PURPLE}
};
//Initialize container positions
Container container[5]{
  {5, RED},
  {40, ORANGE},
  {75, YELLOW},
  {110, GREEN},
  {145, PURPLE} 
};
//OVERFLOW CONTAINER AND EMPTY FOR MISDETECTION
Container excessCont(177, EXCESS);

////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);
  
  //Attach servos
  servo1.attach(0); 
  servo2.attach(1);

  // Setting the outputs
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  //Setting the sensorOut as an input
  pinMode(sensorOut, INPUT);

  //Frequency scaling to 2%
  digitalWrite(S0,LOW);
  digitalWrite(S1,HIGH);

};
//////////////////////////////////////////////////////////////////////////////////
bool stopFull = false;
bool messageSend = false;
unsigned long idleTime = 0;
unsigned long maxIdleTime = 30000;
RGBController RGBcontr = RGBController();
ServoController servoSort(servo1);
ServoController servoDetect(servo2);

void loop() {
  
  if (!net.connected()) {
    connect();
  }
  client.loop();

 if(!stopFull){
  delay(1000);
    switch(RGBcontr.discernColor(RGBcontr.getMeasuredRGB(), colors)){
      case RED:
        idleTime = 0;
        Serial.println("RED skittle detected");
        if(!container[RED].isFull() && servoSort.runTo(container[RED])){
          servoDetect.dispense();
          container[RED].upQty();
          Serial.print("Amount skittles in container: ");
          Serial.print(container[RED].getQty());
        }else if(servoSort.runTo(excessCont)){
          servoDetect.dispense();
          excessCont.upQty();
          Serial.print("CONTAINER FULL! Skittle moved to EXCESS container - ");
          Serial.print(excessCont.getQty());
        }        
        break;
      case ORANGE:
        idleTime = 0;
        Serial.println("ORANGE skittle detected");
        if(!container[ORANGE].isFull() && servoSort.runTo(container[ORANGE])){
          servoDetect.dispense();
          container[ORANGE].upQty();
          Serial.print("Amount skittles in container: ");
          Serial.print(container[ORANGE].getQty());
        }else if(servoSort.runTo(excessCont)){
          servoDetect.dispense();
          excessCont.upQty();
          Serial.print("CONTAINER FULL! Skittle moved to EXCESS container - ");
          Serial.print(excessCont.getQty());
        } 
        break;
      case YELLOW:
        idleTime = 0;
        Serial.println("YELLOW skittle detected");
        if(!container[YELLOW].isFull() && servoSort.runTo(container[YELLOW])){
          servoDetect.dispense();
          container[YELLOW].upQty();
          Serial.print("Amount skittles in container: ");
          Serial.print(container[YELLOW].getQty());
        }else if(servoSort.runTo(excessCont)){
          servoDetect.dispense();
          excessCont.upQty();
          Serial.print("CONTAINER FULL! Skittle moved to EXCESS container - ");
          Serial.print(excessCont.getQty());
        }
        break;
      case GREEN:
        idleTime = 0;
        Serial.println("GREEN skittle detected");
        if(!container[GREEN].isFull() && servoSort.runTo(container[GREEN])){
          servoDetect.dispense();
          container[GREEN].upQty();
          Serial.print("Amount skittles in container: ");
          Serial.print(container[GREEN].getQty());
        }else if(servoSort.runTo(excessCont)){
          servoDetect.dispense();
          excessCont.upQty();
          Serial.print("\nGREEN skittle detected. CONTAINER FULL! Skittle moved to EXCESS container - ");
          Serial.print(excessCont.getQty());
        }      
        break;
      case PURPLE:
        Serial.println("PURPLE skittle detected");
        idleTime = 0;
        if(!container[PURPLE].isFull() && servoSort.runTo(container[PURPLE])){
          servoDetect.dispense();
          container[PURPLE].upQty();
          Serial.print("Amount skittles in container: ");
          Serial.print(container[PURPLE].getQty());
        }else if(servoSort.runTo(excessCont)){
          servoDetect.dispense();
          excessCont.upQty();
          Serial.print("CONTAINER FULL! Skittle moved to EXCESS container - ");
          Serial.print(excessCont.getQty());
        }               
        break;
      default:
        Serial.print("Color not recognized.");
        if(servoSort.runTo(excessCont)){
          servoDetect.dispense();
        }
     
        if(idleTime == 0){
          idleTime = millis();       
        }
        if((millis() - idleTime) > maxIdleTime){
           Serial.println("\nMax. idle time exceeded! Pre-sorting finished.");
           stopFull = true;
        }
    }
}
//////////CHECK WHETHER ALL CONTAINERS FULL////////////
int contFull = 0;
 for(int i = 0; i <= 4; i++){
   if(container[i].isFull()){
      contFull++;
   }
 }
 if(contFull >= sizeof(container)){
    stopFull = true;
    Serial.println("\nAll containers FULL! Pre-sorting finished.");
 }
 
///SEND RESULTS MQTT
if (stopFull && (!messageSend)){
   servo1.detach();
   servo2.detach();
   char message[10];
   sprintf(message, "%i;%i;%i;%i;%i;", container[RED].getQty(), container[ORANGE].getQty(), container[YELLOW].getQty(), container[GREEN].getQty(), container[PURPLE].getQty());
   client.publish(topic, message, true, 2); 
   messageSend = true;    
}
};
