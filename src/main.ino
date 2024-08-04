// Asthma Condition Detector
// stanleymw

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <Adafruit_BME280.h>
#include <EEPROM.h>
#include <SPI.h>

// Screen dimensions
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128

// PIN DEFINITIONS
// oled
#define SCLK_PIN 13
#define MOSI_PIN 11
#define DC_PIN   7
#define CS_PIN   10
#define RST_PIN  8

// grove dust
#define GROVE_DUST 2

// grove air quality
#define GROVE_AQ A2

// MQ-135
#define MQ_135 A0
#define TTL A1

// Rotary Encoder
#define RE_CLK 2
#define RE_DT 3
#define RE_SW 4

// Color definitions
#define BLACK           0x0000
#define BLUE            0x001F
#define RED             0xF800
#define GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF

// for bme280
#define SEALEVELPRESSURE_HPA (1013.25)

//const float AQ_BASE = 57.0;
//const float TTL_BASE = 125.0;
//const float MQ_BASE = 330.0;
//const float HUMIDITY_BASE = 50.0;

// Option 1: use any pins but a little slower
// Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, CS_PIN, DC_PIN, MOSI_PIN, SCLK_PIN, RST_PIN);  

// Option 2: must use the hardware SPI pins 
// (for UNO thats sclk = 13 and sid = 11) and pin 10 must be 
// an output. This is much faster - also required if you want
// to use the microSD card (see the image drawing example)
Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN);
Adafruit_BME280 bme; // I2C

int prevCLK;
void setup(void) {
  Serial.begin(9600); Serial.print("hello!");

  pinMode(GROVE_DUST, INPUT);
  pinMode(GROVE_AQ, INPUT);
  pinMode(MQ_135, INPUT);
  pinMode(TTL, INPUT);

  pinMode(RE_CLK, INPUT);
  pinMode(RE_DT, INPUT);
  pinMode(RE_SW, INPUT_PULLUP);

  prevCLK = digitalRead(RE_CLK);

  tft.begin();

  while (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    delay(1000);
  }

  //tft.fillScreen(BLACK);
  //testdrawtext("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur adipiscing ante sed nibh tincidunt feugiat. Maecenas enim massa, fringilla sed malesuada et, malesuada sit amet turpis. Sed porttitor neque ut ante pretium vitae malesuada nunc bibendum. Nullam aliquet ultrices massa eu hendrerit. Ut sed nisi lorem. In vestibulum purus a tortor imperdiet posuere. ", WHITE);

  Serial.println("init");
}

unsigned long duration;
const unsigned long sampletime_ms = 15000; // sample time
unsigned long lpo = 0;
unsigned long starttime = millis();

float ratio;
float concentration;

void collectDustData() {
    duration = pulseIn(GROVE_DUST, LOW);
    lpo += duration;
    if ( (millis() - starttime) > sampletime_ms)
    {
        ratio = lpo/(sampletime_ms*10.0);  // Integer percentage 0=>100
        concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
        Serial.print(lpo);
        Serial.print(",");
        Serial.print(ratio);
        Serial.print(",");
        Serial.println(concentration);
        lpo = 0;
        starttime = millis();
    }
}

const uint8_t getRatioVal(float ratio) {
    if (ratio < 1.2) {
      return 0;
    } else if (ratio < 1.4) {
      return 1;
    } else if (ratio <1.9) {
      return 2;
    } else {
      return 3;
    };
}

const void displayRatio(String text, float ratio, bool selected = false) {
    tft.setTextColor(selected ? RED : WHITE);
    tft.print(text);

    uint16_t col;
    switch (getRatioVal(ratio)) {
      case 0:
        col = GREEN;
        break;
      case 1:
        col = YELLOW;
        break;
      case 2:
        col = RED;
        break;
      case 3:
        col = MAGENTA;
        break;
    }

    tft.setTextColor(col);
    tft.println(ratio);
    tft.setTextColor(WHITE);
  }

float getBaseline(int idx) {
  return (float)(EEPROM.read(idx) * 2);
}

int currentSelection = 0;
unsigned long lastPushed = 0;

void updateOled() {
    const float AQ_RATIO = analogRead(GROVE_AQ) / getBaseline(0);
    const float TTL_RATIO = analogRead(TTL) / getBaseline(1);
    const float MQ_RATIO = analogRead(MQ_135) / getBaseline(2);
    const float HUM_RATIO = bme.readHumidity() / getBaseline(3);

    const float AVG_RATIO = (AQ_RATIO + TTL_RATIO + MQ_RATIO + HUM_RATIO)/4;

    tft.setCursor(0,0);
    tft.fillScreen(BLACK);
    displayRatio("GrAQ: ", AQ_RATIO, currentSelection == 0);

    displayRatio("TTL: ", TTL_RATIO, currentSelection == 1);

    displayRatio("MQ: ", MQ_RATIO, currentSelection == 2);

    displayRatio("Hum: ", HUM_RATIO, currentSelection == 3);

    tft.setTextColor(WHITE);
    tft.print("Gr Dust: ");
    tft.println(concentration);

    tft.print("Temp: ");
    tft.println(bme.readTemperature());

    tft.print("Pressure: ");
    tft.print(bme.readPressure() / 100.0F);
    tft.println(" hPa");

    //tft.print("Approx. Altitude = ");
    //tft.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    //tft.println(" m");

    tft.println("--------");

    displayRatio("Overall: ", AVG_RATIO);
}

unsigned long lastOledUpdate = 0;
void loop() {
    if (digitalRead(RE_SW) == LOW) {
      if (millis() - lastPushed > 50) {
        currentSelection += 1;
        if (currentSelection > 3) {
          currentSelection = 0;
        }
      }

      lastPushed = millis();
    }

    int currentCLK = digitalRead(RE_CLK);

    if (currentCLK != prevCLK && currentCLK == 1){
      byte n = EEPROM.read(currentSelection);
      if (digitalRead(RE_DT) != currentCLK) {
        // counter clockwise
        EEPROM.write(currentSelection, n - 2);
        //Serial.println("CCW");
      } else {
        EEPROM.write(currentSelection, n + 2);
        //Serial.println("CW");
      }
      //Serial.println(n);
    }

    prevCLK = currentCLK;

    if (millis() - lastOledUpdate > 1000) {
        updateOled();
        lastOledUpdate = millis();
      }

    //collectDustData();
    delay(1);
}
