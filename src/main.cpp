// ServoPrint V1.0
// Bij P.M. de Heij   11-7-2020
// status: ontwikkeling

#include <Arduino.h>
#include <EEPROM.h>
#include <Adafruit_SoftServo.h>

// Pin defenities

const int ServoPin = PB0;   // Servo Pin
const int DKRechts = PB1;  // Drukknop Rechts
const int DKLinks = PB2;   // Drukknop Links
const int StatusLed = PB3;  // Status Led
const int RelConf = PB4;   // Relais of configuratie

/* 
   EEPROM GEBRUIK

   1    DKMode   
        1 = Drukknoppen links en rechts
        2 = Schakelaar op drukknop Rechts

   2    EindLinks
        0-180 voor eindstand servo links

   3    EindRechts
        0-180 voor eindstand servo Rechts

   4    Snelheid
        0-255 voor snelheid in stappen van 500 ms

   5    RelMod
        1 = Schakelt om bij servo tussen 2 stappen
        2 = knippert bij servo links
        3 = knippert bij servo rechts
   
   6    RelKnip
        0-255 voor snelheid knipperen relais in stapjes van 100 ms
        
*/
int DKMode = 1;       //DK Mode 
int EindLinks = 10;  // eind stand servo links
int EindRechts = 140;  // eind stand servo rechts
int Snelheid = 10;  // snelheid servo
int RelMod = 1;  // Relais mode 
int RelKnip = 2; // Relais knipper mode
int Hoek = EindRechts - EindLinks; //Hoek verstelling
int RelaisOm = Hoek/2;

// Variable
int Mode = 0;

Adafruit_SoftServo Servo;

void Mode1()  // Drukknoppen met relais om tijdens servo
{
while(1){

   if(digitalRead(DKLinks)== 0)  //Drukknop Links
  { 
    int Stel = EindRechts; // Begin stand

      // Zet servo naar links en halverwegen het relais 
     
     for (int i = 0  ; i < Hoek; i++ )
     {
         digitalWrite(StatusLed,LOW);      
         Servo.write(Stel);
         delay(Snelheid);
         Stel=Stel-1;
         if (i == RelaisOm)
          {
           digitalWrite(RelConf, LOW);
          } 
         digitalWrite(StatusLed,HIGH);
         delay(Snelheid);   
     }       
  }

  if(digitalRead(DKRechts)== 0)
  { 
    int Stel = EindLinks; // Begin stand

      // Zet Servo naar Links en halverwegen het relais

     for (int i = 0 ; i < Hoek ; i++ )
     {
        digitalWrite(StatusLed,LOW);
        Servo.write(Stel);
        delay(Snelheid);
        Stel=Stel+1;
        if (i == RelaisOm)
         { 
           digitalWrite(RelConf, HIGH);
         }
        digitalWrite(StatusLed,HIGH);
        delay(Snelheid); 
     }
  }

}

}

void Mode2()
{


}





void setup() {
OCR0A = 0xAF;         // elk nummer is goed
TIMSK |= _BV(OCIE0A);  // Zet comperator interupt aan


 // zet pinnen in juiste stand

  pinMode(DKRechts,INPUT_PULLUP);  // drukknop intern pull up
  pinMode(DKLinks, INPUT_PULLUP);  // drukknop intern pull up
  pinMode(StatusLed, OUTPUT);
  pinMode(RelConf, OUTPUT);

 // Servo op pin ServoPin

  Servo.attach(ServoPin);



}

void loop() {
  
// Bepaal werking

if (DKMode == 1 && RelMod == 1 ){
  Mode1();
}
  


  
  
  
  
  
 
}



// Refresh elke 20 miliseconden servo met interupt timer 0
volatile uint8_t counter = 0;
SIGNAL(TIMER0_COMPA_vect)
{
  counter +=2;
  if (counter >= 20)
  {
    counter = 0;
    Servo.refresh();
  }

}