#include <AccelStepper.h>

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

void setup() {
  Serial.begin(9600);

  // Y axis specific tuning
  float yRPM =20000;
  int yAccel = 20000;

  setSpeedRPM(stepperX, 5000);
  setSpeedRPM(stepperZ, 3000);
  setSpeedRPM(stepperYL, yRPM);
  setSpeedRPM(stepperYR, yRPM);

  stepperX.setAcceleration(6000);
  stepperZ.setAcceleration(6000);
  stepperYL.setAcceleration(yAccel);
  stepperYR.setAcceleration(yAccel);

  delay(2000);
}

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
    stepperX.run();
    stepperZ.run();
    stepperYL.run();
    stepperYR.run();
  }
}

void loop() {
  moveTo3D(100, 100, 50);
  delay(2000);
  moveTo3D(0, 0, 0);
  delay(2000);
}
