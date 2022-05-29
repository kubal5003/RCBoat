// BAIT BOAT MAIN COMPUTER

#include <SPI.h>
#include <Wire.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define MESSAGE_DISPLAY_TIME 3000
#define SIZE 32

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#define ENGINE_L_IN1 8
#define ENGINE_L_IN2 7
#define ENGINE_L_PWM 6

#define ENGINE_R_IN1 3
#define ENGINE_R_IN2 2
#define ENGINE_R_PWM 5

#define FORWARD 1
#define BACKWARD -1 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RF24 radio(9, 10); // CE, CSN
const byte address[6] = "BOAT1";
int lastTimeTextWasDisplayed = 0;
char buffer[SIZE + 1];

int X = 0;
int Y = 0;

void setup() {
  Serial.begin(9600);

  setupTTFScreen();
  delay(1000);

  setupRadio();
  delay(1000);


  setupEngineController();
  delay(1000);

  
}

void setupTTFScreen() {
  //check if I2C TTF display is connected
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Display allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Starting..");
  display.display();
}

void setupRadio() {

  if (!radio.begin()) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Radio failure.");
    display.display();
    while (1) {} // hold in infinite loop
  }
  radio.setAutoAck(false);
  radio.openReadingPipe(0, address);   //Setting the address at which we will receive the data
  radio.setPALevel(RF24_PA_MIN);       //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.startListening();
  radio.setPayloadSize(SIZE);


  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Radio setup finished.");
  display.display();
}

void setupEngineController() {
  pinMode(ENGINE_L_IN1, OUTPUT);
  pinMode(ENGINE_L_IN2, OUTPUT);
  pinMode(ENGINE_L_PWM, OUTPUT);
  pinMode(ENGINE_R_IN1, OUTPUT);
  pinMode(ENGINE_R_IN2, OUTPUT);
  pinMode(ENGINE_R_PWM, OUTPUT);
  analogWrite(ENGINE_L_PWM, 0); //0-255
  analogWrite(ENGINE_R_PWM, 0); //0-255
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Engines ready.");
  display.display();
}

void loop() {
  int currentTime = millis();
  if (currentTime - lastTimeTextWasDisplayed > MESSAGE_DISPLAY_TIME) {
    lastTimeTextWasDisplayed = currentTime;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Nothing was received.");
    display.display();
  }


  if (radio.available())              //Looking for the data.
  {
    radio.read(&buffer, SIZE);    //Reading the data
    if (strlen(buffer) > 0) {
      lastTimeTextWasDisplayed = currentTime;

      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Message received");
      display.setCursor(0, 10);
      
      sscanf(buffer, "X: %d Y: %d", &X, &Y);

      display.print("X: ");
      display.print(X, DEC);
      display.print(" Y: ");
      display.print(Y, DEC);

      display.display();
    } else {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Received empty message");
      display.display();
    }
  }
  delay(5);
}