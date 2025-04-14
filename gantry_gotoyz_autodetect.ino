#include <AccelStepper.h>
#include <ezButton.h>

// Motor pins
#define MOTORX_STEP 53
#define MOTORX_DIR 51
#define MOTORZ_STEP 47
#define MOTORZ_DIR 45
#define MOTORYL_STEP 46
#define MOTORYL_DIR 38
#define MOTORYR_STEP 52
#define MOTORYR_DIR 40

// Limit switches
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
const float beltCircumference = 18.57 * 3.14159;
const float leadScrewPitch = 4.0;

// Software travel limits
const float MAX_X_MM = 600.0;
const float MAX_Y_MM = 800.0;
const float MAX_Z_MM = 300.0;

// Stepper motors
AccelStepper stepperX(AccelStepper::DRIVER, MOTORX_STEP, MOTORX_DIR);
AccelStepper stepperZ(AccelStepper::DRIVER, MOTORZ_STEP, MOTORZ_DIR);
AccelStepper stepperYL(AccelStepper::DRIVER, MOTORYL_STEP, MOTORYL_DIR);
AccelStepper stepperYR(AccelStepper::DRIVER, MOTORYR_STEP, MOTORYR_DIR);

// Conversion
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

// Limit switch utilities
void updateSwitches() {
  limitSwitch_X_Left.loop(); limitSwitch_X_Right.loop();
  limitSwitch_Z_Top.loop(); limitSwitch_Z_Bottom.loop();
  limitSwitch_Upper_Right.loop(); limitSwitch_Upper_Left.loop();
  limitSwitch_Lower_Right.loop(); limitSwitch_Lower_Left.loop();
}
bool anySwitchPressed() {
  return limitSwitch_X_Left.isPressed() || limitSwitch_X_Right.isPressed() ||
         limitSwitch_Z_Top.isPressed() || limitSwitch_Z_Bottom.isPressed() ||
         limitSwitch_Upper_Right.isPressed() || limitSwitch_Upper_Left.isPressed() ||
         limitSwitch_Lower_Right.isPressed() || limitSwitch_Lower_Left.isPressed();
}

// Homing
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
    if (!limitSwitch_X_Left.isPressed()) stepperX.run(); else stepperX.stop();
    if (!limitSwitch_Z_Bottom.isPressed()) stepperZ.run(); else stepperZ.stop();
    if (!limitSwitch_Lower_Left.isPressed()) stepperYL.run(); else stepperYL.stop();
    if (!limitSwitch_Lower_Right.isPressed()) stepperYR.run(); else stepperYR.stop();

    if (limitSwitch_X_Left.isPressed() && limitSwitch_Z_Bottom.isPressed() &&
        limitSwitch_Lower_Left.isPressed() && limitSwitch_Lower_Right.isPressed()) break;
  }

  stepperX.setCurrentPosition(0);
  stepperZ.setCurrentPosition(0);
  stepperYL.setCurrentPosition(0);
  stepperYR.setCurrentPosition(0);
}

// Y-Z line move
void moveToYZLine(float y_mm, float z_mm) {
  y_mm = constrain(y_mm, 0, MAX_Y_MM);
  z_mm = constrain(z_mm, 0, MAX_Z_MM);

  int32_t y_target = toSteps_Y(y_mm);
  int32_t z_target = toSteps_XZ(z_mm);
  int32_t dy = abs(y_target - stepperYL.currentPosition());
  int32_t dz = abs(z_target - stepperZ.currentPosition());

  float y_speed = 3000;
  float z_speed = (dy == 0) ? 0 : y_speed * ((float)dz / (float)dy);

  stepperYL.setMaxSpeed(y_speed);
  stepperYR.setMaxSpeed(y_speed);
  stepperZ.setMaxSpeed(z_speed);

  stepperYL.moveTo(y_target);
  stepperYR.moveTo(y_target);
  stepperZ.moveTo(z_target);

  while (stepperYL.distanceToGo() != 0 || stepperYR.distanceToGo() != 0 || stepperZ.distanceToGo() != 0) {
    updateSwitches();
    if (anySwitchPressed()) {
      Serial.println("STOP: Limit switch triggered");
      stepperZ.stop(); stepperYL.stop(); stepperYR.stop();
      break;
    }
    stepperZ.run(); stepperYL.run(); stepperYR.run();
  }
}

// Full 3D move
void moveToXYZ(float x_mm, float y_mm, float z_mm) {
  x_mm = constrain(x_mm, 0, MAX_X_MM);
  y_mm = constrain(y_mm, 0, MAX_Y_MM);
  z_mm = constrain(z_mm, 0, MAX_Z_MM);

  stepperX.moveTo(toSteps_XZ(x_mm));
  stepperYL.moveTo(toSteps_Y(y_mm));
  stepperYR.moveTo(toSteps_Y(y_mm));
  stepperZ.moveTo(toSteps_XZ(z_mm));

  while (stepperX.distanceToGo() != 0 || stepperYL.distanceToGo() != 0 ||
         stepperYR.distanceToGo() != 0 || stepperZ.distanceToGo() != 0) {
    updateSwitches();
    if (anySwitchPressed()) {
      Serial.println("STOP: Limit switch triggered");
      stepperX.stop(); stepperZ.stop(); stepperYL.stop(); stepperYR.stop();
      break;
    }
    stepperX.run(); stepperZ.run(); stepperYL.run(); stepperYR.run();
  }
}

void setup() {
  Serial.begin(9600);
  setSpeedRPM(stepperX, 5000);
  setSpeedRPM(stepperZ, 3000);
  setSpeedRPM(stepperYL, 20000);
  setSpeedRPM(stepperYR, 20000);
  stepperX.setAcceleration(6000);
  stepperZ.setAcceleration(6000);
  stepperYL.setAcceleration(20000);
  stepperYR.setAcceleration(20000);

  // Debounce limit switches
  limitSwitch_X_Left.setDebounceTime(50);
  limitSwitch_X_Right.setDebounceTime(50);
  limitSwitch_Z_Top.setDebounceTime(50);
  limitSwitch_Z_Bottom.setDebounceTime(50);
  limitSwitch_Upper_Right.setDebounceTime(50);
  limitSwitch_Upper_Left.setDebounceTime(50);
  limitSwitch_Lower_Right.setDebounceTime(50);
  limitSwitch_Lower_Left.setDebounceTime(50);

  homeAllAxes();  // Cold start homing
  delay(2000);
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\\n');
    command.trim();

    if (command.startsWith("GOTO")) {
      int idx1 = command.indexOf(' ');
      int idx2 = command.indexOf(' ', idx1 + 1);
      int idx3 = command.indexOf(' ', idx2 + 1);

      if (idx1 > 0 && idx2 > idx1 && idx3 > idx2) {
        float x = command.substring(idx1 + 1, idx2).toFloat();
        float y = command.substring(idx2 + 1, idx3).toFloat();
        float z = command.substring(idx3 + 1).toFloat();

        float currentX = (float)stepperX.currentPosition() * beltCircumference / (stepsPerRev * 2);

        if (abs(currentX - x) < 0.01) {
          moveToYZLine(y, z);  // No X change
        } else {
          moveToXYZ(x, y, z);  // Full 3D move
        }
      }
    }
  }

  stepperX.run(); stepperZ.run(); stepperYL.run(); stepperYR.run();
}
