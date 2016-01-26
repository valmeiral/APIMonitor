int sensorPin=A0;

float zeroValue=212;
float maxValue=800;
float maxValueInGr=80000;

void setup() {
  
  Serial.begin(9600);
  delay(1000);
}

void loop() {
  float measuredValue = TakeMeasurement();
  float fixedValue=measuredValue;
  
  if (measuredValue<zeroValue)
    fixedValue=zeroValue;
    
  if (measuredValue>maxValue)
    fixedValue=maxValue;
  
  float valueInGrams= map(fixedValue, zeroValue, maxValue, 0, maxValueInGr);                    
  float valueInKg=valueInGrams/1000;
 
//  samplek =  map(samplek, 71, 83, 0, 5000);                    
    
  //samplek=mediamovelk(samplek);
    
  Serial.print("Valor Medido: ");
  Serial.print(measuredValue);
  Serial.println("");
  
  Serial.print("Valor Corrigido: ");
  Serial.print(fixedValue);
  Serial.println("");
  
  Serial.print("Valor em Kg: ");
  Serial.print(valueInKg);
  Serial.println("");
  
  Serial.print("");
  Serial.println("..................");
   
  delay (500);
}

float TakeMeasurement()
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
 

