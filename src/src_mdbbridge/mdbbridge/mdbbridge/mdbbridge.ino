#include <Wire.h>
#include <WiFi.h>
#include "ModbusClientTCP.h"


#define I2C_ADDR_SLIDER 5
#define I2C_ADDR_GAUGE 4

#define MB_UPDATE_TOKEN 42

const long gauge_interval = 500;
unsigned long gauge_previousMillis = 0;

char ssid[] = "ProDevMo";                     
char pass[] = "6226054527192856";                


WiFiClient theClient; 
ModbusClientTCP MB(theClient);


void handleData(ModbusMessage response, uint32_t token) 
{

  //IF T MB_UPDATE_TOKEN
  Serial.printf("Response: serverID=%d, FC=%d, Token=%08X, length=%d:\n", response.getServerID(), response.getFunctionCode(), token, response.size());
  for (auto& byte : response) {
    Serial.printf("%02X ", byte);
  }
  Serial.println("");
}

void handleError(Error error, uint32_t token) 
{
  // ModbusError wraps the error code and provides a readable error message for it
  ModbusError me(error);
  Serial.printf("Error response: %02X - %s\n", (int)me, (const char *)me);
}




// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
   Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output


   WiFi.begin(ssid, pass);
  delay(200);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(". ");
    delay(1000);
  }
  IPAddress wIP = WiFi.localIP();
  Serial.printf("WIFi IP address: %u.%u.%u.%u\n", wIP[0], wIP[1], wIP[2], wIP[3]);
  
   MB.onDataHandler(&handleData);
   MB.onErrorHandler(&handleError);
   MB.setTimeout(2000, 200);
   MB.begin();
   MB.setTarget(IPAddress(192, 168, 178, 24), 5020);
}

int get_velocity_level = 0;
int get_break_level = 0;

int set_velocity_level = 0;
int set_break_level = 0;


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


void read_md_gauge(){
Error err = MB.addRequest(MB_UPDATE_TOKEN, 30, READ_HOLD_REGISTER, 0, 30);
  if (err!=SUCCESS) {
    ModbusError e(err);
    Serial.printf("Error creating request: %02X - %s\n", (int)e, (const char *)e);
  }  
}


bool act = false;
// the loop function runs over and over again forever
void loop() {

  //GAUGE UPDATE LOOP
  unsigned long gauge_currentMillis = millis();
  if (gauge_currentMillis - gauge_previousMillis >= gauge_interval){        
        gauge_previousMillis = gauge_currentMillis;
        read_md_gauge();      
        set_gauge(get_velocity_level,get_break_level);
  }

       
  read_i2c_slider();
  

  delay(299);
}
