// Arduino Test Script for Gantry and EndEffector Communication
// This script should be uploaded to both Arduino controllers (ACM0 and ACM1)

#define GANTRY_CONTROLLER  // Comment this line for EndEffector controller

void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait for serial connection
  
  #ifdef GANTRY_CONTROLLER
    Serial.println("Gantry Controller Ready");
    Serial.println("Commands:");
    Serial.println("GOTO x y z - Move to position");
    Serial.println("HOME - Home the gantry");
    Serial.println("GETPOS - Return current position");
    Serial.println("STOP - Emergency stop");
  #else
    Serial.println("EndEffector Controller Ready");
    Serial.println("Commands:");
    Serial.println("ROTATE theta delta - Set angles");
    Serial.println("GETANGLES - Return current angles");
    Serial.println("STOP - Emergency stop");
  #endif
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    #ifdef GANTRY_CONTROLLER
      // Gantry command processing
      if (command.startsWith("GOTO")) {
        float x, y, z;
        if (sscanf(command.c_str(), "GOTO %f %f %f", &x, &y, &z) == 3) {
          Serial.print("Moving to: X=");
          Serial.print(x);
          Serial.print(" Y=");
          Serial.print(y);
          Serial.print(" Z=");
          Serial.println(z);
          // Here you would actually move the motors
        } else {
          Serial.println("ERROR: Invalid GOTO format");
        }
      } 
      else if (command == "HOME") {
        Serial.println("Homing gantry...");
        // Homing routine would go here
        Serial.println("POS: 0.00 0.00 0.00");
      }
      else if (command == "GETPOS") {
        // Simulate returning a position
        Serial.println("POS: 10.00 20.00 5.00");
      }
      else if (command == "STOP") {
        Serial.println("EMERGENCY STOP ACTIVATED");
        // Emergency stop logic would go here
      }
      else {
        Serial.println("ERROR: Unknown command");
      }
    #else
      // EndEffector command processing
      if (command.startsWith("ROTATE")) {
        float theta, delta;
        if (sscanf(command.c_str(), "ROTATE %f %f", &theta, &delta) == 2) {
          Serial.print("Setting angles: Theta=");
          Serial.print(theta);
          Serial.print(" Delta=");
          Serial.println(delta);
          // Here you would actually move the servos
        } else {
          Serial.println("ERROR: Invalid ROTATE format");
        }
      }
      else if (command == "GETANGLES") {
        // Simulate returning angles
        Serial.println("ANGLES: 45.00 -15.00");
      }
      else if (command == "STOP") {
        Serial.println("EMERGENCY STOP ACTIVATED");
        // Emergency stop logic would go here
      }
      else {
        Serial.println("ERROR: Unknown command");
      }
    #endif
  }
}