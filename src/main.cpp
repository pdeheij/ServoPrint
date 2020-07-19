// ServoPrint V1.0
// Bij P.M. de Heij   11-7-2020
// status: ontwikkeling

#include <Arduino.h>
#include <EEPROM.h>
#include <Adafruit_SoftServo.h>

// Pin defenities

const int ServoPin = PB4;   // Servo Pin
const int DKRechts = PB1;  // Drukknop Rechts
const int DKLinks = PB2;   // Drukknop Links
const int StatusLed = PB3;  // Status Led
const int RelConf = PB0;   // Relais of configuratie

/* 
   EEPROM GEBRUIK

   1    DKMode   
        1 = Drukknoppen links en rechts *
        2 = Schakelaar op drukknop Rechts

   2    EindLinks
        0-180 voor eindstand servo links

   3    EindRechts
        0-180 voor eindstand servo Rechts

   4    Snelheid
        1-255 voor snelheid in stappen van 5 ms

   5    RelMod
        1 = Schakelt om bij servo tussen 2 stappen *
        2 = knippert bij servo links 
        3 = knippert bij servo rechts 
   
   6    RelKnip
        0-255 voor snelheid knipperen relais in stapjes van 5 ms *

   7    Voorkeur
        1 = Links voorkeur opstarten
        2 = Rechts voorkeur opstarten     
        
*/
int DKMode = 1;       //DK Mode 
int EindLinks = 140;  // eind stand servo links
int EindRechts = 10;  // eind stand servo rechts
int Snelheid = 1*5;  // snelheid servo
int RelMod = 3;  // Relais mode 
int RelKnip = 100*5; // Relais knipper snelhied
int VoorKeur = 2; // Voorkeur opstart stand


// Variable
int Mode = 0; // Welke mode zitten we
int MomLinks = 0; // Marker links ingedrukt
int MomRechts = 0; // Marker rechts ingedrukt
int ContLinks = 0;
int ContRechts = 0;
int Stel; // Servo positie om te settten
int ledState = LOW;
int PosServo; // Actuele servo positie 0 = Rechts 1 = Links
int RelStatus = 0;


unsigned long ServoMillis = 0; // voor servo
unsigned long RelaisMillis = 0; // voor knipperen relais
unsigned long currentMillis = millis();


Adafruit_SoftServo Servo;

// Configureer mode als jumper op dk staat

void Configureer()
{ while (Mode == 1)
  {
    digitalWrite(StatusLed, LOW);
  }


}





void setup() 
{
  OCR0A = 0xAF;         // elk nummer is goed
  TIMSK |= _BV(OCIE0A);  // Zet comperator interupt aan


  // zet pinnen in juiste stand

  pinMode(DKRechts,INPUT_PULLUP);  // drukknop intern pull up
  pinMode(DKLinks, INPUT_PULLUP);  // drukknop intern pull up
  pinMode(StatusLed, OUTPUT);
  pinMode(RelConf, INPUT);

  // Servo op pin ServoPin

  Servo.attach(ServoPin);

  // Controleer of we gaan programmeren

   if (digitalRead(RelConf)==1)
   {
     Mode=1;
     Configureer();
   } 

   pinMode(RelConf, OUTPUT);


  // Servo in voorkeur stand
  
  if (VoorKeur == 1)
  {
    Servo.write(EindLinks);
    PosServo = 1;
    if(RelMod == 1) digitalWrite(RelConf, HIGH);
  }else{
    Servo.write(EindRechts);
    PosServo = 0;
    if(RelMod == 1)digitalWrite(RelConf, LOW);  
  }
  digitalWrite(StatusLed, HIGH);

}

void loop() 
{
  currentMillis = millis();  // Actueel teller millis     


  if (digitalRead(DKLinks) == 0 && MomRechts == 0 && PosServo == 0)
    {
       MomLinks = 1;
       Stel = EindRechts; // Begin stand
       digitalWrite(StatusLed, LOW);
      
    }
  
  if (digitalRead(DKRechts) == 0 && MomLinks == 0 && PosServo == 1)
    { 
       MomRechts = 1;
       Stel = EindLinks;
       digitalWrite(StatusLed, LOW);
    }

  // Servo stel routine  
  
  if (currentMillis - ServoMillis >= Snelheid)
      { 
        ServoMillis = currentMillis;
      
        // Naar Links
        if ( Stel != EindLinks && MomLinks == 1 )
        {
          Servo.write(Stel);
          Stel = Stel + 1;
          
          // Zet Relais
          if (RelMod == 1 && Stel == ((EindLinks-EindRechts)/2)) digitalWrite(RelConf, HIGH);
          
          // is de eindstand bereikt?
          if (Stel == EindLinks)
           {
             MomLinks = 0;
             PosServo = 1;
             digitalWrite(StatusLed, HIGH);
  
           }

        }

        // Naar Rechts
        if ( Stel != EindRechts && MomRechts == 1 )
        {
          Servo.write(Stel);
          Stel = Stel - 1;

          // Zet Relais
          if (RelMod == 1 && Stel == ((EindLinks-EindRechts)/2)) digitalWrite(RelConf, LOW);
        
          // Is de eindstand bereikt ?          
          if (Stel == EindRechts)
           {
             MomRechts = 0;
             PosServo = 0;
             digitalWrite(StatusLed, HIGH);
           }

        }

        }

    //Relais knipperen in eindstand    
    
   if (currentMillis - RelaisMillis >= RelKnip)
   {
     RelaisMillis = currentMillis;

    if( PosServo == 1 && RelMod == 2)
    { 
      if (RelStatus == LOW)
      { 
        RelStatus = HIGH;
      }else{
        RelStatus = LOW;
      }  
    }

    if( PosServo == 0 && RelMod == 3)
    { 
      if (RelStatus == LOW)
      { 
        RelStatus = HIGH;
      }else{
        RelStatus = LOW;
      }  
    }

     if(RelMod == 2 || RelMod == 3)digitalWrite(RelConf, RelStatus);
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