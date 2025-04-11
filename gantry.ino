#include <AccelStepper.h>

// Motor pins
#define MOTORX_STEP 53
#define MOTORX_DIR 51
#define MOTORZ_STEP 47 
#define MOTORZ_DIR 45
#define MOTORYL_STEP 46
#define MOTORYL_DIR 38
#define MOTORYR_STEP 52
#define MOTORYR_DIR 40

// Constants
const int stepsPerRev = 200;
const float beltCircumference = 18.57 * PI; // X and Z pulley
const float leadScrewPitch = 4.0;          // Y axis

// Motor objects
AccelStepper stepperX(AccelStepper::DRIVER, MOTORX_STEP, MOTORX_DIR);
AccelStepper stepperZ(AccelStepper::DRIVER, MOTORZ_STEP, MOTORZ_DIR);
AccelStepper stepperYL(AccelStepper::DRIVER, MOTORYL_STEP, MOTORYL_DIR);
AccelStepper stepperYR(AccelStepper::DRIVER, MOTORYR_STEP, MOTORYR_DIR);

// Current position tracking
float currentX = 0;
float currentY = 0;
float currentZ = 0;

// Conversion functions
int32_t toSteps_XZ(float mm) {
  return static_cast<int32_t>((mm / beltCircumference) * stepsPerRev * 2);
}

int32_t toSteps_Y(float mm) {
  return static_cast<int32_t>((mm / leadScrewPitch) * stepsPerRev);
}

void setSpeedRPM(AccelStepper &stepper, float rpm) {
  float stepsPerSec = (rpm * stepsPerRev) / 60.0;
  stepper.setMaxSpeed(stepsPerSec);
}

void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait for serial connection

  // Configure motor speeds and accelerations
  setSpeedRPM(stepperX, 5000);
  setSpeedRPM(stepperZ, 3000);
  setSpeedRPM(stepperYL, 20000);
  setSpeedRPM(stepperYR, 20000);

  stepperX.setAcceleration(6000);
  stepperZ.setAcceleration(6000);
  stepperYL.setAcceleration(20000);
  stepperYR.setAcceleration(20000);

  // Home the gantry (you'll need to implement homing switches)
  // homeGantry();
  
  Serial.println("READY"); // Signal ready state to Python
}

void loop() {
  // Process serial commands
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command.startsWith("GOTO")) {
      processGotoCommand(command);
    }
    else if (command == "HOME") {
      homeGantry();
    }
    else if (command == "GETPOS") {
      reportPosition();
    }
    else if (command == "STOP") {
      emergencyStop();
    }
  }

  // Run the steppers
  stepperX.run();
  stepperZ.run();
  stepperYL.run();
  stepperYR.run();
}

void processGotoCommand(String command) {
  // Parse "GOTO x y z" command
  command.remove(0, 5); // Remove "GOTO "
  
  float x = command.substring(0, command.indexOf(' ')).toFloat();
  command.remove(0, command.indexOf(' ') + 1);
  
  float y = command.substring(0, command.indexOf(' ')).toFloat();
  command.remove(0, command.indexOf(' ') + 1);
  
  float z = command.toFloat();

  // Convert to steps and move
  stepperX.moveTo(toSteps_XZ(x));
  stepperZ.moveTo(toSteps_XZ(z));
  stepperYL.moveTo(toSteps_Y(y));
  stepperYR.moveTo(toSteps_Y(y));
  
  // Update current position
  currentX = x;
  currentY = y;
  currentZ = z;
  
  Serial.print("MOVING TO: ");
  Serial.print(x);
  Serial.print(" ");
  Serial.print(y);
  Serial.print(" ");
  Serial.println(z);
}

void homeGantry() {
  // Implement homing routine with limit switches
  // This is a placeholder - you'll need to add your homing logic
  Serial.println("HOMING...");
  
  // For now just reset to (0,0,0)
  stepperX.setCurrentPosition(0);
  stepperZ.setCurrentPosition(0);
  stepperYL.setCurrentPosition(0);
  stepperYR.setCurrentPosition(0);
  
  currentX = 0;
  currentY = 0;
  currentZ = 0;
  
  Serial.println("HOMED");
}

void reportPosition() {
  Serial.print("POS: ");
  Serial.print(currentX);
  Serial.print(" ");
  Serial.print(currentY);
  Serial.print(" ");
  Serial.println(currentZ);
}

void emergencyStop() {
  stepperX.stop();
  stepperZ.stop();
  stepperYL.stop();
  stepperYR.stop();
  
  Serial.println("EMERGENCY STOP");
}