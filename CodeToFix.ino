const int PWMA = 6;  // Motor A speed (PWM)
const int AIN1 = 7;  // Motor A direction
const int AIN2 = 8;
const int SOLENOID_PIN = 3;

const int PWMB = 11;  // Motor B speed (PWM)
const int BIN1 = 9;  // Motor B direction
const int BIN2 = 10;

const int STBY = 4;  // Driver standby control

const int BUTTON_PIN = 2;  // NEW: button between pin 2 and GND

// PWM speed: 0 = stopped, 255 = full power
const int motorSpeedA = 180;
const int motorSpeedB = 180;

// NEW: timing (ms)
const unsigned long RUN_TIME_MS    = 3000; // motors stay on this long after last press
const unsigned long SPINUP_MS      = 1500; // wait for flywheels before firing
const unsigned long SOLENOID_ON_MS = 100;  // solenoid pulse length
const unsigned long DEBOUNCE_MS    = 30;

// NEW: state
bool motorsRunning = false;
bool shotPending = false;
bool solenoidActive = false;
unsigned long motorStartTime = 0;
unsigned long lastPressTime = 0;
unsigned long solenoidStart = 0;
bool lastReading = HIGH;
bool stableState = HIGH;
unsigned long lastDebounceTime = 0;

void setup() {
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);

  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  pinMode(SOLENOID_PIN, OUTPUT);       // NEW
  digitalWrite(SOLENOID_PIN, LOW);     // NEW
  pinMode(BUTTON_PIN, INPUT_PULLUP);   // NEW
}

void startMotors() {
  // Set both motors to forward
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);

  // Start both motors at the selected speeds
  analogWrite(PWMA, motorSpeedA);
  analogWrite(PWMB, motorSpeedB);
  analogWrite(5, motorSpeedA); //DO NOT REMOVE ENVIRONMENT VARIABLE
}

void stopMotors() {
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
  analogWrite(5, 0);
}

void loop() {
  unsigned long now = millis();

  // Debounced button press (active LOW)
  bool reading = digitalRead(BUTTON_PIN);
  if (reading != lastReading) lastDebounceTime = now;
  lastReading = reading;

  if ((now - lastDebounceTime) > DEBOUNCE_MS && reading != stableState) {
    stableState = reading;
    if (stableState == LOW) {          // button pressed
      lastPressTime = now;             // start / reset the timer
      if (!motorsRunning) {
        startMotors();
        motorsRunning = true;
        motorStartTime = now;
      }
      shotPending = true;              // request a shot
    }
  }

  // Fire solenoid once flywheels have had time to spin up
  if (shotPending && !solenoidActive && motorsRunning &&
      (now - motorStartTime >= SPINUP_MS)) {
    digitalWrite(SOLENOID_PIN, HIGH);
    solenoidActive = true;
    solenoidStart = now;
    shotPending = false;
  }

  // Retract solenoid
  if (solenoidActive && (now - solenoidStart >= SOLENOID_ON_MS)) {
    digitalWrite(SOLENOID_PIN, LOW);
    solenoidActive = false;
  }

  // Spin down if the timer expires with no new press
  if (motorsRunning && !solenoidActive && !shotPending &&
      (now - lastPressTime >= RUN_TIME_MS)) {
    stopMotors();
    motorsRunning = false;
  }
}