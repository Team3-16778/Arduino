#include <Servo.h>
#include <AccelStepper.h>

// ----- Servo Setup -----
Servo myservo;

// ----- Stepper Motor Setup -----
const int EN_PIN   = 5;  // Enable pin for TB6600
const int DIR_PIN  = 6;  // Direction pin
const int STEP_PIN = 7;  // Step pin
const int STEPS_PER_REV = 1600; // Steps per revolution (adjust as per your driver)

const float steps_per_mm = 80.0;    // Example: 80 steps per mm (adjust according to your hardware)
const float initial_position_mm = 40;  // Initial position in mm

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// Function to perform a full stepper cycle (40mm -> 0mm -> 40mm)
void performStepperCycle() {
  long initial_steps = initial_position_mm * steps_per_mm;

  // Move stepper from initial position to 0 mm
  stepper.moveTo(0);
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
  
  // Now move stepper back to initial position (40 mm)
  stepper.moveTo(initial_steps);
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
}

void setup() {
  // Start serial communication for receiving commands
  Serial.begin(9600);
  
  // ----- Servo Initialization -----
  myservo.attach(9);        // Attach the servo to pin 9
  myservo.write(0);         // Set initial servo angle to 0 degrees
  
  // ----- Stepper Initialization -----
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);  // Enable the stepper driver (LOW = enabled)
  
  stepper.setMaxSpeed(2000);       // Set maximum speed (in steps per second)
  stepper.setAcceleration(800);    // Set acceleration (in steps per second squared)
  
  // Set the stepper's starting position to 40 mm in steps
  long initial_steps = initial_position_mm * steps_per_mm;
  stepper.setCurrentPosition(initial_steps);
  
  // Optionally, move the stepper to 0 mm and then back to 40 mm on startup.
  stepper.moveTo(0);
  while(stepper.distanceToGo() != 0) {
    stepper.run();
  }
  stepper.moveTo(initial_steps);
}

void loop() {
  // Check if data is available from the serial port
  if (Serial.available() > 0) {
    // Read an incoming line (ends with '\n')
    String command = Serial.readStringUntil('\n');
    command.trim();  // Remove any extra whitespace
    
    // Check if the command begins with "ROTATE"
    if (command.startsWith("ROTATE")) {
      // Expected command format: "ROTATE <theta> <delta>"
      int firstSpace = command.indexOf(' ');
      int secondSpace = command.indexOf(' ', firstSpace + 1);
      
      // Ensure both parameters exist
      if (firstSpace != -1 && secondSpace != -1) {
        String thetaStr = command.substring(firstSpace + 1, secondSpace);
        String deltaStr = command.substring(secondSpace + 1);
        
        float theta = thetaStr.toFloat();
        float delta = deltaStr.toFloat();

        // Set the servo to the specified angle
        myservo.write(theta);  
        Serial.print("Servo rotated to: ");
        Serial.println(theta);
        
        // If delta is 1, run the stepper motor cycle
        if (delta == 1.0) {
          Serial.println("Performing stepper cycle...");
          performStepperCycle();
          Serial.println("Stepper cycle complete.");
        }
      }
    }
  }
  
  // Always run the stepper so it can process any moves in the background.
  stepper.run();
}
