// Library to make a Software UART
#include <SoftwareSerial.h>
#include <AFMotor.h>

// Pin locations
#define RX A4
#define TX A5

#define BAUDRATE 9600

// Length of serial message
#define SERIAL_LENGTH 6

// Code for motor driver
const int ir_pin = A3;
int turnSpeed = 25;

AF_DCMotor left_motor(1);
AF_DCMotor right_motor(2);

// Data needed to control the motors
struct moveVals {
  int velMagnitude; // 2 bytes
  int velDirection; // 2
  int stopTrue;     // 2
};

// Struct is overlaid onto an array for conversion from 
// the binary serial message
union serialInput {
  moveVals moveData;
  byte serialMessage[SERIAL_LENGTH];
};

// Working instance of union
serialInput inputData;

byte serialData[SERIAL_LENGTH];
boolean newData = false;
boolean askForData = true;

// Instantiation of a Software UART
SoftwareSerial BTSerial(RX, TX); // (RX, TX)

void setup() {  

  // Start Serial Monitor for feedback
  Serial.begin(BAUDRATE);
  //while(!Serial);
  
  // Start HM-10
  BTSerial.begin(BAUDRATE);
  //while(!BTSerial);
  
  Serial.println("Receiving data:");
}

void loop() {
  requestData();
  receiveData();
  //moveWheelchair();
  displayData();

  delay(1000);
}

void requestData()
{
  if (askForData) 
  {
    Serial.print("<M>");
    askForData = false;
  }
}

void receiveData() 
{
  if (BTSerial.available() < SERIAL_LENGTH) 
  {
    Serial.println("Error");
  }
  
  for (byte n = 0; n < SERIAL_LENGTH; n++) 
  {
    serialData[n] = BTSerial.read();
  }
  
  for (byte n = 0; n < SERIAL_LENGTH; n++)
  {
    inputData.serialMessage[n] = serialData[n];
  }
  
  newData = true;
}

void displayData()
{
  if (newData == false)
  {
    return;
  }
  Serial.print("<Data: ");
  Serial.print(inputData.moveData.velMagnitude);
  Serial.print(" ");
  Serial.print(inputData.moveData.velDirection);
  Serial.print(" ");
  Serial.print(inputData.moveData.stopTrue);
  Serial.println(">");
  newData = false;
  askForData = true;
}

void moveWheelchair() {

  if (newData == false) {
    return;
  }

  if (inputData.moveData.velMagnitude < 0) {
    int speedLevel = abs(inputData.moveData.velMagnitude);
    driveBackward(speedLevel, inputData.moveData.velDirection);
  }
  else if (inputData.moveData.velMagnitude > 0) {
    driveForward(inputData.moveData.velMagnitude, inputData.moveData.velDirection);
  }
  else if (inputData.moveData.velDirection != 0) {
    turnBot(inputData.moveData.velDirection);
  }
}

// --------------------------------------------------------
// motion functions
//   These just set the correct motor values to drive the bot

void driveForward(int speedLevel, int turn)
{
  int speed = speedLevel * 51;
  left_motor.run(BACKWARD);
  right_motor.run(FORWARD);
  left_motor.setSpeed(min(speed - turn*turnSpeed,255));
  right_motor.setSpeed(min(speed + turn*turnSpeed,255));
}

void driveBackward(int speedLevel, int turn)
{
  int speed = speedLevel * 51;
  left_motor.run(FORWARD);
  right_motor.run(BACKWARD);
  left_motor.setSpeed(min(speed + turn*turnSpeed,255));  //check whether we are doing inverse or normal turning while reversing
  right_motor.setSpeed(min(speed - turn*turnSpeed,255));
}

void turnBot(int turn){

  if (turn == -1){
  
    left_motor.run(FORWARD);
    right_motor.run(FORWARD);
    left_motor.setSpeed(turnSpeed);
    right_motor.setSpeed(turnSpeed);
  }
  else if (turn == 1){
  
    left_motor.run(BACKWARD);
    right_motor.run(BACKWARD);
    left_motor.setSpeed(turnSpeed);
    right_motor.setSpeed(turnSpeed);
  }
}

void stop()
{
  left_motor.setSpeed(0);
  right_motor.setSpeed(0);
  delay(500); //stop for half a second, allow motors to wind down
}
