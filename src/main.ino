/*************************************************** 
 ****************************************************/
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
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
#define GROVE_AQ A5

// MQ-135
#define MQ_135 A0
#define TTL A1

// Color definitions
#define BLACK           0x0000
#define BLUE            0x001F
#define RED             0xF800
#define GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF

// Option 1: use any pins but a little slower
// Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, CS_PIN, DC_PIN, MOSI_PIN, SCLK_PIN, RST_PIN);  

// Option 2: must use the hardware SPI pins 
// (for UNO thats sclk = 13 and sid = 11) and pin 10 must be 
// an output. This is much faster - also required if you want
// to use the microSD card (see the image drawing example)
Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN);

void setup(void) {
  Serial.begin(9600); Serial.print("hello!");

  pinMode(GROVE_DUST, INPUT);
  pinMode(GROVE_AQ, INPUT);
  pinMode(MQ_135, INPUT);
  pinMode(TTL, INPUT);

  tft.begin();

  delay(500);
  tft.fillScreen(BLACK);
  testdrawtext("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur adipiscing ante sed nibh tincidunt feugiat. Maecenas enim massa, fringilla sed malesuada et, malesuada sit amet turpis. Sed porttitor neque ut ante pretium vitae malesuada nunc bibendum. Nullam aliquet ultrices massa eu hendrerit. Ut sed nisi lorem. In vestibulum purus a tortor imperdiet posuere. ", WHITE);

  Serial.println("init");

  // You can optionally rotate the display by running the line below.
  // Note that a value of 0 means no rotation, 1 means 90 clockwise,
  // 2 means 180 degrees clockwise, and 3 means 270 degrees clockwise.
  //tft.setRotation(1);
  // NOTE: The test pattern at the start will NOT be rotated!  The code
  // for rendering the test pattern talks directly to the display and
  // ignores any rotation.

  // uint16_t time = millis();
  // tft.fillRect(0, 0, 128, 128, BLACK);
  // time = millis() - time;
  // 
  // Serial.println(time, DEC);
  // delay(500);
  // 
  // lcdTestPattern();
  // delay(500);
  // 
  // tft.invert(true);
  // delay(100);
  // tft.invert(false);
  // delay(100);
  //

 //  // tft print function!
 //  tftPrintTest();
 //  delay(500);
 //  
 //  //a single pixel
 //  tft.drawPixel(tft.width()/2, tft.height()/2, GREEN);
 //  delay(500);
 //
 //  // line draw test
 //  testlines(YELLOW);
 //  delay(500);    
 // 
 //  // optimized lines
 //  testfastlines(RED, BLUE);
 //  delay(500);    
 //
 //
 //  testdrawrects(GREEN);
 //  delay(1000);
 //
 //  testfillrects(YELLOW, MAGENTA);
 //  delay(1000);
 //
 //  tft.fillScreen(BLACK);
 //  testfillcircles(10, BLUE);
 //  testdrawcircles(10, WHITE);
 //  delay(1000);
 //   
 //  testroundrects();
 //  delay(500);
 //  
 //  testtriangles();
 //  delay(500);
 //  
 //  Serial.println("done");
 //  delay(1000);
}

unsigned long duration;
unsigned long sampletime_ms = 5000; // sample time
unsigned long lpo = 0;
unsigned long starttime = millis();

float ratio;
float concentration;

float p = 1.2344;
void loop() {
    // Serial.print("Sensor value: ");
    // Serial.println(analogRead(A5));
    duration = pulseIn(GROVE_DUST, LOW);
    lpo = lpo+duration;

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

    tft.setCursor(0,0);
    tft.fillScreen(BLACK);
    tft.print("grove aq:");
    tft.println(analogRead(GROVE_AQ));
    tft.print("ttl:");
    tft.println(analogRead(TTL));
    tft.print("mq135:");
    tft.println(analogRead(MQ_135));
    tft.print("dust:");
    tft.println(concentration);
}
