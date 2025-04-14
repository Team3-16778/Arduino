#include <AccelStepper.h>

// Pin assignments
const int EN_PIN = 5;    // Enable pin
const int DIR_PIN = 6;   // Direction pin
const int STEP_PIN = 7;  // Step pin
const int INJECT_PIN = 8; // Injection solenoid control pin

// Motor parameters
const int STEPS_PER_REV = 1600;  // 200 steps/rev * 8 microsteps
const float MAX_SPEED = 1000.0;  // steps per second
const float ACCELERATION = 500.0; // steps per second^2

// Create stepper object
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// Current position tracking
float currentTheta = 0.0;  // Main rotation angle
float currentDelta = 0.0;  // Secondary angle (if applicable)

// Serial communication
const int SERIAL_BAUD = 9600;
const int BUFFER_SIZE = 64;
char inputBuffer[BUFFER_SIZE];
int bufferIndex = 0;

void setup() {
  // Initialize pins
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);  // Enable driver
  pinMode(INJECT_PIN, OUTPUT);
  digitalWrite(INJECT_PIN, LOW);

  // Configure stepper
  stepper.setMaxSpeed(MAX_SPEED);
  stepper.setAcceleration(ACCELERATION);
  stepper.setCurrentPosition(0);

  // Initialize serial communication
  Serial.begin(SERIAL_BAUD);
  while (!Serial);  // Wait for serial port to connect (for USB)
  
  Serial.println("EndEffector Ready");
  Serial.println("Commands: HOME, ROTATE theta delta, STOP, GETANGLES, INJECT");
}

void loop() {
  // Process incoming serial commands
  processSerialCommands();
  
  // Run the stepper motor
  stepper.run();
}

void processSerialCommands() {
  while (Serial.available() > 0) {
    char incomingChar = Serial.read();
    
    // Check for buffer overflow
    if (bufferIndex < BUFFER_SIZE - 1) {
      // Check for end of line
      if (incomingChar == '\n') {
        inputBuffer[bufferIndex] = '\0';  // Null terminate
        handleCommand(inputBuffer);
        bufferIndex = 0;  // Reset buffer
      } else {
        inputBuffer[bufferIndex++] = incomingChar;
      }
    } else {
      // Buffer overflow, reset and discard
      bufferIndex = 0;
    }
  }
}

void handleCommand(char* command) {
  // Convert to uppercase for case-insensitive comparison
  for (int i = 0; command[i]; i++) {
    command[i] = toupper(command[i]);
  }

  // Parse command
  if (strcmp(command, "HOME") == 0) {
    home();
  } 
  else if (strcmp(command, "STOP") == 0) {
    stop();
  } 
  else if (strcmp(command, "INJECT") == 0) {
    inject();
  }
  else if (strcmp(command, "GETANGLES") == 0) {
    getAngles();
  }
  else if (strncmp(command, "ROTATE", 6) == 0) {
    // Parse ROTATE command with two float parameters
    float theta, delta;
    if (sscanf(command + 6, "%f %f", &theta, &delta) == 2) {
      rotate(theta, delta);
    } else {
      Serial.println("ERROR: Invalid ROTATE parameters");
    }
  }
  else {
    Serial.println("ERROR: Unknown command");
  }
}

void home() {
  Serial.println("Homing...");
  stepper.moveTo(0);
  currentTheta = 0.0;
  currentDelta = 0.0;
  Serial.println("HOME complete");
}

void stop() {
  Serial.println("Emergency stop!");
  stepper.stop();  // Abrupt stop
  stepper.disableOutputs();
  digitalWrite(INJECT_PIN, LOW);
}

void inject() {
  Serial.println("Injection started");
  digitalWrite(INJECT_PIN, HIGH);
  delay(500);  // Injection duration (ms)
  digitalWrite(INJECT_PIN, LOW);
  Serial.println("Injection complete");
}

void rotate(float theta, float delta) {
  // Convert angles to steps (example conversion - adjust as needed)
  long thetaSteps = (long)(theta * STEPS_PER_REV / 360.0);
  // For this example, we'll only use theta for the single stepper
  // Delta would control a second motor if available
  
  Serial.print("Moving to theta: ");
  Serial.print(theta);
  Serial.print("°, delta: ");
  Serial.println(delta);
  
  stepper.moveTo(thetaSteps);
  currentTheta = theta;
  currentDelta = delta;
}

void getAngles() {
  Serial.print("ANGLES: ");
  Serial.print(currentTheta);
  Serial.print(" ");
  Serial.println(currentDelta);
}