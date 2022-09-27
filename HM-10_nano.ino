/*--------------------------------------------------------
 Definitions
 --------------------------------------------------------*/

#include <Arduino_LSM6DS3.h>

// Define baudrate
#define BAUDRATE 9600

// Length of serial message
#define SERIAL_LENGTH 6

// Pin that pressure sensor is connected to
#define PRESSURE_PIN A0

/*--------------------------------------------------------
 IMU setup
 --------------------------------------------------------*/

// Commands to be outputted to wheelchair
// Command for speed [-3 -> 5]
short wheelchairSpeed = 0;

// Command for turning [0 for no turn, 1 for left turn, 2 for right turn]
short wheelchairTurn = 0;

// Command for autostop [0 for default, 1 for autostop]
short autostop = 0;

/*--------------------------------------------------------
 Serial setup
 --------------------------------------------------------*/
// Data needed to control the motors
struct moveVals {
  short velMagnitude; // 2 bytes
  short velDirection; // 2
  short stopTrue;     // 2
};

// Struct is overlaid onto an array for conversion from 
// the binary serial message
union serialOutput {
  moveVals moveData;
  byte serialMessage[SERIAL_LENGTH];
};

// Working instance of union
serialOutput outputData;

/*---------------------------------------------------------*/

int speedChange = 0;

void setup() {

  // Start serial monitor for feedback
  Serial.begin(BAUDRATE);
  
  // Start HM-10
  Serial1.begin(BAUDRATE);
  
  // Start the IMU
  IMU.begin();

  // set DAC resolution to 10-bit:
  analogWriteResolution(10);
  pinMode(PRESSURE_PIN, INPUT_PULLUP);


}

void loop() {

  

  //if (takeCommandsTrue()) {
  
    findIfTurning();
    findSpeed(); 
    findStop();
    //Serial.println(wheelchairSpeed);
    //Serial.println(wheelchairTurn);
    Serial.println(autostop);
    sendData(wheelchairSpeed, wheelchairTurn, autostop);
  
    if (speedChange == 1){
      delay(1000);
      speedChange = 0;
    }
    else {
      delay(100);
    }
  //}
  
}

/*--------------------------------------------------------
 If the user wishes to give commands, they bite on a
 pressure sensor. This function detects if they are biting
 --------------------------------------------------------*/
int takeCommandsTrue()
{
  // Default to not biting 
  int isBiting = 0;

  // Read in the voltage from the pin
  int level = analogRead(PRESSURE_PIN);
  float pressure = (level * 100) / 1024.0;
  //Serial.println(pressure);

  // If high, pressure sensor is bitten
  if (pressure > 80.0)
  {
    isBiting = 1;
  }
  
  return isBiting;
}

/*--------------------------------------------------------
 Function to send movement commands to the wheelchair via 
 bluetooth serial.
 --------------------------------------------------------*/
void sendData(short velMagnitude, short velDirection, short stopTrue)
{
  // Assign data to struct
  outputData.moveData.velMagnitude = velMagnitude;
  outputData.moveData.velDirection = velDirection;
  outputData.moveData.stopTrue     = stopTrue;

  // Send data to the bluetooth module;
  Serial1.write(outputData.serialMessage, SERIAL_LENGTH); 
}

/*--------------------------------------------------------
 Function to determine if the user is indicating for the 
 wheelchair to turn left or right
 --------------------------------------------------------*/
void findIfTurning()
{
  // Set up floats for accelerometer data used for direction
  float x, y, z;
  int degreesY;
  IMU.readAcceleration(x, y, z);

  //Serial.println(y);
  //Serial.println(x);
  
  // Always have default of wheelchair not turning
  wheelchairTurn = 0;
  
  // Convert to degrees
  if (y > 0.1) {
    y = 100 * y;
    degreesY = map(y, 0, 97, 0, 90);

    // If head is tilted to the right set turn to the right
    if (degreesY > 15) {
      wheelchairTurn = 1;
    }
  }
  if (y < -0.1) {
    y = 100 * y;
    degreesY = map(y, 0, -100, 0, 90);

    if (degreesY > 15) {

      wheelchairTurn = -1;
    }
  }

  if (wheelchairTurn < -1 || wheelchairTurn > 1){

    wheelchairTurn = 0;
  }
}

/*--------------------------------------------------------
 Function to determine if the user is needing to stop quickly
 --------------------------------------------------------*/

void findStop() {

  float xStop, yStop, zStop;
  IMU.readGyroscope(xStop, yStop, zStop);

  if (xStop > 100.0 || xStop < -100.0) {
    wheelchairTurn = 0;
    wheelchairSpeed = 0;
    autostop = 1;
  } 
  else {

    autostop = 0;
  }

  
}

/*--------------------------------------------------------
 Find if the user is indicating to increment or decrement
 the speed.
 --------------------------------------------------------*/
void findSpeed()
{
  // Set up floats for the gyroscope data which is used for speed
  float xGyro, yGyro, zGyro;
  IMU.readGyroscope(xGyro, yGyro, zGyro);
  
  // Forward speeds
  if (yGyro > 75.0) {

    // Increase the wheelchair speed or slow down reverse
    //if (wheelchairSpeed < 5) {
    wheelchairSpeed = min(wheelchairSpeed+1,5);
    speedChange = 1;
    //delay(2000);
    //}
  }
  
  // Situation for reverse speed
  if (yGyro < -75) {

    // Decrease wheelchair speed or reverse faster
    //if (wheelchairSpeed > -3) {

    wheelchairSpeed = max(wheelchairSpeed-1,-3);
    speedChange = 1;
    //delay(2000);
    //}
  }  

  if (wheelchairSpeed < -3 || wheelchairSpeed > 5){

    wheelchairSpeed = 0;
  }

}
