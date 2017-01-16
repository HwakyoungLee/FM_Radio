#include <Wire.h>
#include <LiquidCrystal.h>
#include <radio.h>
#include <RDA5807M.h>
#include <RDSParser.h>

// button state
enum BUTTONSTATE {
  BUTTONSTATE_NONE, BUTTONSTATE_SELECT, BUTTONSTATE_LEFT, BUTTONSTATE_UP, BUTTONSTATE_DOWN, BUTTONSTATE_RIGHT, BUTTONSTATE_ENCODE_OK
} __attribute__((packed));

const int Encoder_A = 3;            // Incremental Encoder signal A is PD3
const int Encoder_B = 2;            // Incremental Encoder signal B is PD2

RADIO_FREQ minFrequency;
RADIO_FREQ maxFrequency;
RADIO_FREQ frequencyStep;

RADIO_FREQ currentFrequency = 8910;
uint8_t currentVolume = 4;

// initialize lcd library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

RDA5807M radio;    // Create an instance of Class for RDA5807M Chip
RDSParser rds;

// forward declaration
BUTTONSTATE readLCDButtons();
void displayFrequency();
void encoderChanged();
void printRadioInfo();

void setup()
{
  Serial.begin(9600);
  Serial.println("Radio...");

  lcd.begin(16, 2);
  lcd.clear();
  pinMode(10, OUTPUT);
  digitalWrite(10, 1);

  // Rotary encoder
  pinMode(Encoder_A, INPUT);
  pinMode(Encoder_B, INPUT);
  digitalWrite(Encoder_A, 1);
  digitalWrite(Encoder_B, 1);
  attachInterrupt(1, encoderChanged, FALLING);        //interrupts: numbers 0 (on digital pin 2) and 1 (on digital pin 3).

  // Initialize the Radio
  radio.init();

  // Enable information to the Serial port
  radio.debugEnable();

  minFrequency = radio.getMinFrequency();
  maxFrequency = radio.getMaxFrequency();
  frequencyStep = radio.getFrequencyStep();

  radio.setBandFrequency(RADIO_BAND_FMWORLD, currentFrequency);
  radio.setVolume(currentVolume);
  radio.setMono(false);
  radio.setMute(false);
}

void loop()
{
  static RADIO_FREQ lastFrequency = 0;

  if (lastFrequency != currentFrequency) {
    radio.setFrequency(currentFrequency);
    displayFrequency();
    printRadioInfo();
    lastFrequency = currentFrequency;
  }

//  BUTTONSTATE buttonState = readLCDButtons();
//  if (buttonState == BUTTONSTATE_RIGHT) {
//    radio.seekUp();
//    currentFrequency = radio.getFrequency();
//  }

//  RADIO_INFO ri;
//  radio.getRadioInfo(&ri);
//  Serial.print("rssi: ");
//  Serial.print(ri.rssi);
}

BUTTONSTATE readLCDButtons()
{
  int v = analogRead(0);

  // read the value from the sensor
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  if (v < 50) {
    return BUTTONSTATE_LEFT;
  } else if (v < 150) {
    return BUTTONSTATE_UP;
  } else if (v < 250) {
    return BUTTONSTATE_RIGHT;
  } else if (v < 450) {
    return BUTTONSTATE_SELECT;
  } else if (v < 700) {
    return BUTTONSTATE_DOWN;
  } else if (v < 850) {
    return BUTTONSTATE_ENCODE_OK;
  }

  return BUTTONSTATE_NONE;
}

void displayFrequency()
{
  char s[16];
  dtostrf(currentFrequency / 100.0, 5, 1, s);
  
  lcd.setCursor(0, 0);
  lcd.print(s);
  lcd.print("MHz");
}

void encoderChanged()
{
  if (digitalRead(Encoder_B)) {
    currentFrequency += frequencyStep;
  } else {
    currentFrequency -= frequencyStep;
  }

  if (currentFrequency >= maxFrequency) {
    currentFrequency = maxFrequency;
  } else if (currentFrequency <= minFrequency) {
    currentFrequency = minFrequency;
  }
}

void printRadioInfo()
{
  char s[16];
  dtostrf(currentFrequency / 100.0, 5, 1, s);
  Serial.print("Station:");
  Serial.print(s);
  Serial.println(" MHz");
  
  Serial.print("Radio:");
  radio.debugRadioInfo();

  Serial.print("Audio:");
  radio.debugAudioInfo();
}

