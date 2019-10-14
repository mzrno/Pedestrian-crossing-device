/*
   Arduino code for scanning pedestrian crossings
   Compatibility of Scanse Sweep, Teensy 3.6, LIDAR-Lite v3, 16x2 LCD, Nordic nrf2401l radio modem 
   Steps:
   1. Waiting for the signal from Master.ino code
   2. If the received signal is valid, calibration starts ("S" char data for start and "F" char data for making a new file in SD card and continue to save collected scans there)
   3. Gathering scans and printing them on Serial Monitor and microSD card   
   4. Displaying speed and distance of vehicles on LCD
   5. Reseting the variables and starting again with gathering scans
   6. If the distance of the vechicles is lower then 400 cm, a signal is sent to ArduCAM_Mini_Capture2SD_Edited.ino code and it captures an image
   
   Created by Mario Zrno, 21 July 2019.
*/

#include <SPI.h>
#include <Sweep.h>
#include <SD.h>
#include <Wire.h>
#include <LIDARLite.h>
#include <LiquidCrystal_I2C.h>
#include <nRF24L01.h>
#include <RF24.h>
//declaration of PINs for Nordic 2401l modem
#define CE_PIN   9
#define CSN_PIN 10
#define MSG_NOSD F("MicroSD card not found") //message when there is no SD card

LiquidCrystal_I2C lcd(0x27, 16, 2); //setting LCD address on 0x27 for 16 chars and 2 lines
Sweep device(Serial3); //Sweep object witch uses Serial #3 of Teensy (RX3 and TX3)
File myFile; //SD object file
RF24 radio(CE_PIN, CSN_PIN); //setting nRF24LO1 radio on SPI bus with CE_PIN and CSN_PIN pins
const byte pin_sd_cs = BUILTIN_SDCARD;
uint8_t scanCount = 0; //variable for recording the number of scans
uint16_t sampleCount = 0; //variable for recording the number of samples
//arrays to store attributes of collected scans
bool syncValues[500]; //1 -> first reading of new scan, 0 otherwise
float angles[500]; //in degrees (accurate to the millidegree)
uint16_t distances[500]; //in cm
//finite States for the program sequence
const uint8_t START_INPUT = 0;
const uint8_t STATE_ADJUST_DEVICE_SETTINGS = 1;
const uint8_t STATE_VERIFY_CURRENT_DEVICE_SETTINGS = 2;
const uint8_t STATE_BEGIN_DATA_ACQUISITION = 3;
const uint8_t STATE_DATA_ACQUISITION = 4;
const uint8_t STATE_GATHER_DATA = 5;
const uint8_t STATE_STOP_DATA_ACQUISITION = 6;
const uint8_t STATE_REPORT_COLLECTED_DATA = 7;
const uint8_t STATE_RESET = 8;
const uint8_t STATE_ERROR = 9;
float speed;
uint8_t currentState; //current state in the program sequence
int range1;
int range2;
int m = 0;
char str[50] = "DATA.txt"; //current name of text file on SD card
char d;
void setup() {
  //initialize serial
  Serial.begin(115200); //serial terminal on the computer
  Serial3.begin(115200); //sweep device
  //microSD startup
  while (!SD.begin(pin_sd_cs))
  {
    Serial.println(MSG_NOSD);
    delay(500);
  }
  pinMode(2, INPUT); //setting pin 2 as monitor pin
  pinMode(6, OUTPUT); //setting pin 6 for sending impulse (HIGH)
  lcd.begin();
  lcd.backlight();
  lcd.print("Hello :)"); //starting message on LDC display
  radio.begin();
  radio.setPayloadSize(4); //setting the largest payload size for receiving
  radio.openReadingPipe(1, 0xF0F0F0F0E1LL); //open pipe for reading and receiving data
  radio.startListening(); //start listening
  reset(); //initialize counter variables and reset the current state
}

