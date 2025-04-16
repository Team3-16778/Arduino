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


const int stepsPerRev = 200;
const int microstepping = 2;
const float mmPerRev_XZ = 58.33;
const float stepsPerMM_XZ = (stepsPerRev * microstepping) / mmPerRev_XZ;  // ≈ 7.5




const float lead = 8.0;                          // For Y


const float stepsPerMM_Y = (stepsPerRev * microstepping) / lead;                // 50 steps/mm




// Motor definitions
AccelStepper stepperX(AccelStepper::DRIVER, MOTORX_STEP, MOTORX_DIR);
AccelStepper stepperZ(AccelStepper::DRIVER, MOTORZ_STEP, MOTORZ_DIR);
AccelStepper stepperYL(AccelStepper::DRIVER, MOTORYL_STEP, MOTORYL_DIR);
AccelStepper stepperYR(AccelStepper::DRIVER, MOTORYR_STEP, MOTORYR_DIR);


int32_t toSteps_XZ(float mm) {
  return static_cast<int32_t>(mm * stepsPerMM_XZ);
}

int32_t toSteps_Y(float mm) {
  return static_cast<int32_t>(mm * stepsPerMM_Y);
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


void moveYBoth(float y_mm) {
  int32_t ySteps = toSteps_Y(y_mm);
  stepperYL.moveTo(ySteps);
  stepperYR.moveTo(ySteps);

  while (stepperYL.distanceToGo() != 0 || stepperYR.distanceToGo() != 0) {
    updateSwitches();  // Optional: for safety
    stepperYL.run();
    stepperYR.run();
  }
}


void homeAllAxes() {
  Serial.println("Starting Y-axis homing...");
  stepperYL.setMaxSpeed(1600);
  stepperYR.setMaxSpeed(1600);
  stepperYL.moveTo(-100000);
  stepperYR.moveTo(-100000);

  bool yLeftHomed = false;
  bool yRightHomed = false;

  while (true) {
    updateSwitches();

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

    if (yLeftHomed && yRightHomed) {
      break;
    }
  }

  stepperYL.setCurrentPosition(0);
  stepperYR.setCurrentPosition(0);

  // Move Y to 260mm
  moveYBoth(260.0);

  Serial.println("Y-axis homed and moved to 350mm.");

  // --------------------------

  Serial.println("Starting X-axis homing...");
  stepperX.setMaxSpeed(600);
  stepperX.moveTo(-10000);

  bool xHomed = false;

  while (true) {
    updateSwitches();

    if (!limitSwitch_X_Left.isPressed()) {
      stepperX.run();
    } else if (!xHomed) {
      Serial.println("Homing Hit: X_Left Limit Switch");
      stepperX.stop();
      xHomed = true;
    }

    if (xHomed) break;
  }

  stepperX.setCurrentPosition(0);

  // Move X to 230mm
  int32_t xTarget = toSteps_XZ(230.0);
  stepperX.moveTo(xTarget);
  stepperX.runToPosition();
  Serial.println("X-axis homed and moved to 100mm.");

  // --------------------------

  Serial.println("Starting Z-axis homing...");
  stepperZ.setMaxSpeed(300);
  stepperZ.moveTo(-10000);

  bool zHomed = false;

  while (true) {
    updateSwitches();

    if (!limitSwitch_Z_Bottom.isPressed()) {
      stepperZ.run();
    } else if (!zHomed) {
      Serial.println("Homing Hit: Z_Bottom Limit Switch");
      stepperZ.stop();
      zHomed = true;
    }

    if (zHomed) break;
  }

  stepperZ.setCurrentPosition(0);

  // Move Z to 140mm
  int32_t zTarget = toSteps_XZ(140.0);
  stepperZ.moveTo(zTarget);
  stepperZ.runToPosition();
  Serial.println("Z-axis homed and moved to 200mm.");

  Serial.println("Homing complete.");
}


// Modified moveTo3D with safety
void moveTo3D(float x_mm, float y_mm, float z_mm) {
  int32_t stepsX = toSteps_XZ(x_mm);
  int32_t stepsZ = toSteps_XZ(z_mm);
  int32_t stepsY = toSteps_Y(y_mm);

  Serial.println("Moving to 3D position:");
  Serial.print("  Target X (mm): "); Serial.print(x_mm); Serial.print(" → Steps: "); Serial.println(stepsX);
  Serial.print("  Target Y (mm): "); Serial.print(y_mm); Serial.print(" → Steps: "); Serial.println(stepsY);
  Serial.print("  Target Z (mm): "); Serial.print(z_mm); Serial.print(" → Steps: "); Serial.println(stepsZ);

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


void inject(float y_mm) {
  const float angleTan = 0.577;  // tan(30°)
  float z_mm = y_mm * angleTan;

  // ----- Current Position in mm -----
  float currentY = stepperYL.currentPosition() / stepsPerMM_Y;
  float currentZ = stepperZ.currentPosition() / stepsPerMM_XZ;
  float currentX = stepperX.currentPosition() / stepsPerMM_XZ;

  // ----- Target Positions in mm -----
  float targetY = currentY + y_mm;
  float targetZ = currentZ + z_mm;

  // ----- Time Sync -----
  float desiredMoveTime = 0.5;  // seconds
  float y_mm_per_sec = y_mm / desiredMoveTime;
  float z_mm_per_sec = z_mm / desiredMoveTime;

  const float mmPerRev_Y = 8.0;
  const float mmPerRev_Z = 53.33;

  float y_RPM = (y_mm_per_sec / mmPerRev_Y) * 60.0;
  float z_RPM = (z_mm_per_sec / mmPerRev_Z) * 60.0 * 0.8;

  setSpeedRPM(stepperYL, y_RPM);
  setSpeedRPM(stepperYR, y_RPM);
  setSpeedRPM(stepperZ, z_RPM);

  // ----- Acceleration Sync (steps/sec²), scaled for linear distance per step
  const float linearAccel_mm_per_s2 = 900.0;  // target acceleration in mm/s²

  float accelY_steps_per_s2 = linearAccel_mm_per_s2 * stepsPerMM_Y;
  float accelZ_steps_per_s2 = linearAccel_mm_per_s2 * stepsPerMM_XZ * 0.5;

  stepperYL.setAcceleration(accelY_steps_per_s2);
  stepperYR.setAcceleration(accelY_steps_per_s2);
  stepperZ.setAcceleration(accelZ_steps_per_s2);

  // ----- Log -----
  Serial.print("Injecting at 30° to Y: ");
  Serial.print(targetY); Serial.print(" mm, Z: ");
  Serial.print(targetZ); Serial.print(" mm | ");
  Serial.print("Y RPM: "); Serial.print(y_RPM);
  Serial.print(", Z RPM: "); Serial.print(z_RPM);
  Serial.print(" | Accel Y: "); Serial.print(accelY_steps_per_s2);
  Serial.print(", Accel Z: "); Serial.println(accelZ_steps_per_s2);

  // ----- Move -----
  moveTo3D(currentX, targetY, targetZ);
}


void handleSerialCommands() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    Serial.print("RECV: ");
    Serial.println(command);

    if (command.startsWith("GOTO")) {
      Serial.println("ACK: Gantry position command received");

      // Inline parsing logic from processGotoCommand
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
        Serial.println("ERR: Invalid GOTO format. Use: GOTO x y z");
      }
    }
    else if (command.startsWith("ROTATE")) {
      Serial.println("ACK: End effector rotation command received");
      // Add logic if needed
    }
    else if (command == "HOME") {
      Serial.println("ACK: Homing command received");
      homeAllAxes();
    }
    else if (command == "GETPOS") {
      Serial.println("ACK: Sending position");
      reportPosition();
    }
    else if (command == "GETANGLES") {
      Serial.println("ACK: Sending simulated angles: Theta45.0 Delta-15.0");
    }
    else if (command == "STOP") {
      Serial.println("ACK: Emergency stop activated");
      emergencyStop();
    }
    else if (command == "INJECTA") {
      Serial.println("ACK: Inject A command executed");
      inject(866);
    }
    else if (command == "INJECT") {
      Serial.println("ACK: Inject command executed");
      inject(26);  // Replace with injectA() if needed
    }
    else if (command == "INJECTC") {
      Serial.println("ACK: Inject C (retraction) command executed");
      inject(-26);  // Replace with injectB() if needed
    }
    else {
      Serial.println("ERR: Unknown command format");
    }
  }
}



void setup() {
  Serial.begin(9600);

  float yRPM = 8000;
  int yAccel = 2000;

  setSpeedRPM(stepperX, 800);
  setSpeedRPM(stepperZ, 600);
  setSpeedRPM(stepperYL, yRPM);
  setSpeedRPM(stepperYR, yRPM);

  stepperX.setAcceleration(800);
  stepperZ.setAcceleration(600);
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

  stepperX.setCurrentPosition(0);
  stepperZ.setCurrentPosition(0);
  stepperYL.setCurrentPosition(0);
  stepperYR.setCurrentPosition(0);



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
  handleSerialCommands();  // Always listen for commands

}
