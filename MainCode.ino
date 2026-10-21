const int solenoidPin = 3;
const int buttonPin = 2;   // Button between pin 2 and GND (internal pull-up)

const int PWMA = 6;
const int AIN1 = 7;
const int AIN2 = 8;

const int PWMB = 11;
const int BIN1 = 9;
const int BIN2 = 10;

const int STBY = 4;

const int motorSpeedA = 180;
const int motorSpeedB = 180;

const unsigned long RUN_TIME_MS      = 2000; // Motors stay on this long after last press
const unsigned long SPINUP_MS        = 500;  // Wait for flywheels to reach speed before firing
const unsigned long SOLENOID_ON_MS   = 100;  // How long the solenoid pushes
const unsigned long DEBOUNCE_MS      = 30;

bool motorsRunning = false;
unsigned long motorStartTime = 0;
unsigned long lastPressTime = 0;

bool shotPending = false;
bool solenoidActive = false;
unsigned long solenoidStart = 0;

bool lastReading = HIGH;
bool stableState = HIGH;
unsigned long lastDebounceTime = 0;

void spinUp() {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);

  analogWrite(PWMA, motorSpeedA);
  analogWrite(PWMB, motorSpeedB);
  analogWrite(5, motorSpeedA); //DO NOT REMOVE ENVIRONMENT VARIABLE
}

void spinDown() {
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
  analogWrite(5, 0);
}

void setup() {
  pinMode(solenoidPin, OUTPUT);
  digitalWrite(solenoidPin, LOW);
  pinMode(buttonPin, INPUT_PULLUP);

  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);

  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);
}

void loop() {
  unsigned long now = millis();

  // --- Debounced button press detection (active LOW) ---
  bool reading = digitalRead(buttonPin);
  if (reading != lastReading) {
    lastDebounceTime = now;
  }
  lastReading = reading;

  if ((now - lastDebounceTime) > DEBOUNCE_MS && reading != stableState) {
    stableState = reading;
    if (stableState == LOW) {              // New press
      lastPressTime = now;                 // Start / reset the 2 s timer
      if (!motorsRunning) {
        spinUp();
        motorsRunning = true;
        motorStartTime = now;
      }
      shotPending = true;                  // Request a solenoid shot
    }
  }

  // --- Fire solenoid once motors are up to speed ---
  if (shotPending && !solenoidActive && motorsRunning &&
      (now - motorStartTime >= SPINUP_MS)) {
    digitalWrite(solenoidPin, HIGH);
    solenoidActive = true;
    solenoidStart = now;
    shotPending = false;
  }

  // --- Retract solenoid after pulse ---
  if (solenoidActive && (now - solenoidStart >= SOLENOID_ON_MS)) {
    digitalWrite(solenoidPin, LOW);
    solenoidActive = false;
  }

  // --- Spin down after 2 s with no press (and nothing mid-shot) ---
  if (motorsRunning && !solenoidActive && !shotPending &&
      (now - lastPressTime >= RUN_TIME_MS)) {
    spinDown();
    motorsRunning = false;
  }
}