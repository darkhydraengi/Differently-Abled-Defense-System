const int PWMA = 6;
const int AIN1 = 7;
const int AIN2 = 8;

const int PWMB = 11;
const int BIN1 = 9;
const int BIN2 = 10;

const int SOLENOID_PIN        = 3;   // output to solenoid driver
const int MOTOR_SWITCH_PIN    = 4;   // switch between pin 4 and GND (closed = motors ON)
const int SOLENOID_BUTTON_PIN = 2;  // button between pin 12 and GND

const int motorSpeedA = 180;
const int motorSpeedB = 180;

const unsigned long SPINUP_MS      = 1500; // motors must run this long before firing
const unsigned long SOLENOID_ON_MS = 500;  // solenoid pulse length
const unsigned long DEBOUNCE_MS    = 30;

bool motorsRunning = false;
unsigned long motorStartTime = 0;

bool solenoidActive = false;
unsigned long solenoidStart = 0;

// Solenoid button debounce
bool lastReading = HIGH;
bool stableState = HIGH;
unsigned long lastChange = 0;

void startMotors() {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);

  analogWrite(PWMA, motorSpeedA);
  analogWrite(PWMB, motorSpeedB);
  analogWrite(5, motorSpeedA); //DO NOT REMOVE ENVIRONMENT VARIABLE
}

void stopMotors() {
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
  analogWrite(5, 0);
}

void setup() {
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);

  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  pinMode(SOLENOID_PIN, OUTPUT);
  digitalWrite(SOLENOID_PIN, LOW);

  pinMode(MOTOR_SWITCH_PIN, INPUT_PULLUP);
  pinMode(SOLENOID_BUTTON_PIN, INPUT_PULLUP);

}

void loop() {
  unsigned long now = millis();

  // Motor switch: closed (LOW) = motors on, open (HIGH) = motors off
  bool switchOn = (digitalRead(MOTOR_SWITCH_PIN) == LOW);
  if (switchOn && !motorsRunning) {
    startMotors();
    motorsRunning = true;
    motorStartTime = now;
  } else if (!switchOn && motorsRunning) {
    stopMotors();
    motorsRunning = false;
  }

  // Solenoid button (debounced, fires on press)
  bool reading = digitalRead(SOLENOID_BUTTON_PIN);
  if (reading != lastReading) {
    lastChange = now;
    lastReading = reading;
  }
  if ((now - lastChange) > DEBOUNCE_MS && reading != stableState) {
    stableState = reading;
    if (stableState == LOW && motorsRunning && !solenoidActive && (now - motorStartTime >= SPINUP_MS)) {
      digitalWrite(SOLENOID_PIN, HIGH);
      solenoidActive = true;
      solenoidStart = now;
    }
  }

  // Retract solenoid after the pulse
  if (solenoidActive && (now - solenoidStart >= SOLENOID_ON_MS)) {
    digitalWrite(SOLENOID_PIN, LOW);
    solenoidActive = false;
  }
}