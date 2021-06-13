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
#include <ModbusIP_ESP8266.h>
#include <Wire.h>

//GAUGE REGISTER
const int MB_REGISTER_KMH =  3; //GET!!
const int MB_REGISTER_KN = 15; //GET!!
//SLIDER REGISTER
const int MB_REGISTER_BRKLVL = 0; //5-100 SET!!
const int MB_REGISTER_VEL = 1; //2*+900 SET!!
//BTN REGGISTER
const int MB_REGISTER_EMGSTOP =28;
const int MB_REGISTER_ALARM = 29;

//TIMER VARIABLES
const long gauge_interval = 500;
unsigned long gauge_previousMillis = 0;
const long slider_interval = 150;
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
char ssid[] = "ProDevMo";                     
char pass[] = "6226054527192856";

//MODBUS SERVER CONFIG
IPAddress remote(192, 168, 178, 24);  // Address of Modbus Slave device
const int remote_port = 5020;
ModbusIP mb;  //ModbusIP object

//I2C BUS SLAVE ADDR CONFIG
#define I2C_ADDR_SLIDER 5
#define I2C_ADDR_GAUGE 4

//BTN PINS ASSIGNMENT
const int BTN_EMG_STOP = 2;
const int BTN_ALARM = 4;

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

  
  mb.client();
  mb.connect(remote,remote_port);           // Try to connect if no connection 

  
}

bool gauges_updates = true; //PERFOMRS A INITIAL UPDATE

void loop() {

   mb.task();                      // Common local Modbus task

   
  if (!mb.isConnected(remote)) {   // Check if connection to Modbus Slave is established
    mb.connect(remote,remote_port);           // Try to connect if no connection 
    return;
  }


  //GAUGE UPDATE LOOP
  unsigned long gauge_currentMillis = millis();
  if (gauge_currentMillis - gauge_previousMillis >= gauge_interval){        
        gauge_previousMillis = gauge_currentMillis;
        //READ MODBUS
        if(mb.readHreg(remote, MB_REGISTER_KMH, &get_velocity_level)){
          gauges_updates = true;
        }
         if(mb.readHreg(remote, MB_REGISTER_KN, &get_break_level)){
          gauges_updates = true;
        }
        if(gauges_updates){
          set_gauge(get_velocity_level,get_break_level);
        }
  }

  
   unsigned long slider_currentMillis = millis();
  if (slider_currentMillis - slider_previousMillis >= slider_interval){        
          slider_previousMillis = slider_currentMillis;
          read_i2c_slider();
          Serial.printf("set_velocity_level %i set_break_level %i",set_velocity_level,set_break_level);
          Serial.println();
          set_break_level_conv = set_break_level;
          if(set_break_level_conv > 80){set_break_level_conv = 4;}
          else if(set_break_level_conv > 50){set_break_level_conv = 3;}
          else if(set_break_level_conv > 30){set_break_level_conv = 2;}
          else if(set_break_level_conv >15){set_break_level_conv = 1;}
          else if(set_break_level_conv <= 15){set_break_level_conv = 0;}
  
          if(!mb.writeHreg(remote, MB_REGISTER_BRKLVL, set_break_level_conv)){
            Serial.println("MB_REGISTER_BRKLVL SET FAILED");
          }
          if(!mb.writeHreg(remote, MB_REGISTER_VEL, (2*set_velocity_level)+900)){
            Serial.println("MB_REGISTER_VEL SET FAILED");
          }
  }


   unsigned long btn_currentMillis = millis();
  if (btn_currentMillis - btn_previousMillis >= btn_interval){        
          btn_previousMillis = btn_currentMillis;
          
          if(!mb.writeHreg(remote, MB_REGISTER_EMGSTOP, digitalRead(BTN_EMG_STOP))){
            Serial.println("MB_REGISTER_BRKLVL SET FAILED");
          }
          if(!mb.writeHreg(remote, MB_REGISTER_ALARM, digitalRead(BTN_ALARM))){
            Serial.println("MB_REGISTER_VEL SET FAILED");
          }
  }
  

  
 
  delay(50);                     // Pulling interval
  
}
