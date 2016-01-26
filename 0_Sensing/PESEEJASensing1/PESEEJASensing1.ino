
/*
    PIN Layout for RF24 Libraryd
    
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
#include <math.h>

#define LED_PIN 8
#define CE_PIN   9
#define CSN_PIN 10
#define DEVICE_ID 1
#define CHANNEL 1 //MAX 127

const uint64_t pipe = 0xE8E8F0F0E1LL; // Define the transmit pipe
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio
float stationID=1.00; // The station ID corresponding to this radio
SimpleTimer timer;// the timer object

int sensorPin=A0;
float zeroValue=212;
float maxValue=800;
float maxValueInGr=80000;
int sendValuesPeriode=1; //in minutes

void setup() {
  
  Serial.begin(9600);
  
  startRadio();

  float periode=5000.00;//sendValuesPeriode*60.00*1000.00;
  
  Serial.print("Ready to send values. Periode=");
  Serial.print(periode);
  Serial.println(" milis");
  
  timer.setInterval(periode, sendCurrentMeasurement);

  pinMode(LED_PIN, OUTPUT);
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

void sendReadingViaRadio(float measuredValue)
{
  float value[2];
  
  value[0] = stationID;
  value[1] = measuredValue;
    
  radio.write(value, sizeof(value) );
  
  Serial.print("Enviado:");
  Serial.print(measuredValue);
  Serial.print("kg\n");

  digitalWrite(LED_PIN, HIGH);
  delay(3000);
  digitalWrite(LED_PIN, LOW);
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

float readWeightValueFromSensor()
{
  float measuredValue = takeMeasurement();
  float fixedValue=measuredValue;
  
  if (measuredValue<zeroValue)
    fixedValue=zeroValue;

  float valueInKg=0;

  valueInKg= 0.160*fixedValue-34.553; // Equação da Reta: y = 0,160X - 34,561                    
  
  if (valueInKg<0)
    valueInKg=0;

  float roundedValue=round(valueInKg*2)/2;

  Serial.print("Valor em Volt:");
  Serial.println(measuredValue);
  Serial.print("Valor em Kg:");
  Serial.println(roundedValue);
  
  return roundedValue;
}

float takeMeasurement()
{
  int total=0;
  for (int i=0; i<50; i++)
  {
    total+=analogRead(sensorPin);
    delay (50);
  }
  
  float media=total/50;
  
  return media;
}

float mediamovelk(float xis)
{
  static float media = 0.0;
  static int indice = 1;
  if(indice ==0 || indice == 50)
  {
    indice = 1;
    media = 0 ;
  }

  media = media + (xis - media) / indice++;
  
  return media;
} 
 

