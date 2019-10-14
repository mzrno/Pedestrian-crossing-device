/*
   Arduino code for sending commands to Slave.ino 
   Uses Arduino Nano 2.0 and Nordic nrf2401l radio modem 
   Typing "s" in Serial Monitor the code sends "S" char data to Slave.ino code.
   Typing "f" in Serial Monitor the code sends "F" char data to Slave.ino code.
   
   Created by Mario Zrno, 19 July 2019.
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
//declaration of PINs for Nordic 2401l modem
#define CE_PIN   9
#define CSN_PIN 10
//strings for gathering user input in Serial
String startInput = "";
String nextFile = "";
char d[] = "SF"; //char data intended for sending
int c; //variable used for questioning certain states
//states used in switch-case loop
const uint8_t A = 0;
const uint8_t B = 1;
const uint8_t C = 2;
RF24 radio(CE_PIN, CSN_PIN); //setting nRF24LO1 radio to SPI bus on pins CE_PIN and CSN_PIN
void setup() {
  Serial.begin(115200); //serial terminal for computer
  Serial.println("Nrf24L01 Receiver Starting");
  radio.begin();
  radio.setPayloadSize(4); //setting the largest payload size for sending
  radio.openWritingPipe(0xF0F0F0F0E1LL); //open pipe for writing and sending data
  startInput.reserve(1000); //reserved space in memeory for user input on startInput
  nextFile.reserve(1000); //reserved space in memeory for user input on nextFile
  radio.startListening(); //start listening
}

void loop() {
  radio.stopListening(); //stop listening
  switch (c)
  { //condition loop test
    case A:
      c = StartInput() ? C : B;
      break;
    case B:
      c = NextFile() ? C : A;
      break;
    case C:
      RESET();
      break;
  }
  radio.startListening(); //start listening
}

bool StartInput() //executed function when user inserts "s"
{
  while (Serial.available())
  {
    startInput += (char)Serial.read();
  }
  if (startInput.indexOf("s") != -1)
  {
    radio.write(&d[0], sizeof(d)); //send data
    Serial.println("Start!");
    return true;
  }
  return false;
}

bool NextFile() //executed function when user inserts "f"
{
  while (Serial.available())
  {
    nextFile += (char)Serial.read();
  }
  if (nextFile.indexOf("f") != -1)
  {
    radio.write(&d[1], sizeof(d)); //send data
    Serial.println("Next file!");
    return true;
  }
  return false;
}

void RESET() //function for reseting variables
{
  startInput = "";
  nextFile = "";
  c = 0;
}
