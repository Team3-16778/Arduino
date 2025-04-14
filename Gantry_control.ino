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
ezButton limitSwitch_Lower_Right(25);
ezButton limitSwitch_Lower_Left(35);
ezButton limitSwitch_Upper_Right(28);
ezButton limitSwitch_Upper_Left(32); 
ezButton limitSwitch_Z_Top(23);
ezButton limitSwitch_Z_Bottom(34);
ezButton limitSwitch_X_Left(30);
ezButton limitSwitch_X_Right(27);

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

void homeAllAxes() {
  stepperX.setMaxSpeed(600);
  stepperZ.setMaxSpeed(400);
  stepperYL.setMaxSpeed(800);
  stepperYR.setMaxSpeed(800);

  stepperX.moveTo(-10000);
  stepperZ.moveTo(-10000);
  stepperYL.moveTo(-100000);
  stepperYR.moveTo(-100000);

  bool xHomed = false;
  bool zHomed = false;
  bool yLeftHomed = false;
  bool yRightHomed = false;

  while (true) {
    updateSwitches();

    if (!limitSwitch_X_Left.isPressed()) {
      stepperX.run();
    } else if (!xHomed) {
      Serial.println("Homing Hit: X_Left Limit Switch");
      stepperX.stop();
      xHomed = true;
    }

    if (!limitSwitch_Z_Bottom.isPressed()) {
      stepperZ.run();
    } else if (!zHomed) {
      Serial.println("Homing Hit: Z_Bottom Limit Switch");
      stepperZ.stop();
      zHomed = true;
    }

    if (!limitSwitch_Upper_Left.isPressed()) {
      stepperYL.run();
    } else if (!yLeftHomed) {
      Serial.println("Homing Hit: Y_Upper_Left Limit Switch");
      stepperYL.stop();
      yLeftHomed = true;
    }

    if (!limitSwitch_Upper_Right.isPressed()) {
      stepperYR.run();
    } else if (!yRightHomed) {
      Serial.println("Homing Hit: Y_Upper_Right Limit Switch");
      stepperYR.stop();
      yRightHomed = true;
    }

    if (xHomed && zHomed && yLeftHomed && yRightHomed) {
      break;
    }
  }

  // Reset current positions to 0
  stepperX.setCurrentPosition(0);
  stepperZ.setCurrentPosition(0);
  stepperYL.setCurrentPosition(0);
  stepperYR.setCurrentPosition(0);

  // Move all axes slightly away from switches (e.g. 10 mm up)
  int32_t liftStepsY = toSteps_Y(10.0);    // 10mm up for Y
  int32_t liftStepsZ = toSteps_XZ(10.0);   // 10mm up for Z
  int32_t liftStepsX = toSteps_XZ(10.0);   // Optional: move X a bit too

  stepperX.moveTo(liftStepsX);
  stepperZ.moveTo(liftStepsZ);
  stepperYL.moveTo(liftStepsY);
  stepperYR.moveTo(liftStepsY);

  // Move until reached
  stepperX.runToPosition();
  stepperZ.runToPosition();
  stepperYL.runToPosition();
  stepperYR.runToPosition();

  Serial.println("Post-homing lift completed (10 mm).");
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

void gotoXYZ(float x_mm, float y_mm, float z_mm) {
  Serial.print("GOTO Command Received. Moving to: ");
  Serial.print("X = "); Serial.print(x_mm);
  Serial.print(", Y = "); Serial.print(y_mm);
  Serial.print(", Z = "); Serial.println(z_mm);

  moveTo3D(x_mm, y_mm, z_mm);
}

void inject(float y_mm) {
  // Compute corresponding Z movement using tan(20°)
  float z_mm = y_mm * 0.364;  // tan(20°)

  // Get current position in mm
  float currentY = stepperYL.currentPosition() * (leadScrewPitch / stepsPerRev);
  float currentZ = stepperZ.currentPosition() * (beltCircumference / (stepsPerRev * 2));
  float currentX = stepperX.currentPosition() * (beltCircumference / (stepsPerRev * 2));

  // Calculate target Y and Z positions
  float targetY = currentY + y_mm;
  float targetZ = currentZ + z_mm;

  // ----- Speed Ratio Calculation -----
  float ratioY = abs(y_mm);
  float ratioZ = abs(z_mm);
  float baseSpeed = 6000; // adjust as needed (steps/sec)

  float speedY = baseSpeed;
  float speedZ = (ratioZ / ratioY) * baseSpeed;

  // Set speeds accordingly
  stepperYL.setMaxSpeed(speedY);
  stepperYR.setMaxSpeed(speedY);
  stepperZ.setMaxSpeed(speedZ);

  // Optionally set matching accelerations
  stepperYL.setAcceleration(2000);
  stepperYR.setAcceleration(2000);
  stepperZ.setAcceleration(2000);

  // ---- Perform the move ----
  Serial.print("Injecting to Y: ");
  Serial.print(targetY);
  Serial.print(" mm, Z: ");
  Serial.print(targetZ);
  Serial.println(" mm");

  moveTo3D(currentX, targetY, targetZ);
}





void setup() {
  Serial.begin(9600);

  float yRPM = 8000;
  int yAccel = 2000;

  setSpeedRPM(stepperX, 800);
  setSpeedRPM(stepperZ, 600);
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


void loop() {
 
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();  // Remove any whitespace or newline

    if (input == "injectA") {
      Serial.println("Command: injectA (Injecting 50mm)");
      inject(50);
    } else if (input == "injectB") {
      Serial.println("Command: injectB (Retracting 50mm)");
      inject(-50);
    } else if (input.startsWith("Goto")) {
      // Parse the command, expecting: gotoXYZ x y z
      float x_mm, y_mm, z_mm;
      int firstSpace = input.indexOf(' ');
      int secondSpace = input.indexOf(' ', firstSpace + 1);
      int thirdSpace = input.indexOf(' ', secondSpace + 1);

      if (firstSpace > 0 && secondSpace > 0 && thirdSpace > 0) {
        x_mm = input.substring(firstSpace + 1, secondSpace).toFloat();
        y_mm = input.substring(secondSpace + 1, thirdSpace).toFloat();
        z_mm = input.substring(thirdSpace + 1).toFloat();

        Serial.println("Parsed GOTO command.");
        gotoXYZ(x_mm, y_mm, z_mm);
      } else {
        Serial.println("Invalid gotoXYZ format. Use: gotoXYZ x y z");
      }
    } else {
      Serial.print("Unknown command: ");
      Serial.println(input);
    }
  }
}





