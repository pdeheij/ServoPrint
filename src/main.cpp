// ServoPrint V1.0
// Bij P.M. de Heij   11-7-2020
// status: ontwikkeling

#include <Arduino.h>
#include <EEPROM.h>
#include <Adafruit_SoftServo.h>
#include <Adafruit_NeoPixel.h>
#include <Bounce2.h>




// Pin defenities

const int ServoPin = PB4;   // Servo Pin
const int BUTTON_RECHTS = PB1;  // Drukknop Rechts
const int BUTTON_LINKS = PB2;   // Drukknop Links
const int Status = PB3;  // Status Led
const int RelConf = PB0;   // Relais of configuratie
const int LedCount = 1;

Bounce DKRechts = Bounce();
Bounce DKLinks = Bounce();
/*
   EEPROM GEBRUIK
   0    Configuratie aanwezig dan is deze 1

   1    EindLinks
        0-180 voor eindstand servo links

   2    EindRechts
        0-180 voor eindstand servo Rechts

   3    Snelheid
        1-255 voor snelheid in stappen van 5 ms

   4    Voorkeur
        1 = Links voorkeur opstarten
        2 = Rechts voorkeur opstarten

   5    DKMode
        1 = Drukknoppen links en rechts *
        2 = Schakelaar op drukknop Rechts

   6    LedMod
        1 = Uit
        2 = Flash
        3 = knippert tijdens servo
        2 = knippert bij servo links
        3 = knippert bij servo rechts

   7    LedKnip
        0-255 voor snelheid knipperen relais in stapjes van 5 ms *



*/



int DKMode = 1;       //DK Mode 
int EindLinks = 180;  // eind stand servo links
int EindRechts = 5;  // eind stand servo rechts
const long Snelheid = 5*5;  // snelheid servo
int RelMod = 1;  // Relais mode 
const long RelKnip = 100*5; // Relais knipper snelhied
int VoorKeur = 0; // Voorkeur opstart stand
int Configuratie = 1;

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
Adafruit_NeoPixel StatusLed(LedCount, Status, NEO_GRB + NEO_KHZ800);

// Configureer mode als jumper op dk staat

void Configureer()
{
    Servo.write(90); // zet servo in middenstand
    StatusLed.setPixelColor(0, 0, 0, 255);  //statusled gaat naar programeer stand
    StatusLed.show();
    int ProgrammerStap = 1;  //Terugmelding via statusled
    const long FlashShort = 100;  // snelknipperen
    // const long Pauze = 500; // pauzen tussen snelknipperen
    int TimerPrevious = 0; // Voor de timer
    int CalLinks=110;
    int CalRechts=70;


    
    
      
      




    while (digitalRead(RelConf == 1)) {    
}
    delay(1000);
    while (Mode == 1)
    {
        currentMillis = millis();  // Actueel teller millis
        DKRechts.update();
        DKLinks.update();

        switch (ProgrammerStap)
        {
        case  1:

            StatusLed.setPixelColor(0, 255, 0, 0);
            StatusLed.show();
            Servo.write(CalLinks);
            if (DKLinks.read() == 1) CalLinks=CalLinks+1;
            delay(100);
            if (DKRechts.read() == 1) CalLinks=CalLinks-1;
            delay(100);
            if (digitalRead(RelConf) == 0){
              EEPROM.write(1,CalLinks);
              ProgrammerStap = 2;
              EindLinks = CalLinks;
              delay(1000);
            }
        
            break;
        case 2:
            StatusLed.setPixelColor(0, 0, 255, 0);
            StatusLed.show();
            Servo.write(CalRechts);
            if (DKLinks.read() == 1) CalRechts=CalRechts-1;
            delay(100);
            if (DKRechts.read() == 1) CalRechts=CalRechts+1;
            delay(100);
            if (digitalRead(RelConf) == 0){
              EEPROM.write(2,CalRechts);
              ProgrammerStap = 3;
              EindRechts = CalRechts;
              delay(1000);

            }

            break;
        case 3:
            Mode = 0;
            break;

        }





        // statusLed knipper routine
        if (currentMillis - TimerPrevious >= FlashShort)
        {
            TimerPrevious  = currentMillis;



        }



    }

 return;
}





