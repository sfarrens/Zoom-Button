#include <LiquidCrystal.h>

#define RED 4
#define GREEN 5
#define BLUE 6
#define MUTE_BUTTON_PIN 3
#define HAND_BUTTON_PIN 2

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// Mute button
bool lastMuteButtonState = HIGH;
unsigned long muteButtonPressStart = 0;
bool muteButtonIsHeld = false;
const unsigned long muteButtonLongPressTime = 500;

// Hand button
bool lastHandButtonState = HIGH;
unsigned long handButtonPressStart = 0;
bool handButtonIsHeld = false;
bool handButtonWaitingForDouble = false;
unsigned long handButtonFirstPressTime = 0;
const unsigned long handButtonDoubleTapTime = 300;
const unsigned long handButtonLongPressTime = 2000;

enum LedState { RED_STATE, YELLOW_STATE, GREEN_STATE };
LedState currentLed = RED_STATE;

void setup() {
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(MUTE_BUTTON_PIN, INPUT);
  pinMode(HAND_BUTTON_PIN, INPUT);

  Serial.begin(9600);

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Zoom Button v1.0");
  lcd.setCursor(0, 1);
  lcd.print("No Zoom         ");

  setRed();
  currentLed = RED_STATE;
  Serial.println("v1.0");
}

void loop() {
  bool muteButtonState = digitalRead(MUTE_BUTTON_PIN);
  bool handButtonState = digitalRead(HAND_BUTTON_PIN);

  // --- Mute button logic ---
  if (muteButtonState != lastMuteButtonState) {
    if (muteButtonState == LOW) {
      muteButtonPressStart = millis();
      muteButtonIsHeld = false;
    } else {
      if (!muteButtonIsHeld) {
        Serial.println("MUTE");
      } else {
        Serial.println("HOLD_END");
        updateLcdByState();
      }
      muteButtonIsHeld = false;
    }
    lastMuteButtonState = muteButtonState;
  }

  if (muteButtonState == LOW && !muteButtonIsHeld && (millis() - muteButtonPressStart >= muteButtonLongPressTime)) {
    muteButtonIsHeld = true;
    Serial.println("HOLD_START");
    lcd.setCursor(0, 1);
    lcd.print("Unmuted         ");
  }

  if (muteButtonIsHeld) {
    setBlue();
  } else {
    updateLedByState();
  }

  // --- Hand button logic ---
  if (handButtonState != lastHandButtonState) {
    if (handButtonState == LOW) {
      handButtonPressStart = millis();
      handButtonIsHeld = false;

      if (handButtonWaitingForDouble && (millis() - handButtonFirstPressTime <= handButtonDoubleTapTime)) {
        handButtonWaitingForDouble = false;
        Serial.println("THUMBS_UP");
      } else {
        handButtonWaitingForDouble = true;
        handButtonFirstPressTime = millis();
      }
    } else {
      if (!handButtonIsHeld) {
        // short press release - single tap handled by timeout below
      } else {
        handButtonIsHeld = false;
      }
    }
    lastHandButtonState = handButtonState;
  }

  // Long press hand button
  if (handButtonState == LOW && !handButtonIsHeld && (millis() - handButtonPressStart >= handButtonLongPressTime)) {
    handButtonIsHeld = true;
    handButtonWaitingForDouble = false;
    Serial.println("QUIT_ZOOM");
  }

  // Single tap timeout
  if (handButtonWaitingForDouble && (millis() - handButtonFirstPressTime > handButtonDoubleTapTime) && !handButtonIsHeld && handButtonState == HIGH) {
    handButtonWaitingForDouble = false;
    Serial.println("HAND");
  }

  handleSerial();
}

void handleSerial() {
  if (muteButtonIsHeld) return;

  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();

    if (msg == "OFF") {
      currentLed = RED_STATE;
      updateLcdByState();
    }
    else if (msg == "CALL_OFF") {
      currentLed = YELLOW_STATE;
      updateLcdByState();
    }
    else if (msg == "CALL_ON") {
      currentLed = GREEN_STATE;
      updateLcdByState();
    }
    else if (msg == "BLINK") {
      lcd.setCursor(0, 1);
      lcd.print("Mute toggled    ");
      for (int i = 0; i < 2; i++) {
        setGreen();
        delay(100);
        setOff();
        delay(100);
      }
      delay(1600); // total with blink = ~2000ms
      updateLcdByState();
    }
    else if (msg == "NOT_IN_MEETING") {
      lcd.setCursor(0, 1);
      lcd.print("Not in meeting  ");
      delay(2000);
      updateLcdByState();
    }
    else if (msg == "HAND_OK") {
      lcd.setCursor(0, 1);
      lcd.print("Hand toggled    ");
      delay(2000);
      updateLcdByState();
    }
    else if (msg == "THUMBS_UP_OK") {
      lcd.setCursor(0, 1);
      lcd.print("Thumbs Up!      ");
      delay(2000);
      updateLcdByState();
    }
    else if (msg == "ENDING_CALL") {
      lcd.setCursor(0, 1);
      lcd.print("Ending call     ");
      delay(2000);
      updateLcdByState();
    }
    else if (msg == "QUITTING_ZOOM") {
      lcd.setCursor(0, 1);
      lcd.print("Quitting Zoom   ");
      delay(2000);
      updateLcdByState();
    }
  }
}

void updateLcdByState() {
  lcd.setCursor(0, 1);
  switch(currentLed) {
    case RED_STATE:    lcd.print("No Zoom         "); break;
    case YELLOW_STATE: lcd.print("Zoom Open       "); break;
    case GREEN_STATE:  lcd.print("In Meeting      "); break;
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
void setOff()    { analogWrite(RED, 0);   analogWrite(GREEN, 0);   analogWrite(BLUE, 0); }