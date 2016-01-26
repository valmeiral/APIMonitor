
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
#include <SimpleTimer.h>

#define CE_PIN   9
#define CSN_PIN 10
#define DEVICE_ID 1
#define CHANNEL 1 //MAX 127

const uint64_t pipe = 0xE8E8F0F0E1LL; // Define the transmit pipe
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio
float stationID=1.00; // The station ID corresponding to this radio
SimpleTimer timer;// the timer object

void setup()
{
  Serial.begin(9600);
  
//  radio.begin();
//  radio.openWritingPipe(pipe);

  startRadio();
  
  timer.setInterval(5000, sendCurrentMeasurement);
}

void loop()
{
   timer.run();
}

void sendCurrentMeasurement() {
    Serial.print("Uptime (s): ");
    Serial.println(millis() / 1000);
    
    float value=readWeightValueFromSensor();
    sendReadingViaRadio(value);
}

float readWeightValueFromSensor()
{
  float measure=10.5;
  
  return measure;
}

void sendReadingViaRadio(float measuredValue)
{
  float value[2];
  
  value[0] = stationID;
  value[1] = measuredValue;
    
  radio.write(value, sizeof(value) );
  
  Serial.print("Enviado:");
  Serial.print(measuredValue);
  Serial.print("kg\n");

}

void stopRadio(){
  
  radio.powerDown();
  
  pinMode(13, LOW);
  pinMode(12, LOW);
  pinMode(11, LOW);
  pinMode(10, LOW);
  pinMode(9, LOW);
  
  Serial.print("Power Down\n");
}

void startRadio(){
  radio.begin(); // Start up the radio
  radio.setChannel(CHANNEL);
  radio.setAutoAck(1);     // Ensure autoACK is enabled
  radio.setRetries(15,15); // Max delay between retries & number of retries
  radio.openWritingPipe(pipe);
  radio.stopListening();
  
  Serial.print("Power Up\n");
}
