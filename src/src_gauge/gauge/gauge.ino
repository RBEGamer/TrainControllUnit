#include <Wire.h>
#include <CheapStepper.h>


// and connect pins 8,9,10,11 to IN1,IN2,IN3,IN4 on ULN2003 board
CheapStepper stepper_left (8,9,10,11);  //LEFT
CheapStepper stepper_right (7,6,5,4);  //RIGHZ


int target_i2c_degree_left = 0;
int target_i2c_degree_right = 0;


int before_set_right = 0;

int cmd_read = -1;
int cmd_counter =0;
const int CMDLEN = 2;
int i2ccmd[CMDLEN] = {0, 0}; //LEFT RIGHT

const int ZERO_POINT_LEFT_CAL = 120;
const int ZERO_POINT_RIGHT_CAL = 120;
const int ZERO_POINT_LEFT_MAX = 2.2*ZERO_POINT_LEFT_CAL;
const int ZERO_POINT_RIGHT_MAX = 2.2*ZERO_POINT_RIGHT_CAL;
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
  Serial.println("i1c");
  Serial.println(target_i2c_degree_left);
  target_i2c_degree_left = i2ccmd[0];
  target_i2c_degree_right  = i2ccmd[1];
  target_i2c_degree_left = map(target_i2c_degree_left, 0, 100, 0, 270);
  target_i2c_degree_right = map(target_i2c_degree_right, 0, 100, 0, 270);
  
}


int curr_deg_left = 0;
int target_degree_left = 0;
bool dir_left = false;
void move_left(int dg){
  
   if(dg < 0){dg = 0;}else if(dg > 270){dg = 270;}
   
   target_degree_left =  map(dg,0,270,0,ZERO_POINT_LEFT_MAX);
   if(target_degree_left == curr_deg_left){return;}
   dir_left = false;
  if(curr_deg_left < target_degree_left){
    dir_left = true;
  }
  stepper_left.moveDegrees (dir_left,abs(curr_deg_left - target_degree_left));
  curr_deg_left = target_degree_left;
  }

int curr_deg_right = 0;
int target_degree_right = 0;
bool dir_right = false;
void move_right(int dg){
  
   if(dg < 0){dg = 0;}else if(dg > 270){dg = 270;}
   
   target_degree_right =  map(dg,0,270,0,ZERO_POINT_RIGHT_MAX);
   if(target_degree_right == curr_deg_right){return;}
   dir_right = false;
  if(curr_deg_right < target_degree_right){
    dir_right = true;
  }
  stepper_right.moveDegrees (dir_right,abs(curr_deg_right - target_degree_right));
  curr_deg_right = target_degree_right;
  }


const int HALL_PIN_LEFT = 2;
const int HALL_PIN_RIGHT = 3;



void setup() {

delay(2000);
pinMode(HALL_PIN_LEFT,INPUT_PULLUP);
pinMode(HALL_PIN_RIGHT,INPUT_PULLUP);
Wire.begin(4);                // join i2c bus with address #4
Wire.onReceive(receiveEvent); // register event

  // let's run the stepper at 12rpm (if using 5V power) - the default is ~16 rpm

  stepper_left.setRpm(5);
  stepper_right.setRpm(5);

  // let's print out the RPM to make sure the setting worked
  
  Serial.begin(9600);
  Serial.println();

  // and let's print the delay time (in microseconds) between each step
  // the delay is based on the RPM setting:
  // it's how long the stepper will wait before each step

  //Serial.print("stepper delay (micros): "); Serial.print(stepper.getDelay());
  Serial.println(); Serial.println();

 

  //MOVE HOME
  int c = 0;
  while(digitalRead(HALL_PIN_LEFT)){
     stepper_left.move(1,10);
     c++;
     if(c > 4076*2){
     break;
     }
      delay(1);
  }

    //MOVE HOME
  c = 0;
  while(digitalRead(HALL_PIN_RIGHT)){
     stepper_right.move(1,10);
     c++;
     if(c > 4076*2){
     break;
     }
      delay(1);
  }

  stepper_left.moveDegrees (0,ZERO_POINT_LEFT_CAL);
  stepper_right.moveDegrees (0,ZERO_POINT_RIGHT_CAL);
  stepper_left.set4076StepMode();
  stepper_right.set4076StepMode();

  
}

void loop() {


  stepper_left.run();
  stepper_right.run();

move_right(target_i2c_degree_left);
move_left(target_i2c_degree_right);
  
 Serial.println("TL:" + String(target_i2c_degree_left)+" TR:" + String(target_i2c_degree_right));

 delay(100);
}
