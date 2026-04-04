#include <Wire.h>
#include <OneButton.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define I2C_SDA 8
#define I2C_SCL 9

#define BTN_UP 2
#define BTN_DOWN 1
#define BTN_LEFT 0
#define BTN_RIGHT 3

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
OneButton buttonUp(BTN_UP, true);
OneButton buttonDown(BTN_DOWN, true);
OneButton buttonLeft(BTN_LEFT, true);
OneButton buttonRight(BTN_RIGHT, true);

void setupDisplay() {
  Wire.begin(I2C_SDA, I2C_SCL);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    for(;;);
  }

  display.clearDisplay();     
  display.setTextSize(1);     
  display.setTextColor(WHITE);
  display.setRotation(2);

  display.println("ESP32-C3 Ready!");
  display.println("OLED 0.96 Active");
  display.display();
}

void buttonUpClick() {
}

void buttonDownClick() {
}

void buttonLeftClick() {
}

void buttonRightClick() {
}

void setupButtons() {
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);

  buttonUp.setClickMs(200);
  buttonUp.setPressMs(500);
  buttonUp.setDebounceMs(20);
  buttonUp.attachClick(buttonUpClick);

  buttonDown.setClickMs(200);
  buttonDown.setPressMs(500);
  buttonDown.setDebounceMs(20);
  buttonDown.attachClick(buttonDownClick);

  buttonLeft.setClickMs(200);
  buttonLeft.setPressMs(500);
  buttonLeft.setDebounceMs(20);
  buttonLeft.attachClick(buttonLeftClick);

  buttonRight.setClickMs(200);
  buttonRight.setPressMs(500);
  buttonRight.setDebounceMs(20);
  buttonRight.attachClick(buttonRightClick);
}

void setup() {
  Serial.begin(115200);

  setupDisplay();
  setupButtons();

  delay(2000);
}

void loop() {
  buttonUp.tick();
  buttonDown.tick();
  buttonLeft.tick();
  buttonRight.tick();
}