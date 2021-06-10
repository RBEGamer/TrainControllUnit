#include <Wire.h>
#include <CheapStepper.h>


// and connect pins 8,9,10,11 to IN1,IN2,IN3,IN4 on ULN2003 board
CheapStepper stepper_left (8,9,10,11);  //LEFT
CheapStepper stepper_right (7,6,5,4);  //RIGHZ


int target_i2c_degree_left = 0;
int target_i2c_degree_right = 0;

int target_left = 0;
int target_right = 0;
//CURRENT DEGREE
int current_left = 0;
int current_right = 0;
int dir_left = 0;
int dir_right = 0;
int before_set_left = 0;
int before_set_right = 0;

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
  
  target_i2c_degree_left = i2ccmd[0];
  target_i2c_degree_right  = i2ccmd[1];
  //map(_target_degree, 0, 255, 0, 270);
  
}



void set_target_left(int _target_degree){
//  Serial.println("current_left:" + String(current_left));
 // Serial.println("_target_degree:" + String(_target_degree));
  if(_target_degree > 270){_target_degree = 270;}
  if(_target_degree <= 0){_target_degree = 0;}
  
  int td = _target_degree;//;map(_target_degree, 0, 255, 0, 270);
  int dir = 1;
  if(_target_degree > current_left){
    td = td - current_left;
    dir = 1; //BACKWARD
  }
  else if(_target_degree < current_left){
    td = current_left -td;
    dir = 0; //FORWARD
  }
//  Serial.print("diff: ");
  if(dir){
 //   Serial.print("+");
   }else{
  //  Serial.print("-");
    }
 // Serial.println(td);
  if(td != current_left){
//  stepper_left.newMoveToDegree(dir,td);
  target_left = td;
  dir_left = dir;
  }
}

void set_target_right(int _target_degree){
//  Serial.println("current_left:" + String(current_left));
 // Serial.println("_target_degree:" + String(_target_degree));
  if(_target_degree > 270){_target_degree = 270;}
  if(_target_degree <= 0){_target_degree = 0;}
  
  int td = _target_degree;//;map(_target_degree, 0, 255, 0, 270);
  int dir = 1;
  if(_target_degree > current_right){
    td = td - current_right;
    dir = 1; //BACKWARD
  }
  else if(_target_degree < current_right){
    td = current_right -td;
    dir = 0; //FORWARD
  }
//  Serial.print("diff: ");
  if(dir){
 //   Serial.print("+");
   }else{
  //  Serial.print("-");
    }
 // Serial.println(td);
  if(td != current_right){
//  stepper_left.newMoveToDegree(dir,td);
  target_right = td;
  dir_right = dir;
  }
}

const int HALL_PIN_LEFT = 2;
const int HALL_PIN_RIGHT = 3;


const int ZERO_POINT_LEFT_CAL = 120;
const int ZERO_POINT_RIGHT_CAL = 120;
void setup() {


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

  // now let's set up our first move...
  // let's move a half rotation from the start point

  stepper_left.newMoveToDegree(1,0);
  current_left = 0;


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
  
  //stepper_right.newMoveToDegree(1,0);

  target_left = 0;
  target_right = 0;
  current_left = 0;
  current_right = 0;
  before_set_left = 0;
  before_set_right = 0;

 // set_target_left(30);delay(1000);
 // current_left = target_left;


  
}

void loop() {

 // Serial.println(digitalRead(HALL_PIN_LEFT));
  
  stepper_left.run();
  stepper_right.run();


  


  // if the current move is done...
  
  if (stepper_left.getStepsLeft() <= 0){
    //delay(5000);
    current_left = target_i2c_degree_left;
    //target_i2c_degree_left += 10;
    set_target_left(target_i2c_degree_left);
    if(target_left != 0){
      stepper_left.run();
      stepper_left.moveDegrees (dir_left, target_left);
    }else{
      stepper_left.stop();
      }
     
  }

  if (stepper_right.getStepsLeft() <= 0){
     //delay(5000);
    current_right = target_i2c_degree_right;
    //target_i2c_degree_left += 10;
    set_target_right(target_i2c_degree_right);
    if(target_right != 0){
      stepper_right.run();
      stepper_right.moveDegrees (dir_left, target_right);
    }else{
      stepper_right.stop();
      }
  }

}
