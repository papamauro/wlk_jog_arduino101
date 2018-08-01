#include <MemoryFree.h>
#include <SPI.h>
#include <SD.h>
#include "CurieIMU.h"
#include "CuriePME.h"

const unsigned int vectorNumBytes = 128;
const unsigned int sensorBufSize = 2048;

unsigned int wlk[25]={3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
unsigned int jog[25]={3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};

const int DataSetMin = -32768;
const int DataSetMax = 32768;

void trainMovements();
void trainMovement(int number, char movement);
void readFromFile(int index, String fileName, byte vector[]);

void undersample(byte samples[], byte vector[]);
byte getAverageSample(byte samples[], unsigned int num, unsigned int pos, unsigned int step);
void classifyMovements();

void setup() {
  Serial.begin(9600);
  while(!Serial);

  pinMode(10,OUTPUT);
  digitalWrite(10,HIGH);
  
  if (!SD.begin(4)) {
      Serial.println("initialization failed!");
      while (1);
    }
    
  Serial.println("initialization done.");
  CuriePME.begin();
  
  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(0));
  
  trainMovements();
  Serial.print("Jogging: ");
  for(int i = 0; i < 24; i++)
  {
    Serial.print(i+1);
    Serial.print(": ");
    Serial.print(jog[i]);
    Serial.print(" ");
  }

Serial.print("Walking: ");
    for(int i = 0; i < 24; i++)
  {
    Serial.print(i+1);
    Serial.print(": ");
    Serial.print(wlk[i]);
    Serial.print(" ");
  }
  Serial.print("\n");
  classifyMovements();
}

void loop() {}

void trainMovements(){
    int r;
    char flag=1;
    
    for ( unsigned int i = 0; i < 128; ) {
        flag = 1;
        
        
        r = (int) random(0, 24);
        
        if ( (i % 2) == 0){
          if(jog[r]>0){
            i++;
            trainMovement(r+1, 106);
            jog[r]-=1;
          }else
            flag=106;
         }else{
          if(wlk[r]>0){
            i++;
            trainMovement(r+1, 119);
            wlk[r]-=1;
            }
           else
            flag=119;}

        
          if (flag==106){
            String aux="Skipped: jog_9/sub_";
            aux+=(r+1);
            aux+=".csv";
            Serial.println(aux);}
          
          if (flag==119){
            String aux="Skipped: wlk_9/sub_";
            aux+=(r+1);
            aux+=".csv";
            Serial.println(aux);}
           
          
        if(flag==1){
          Serial.print("Training ");
          Serial.print(i);
          Serial.print("-th movement \n");
  
          }
        //delay(1000);
    }
    
}

void trainMovement(int number, char movement)
{
    String fileName="";
    int folderIndex=7;
    String folderName="wlk";
    byte vector[vectorNumBytes];

    if(movement==106){
      folderIndex=9;
      folderName="jog";
    }
    
    fileName = folderName+"_"+folderIndex+"/sub_"+number+".csv";
    

    readFromFile(number, fileName, vector);
    
    /*for(int i = 0; i < sizeof(vector); i++)
    {
      Serial.println(vector[i]);
      }*/
    CuriePME.learn(vector, vectorNumBytes, movement);

}

void readFromFile(int index, String fileName, byte vector[])
{
    double y;
    unsigned int skip;
   
    File myFile;
    Serial.print(fileName);
    Serial.print("\n");
    myFile=SD.open(fileName.c_str());

    if (myFile) {

      if(fileName.charAt(0)==106)
        skip = 500 + ((3-jog[index])*200);
      else
        skip = 500 + ((3-wlk[index])*200);
        
      for (unsigned int j = 0; j < skip && myFile.available(); j++)
        myFile.readStringUntil('\n'); //Skip lines

      for (unsigned int i = 0; i < vectorNumBytes && myFile.available(); i++) {
        
        myFile.readStringUntil(',');
        myFile.readStringUntil(',');
        String yString = myFile.readStringUntil(',');
        myFile.readStringUntil('\n');
        
        y = atof(yString.c_str());
        y = y*4096;
        
        /* Map raw values to 0-255 */
        vector[i] = (byte) map(y, DataSetMin, DataSetMax, 0, 255);
        //Serial.println(vector[i]);
        }
    }
    myFile.close();
}

void classifyMovements()
{
    String fileName="";
    String movement="";
    unsigned int category;
    byte vector[vectorNumBytes];

    for ( unsigned int i = 0; i < 24; i++) {
      for ( unsigned int j = 0; j < jog[i];) {
        String aux = "jog_9/sub_";
        fileName = aux+(i+1)+".csv";
        readFromFile(i+1, fileName, vector);
        category = CuriePME.classify(vector, vectorNumBytes);

        Serial.print("jog: ");
        
        if (category == CuriePME.noMatch) {
            Serial.print("Don't recognise that one-- try again. \n");
        } else {
            Serial.print(category);
            Serial.print("\n");}
        jog[i]--;
      }
    }


    for ( unsigned int i = 0; i < 24; i++) {
      for ( unsigned int j = 0; j < wlk[i];) {
        String aux = "wlk_7/sub_";
        fileName = aux +(i+1)+".csv";
        readFromFile(i+1, fileName, vector);
        category = CuriePME.classify(vector, vectorNumBytes);

        Serial.print("wlk: ");
        
        if (category == CuriePME.noMatch) {
            Serial.print("Don't recognise that one-- try again. \n");
        } else {
            Serial.print(category);
            Serial.print("\n");}
        wlk[i]--;
      }
    }
    
}
