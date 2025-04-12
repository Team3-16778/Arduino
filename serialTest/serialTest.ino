void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait for serial connection
  Serial.println("Command logger ready");
  Serial.println("Will echo all received commands");
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    // Simply log whatever command was received
    Serial.print("RECV: ");
    Serial.println(command);
    
    // Add specific responses for expected commands
    if (command.startsWith("GOTO")) {
      Serial.println("ACK: Gantry position command received");
    }
    else if (command.startsWith("ROTATE")) {
      Serial.println("ACK: End effector rotation command received");
    }
    else if (command == "HOME") {
      Serial.println("ACK: Homing command received");
    }
    else if (command == "GETPOS") {
      Serial.println("ACK: Sending simulated position: X10.0 Y20.0 Z5.0");
    }
    else if (command == "GETANGLES") {
      Serial.println("ACK: Sending simulated angles: Theta45.0 Delta-15.0");
    }
    else if (command == "STOP") {
      Serial.println("ACK: Emergency stop activated");
    }
    else {
      Serial.println("ERR: Unknown command format");
    }
  }
}