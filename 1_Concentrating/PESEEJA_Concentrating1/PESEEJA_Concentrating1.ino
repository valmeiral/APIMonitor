/*
    PIN Layout for RF24 Library
    
   1 - GND
   2 - VCC 3.3V !!! NOT 5V
   3 - CE to Arduino pin 9
   4 - CSN to Arduino pin 10
   5 - SCK to Arduino pin 13
   6 - MOSI to Arduino pin 11
   7 - MISO to Arduino pin 12
   8 - UNUSED
   - 
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include "Adafruit_FONA.h"
#include <SoftwareSerial.h>

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

#define LED_PIN 8
#define CE_PIN   9
#define CSN_PIN 10
#define DEVICE_ID 2
#define CHANNEL 1 //MAX 127

const uint64_t pipe = 0xE8E8F0F0E1LL; // Define the transmit pipe
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
uint8_t type;

char baseStationNumber[]="965800413"; //Put here the base station Phone Number for SMS
char apiaryID[]="1"; //Put here the apiary ID

void setup()   /****** SETUP: RUNS ONCE ******/
{
  Serial.begin(9600);
  
  delay(1000);
  
  initializeFona();
  
  startRadio();

  pinMode(LED_PIN, OUTPUT);

  Serial.println("Ready to receive values.");
}


void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
  if ( radio.available() )
  {
      float values[2];  // 2 element array holding the received values
      // Fetch the data payload
      radio.read( values, sizeof(values) );

      Serial.println("Value Received:");
      Serial.print("  Station ID: ");
      Serial.print(values[0]);
      Serial.print("\n  Weight measurement: ");      
      Serial.println(values[1]);

       digitalWrite(LED_PIN, HIGH);
       delay(3000);
       digitalWrite(LED_PIN, LOW);
      
      sendValuesBySMS(values);
  }
  else
  {    
      //Serial.println("No radio available");
  }

  //delay(5000);

}

void startRadio(){
  Serial.println("Nrf24L01 Receiver Starting");
  
  radio.begin(); // Start up the radio
  radio.setChannel(CHANNEL);
  radio.setAutoAck(1);     // Ensure autoACK is enabled
  radio.setRetries(15,15); // Max delay between retries & number of retries
  radio.openReadingPipe(1,pipe);
  radio.startListening();;
  
  Serial.print("Radio Power is Up\n");
}

void stopRadio(){
  
  radio.powerDown();
  
  pinMode(13, LOW);
  pinMode(12, LOW);
  pinMode(11, LOW);
  pinMode(10, LOW);
  pinMode(9, LOW);
  
  Serial.print("Radio Power is Down\n");
}

void initializeFona()
{
  Serial.println("Initializing Fona");
  
  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    //while (1);
  }
  type = fona.type();
  Serial.println(F("FONA is OK"));
  Serial.print(F("Found "));
  switch (type) {
    case FONA800L:
      Serial.println(F("FONA 800L")); break;
    case FONA800H:
      Serial.println(F("FONA 800H")); break;
    case FONA808_V1:
      Serial.println(F("FONA 808 (v1)")); break;
    case FONA808_V2:
      Serial.println(F("FONA 808 (v2)")); break;
    case FONA3G_A:
      Serial.println(F("FONA 3G (American)")); break;
    case FONA3G_E:
      Serial.println(F("FONA 3G (European)")); break;
    default: 
      Serial.println(F("???")); break;
  }
  
  // Print module IMEI number.
  char imei[15] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("Module IMEI: "); 
    Serial.println(imei);
  }  

  delay(5000);

  Serial.println("Fona is Initialized");
}

void sendValuesBySMS(float values[]){
    //Message Format
    //Semicolon separated values
    //The message format is the following:
    //  First: The sender base station type; API
    //  Second: The sender base station ID; Apiary ID
    //  Third: The sender sensor ID; Hive ID
    //  Fourth: The weigth value

    String separator=";";
    String stationType="API";

    String message=stationType + separator + apiaryID + separator + String(values[0],0) + separator + String(values[1],4);
    char messageChar[]="";
    
    message.toCharArray(messageChar,140);
           
    bool smsWasSent=false;
    int numberOfRetries=0;

    while (!smsWasSent && numberOfRetries<3)
    {
      Serial.println("Sending SMS");
      Serial.print(F("Send to #"));
      Serial.println(baseStationNumber);
      Serial.println(message);
      
      smsWasSent=fona.sendSMS(baseStationNumber, messageChar);
      if (!smsWasSent) 
      {
        Serial.println(F("Failed"));
        numberOfRetries++;
        delay(5000);
      } 
      else 
      {
        Serial.println(F("Sent!"));
      }
      Serial.println("----------------------------");
    }

    Serial.println(F("Ending"));
}


