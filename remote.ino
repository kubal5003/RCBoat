// BAIT BOAT REMOTE CONTROL

#include <SPI.h>
#include <Wire.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define X_PIN 0 // analog pin connected to X output of joystick
#define Y_PIN 1 // analog pin connected to Y output of joystick

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define BUTTON_PIN 2

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RF24 radio(9, 10); // CE, CSN
boolean button_state = 0;
const uint8_t address[6] = "BOAT1";
int iteration = 1;
uint8_t failures = 0;
int failure_seconds_left_until_retry = -1;

#define SIZE 32            // this is the maximum for this example. (minimum is 1)
char buffer[SIZE + 1];

void setup() {
  pinMode(BUTTON_PIN, INPUT);
  Serial.begin(9600);

  setupTTFScreen();
  delay(1000);

  setupRadio();
  delay(1000);
}

void loop() {
  display.clearDisplay();

  if ( failure_seconds_left_until_retry > 0 ) {
    String errorMessage = "Transmission failed. Retry in " ;
    errorMessage += failure_seconds_left_until_retry;
    String retriesLeftMessage = "Retries left:";
    retriesLeftMessage += 10 - failures;
    display.setCursor(0, 0);
    display.println(errorMessage);
    display.setCursor(0, 20);
    display.println(retriesLeftMessage);
    display.display();
    delay(1000);
    failure_seconds_left_until_retry--;
    return;
  }

  if(failure_seconds_left_until_retry == 0) {
    display.setCursor(0, 0);
    display.println("Retrying...");
    display.display();
    delay(1000);
    failure_seconds_left_until_retry--;
    return;
  }

  if (failures >= 9) {
    display.setCursor(0, 0);            // Start at top-left corner
    display.println("Permanent radion failure.");
    display.display();
    delay(5000);
    return;
  }

  int xAxis = analogRead(X_PIN);
  int yAxis = analogRead(Y_PIN);

  sprintf(buffer, "X: %d Y: %d", xAxis, yAxis);
  if (!radio.writeFast(&buffer, SIZE)) {
    failures++;
    failure_seconds_left_until_retry = 5;
    radio.reUseTX();

  } else {
    display.setCursor(0, 10);            // Start at top-left corner
    display.println(buffer);
    display.display();
  }
  delay(100);
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
  radio.setAddressWidth(5);
  radio.stopListening();
  radio.openWritingPipe(address); //Setting the address where we will send the data
  radio.setPALevel(RF24_PA_MIN);  //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.setPayloadSize(SIZE);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Radio setup finished.");
  display.display();
}


//  button_state = digitalRead(BUTTON_PIN);
//  if (button_state == HIGH)
//  {
//    display.setCursor(0, 0);
//    display.println("Sending data...");
//    sprintf(buffer, "Iteration %d", iteration++);
//    radio.flush_tx();
//
//    if (!radio.writeFast(&buffer, SIZE)) {
//      failures++;
//      radio.reUseTX();
//      display.setCursor(0, 10);            // Start at top-left corner
//      display.println("Transmission failed");
//      display.display();
//    } else {
//      display.setCursor(0, 10);            // Start at top-left corner
//      display.println(buffer);
//      display.display();
//
//    }
//    delay(1000);
//  }
//  else
//  {
//    display.setCursor(0, 0);
//    display.println("Nothing to transmit.");
//    display.display();
//    delay(100);
//  }