#define RED 6
#define GREEN 5
#define BLUE 3
#define BUTTON_PIN 2

bool lastButtonState = HIGH;
unsigned long buttonPressStart = 0;
bool buttonIsHeld = false;
const unsigned long longPressTime = 500;

enum LedState { RED_STATE, YELLOW_STATE, GREEN_STATE };
LedState currentLed = RED_STATE;

void setup() {
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  Serial.begin(9600);

  setRed();
  currentLed = RED_STATE;
  Serial.println("v1.0");
}

void loop() {
  bool buttonState = digitalRead(BUTTON_PIN);

  // Detect press/release transitions first
  if (buttonState != lastButtonState) {
    if (buttonState == LOW) {
      buttonPressStart = millis();
      buttonIsHeld = false;
    } else {
      // Released
      if (!buttonIsHeld) {
        Serial.println("MUTE"); // short press
      } else {
        Serial.println("HOLD_END"); // long press released
      }
      buttonIsHeld = false;
    }
    lastButtonState = buttonState;
  }

  // Check if button has now been held long enough
  if (buttonState == LOW && !buttonIsHeld && (millis() - buttonPressStart >= longPressTime)) {
    buttonIsHeld = true;
    Serial.println("HOLD_START"); // long press begun
  }

  // Drive LED
  if (buttonIsHeld) {
    setBlue();
  } else {
    updateLedByState();
  }

  handleSerial();
}

void handleSerial() {
  if (buttonIsHeld) return;

  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();

    if (msg == "OFF") currentLed = RED_STATE;
    else if (msg == "CALL_OFF") currentLed = YELLOW_STATE;
    else if (msg == "CALL_ON") currentLed = GREEN_STATE;
    else if (msg == "BLINK") {
      for (int i = 0; i < 2; i++) {
        setGreen();
        delay(100);
        setOff();
        delay(100);
      }
    }
  }
}

void updateLedByState() {
  switch(currentLed) {
    case RED_STATE: setRed(); break;
    case YELLOW_STATE: setYellow(); break;
    case GREEN_STATE: setGreen(); break;
  }
}

void setRed()    { analogWrite(RED, 255); analogWrite(GREEN, 0);   analogWrite(BLUE, 0); }
void setGreen()  { analogWrite(RED, 0);   analogWrite(GREEN, 255); analogWrite(BLUE, 0); }
void setYellow() { analogWrite(RED, 255); analogWrite(GREEN, 150); analogWrite(BLUE, 0); }
void setBlue()   { analogWrite(RED, 0);   analogWrite(GREEN, 0);   analogWrite(BLUE, 255); }
void setOff() { analogWrite(RED, 0); analogWrite(GREEN, 0); analogWrite(BLUE, 0); }