void setup()
{
    OCR0A = 0xAF;         // elk nummer is goed
    TIMSK |= _BV(OCIE0A);  // Zet comperator interupt aan


    // zet pinnen in juiste stand

    pinMode(BUTTON_RECHTS, INPUT_PULLUP);
    pinMode(BUTTON_LINKS, INPUT_PULLUP);  // drukknop intern pull up
    pinMode(RelConf, INPUT);

    StatusLed.begin();
    StatusLed.show();

    DKRechts.attach(BUTTON_RECHTS);
    DKLinks.attach(BUTTON_LINKS);
    DKRechts.interval(10);
    DKLinks.interval(10);

    // Servo op pin ServoPin

    Servo.attach(ServoPin);

    // Controleer of we gaan programmeren

    if ((digitalRead(RelConf) == 1) | (Configuratie != 1))
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
        StatusLed.setPixelColor(0, 255, 0, 0);
        StatusLed.show();
        if (RelMod == 1) digitalWrite(RelConf, HIGH);
    }    
else {
        Servo.write(EindRechts);
        PosServo = 0;
        StatusLed.setPixelColor(0, 0, 255, 0);
        StatusLed.show();
        if (RelMod == 1)digitalWrite(RelConf, LOW);
    }


}

void loop()
{
    currentMillis = millis();  // Actueel teller millis     
    DKRechts.update();
    DKLinks.update();

    if (DKRechts.read() == 0 && MomRechts == 0 && PosServo == 0)
    {
        MomLinks = 1;
        Stel = EindRechts; // Begin stand


    }

    if (DKLinks.read() == 0 && MomLinks == 0 && PosServo == 1)
    {
        MomRechts = 1;
        Stel = EindLinks;

    }

    // Servo stel routine  

    if (currentMillis - ServoMillis >= Snelheid)
    {
        ServoMillis = currentMillis;

        // Naar Links
        if (Stel != EindLinks && MomLinks == 1)
        {
            Servo.write(Stel);
            Stel = Stel + 1;

            // Zet Relais
            if (RelMod == 1 && Stel == ((EindLinks-EindRechts)/2)) {

                digitalWrite(RelConf, HIGH);
                StatusLed.setPixelColor(0, 255, 0, 0);
                StatusLed.show();
            }
            // is de eindstand bereikt?
            if (Stel == EindLinks)
            {
                MomLinks = 0;
                PosServo = 1;
            }

        }

        // Naar Rechts
        if (Stel != EindRechts && MomRechts == 1)
        {
            Servo.write(Stel);
            Stel = Stel - 1;

            // Zet Relais
            if (RelMod == 1 && Stel == ((EindLinks-EindRechts)/2))
            {
                digitalWrite(RelConf, LOW);
                StatusLed.setPixelColor(0, 0, 255, 0);
                StatusLed.show();
            }
            // Is de eindstand bereikt ?          
            if (Stel == EindRechts)
            {
                MomRechts = 0;
                PosServo = 0;

            }

        }

    }

    //Relais knipperen in eindstand    

    if (currentMillis - RelaisMillis >= RelKnip)
    {
        RelaisMillis = currentMillis;

        if ((PosServo == 1 && RelMod == 2) | (MomRechts == 1))
        {
            if (RelStatus == LOW)
            {
                RelStatus = HIGH;
            }            
else {
                RelStatus = LOW;
            }
        }

        if ((PosServo == 0 && RelMod == 3) | (MomLinks ==1))
        {
            if (RelStatus == LOW)
            {
                RelStatus = HIGH;
            }            
else {
                RelStatus = LOW;
            }
        }

        if (RelMod == 2 || RelMod == 3)digitalWrite(RelConf, RelStatus);
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