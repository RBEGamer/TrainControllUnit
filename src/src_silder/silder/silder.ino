
#include <Wire.h>
#include <PID_v1.h>

#define PIN_INPUT_VEL A0 //analog input for velocity slider
#define PIN_INPUT_BRK A1 //analog input for break slider

#define PIN_OUTPUT_B 5
#define PIN_OUTPUT_A 3

#define PIN_OUTPUT_B_BRK 9
#define PIN_OUTPUT_A_BRK 5
//Define Variables we'll be connecting to
double Setpoint_VEL,Setpoint_BRK, Input_VEL,Input2_VEL,Input_BRK,Input2_BRK, Output_VEL,Output_BRK;

//Specify the links and initial tuning parameters
double Kp=10, Ki=4.1, Kd=1.1;
PID myPID(&Input_VEL, &Output_VEL, &Setpoint_VEL, Kp, Ki, Kd, DIRECT);
//PID myPID2(&Input2_VEL, &Output_VEL, &Setpoint_VEL, Kp, Ki, Kd, DIRECT);

//PID myPID_BRK(&Input_BRK, &Output_BRK, &Setpoint_BRK, Kp, Ki, Kd, DIRECT);
//PID myPID2_BRK(&Input2_BRK, &Output_BRK, &Setpoint_BRK, Kp, Ki, Kd, DIRECT);


int hapitc_feedback_strengh = 200;
int current_haptic_feedback = 100;

void hf_off(){
  current_haptic_feedback = 1;
  myPID.SetOutputLimits(0, current_haptic_feedback);

  }

  void hf_max(){
  current_haptic_feedback = 255;
  myPID.SetOutputLimits(0, current_haptic_feedback);

  }

  void hf_normal(){
  current_haptic_feedback = 180;
  myPID.SetOutputLimits(-current_haptic_feedback, current_haptic_feedback);

  }


int hf_mode = 0;

unsigned long startMillis;
unsigned long led_blink_milis = 0;
bool led_blink_state = false;


int cmd_read = -1;
int cmd_counter =0;
const int CMDLEN = 2;
int i2ccmd[CMDLEN] = {0, 0}; //LEFT RIGHT

void receiveEvent(int howMany)
{
  cmd_counter = 0;
  while (Wire.available() < 1)
  {
  }
  while (Wire.available())
  {
    i2ccmd[cmd_counter] = -1;
    cmd_read = Wire.read();
    
    if (cmd_counter >= CMDLEN)
    {
      break;
    }
    else
    {
      i2ccmd[cmd_counter] = cmd_read;
    }
    cmd_counter++;
  }
  
  Setpoint_VEL = i2ccmd[0];
  Setpoint_BRK  = i2ccmd[1];  
}

const int STATELEN = 4;
int i2cstate[STATELEN] = {0, 0, 0, 0};
void requestEvent()
{
  //speichere alle Ist-Werte in dem Array welches über den I2C Bus gesendet werden sollen
  i2cstate[0] = Input_BRK;
  i2cstate[1] = Input_VEL;
  i2cstate[2] = Setpoint_BRK;
  i2cstate[3] = Setpoint_VEL;
  
  //sende das Array mit allen Achsenwerten zum RaspberryPI zurueck
  for (int i = 0; i < STATELEN; i++)
  {
    Wire.write(i2cstate[i]); // respond with message of 6 bytes
  }
}


void setup()
{

  Serial.begin(9600);
  Wire.begin(5);
 Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent); // was tuen wenn Daten angefragt werden
  //initialize the variables we're linked to
  Setpoint_VEL = 15;
  Setpoint_BRK = 0;


   analogWrite(PIN_OUTPUT_A, 0);
   analogWrite(PIN_OUTPUT_B, 0);
   analogWrite(PIN_OUTPUT_A_BRK, 0);
   analogWrite(PIN_OUTPUT_B_BRK, 0);
   hf_normal();
  //turn the PID on
    myPID.SetMode(AUTOMATIC);
  //  myPID2.SetMode(AUTOMATIC);
  //  myPID_BRK.SetMode(AUTOMATIC);
  //  myPID2_BRK.SetMode(AUTOMATIC);
    startMillis = millis();
}


bool wait_for_reset = false;
void loop()
{




Input_VEL = map(analogRead(PIN_INPUT_VEL), 0, 1024, 0, 100);
Input2_VEL = 100-Input_VEL;
Input_BRK = map(analogRead(PIN_INPUT_BRK), 0, 1024, 0, 100);
Input2_BRK = 100-Input_BRK;


 //    myPID.Compute();
  //analogWrite(PIN_OUTPUT_A, 0);
Serial.println("V" + String(Input_VEL) + " B" + String(Input_BRK));
  if(abs(Output_VEL) > 15 && Setpoint_VEL > 0){ 
  if(Output_VEL > 0){
   analogWrite(PIN_OUTPUT_B, abs(Output_VEL));
   analogWrite(PIN_OUTPUT_A, 0);
  }else{
    analogWrite(PIN_OUTPUT_A, abs(Output_VEL));
    analogWrite(PIN_OUTPUT_B, 0);
    }
  }else{
     analogWrite(PIN_OUTPUT_B, 0);
   analogWrite(PIN_OUTPUT_A, 0);
   }
 

  
 /*
 else if((Setpoint_VEL-Input_VEL)> 0){ 
    myPID.Compute();
   analogWrite(PIN_OUTPUT_A, 0);
   analogWrite(PIN_OUTPUT_B, abs(Output_VEL));
  // Serial.println(">0");
}
else if((Setpoint_VEL-Input_VEL)< 0){ 
    myPID2.Compute();
 //  analogWrite(PIN_OUTPUT_B, 0);
//   analogWrite(PIN_OUTPUT_A, abs(Output_VEL));
//Serial.println("<0");
}  
//  }
*/


}
