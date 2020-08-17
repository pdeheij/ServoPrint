// ServoPrint V0.104b
// Bij P.M. de Heij   11-7-2020
// status: ontwikkeling

#include <Arduino.h>
#include <EEPROM.h>
#include <Adafruit_SoftServo.h>
#include <Adafruit_NeoPixel.h>
#include <Bounce2.h>




// Pin defenities

const int ServoPin = PB1;   // Servo Pin
const int BUTTON_RECHTS = PB4;  // Drukknop Rechts
const int BUTTON_LINKS = PB3;   // Drukknop Links
const int Status = PB2;  // Status Led
const int RelConf = PB5;   // Relais of configuratie
const int LED = PB0; // LED aan uitgang
const int LedCount = 1; // aantal Leds
const int Rechts = 0; //Rechts = 0
const int Links = 1; // Links = 1






// Schrijven Long naar EEPROM
void EEPROMWriteLong(int adres,long waarde)
{
    byte four = (waarde & 0xFF);
    byte three = ((waarde >> 8) & 0xFF);
    byte two = ((waarde >> 16) & 0xFF);
    byte one = ((waarde >> 24) & 0xFF);

    EEPROM.write(adres,four);
    EEPROM.write(adres + 1,three);
    EEPROM.write(adres + 2,two);
    EEPROM.write(adres + 3,one);

}

