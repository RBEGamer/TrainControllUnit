/*
  Modbus-Arduino Example - Master Modbus IP Client (ESP8266/ESP32)
  Read Holding Register from Server device

  (c)2018 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
*/

#ifdef ESP8266
 #include <ESP8266WiFi.h>
#else
 #include <WiFi.h>
#endif
#include <PubSubClient.h>
#include <Wire.h>

#define MQTT_TOPIC_SEND_EVENT "remote_control_event"



//TIMER VARIABLES
const long gauge_interval =300;
unsigned long gauge_previousMillis = 0;
const long slider_interval = 200;
unsigned long slider_previousMillis = 0;
const long btn_interval = 50;
unsigned long btn_previousMillis = 0;

//CHACHE VARIABLES
uint16_t get_velocity_level = 0;
uint16_t get_break_level = 0;
uint16_t set_velocity_level = 0;
uint16_t set_break_level = 0;
uint16_t set_break_level_conv = 0;
//WIFI CONFIG 
char ssid[] = "emma2019";                     
char pass[] = "emma2019";

//MODBUS SERVER CONFIG
IPAddress remote(192, 168, 1, 43);  // Address of Modbus Slave device

WiFiClient espClient;
PubSubClient mqtt_client(espClient);



//I2C BUS SLAVE ADDR CONFIG
#define I2C_ADDR_SLIDER 5
#define I2C_ADDR_GAUGE 4

//BTN PINS ASSIGNMENT
const int BTN_EMG_STOP = 15;
const int BTN_ALARM =4;

const int STATELEN_SLIDER = 4;
int i2cstate_slider[STATELEN_SLIDER] = {0, 0, 0, 0};

void read_i2c_slider(){
   Wire.requestFrom(I2C_ADDR_SLIDER, 4);    // request 6 bytes from slave device #8
  int cc = 0;
  while (Wire.available()) { // slave may send less than requested 
    int c = Wire.read(); // receive a byte as character
    i2cstate_slider[cc] = c;
    cc++;
    if(cc >= STATELEN_SLIDER){
      break;
    }
  }
  set_break_level = i2cstate_slider[0];
  set_velocity_level = i2cstate_slider[1];
  }

  

void set_gauge(int _vel, int _brk){
  Wire.beginTransmission(I2C_ADDR_GAUGE); // transmit to device #44 (0x2c)                             // device address is specified in datasheet
  Wire.write(_vel);             // sends value byte  
  Wire.write(_brk);             // sends value byte  
  Wire.endTransmission();     // stop transmitting
  }

  

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt_client.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      
      // ... and resubscribe
      //mqtt_client.subscribe("remote_control_setpoint");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() {
  Serial.begin(9600);
  Wire.begin();
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  
  pinMode(BTN_EMG_STOP, INPUT_PULLUP);
  pinMode(BTN_ALARM, INPUT_PULLUP);

  
  mqtt_client.setServer(remote, 1883);
  mqtt_client.setCallback(callback);
  
}


int last_set_velocity_level = -1;
int last_set_break_level = -1;


bool gauges_updates = true; //PERFOMRS A INITIAL UPDATE

void loop() {

    if (!mqtt_client.connected()) {
    reconnect();
  }
  mqtt_client.loop();

   
 


  //GAUGE UPDATE LOOP
  unsigned long gauge_currentMillis = millis();


  
   unsigned long slider_currentMillis = millis();
  if (slider_currentMillis - slider_previousMillis >= slider_interval){        
          slider_previousMillis = slider_currentMillis;
          read_i2c_slider();
          Serial.printf("set_velocity_level %i set_break_level %i",set_velocity_level,set_break_level);
          Serial.println();
          if(set_break_level <= 100 && set_break_level >= 0){
          set_break_level_conv = set_break_level;
          if(set_break_level_conv > 80){set_break_level_conv = 4;}
          else if(set_break_level_conv > 50){set_break_level_conv = 3;}
          else if(set_break_level_conv > 30){set_break_level_conv = 2;}
          else if(set_break_level_conv >15){set_break_level_conv = 1;}
          else if(set_break_level_conv <= 15){set_break_level_conv = 0;}

          if(set_velocity_level >= 255){
            }
          
          if(set_break_level_conv != last_set_break_level){
            last_set_break_level = set_break_level_conv;
            String tmp = "{\"breaklevel\":"+String(set_break_level_conv)+"}";//,\"velocity\":"+String(set_velocity_level)+"}";
            int ssid_len = tmp.length() + 1;
            char ssid_array[ssid_len];
            tmp.toCharArray(ssid_array, ssid_len);
            mqtt_client.publish(MQTT_TOPIC_SEND_EVENT, ssid_array);
          }
          }
          if(set_velocity_level <= 100 && set_velocity_level >= 0){
           
          set_velocity_level = map(set_velocity_level,0,100,0,50);
          set_velocity_level = map(set_velocity_level,0,50,0,100);
          if(set_velocity_level != last_set_velocity_level){
            last_set_velocity_level = set_velocity_level;
            String tmp = "{\"velocity\":"+String(set_velocity_level)+"}";//,\"velocity\":"+String(set_velocity_level)+"}";
            int ssid_len = tmp.length() + 1;
            char ssid_array[ssid_len];
            tmp.toCharArray(ssid_array, ssid_len);
            mqtt_client.publish(MQTT_TOPIC_SEND_EVENT, ssid_array);
          }
          }

  }


   unsigned long btn_currentMillis = millis();
  if (btn_currentMillis - btn_previousMillis >= btn_interval){        
          btn_previousMillis = btn_currentMillis;
          
       //   if(!mb.writeHreg(remote, MB_REGISTER_EMGSTOP, digitalRead(BTN_EMG_STOP))){
       //     Serial.println("MB_REGISTER_BRKLVL SET FAILED");
       //   }
       //   if(!mb.writeHreg(remote, MB_REGISTER_ALARM, digitalRead(BTN_ALARM))){
       //     Serial.println("MB_REGISTER_VEL SET FAILED");
       //   }
  }
  

  
 
  delay(50);                     // Pulling interval
  
}
