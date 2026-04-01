#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define I2C_SDA 8
#define I2C_SCL 9

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
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

  delay(2000);
  
  Serial.begin(115200);
  Serial.println("Start loop");
}

void loop() {
}