void loop() {
  if (radio.available()) //radio communication check
  {
    bool done = false;
    while (!done)
    {
      //gather data
      done = radio.read(&d, sizeof(d));
      delay(20);
    }
  }
  switch (currentState)
  {
    case START_INPUT:
      if (String(d) == "S") //inquiry of received information
      {
        lcd.clear();
        lcd.print("Start!"); //start message on LCD display
        currentState = STATE_ADJUST_DEVICE_SETTINGS;
        d = 0;
      }
      break;
    case STATE_ADJUST_DEVICE_SETTINGS: //calibration
      currentState = adjustDeviceSettings() ? STATE_VERIFY_CURRENT_DEVICE_SETTINGS : STATE_ERROR;
      break;
    case STATE_VERIFY_CURRENT_DEVICE_SETTINGS: //settings validation of Sweep
      currentState = verifyCurrentDeviceSettings() ? STATE_BEGIN_DATA_ACQUISITION : STATE_ERROR;
      break;
    case STATE_BEGIN_DATA_ACQUISITION: //collecting data startup
      currentState = beginDataCollectionPhase() ? STATE_GATHER_DATA : STATE_ERROR;
      break;
    case STATE_DATA_ACQUISITION: //collection data sequel
      currentState = dataCollectPhase() ? STATE_GATHER_DATA : STATE_ERROR;
      break;
    case STATE_GATHER_DATA: //saving collected data to arrays
      gatherSensorReading();
      if (scanCount > 1)
        currentState = STATE_STOP_DATA_ACQUISITION;
      break;
    case STATE_STOP_DATA_ACQUISITION: //stoping data gathering
      currentState = stopDataCollectionPhase() ? STATE_REPORT_COLLECTED_DATA : STATE_ERROR;
      break;
    case STATE_REPORT_COLLECTED_DATA: //printing collected data on Serial Monitor and SD card
      NextFile();
      printCollectedData();
      currentState = STATE_RESET;
      break;
    case STATE_RESET: //reseting variables and returning to gathering phase
      reset();
      currentState = STATE_DATA_ACQUISITION;
      break;
    default: //in case there was some error
      Serial.println("\n\nAn error occured. Attempting to reset and run program again...");
      reset();
      lcd.clear();
      lcd.print("Error!"); //error message on LCD display
      currentState = STATE_ADJUST_DEVICE_SETTINGS; //going back to calibration phase
      break;
  }
}

bool adjustDeviceSettings() //calibration
{
  bool bSuccess = device.setMotorSpeed(MOTOR_SPEED_CODE_6_HZ); //setting motor speed on 6HZ
  Serial.println(bSuccess ? "\nSuccessfully set motor speed." : "\nFailed to set motor speed");
  bool bSuccess1 = device.setSampleRate(SAMPLE_RATE_CODE_1000_HZ); //setting sample rate on 1000HZ
  Serial.println(bSuccess ? "\nSuccessfully set sample rate." : "\nFailed to set sample rate.");
  return bSuccess;
  return bSuccess1;
}

bool verifyCurrentDeviceSettings() //validation and print Sweep settings
{
  //reading the motor speed and sample rate
  int32_t currentMotorSpeed = device.getMotorSpeed();
  if (currentMotorSpeed < 0)
  {
    Serial.println("\nFailed to get current motor speed");
    return false;
  }
  int32_t currentSampleRate = device.getSampleRate();
  if (currentSampleRate < 0)
  {
    Serial.println("\nFailed to get current sample rate");
    return false;
  }
  //printing the current motor speed and sample rate
  Serial.println("\nMotor Speed Setting: " + String(currentMotorSpeed) + " HZ");
  Serial.println("Sample Rate Setting: " + String(currentSampleRate) + " HZ");
  return true;
}

