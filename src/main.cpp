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
const int LedCount = 1; // aantal Leds
const int Rechts = 0; //Rechts = 0
const int Links = 1; // Links = 1


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



int DKMode = 2;//DK Mode 
int EindLinks = 180;  // eind stand servo links
int EindRechts = 5;  // eind stand servo rechts
const long Snelheid = 5*5;  // snelheid servo
int RelMod = 1;  // Relais mode 
const long RelKnip = 100*5; // Relais knipper snelhied
int VoorKeur = 1; // Voorkeur opstart stand
int Configuratie = 1;

// Variable
int Mode = 0; // Welke mode zitten we
int NaarLinks = 0; // Marker links ingedrukt
int NaarRechts = 0; // Marker rechts ingedrukt
int ContLinks = 0;
int ContRechts = 0;
int Stel; // Servo positie om te settten
int ledState = LOW;
int PosServo; // Actuele servo positie 0 = Rechts 1 = Links
int RelStatus = 0;
int Vergrendel = 0;


unsigned long ServoMillis = 0; // voor snelheid dervo
unsigned long RelaisMillis = 0; // voor snelheid LED
unsigned long currentMillis = millis();

// class defenities

Adafruit_SoftServo Servo;
Adafruit_NeoPixel StatusLed(LedCount, Status, NEO_GRB + NEO_KHZ800);
Bounce DKRechts = Bounce();
Bounce DKLinks = Bounce();






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
    // Zet timer interupt voor servopuls    
    OCR0A = 0xAF;         // elk nummer is goed
    TIMSK |= _BV(OCIE0A);  // Zet comperator interupt aan


    // zet pinnen in juiste stand
    pinMode(BUTTON_RECHTS, INPUT_PULLUP); // ingang met Pullup
    pinMode(BUTTON_LINKS, INPUT_PULLUP);  // ingang met pulup
    pinMode(RelConf, INPUT); // ingang zonder pullup voor mode 

    // start en reset NeoPixel 
    StatusLed.begin();   
    StatusLed.show();

    // Debounce de drukknoppen links en rechts
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

    // zet PB4 terug op uitgang voor relais
    pinMode(RelConf, OUTPUT);
   

    // Servo in voorkeur stand
    if (VoorKeur == 1 | digitalRead(BUTTON_RECHTS == 0))
    {
        Servo.write(EindLinks);
        PosServo = Links;
        StatusLed.setPixelColor(0, 255, 0, 0);
        StatusLed.show();
        if (RelMod == 1) digitalWrite(RelConf, HIGH);
    }
    else 
    {
        Servo.write(EindRechts);
        PosServo = Rechts;
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

    // Drukknop servo naar rechts ingedrukt 
    if (DKRechts.read() == 0 &&  PosServo == Links && DKMode == 1)
    {
        Stel = EindLinks; // Begin stand
        NaarRechts = 1; // servo gaat naar rechts
    }

    // Drukknop servo naar links ingedrukt
    if (DKLinks.read() == 0 && PosServo == Rechts && DKMode == 1)
    {
        Stel = EindRechts; //Begin stand
        NaarLinks = 1; //servo gaat naar links
    }

    if (DKRechts.read() == 0 && PosServo == Links && DKMode == 2 && Vergrendel == 0)
    {
        Stel = EindLinks; //Begin stand
        NaarRechts = 1; //servo gaat naar links
        Vergrendel = 1;
    }

    if (DKRechts.read() == 1 && PosServo == Rechts && DKMode == 2 && Vergrendel == 0)
    {
        Stel = EindRechts; //Begin stand
        NaarLinks = 1; //servo gaat naar Rechts
        Vergrendel = 1;
    }

    // Servo stel routine  
    if (currentMillis - ServoMillis >= Snelheid)
    {
        ServoMillis = currentMillis;

        // Naar Links
        if (Stel != EindLinks && NaarLinks == 1)
        {
            Servo.write(Stel);
            Stel = Stel + 1;

            // Zet Relais
            if (RelMod == 1 && Stel == ((EindLinks-EindRechts)/2))
            {

                digitalWrite(RelConf, HIGH);
                StatusLed.setPixelColor(0, 255, 0, 0);
                StatusLed.show();
            }
            // is de eindstand bereikt?
            if (Stel == EindLinks)
            {
                NaarLinks = 0;
                PosServo = Links;
                Vergrendel = 0;
            }

        }

        // Naar Rechts
        if (Stel != EindRechts && NaarRechts == 1)
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
                NaarRechts = 0;
                PosServo = Rechts;
                Vergrendel = 0;

            }

        }

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