/*****************************************************************
 * End Effector – Servo + TB6600 stepper
 *   ENA+  → 5   (active LOW)
 *   DIR+  → 6
 *   PUL+  → 7
 *   Servo signal → 9
 *   All “−” pins → Arduino GND
 *****************************************************************/

#include <Arduino.h>
#include <Servo.h>
#include <AccelStepper.h>
#include <math.h>          // lround()

/* ── Servo ──────────────────────────────────────────────── */
Servo       myServo;
const uint8_t SERVO_PIN = 9;
float       currentTheta = 0.0f;

/* ── TB6600 wiring ──────────────────────────────────────── */
constexpr uint8_t EN_PIN   = 5;
constexpr uint8_t DIR_PIN  = 6;
constexpr uint8_t STEP_PIN = 7;

/* ── Mechanical constants ──────────────────────────────── */
constexpr float STEPS_PER_MM = 80.0f;   // tune to your lead‑screw
constexpr float STROKE_MM    = 100.0f;   // one‑way stroke length

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

/* ── Helper: move exactly +40 mm from current position ──── */
void moveByStroke()
{
  long deltaSteps = lround(STROKE_MM * STEPS_PER_MM);
  stepper.move(deltaSteps);                   // relative +40 mm
  while (stepper.distanceToGo() != 0) stepper.run();
}

/* ── Helper: go back to absolute 0 mm position ──────────── */
void goHome()
{
  stepper.moveTo(0);                          // absolute 0 mm
  while (stepper.distanceToGo() != 0) stepper.run();
}

/* ───────────────────────────────────────────────────────── */
void setup()
{
  Serial.begin(9600);

  /* Servo */
  myServo.attach(SERVO_PIN);
  myServo.write(currentTheta);                // start at 0°

  /* Stepper */
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);                  // enable driver

  stepper.setPinsInverted(true, false, false); // invert DIR
  stepper.setMaxSpeed(2000);                  // steps / s
  stepper.setAcceleration(800);               // steps / s²

  stepper.setCurrentPosition(0);              // we are at physical 0 mm
}

void loop()
{
  /* ── Handle incoming serial commands ── */
  if (Serial.available())
  {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    /* ROTATE <theta> <delta> */
    if (cmd.startsWith("ROTATE"))
    {
      // split after first and second spaces
      int sp1 = cmd.indexOf(' ');
      int sp2 = cmd.indexOf(' ', sp1 + 1);

      if (sp1 != -1 && sp2 != -1)
      {
        float theta = cmd.substring(sp1 + 1, sp2).toFloat();
        float delta = cmd.substring(sp2 + 1).toFloat();

        currentTheta = constrain(theta, 0.0f, 180.0f);
        myServo.write(currentTheta);

        if (fabs(delta) < 0.5f) {            // treat 0 ±0.5 as zero
          goHome();                          // δ = 0 → back to 0 mm
        } else {
          moveByStroke();                    // δ ≠ 0 → +40 mm stroke
        }
      }
    }

    /* INJECT – always +40 mm stroke */
    else if (cmd.equalsIgnoreCase("INJECT"))
    {
      moveByStroke();
    }
  }

  stepper.run();  // allow AccelStepper background processing
}
