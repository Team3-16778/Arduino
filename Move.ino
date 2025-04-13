#include <AccelStepper.h>
#include <ezButton.h>

// X-axis pins
#define MOTORX_STEP 53
#define MOTORX_DIR 51

// Z-axis pins 
#define MOTORZ_STEP 47
#define MOTORZ_DIR 45

// Y-left motor pins
#define MOTORYL_STEP 46
#define MOTORYL_DIR 38

// Y-right motor pins
#define MOTORYR_STEP 52
#define MOTORYR_DIR 40

// Limit switch definitions
ezButton limitSwitch_X_Left(33);
ezButton limitSwitch_X_Right(27);
ezButton limitSwitch_Z_Top(23);
ezButton limitSwitch_Z_Bottom(37);
ezButton limitSwitch_Upper_Right(25);
ezButton limitSwitch_Upper_Left(35);
ezButton limitSwitch_Lower_Right(29);
ezButton limitSwitch_Lower_Left(31);

// Constants
const int stepsPerRev = 200;
const float beltCircumference = 18.57 * 3.14159; // X and Z pulley
const float leadScrewPitch = 4.0;                // Y axis

// Motor definitions
AccelStepper stepperX(AccelStepper::DRIVER, MOTORX_STEP, MOTORX_DIR);
AccelStepper stepperZ(AccelStepper::DRIVER, MOTORZ_STEP, MOTORZ_DIR);
AccelStepper stepperYL(AccelStepper::DRIVER, MOTORYL_STEP, MOTORYL_DIR);
AccelStepper stepperYR(AccelStepper::DRIVER, MOTORYR_STEP, MOTORYR_DIR);

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

// Update limit switches
void updateSwitches() {
  limitSwitch_X_Left.loop();
  limitSwitch_X_Right.loop();
  limitSwitch_Z_Top.loop();
  limitSwitch_Z_Bottom.loop();
  limitSwitch_Upper_Right.loop();
  limitSwitch_Upper_Left.loop();
  limitSwitch_Lower_Right.loop();
  limitSwitch_Lower_Left.loop();
}

// Return true if any limit switch is pressed
bool anySwitchPressed() {
  return limitSwitch_X_Left.isPressed() || limitSwitch_X_Right.isPressed() ||
         limitSwitch_Z_Top.isPressed() || limitSwitch_Z_Bottom.isPressed() ||
         limitSwitch_Upper_Right.isPressed() || limitSwitch_Upper_Left.isPressed() ||
         limitSwitch_Lower_Right.isPressed() || limitSwitch_Lower_Left.isPressed();
}

// Homing routine
void homeAllAxes() {
  stepperX.setMaxSpeed(200);
  stepperZ.setMaxSpeed(200);
  stepperYL.setMaxSpeed(200);
  stepperYR.setMaxSpeed(200);

  stepperX.moveTo(-10000);
  stepperZ.moveTo(-10000);
  stepperYL.moveTo(-10000);
  stepperYR.moveTo(-10000);

  while (true) {
    updateSwitches();
    if (!limitSwitch_X_Left.isPressed()) stepperX.run();
    else stepperX.stop();

    if (!limitSwitch_Z_Bottom.isPressed()) stepperZ.run();
    else stepperZ.stop();

    if (!limitSwitch_Lower_Left.isPressed()) stepperYL.run();
    else stepperYL.stop();

    if (!limitSwitch_Lower_Right.isPressed()) stepperYR.run();
    else stepperYR.stop();

    if (limitSwitch_X_Left.isPressed() && limitSwitch_Z_Bottom.isPressed() &&
        limitSwitch_Lower_Left.isPressed() && limitSwitch_Lower_Right.isPressed()) {
      break;
    }
  }

  stepperX.setCurrentPosition(0);
  stepperZ.setCurrentPosition(0);
  stepperYL.setCurrentPosition(0);
  stepperYR.setCurrentPosition(0);
}

// Modified moveTo3D with safety
void moveTo3D(float x_mm, float y_mm, float z_mm) {
  int32_t stepsX = toSteps_XZ(x_mm);
  int32_t stepsZ = toSteps_XZ(z_mm);
  int32_t stepsY = toSteps_Y(y_mm);

  stepperX.moveTo(stepsX);
  stepperZ.moveTo(stepsZ);
  stepperYL.moveTo(stepsY);
  stepperYR.moveTo(stepsY);

  while (stepperX.distanceToGo() != 0 || stepperZ.distanceToGo() != 0 ||
         stepperYL.distanceToGo() != 0 || stepperYR.distanceToGo() != 0) {

    updateSwitches();
    if (anySwitchPressed()) {
      Serial.println("Safety Stop: Limit switch triggered!");
      stepperX.stop(); stepperZ.stop(); stepperYL.stop(); stepperYR.stop();
      break;
    }

    stepperX.run();
    stepperZ.run();
    stepperYL.run();
    stepperYR.run();
  }
}

void setup() {
  Serial.begin(9600);

  float yRPM = 20000;
  int yAccel = 20000;

  setSpeedRPM(stepperX, 5000);
  setSpeedRPM(stepperZ, 3000);
  setSpeedRPM(stepperYL, yRPM);
  setSpeedRPM(stepperYR, yRPM);

  stepperX.setAcceleration(6000);
  stepperZ.setAcceleration(6000);
  stepperYL.setAcceleration(yAccel);
  stepperYR.setAcceleration(yAccel);

  // Setup limit switches
  limitSwitch_X_Left.setDebounceTime(50);
  limitSwitch_X_Right.setDebounceTime(50);
  limitSwitch_Z_Top.setDebounceTime(50);
  limitSwitch_Z_Bottom.setDebounceTime(50);
  limitSwitch_Upper_Right.setDebounceTime(50);
  limitSwitch_Upper_Left.setDebounceTime(50);
  limitSwitch_Lower_Right.setDebounceTime(50);
  limitSwitch_Lower_Left.setDebounceTime(50);

  homeAllAxes(); // Perform homing at cold start
  delay(2000);
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
  // Format: GOTO X Y Z
  float x = 0, y = 0, z = 0;
  int idx1 = command.indexOf(' ');
  int idx2 = command.indexOf(' ', idx1 + 1);
  int idx3 = command.indexOf(' ', idx2 + 1);

  if (idx1 > 0 && idx2 > idx1 && idx3 > idx2) {
    x = command.substring(idx1 + 1, idx2).toFloat();
    y = command.substring(idx2 + 1, idx3).toFloat();
    z = command.substring(idx3 + 1).toFloat();
    moveTo3D(x, y, z);
  } else {
    Serial.println("Invalid GOTO format. Use: GOTO x y z");
  }
}

void homeGantry() {
  Serial.println("Homing...");
  homeAllAxes();
  Serial.println("Homing done.");
}

void reportPosition() {
  Serial.print("X: ");
  Serial.print(stepperX.currentPosition());
  Serial.print(" | Y: ");
  Serial.print(stepperYL.currentPosition());
  Serial.print(" | Z: ");
  Serial.println(stepperZ.currentPosition());
}

void emergencyStop() {
  Serial.println("Emergency Stop Triggered!");
  stepperX.stop();
  stepperZ.stop();
  stepperYL.stop();
  stepperYR.stop();
}