// Lezen long van EEPROM
long EEPROMReadlong(long adres)
{
    long four = EEPROM.read(adres);
    long three = EEPROM.read(adres + 1);
    long two = EEPROM.read(adres + 2);
    long one = EEPROM.read(adres + 3);

    return((four << 0) & 0xFF) + ((three  << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}




/*
   EEPROM GEBRUIK
   0    Configuratie aanwezig dan is deze 1

   1    EindLinks
        0-180 voor eindstand servo links

   2    EindRechts
        0-180 voor eindstand servo Rechts

   3,4,5,6   Snelheid
        1-255 voor snelheid in stappen van 5 ms

   7   Voorkeur
        1 = Links voorkeur opstarten
        2 = Rechts voorkeur opstarten

   8    DKMode
        1 = Drukknoppen links en rechts *
        2 = Schakelaar op drukknop Rechts

   9   LedMod
        1 = Uit
        2 = knippert altijd
        3 = Flash bij servo naar rechts
        4 = Flash bij servo naar links
        5 = Knippert tijdens Servo beweging
        6 = Knippert bij servo naar Rechts 
        7 = knippert bij servo naar rechts

   10,11,12,13    LedKnip
        0-255 voor knipper snelheid led



*/

int Configuratie = EEPROM.read(0);
int EindLinks = EEPROM.read(1);  // eind stand servo links
int EindRechts = EEPROM.read(2);  // eind stand servo rechts
int DKMode = EEPROM.read(8);//DK Mode 
int RelMod = 1;
int LedMod = 5;  // Led Mode
const long LedKnip = 100*5; // Relais knipper snelhied
const long Flash = 500;
int VoorKeur = 1; // Voorkeur opstart stand


// Variable
int Mode = 0; //Normaal 0 in progmode 1
int NaarLinks = 0; // Marker links ingedrukt
int NaarRechts = 0; // Marker rechts ingedrukt
int ContLinks = 0;
int ContRechts = 0;
int Stel; // Servo positie om te settten
int ledState = HIGH;
int endstate;
int PosServo; // Actuele servo positie 0 = Rechts 1 = Links
int RelStatus = 0;
int Vergrendel = 0;
int Goto;
int State;
int CalLinks;
int CalRechts;
int KnipperenMag = 0;
int ingedrukt = 0;

const long Rood = 0x00FF00;  // gRb
const long Groen = 0xFF0000; // Grb
const long Blauw = 0x0000FF; //grB
const long Geel = 0xFF00FF; //GrB

unsigned long ServoMillis = 0; // voor snelheid dervo
unsigned long RelaisMillis = 0; // voor snelheid LED
unsigned long Snelheid = EEPROMReadlong(3);  // snelheid servo
unsigned long currentMillis = millis();
unsigned long FlashMillis = 0;
unsigned long KnipperMillis = 0;
unsigned long ConfKnipper = 0;

// class defenities

Adafruit_SoftServo Servo;
Adafruit_NeoPixel StatusLed(LedCount, Status, NEO_GRB + NEO_KHZ800);
Bounce DKRechts = Bounce();
Bounce DKLinks = Bounce();

// tweekleuren knipper voor config
void DuoKnipper(int kleur1, int kleur2)
{
    currentMillis = millis();

    if (currentMillis - ConfKnipper >= 500)
    {
      ConfKnipper = currentMillis;
    }
}


// Configureer mode als jumper op dk staat

void Configureer()
{
    Servo.write(90); // zet servo in middenstand
    StatusLed.setPixelColor(0, 0, 230, 230);  //statusled gaat naar programeer stand
    StatusLed.show();
    int ProgrammerStap = 1;  //Terugmelding via statusled
    
   
    
    
    if(EindLinks >180){
         CalLinks = 90;
    }else
    {
         CalLinks=EindLinks;
    }

    if(EindRechts >180){
        CalRechts = 90;
    }else
    {
        
        CalRechts=EindRechts;
    }

    if(Snelheid >255)Snelheid = 128;
    
    
    










    
    while (ingedrukt == 0)
    {
        if (digitalRead(RelConf) == 0)ingedrukt = 1;
    }

    delay(1000);

    while (Mode == 1)
    {
        currentMillis = millis();  // Actueel teller millis

        DKRechts.update();
        DKLinks.update();

        switch (ProgrammerStap)
        {
        case  1:  // afstellen postitie Links

            StatusLed.setPixelColor(0, 255, 0, 0);
            StatusLed.show();
            Servo.write(CalLinks);
            if (DKLinks.read() == 0) CalLinks=CalLinks+1;
            delay(100);
            if (DKRechts.read() == 0) CalLinks=CalLinks-1;
            delay(100);
            if (digitalRead(RelConf) == 0) {
                EEPROM.write(1, CalLinks);
                ProgrammerStap = 2;
                EindLinks = CalLinks;
                EEPROM.write(EindLinks, 1);
                delay(1000);
            }

            break;
        case 2:  // afstellen postitie Rechts

            StatusLed.setPixelColor(0, 0, 255, 0);
            StatusLed.show();
            Servo.write(CalRechts);
            if (DKLinks.read() == 0) CalRechts=CalRechts+1;
            delay(100);
            if (DKRechts.read() == 0) CalRechts=CalRechts-1;
            delay(100);
            if (digitalRead(RelConf) == 0) {
                EEPROM.write(2, CalRechts);
                ProgrammerStap = 3;
                EindRechts = CalRechts;
                EEPROM.write(EindRechts, 2);
                Stel = EindRechts;
                PosServo = Rechts;
                delay(1000);

            }

            break;
        case 3: // afstellen snelheid

            if (currentMillis - ServoMillis >= Snelheid)
            {
                ServoMillis = currentMillis;
                // Naar Links
                if (Stel != EindLinks && PosServo == Rechts)
                {
                    Servo.write(Stel);
                    Stel = Stel + 1;
                    ledState = !ledState;

                    if (Stel == EindLinks)
                    {
                        PosServo = Links;
                        Stel = EindLinks;
                    }
                }
                // Naar Rechts
                if (Stel != EindRechts && PosServo == Links)
                {
                    Servo.write(Stel);
                    Stel = Stel - 1;
                    ledState = !ledState;

                    if (Stel == EindRechts)
                    {
                        PosServo = Rechts;
                        Stel = EindRechts;
                    }

                }

                if (ledState == 1 && endstate == 0)
                {
                    StatusLed.setPixelColor(0, 0, 0, 0);
                    StatusLed.show();
                }
                else
                {
                    StatusLed.setPixelColor(0, 255, 0, 0);
                    StatusLed.show();
                }

                if (DKLinks.read() == 0) {
                    Snelheid = Snelheid +1;
                    if (Snelheid >= 255)Snelheid = 255;
                }
                delay(5);
                if (DKRechts.read() == 0) {
                    Snelheid = Snelheid -1;
                    if (Snelheid <= 0)Snelheid = 0;
                }
                delay(5);
                if (digitalRead(RelConf) == 0) {

                    ProgrammerStap = 4;
                    delay(1000);
                    EEPROMWriteLong(3,Snelheid);

                }
                if (Snelheid > 253)
                {
                    StatusLed.setPixelColor(0, 128 , 0 , 128);
                    StatusLed.show();
                    endstate = 1;
                    Snelheid = 254;

                }
                else if (Snelheid < 1)
                {
                    StatusLed.setPixelColor(0, 128 , 0 , 128);
                    StatusLed.show();
                    endstate = 1;
                    Snelheid = 2;
                }
                else
                {
                    endstate = 0;
                }
                

            }

            break;

        case 4: // Drukknop mode
           
            if (DKMode == 1 )
            {
                StatusLed.setPixelColor(0,Groen);
                StatusLed.show();
            }
            else if (DKMode == 2)
            {
                StatusLed.setPixelColor(0,Rood);
                StatusLed.show();
            }
            if (DKLinks.read() == 0) DKMode=2;
            delay(5);
            if (DKRechts.read() == 0)DKMode=1;
            delay(5);

            if (digitalRead(RelConf) == 0) 
            {
                    EEPROM.write(8,DKMode); 
                    ProgrammerStap = 5;
                    delay(1000);
                    

            }    
            break;

        case 5:
            
            EEPROM.write(0,1);
            Mode = 0;

            break;    

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
    pinMode(LED, OUTPUT);
    pinMode(RelConf, INPUT); // ingang zonder pullup voor mode 

    // start en reset NeoPixel 
    StatusLed.begin();
    StatusLed.setBrightness(32);
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
        Mode = 1;
        Configureer();
    }

    // Terug uit programmeren vergeet niet de jumper terug te zetten
    while (digitalRead(RelConf) == 1 && Mode ==0)
    {
        currentMillis = millis();
        if (currentMillis - FlashMillis >= Flash)
        {
            FlashMillis = currentMillis;

            if (State == LOW)
            {
                StatusLed.setPixelColor(0, 230, 230, 0);
                State = HIGH;
            }
            else
            {
                StatusLed.setPixelColor(0, 0, 0, 0);
                State = LOW;
            }

            StatusLed.show();

        }

    }

    // zet PB4 terug op uitgang voor relais
    pinMode(RelConf, OUTPUT);
 
    // Servo in voorkeur stand (OPM 9-8 nog niet goed voor vaste schakelaar)
    if ((VoorKeur == 1) | (digitalRead(BUTTON_RECHTS == 0)))
    {
        
        Servo.write(EindLinks);
        delay(15);
        PosServo = Links;
        StatusLed.setPixelColor(0, Groen);
        StatusLed.show();
        if (RelMod == 1) digitalWrite(RelConf, HIGH);
    }
    else
    {        
        
        Servo.write(EindRechts);
        delay(15);        
        PosServo = Rechts;
        StatusLed.setPixelColor(0, Rood);
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
        Vergrendel = 1 ;       
    }

    // Drukknop servo naar links ingedrukt
    if (DKLinks.read() == 0 && PosServo == Rechts && DKMode == 1)
    {
        Stel = EindRechts; //Begin stand
        NaarLinks = 1; //servo gaat naar links 
        Vergrendel = 1;      
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
            if (RelMod == 1 && Stel == ((EindLinks-EindRechts)/2)+EindRechts)
            {

                digitalWrite(RelConf, HIGH);
                StatusLed.setPixelColor(0, Groen);
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
            if (RelMod == 1 && Stel == ((EindLinks-EindRechts)/2)+EindRechts)
            {
                digitalWrite(RelConf, LOW);
                StatusLed.setPixelColor(0, Rood);
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
     
     if ((LedMod == 2) | (LedMod == 7 && PosServo == Links && Vergrendel == 0) | (LedMod == 6 && PosServo == Rechts && Vergrendel == 0)|(LedMod == 5 && Vergrendel == 1))
     { KnipperenMag = 1;}
     else
     {
         KnipperenMag = 0;
     }
     



    // knipperroutine LED
    if (currentMillis-KnipperMillis >= LedKnip)
     { 
         KnipperMillis = currentMillis;
         if (KnipperenMag == 1 && ledState == LOW)
         {
          ledState = HIGH;
         }
         else if (KnipperenMag == 1 && ledState == HIGH)
         {
          ledState = LOW;
         }
         else if (KnipperenMag == 0) ledState = HIGH;
      
        digitalWrite(LED,ledState);
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