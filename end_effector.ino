#include <AccelStepper.h>


// Pin assignments (common-cathode style)
const int EN_PIN   = 5;  // EN+ on TB6600
const int DIR_PIN  = 6;  // DIR+ on TB6600
const int STEP_PIN = 7;  // PUL+ on TB6600


// Steps per revolution depends on your microstepping setting.
// For example, 1.8° motor (200 steps/rev) * 1/8 microstep = 1600 steps/rev.
const int STEPS_PER_REV = 1600;


// Create AccelStepper object in DRIVER mode (step/dir).
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);


// We'll switch between position 0 and position STEPS_PER_REV
// to move back and forth one revolution.
void setup() {
  // Enable pin (if your TB6600 is active-high for enable)
 
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);  // or LOW if your module needs EN-LOW = "on"


  // Set max speed and acceleration (tune to your preference).
  stepper.setMaxSpeed(2000);       // steps per second
  stepper.setAcceleration(800);    // steps per second^2


  // Start at position 0.
  stepper.setCurrentPosition(0);
  // First move command: go to STEPS_PER_REV (one revolution forward).
  stepper.moveTo(STEPS_PER_REV);
}


void loop() {
  // This line runs the motor (accelerates/decelerates) until it reaches the target
  stepper.run();


  // Once we get where we’re going, pick the next target
  if (stepper.distanceToGo() == 0) {
    // If we're currently at 0, move to STEPS_PER_REV
    if (stepper.currentPosition() == 0) {
      stepper.moveTo(STEPS_PER_REV);
    }
    // Otherwise, move back to 0
    else {
      stepper.moveTo(0);
    }
  }
}





#include <AccelStepper.h>


// Pin assignments (common-cathode style)
const int EN_PIN   = 5;  // EN+ on TB6600
const int DIR_PIN  = 6;  // DIR+ on TB6600
const int STEP_PIN = 7;  // PUL+ on TB6600


// Steps per revolution depends on your microstepping setting.
// For example, 1.8° motor (200 steps/rev) * 1/8 microstep = 1600 steps/rev.
const int STEPS_PER_REV = 1600;


// Create AccelStepper object in DRIVER mode (step/dir).
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);


// We'll switch between position 0 and position STEPS_PER_REV
// to move back and forth one revolution.
void setup() {
  // Enable pin (if your TB6600 is active-high for enable)
 
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);  // or LOW if your module needs EN-LOW = "on"


  // Set max speed and acceleration (tune to your preference).
  stepper.setMaxSpeed(2000);       // steps per second
  stepper.setAcceleration(800);    // steps per second^2


  // Start at position 0.
  stepper.setCurrentPosition(0);
  // First move command: go to STEPS_PER_REV (one revolution forward).
  stepper.moveTo(STEPS_PER_REV);
}


void loop() {
  // This line runs the motor (accelerates/decelerates) until it reaches the target
  stepper.run();


  // Once we get where we’re going, pick the next target
  if (stepper.distanceToGo() == 0) {
    // If we're currently at 0, move to STEPS_PER_REV
    if (stepper.currentPosition() == 0) {
      stepper.moveTo(STEPS_PER_REV);
    }
    // Otherwise, move back to 0
    else {
      stepper.moveTo(0);
    }
  }
}