bool beginDataCollectionPhase() //collection data startup
{
  //start of scanning
  Serial.println("\nWaiting for motor speed to stabilize and calibration routine to complete...");
  bool bSuccess = device.startScanning();
  Serial.println(bSuccess ? "\nSuccessfully initiated scanning..." : "\nFailed to start scanning.");
  if (bSuccess)
    Serial.println("\nGathering scans...");
  return bSuccess;
}

bool dataCollectPhase() //collection data sequel
{
  range1 = pulseIn(2, HIGH); //measurement of first range, with measuring time of impulse in HIGH state (microseconds)
  range1 = range1 / 10;
  bool bSuccess = device.startScanConst();
  Serial.println(bSuccess ? "\nSuccessfully scanning..." : "\nFailed scanning.");
  if (bSuccess)
    Serial.println("\nGathering scans...");
  return bSuccess;
}

void gatherSensorReading()
{
  //attempt to get the next scan packet
  //Note: getReading() will write values into the "reading" variable
  bool success = false;
  ScanPacket reading = device.getReading(success);
  if (success)
  {
    //check if this reading was the very first reading of a new 360 degree scan
    if (reading.isSync())
      scanCount++;
    //don't collect more than 1 scan
    if (scanCount > 1)
      return;
    //store the info for this sample in arrays
    syncValues[sampleCount] = reading.isSync();
    angles[sampleCount] = reading.getAngleDegrees();
    distances[sampleCount] = reading.getDistanceCentimeters();
    //increment sample count
    sampleCount++;
  }
}

bool stopDataCollectionPhase() //stoping data gathering
{
  //attempt to stop scanning
  bool bSuccess = device.stopScanning();
  return bSuccess;
}

void printCollectedData() //printing the collected data on Serial Monitor and on SD card
{
  int indexOfFirstSyncReading = 0;
  //don't print the trailing readings from the first partial scan
  while (!syncValues[indexOfFirstSyncReading])
  {
    indexOfFirstSyncReading++;
  }
  range2 = pulseIn(2, HIGH); //measurement of second range, with measuring time of impulse in HIGH state (microseconds)
  range2 = range2 / 10;
  speed = (float)1.0 * (-range2 + range1) * 2 / 100000 * 3600; //speed calculation
  lcd.clear(); //clearing LCD display
  lcd.setCursor(0, 0); //setting cursor on the first line of display
  lcd.print(String(range2) + " cm"); //display range in cm
  lcd.setCursor(0, 1); //setting cursor on the second line of display
  lcd.print(String(speed) + " km/h"); //display speed in km/h
  Serial.print("[");
  myFile.print("[");
  //printing collected data
  for (int i = indexOfFirstSyncReading; i < sampleCount; i++)
  {
    Serial.print(String(angles[i]) + ", " + String(distances[i]) + ", ");
    myFile.print(String(angles[i]) + ", " + String(distances[i]) + ", ");
  }
  Serial.println("]");
  Serial.print("\n");
  myFile.println("]");
  myFile.print("\n");
  myFile.close(); //closing the data file
  if (range2 < 400) //check if the state on pin 6 is HIGH
  {
    digitalWrite(6, HIGH);
  }
  else
  {
    digitalWrite(6, LOW);
  }
}

void NextFile() //function for opening current SD file, creating a new text file and recording collected data in her 
{
  if (String(d) == "F") //check for received information
  {
    Serial.println("Next file!");
    m = m + 1; //constructing new file name
    itoa(m, str, 10);
    strcat(str, ".txt");
    myFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC); //opening new text file
    if (!myFile) {
      Serial.println("File open failed");
      return;
    }
    d = 0;
  }
  else
  {
    myFile = SD.open(str, FILE_WRITE); //opening current text file
    if (! myFile) {
      Serial.println("err opng file");
      return;
    }
  }
}

void reset() //reseting variables
{
  scanCount = 0;
  sampleCount = 0;
  Serial.flush();
  digitalWrite(6, LOW);
  currentState = 0;
  d = 0;
}
