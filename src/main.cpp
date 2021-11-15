//#################################################################################################################//
//#################################################################################################################//
//#################################################################################################################//

//  ////////////////////////////////////////////////////////////  //
//  // HIER WERDEN DIE LICHTSCHWELLWERTE DES LDR EINGESTELLT  //  //
//  //       Hier sind Werte von 0 bis 1023 moeglich          //  //
//  //     kleine Werte = z.B 40 dunkel / Abend-Daemmerung    //  //
//     große Werte = z.B 550 hell / Morgen-Daemmerung     //
//  ////////////////////////////////////////////////////////////  //

int LDR_SW_MORGEN = 550;   // z.B 550 als Lichtschwellwert fuer das Oeffnen am Morgen
int LDR_SW_ABEND = 40;     // z.B 40 als Lichtschwellwert fuer das Schließen am Abend
int LDR_SW_LEGELICHT = 50; // z.B 50 als Lichtschwellwert fuer das Einschalten des Legelichts (JN)

//  //////////////////////////////////////////////////////////  //
//  // HIER WIRD DIE OEFFNUGS- & SCHLIESSZEIT EINGESTELLT   //  //
//  //   Eingabe ohne fuehrende Null z.B. 9 statt 09  !!!   //  //
//    Voreinstellung     AUF: 7:30     ZU: 21:45        //
//  //////////////////////////////////////////////////////////  //

int ZEIT_AUF_STD = 7;  // Zeit in Stunden fuer das Oeffnen der Klappe
int ZEIT_AUF_MIN = 30; // Zeit in Minuten fuer das Oeffnen der Klappe
int ZEIT_ZU_STD = 21;  // Zeit in Stunden fuer das Schließen der Klappe
int ZEIT_ZU_MIN = 45;  // Zeit in Minuten fuer das Schließen der Klappe
int ZEIT_RGB = 10;     // RGB-Statusanzeige in Sekunden zu jeder vollen Minute

//  //////////////////////////////////////////////////////////  //
//  // HIER WIRD DIE OEFFNUGS- & SCHLIESSZEIT EINGESTELLT   //  //
//  //   Eingabe ohne fuehrende Null z.B. 9 statt 09  !!!   //  //
//    Voreinstellung     AUF: 7:30     ZU: 21:45        //
//  //////////////////////////////////////////////////////////  //

int LEGELICHT_ZEITFENSTER_BEGINN_STD = 15; // Zeit in Stunden fuer den Beginn des Leuchtzeitfensters für das Legelicht (JN)
int LEGELICHT_ZEITFENSTER_BEGINN_MIN = 00; // Zeit in Minuten fuer den Beginn des Leuchtzeitfensters für das Legelicht (JN)
int LEGELICHT_ZEITFENSTER_ENDE_STD = 21;   // Zeit in Stunden fuer das Ende des Leuchtzeitfensters für das Legelicht (JN)
int LEGELICHT_ZEITFENSTER_ENDE_MIN = 00;   // Zeit in Minuten fuer das Ende des Leuchtzeitfensters für das Legelicht (JN)
int LEGELICHT_LEUCHTDAUER_STD = 0;
int LEGELICHT_LEUCHTDAUER_MIN = 30; // Leuchtdauer des Legelichts (JN)
int LEGELICHT_VERZÖGERUNG_SEK = 60;

//  //////////////////////////////////////////////////////////////////////  //
//  //   HIER WIRD DIE EIN- & AUSSCHALTZEIT DES RELAIS EINGESTELLT   	  //  //
//  //        Eingabe ohne fuehrende Null z.B. 9 statt 09  !!!          //  //
//  //  Bei Relaiszeiten über Mitternacht 0:00 muessen zwei getrennte   //  //
//  //       Zeiteinstellungen vorgenommen werden (siehe Beispiel)      //  //
//   z.B.   EIN1: 7:30 AUS1: 23:59,     EIN2: 00:00 AUS2: 10:30     //
//  //////////////////////////////////////////////////////////////////////  //

int REL_ZEIT_1_EIN = 1200; // Relais - EIN-Schaltzeit 1 - Eingabeformat (hhmm)
int REL_ZEIT_1_AUS = 1200; // Relais - AUS-Schaltzeit 1 - Eingabeformat (hhmm)

int REL_ZEIT_2_EIN = 1200; // Relais - EIN-Schaltzeit 2 - Eingabeformat (hhmm)
int REL_ZEIT_2_AUS = 1200; // Relais - AUS-Schaltzeit 2 - Eingabeformat (hhmm)

//#################################################################################################################//
//#################################################################################################################//
//#################################################################################################################//

//Bibliotheken

// #include <Arduino.h>
#include "Wire.h"              // I2C-Library
#include "DS3231.h"            // Library V1.0.1 von Rinky-DinkElectronics (http://www.rinkydinkelectronics.com)
#include "LiquidCrystal_I2C.h" // Library V1.1.2 von Frank de Brabander

LiquidCrystal_I2C LCD(0x27, 16, 2); // Displayadresse 0x27, 16 Zeichen, 2 Zeilen
DS3231 RTC(SDA, SCL);               // Init des DS3231-Modules über I2C-Schnittstelle
Time UHRZEIT;                       // Init Time-Structure

// Deklarieren der Variablen, Konstanten usw.

const byte TASTER_AUF = 2; // Tasterpin zum Oeffnen der HKS
const byte TASTER_ZU = 3;  // Tasterpin zum Schließen der HKS
const byte REED_OBEN = 4;  // Reed-Kontakt oben
const byte REED_UNTEN = 5; // Reed-Kontakt unten
const byte MOTOR_1 = 6;    // MOTOR_PIN +/-
const byte MOTOR_2 = 7;    // MOTOR_PIN -/+
const byte RELAIS = 13;    // Relais Control Pin
const int LDR = A0;        // Lichtsensor

bool REEDSTAT_OBEN = 0;  // Zwischenspeicher-Variable fuer REED_OBEN
bool REEDSTAT_UNTEN = 0; // Zwischenspeicher-Variable fuer REED_UNTEN
bool RTC_FLAG = 0;       // Flag für RTC-Modul

volatile bool ISR_FLAG_AUF = 0; // Flag für INTERRUPT_TASTER_AUF
volatile bool ISR_FLAG_ZU = 0;  // Flag für INTERRUPT_TASTER_ZU

int LDR_WERT = 1023;                                                                                                              // Zwischenspeicher-Variable für LDR-WERT
int DISPLAY_ON_ZEIT = 5000;                                                                                                       // Zeit in (ms) die das Display bei Tastendruck leuchtet
int ZEIT_1000er_FORMAT = 0;                                                                                                       // Variable für die Konvertierung der Zeit in eine Integer-Zahl im 1000er-Format für das Relais
int LEGELICHT_ZEITFENSTER_BEGINN_1000er_FORMAT = ((LEGELICHT_ZEITFENSTER_BEGINN_STD * 100) + (LEGELICHT_ZEITFENSTER_BEGINN_MIN)); // Speichert den Beginn des Lichtzeitfensters im 1000er-Format für das Relais (JN)
int LEGELICHT_ZEITFENSTER_ENDE_1000er_FORMAT = ((LEGELICHT_ZEITFENSTER_ENDE_STD * 100) + (LEGELICHT_ZEITFENSTER_ENDE_MIN));       // Speichert den Beginn des Lichtzeitfensters im 1000er-Format für das Relais (JN)
int LEGELICHT_LEUCHTDAUER_1000er_FORMAT = ((LEGELICHT_LEUCHTDAUER_STD * 100) + (LEGELICHT_LEUCHTDAUER_MIN));
int LEGELICHT_ENDZEIT_1000er_FORMAT = -1; // Speichert die Ausschaltuhrzeit des Legelichts basierend auf dem helligkeitsgesteuerten Einnschalten und der festgelegten Leuchtdauer (JN)
int LEGELICHT_START_COUNTER = 0;
unsigned long ZEITWERT = 0; // Zwischenspeicher-Variable für ZEITWERT

int LED_GRUEN = 9; // RGB GRUEN
int LED_ROT = 10;  // RGB ROT
int LED_BLAU = 11; // RGB BLAU

int BLINK_ON = 500;  // Zeitdauer in (ms) fuer die RGB-BLINK-EIN-ZEIT der RGB beim Oeffnen & Schließen
int BLINK_OFF = 200; // Zeitdauer in (ms) fuer die RGB-BLINK-AUS-ZEIT der RGB beim Oeffnen & Schließen

//#################################################################################################################//
//#################################################################################################################//
//#################################################################################################################//

void setup()
{

  RTC.begin(); // Init des RTC-Objektes

  /////////////////////////////////////////////////////////////////////////////////////////////
  //             Fuer das einmalige Zeiteinstellen auskommentieren und hochladen.            //
  //          [Format (std,min,sec)]     Eingabe ohne fuehrende Null z.B. 9 statt 09  !!!    //
  //     Im Anschluss an das Zeitstellen wieder einkommentieren und so nochmals hochladen    //
  /////////////////////////////////////////////////////////////////////////////////////////////

  //RTC.setTime(8,5,00);                           // Nach dem Einstellen(Std, Min, Sec)und Hochladen nochmals einkommentieren und nochmals hochladen
  //RTC.setDate(24,10,2021);                       // Optional kann auch das Datum eingestellt werden, wird jedoch nicht benoetigt

  Serial.begin(9600); // Init fuer Serial-Monitor

  WELCOME(); // Funktion WELCOME siehe unten

  pinMode(MOTOR_1, OUTPUT);          // Motoranschluss_1 als Ausgaenge deklarieren
  pinMode(MOTOR_2, OUTPUT);          // Motoranschluss_2 als Ausgaenge deklarieren
  pinMode(TASTER_AUF, INPUT_PULLUP); // Taster-Anschluss OBEN mit internem Pullup
  pinMode(TASTER_ZU, INPUT_PULLUP);  // Taster-Anschluss UNTEN mit internem Pullup
  pinMode(REED_OBEN, INPUT_PULLUP);  // Reed-Anschluss OBEN mit internem Pullup
  pinMode(REED_UNTEN, INPUT_PULLUP); // Reed-Anschluss UNTEN mit internem Pullup
  pinMode(RELAIS, OUTPUT);           // Relais als Ausgang deklarieren
  pinMode(LED_BLAU, OUTPUT);         // Blauer-RGB-Anschluss als Ausgang deklarieren
  pinMode(LED_ROT, OUTPUT);          // Roter-RGB-Anschluss als Ausgabg deklarieren
  pinMode(LED_GRUEN, OUTPUT);        // Gruener-RGB-Anschluss als Ausgang deklarieren

  attachInterrupt(0, INTERRUPT_TASTER_AUF, LOW); // Interrupt fuer Taster_AUF
  attachInterrupt(1, INTERRUPT_TASTER_ZU, LOW);  // Interrupt fuer Taster_ZU

  while (!RTC_FLAG) // Pruefroutine fuer das RTC-Modul beim Programmstart

  {
    Serial.println("Suche RTC-Modul: ....");
    LCD.clear();
    LCD.backlight();
    LCD.setCursor(0, 0);
    LCD.print("PRUEFE ZEITMODUL");

    for (int i = 0; i <= 13; i++)
    {
      LCD.setCursor(i, 1);
      LCD.print(".");
      delay(200);
    }

    RTC.getTime(); // Einlesen der Zeit vom DS3231
    Serial.println("RTC-Modul gefunden!");
    LCD.println("OK");
    RTC_FLAG = 1;
    delay(5000);
  }

  LCD.noBacklight();
}

//#################################################################################################################//
//#################################################################################################################//
//#################################################################################################################//

void loop()
{

  REEDSTAT_OBEN = digitalRead(REED_OBEN);   // Einlesen des Reed-Kontakt-Status fuer Oben (offen od. geschlossen)
  REEDSTAT_UNTEN = digitalRead(REED_UNTEN); // Einlesen des Reed-Kontakt-Status fuer Unten (offen od. geschlossen)

  LDR_WERT = analogRead(LDR); // Einlesen des aktuellen LDR-Wertes
  UHRZEIT = RTC.getTime();
  ZEIT_1000er_FORMAT = ((UHRZEIT.hour * 100) + (UHRZEIT.min)); // Speicherung der aktuellen Zeit im 1000er-Format (int) fuer das Relais

  AUSGABE_LCD();    // Funktion AUSGABE_LCD siehe unten (fuer Standard-Anzeige)
  AUSGABE_SERIAL(); // Funktion AUSGABE_SERIAL siehe unten (fuer Diagnose-Zwecke)

  //#################################################################################################################//
  /////////////////////////////////////////////////
  //  BEDINGUNGEN : DISPLAY_ON_TIME ausschalten  //
  /////////////////////////////////////////////////
  //#################################################################################################################//

  if (ZEITWERT < (millis() - DISPLAY_ON_ZEIT)) // Schleife für Displaybeleuchtung
  {
    ZEITWERT = millis();
    LCD.noBacklight(); // Display wird nach DISPLAY_ON_ZEIT abgeschaltet
    //**Serial.println("Programm befindet sich in Bedingung DISPLAY_ON_TIME");		//(fuer Diagnose-Zwecke)
  }

  //#################################################################################################################//
  //////////////////////////////////////////////
  //  BEDINGUNGEN : Tastendruck - Interrupt   //
  //////////////////////////////////////////////
  //#################################################################################################################//

  //######  BEDINGUNG TASTENDRUCK AUF ###############################################################################//

  if (ISR_FLAG_AUF == 1) // Bedingung bei Tastendruck AUF => gesetztes ISR_FLAG_AUF
  {
    ISR_FLAG_AUF = 0;
    LCD.backlight();

    while (REEDSTAT_OBEN) // Schleife die soloange durchlaufen wird bis REED_OBEN schließt
    {
      MOTOR_AUF();
      REEDSTAT_OBEN = digitalRead(REED_OBEN); // Pruefung ob REED_OBEN "offen" oder "geschlossen" ist
    }

    MOTOR_STOP();
    //**Serial.println("Programm befindet sich in Bedingung TASTENDRUCK AUF");		//(fuer Diagnose-Zwecke)
  }

  //######  BEDINGUNG TASTENDRUCK ZU  ###############################################################################//

  else if (ISR_FLAG_ZU == 1) // Bedingung bei Tastendruck ZU => gesetztes ISR_FLAG_ZU
  {
    ISR_FLAG_ZU = 0;
    LCD.backlight();

    while (REEDSTAT_UNTEN) // Schleife die soloange durchlaufen wird bis REED_UNTEN schließt
    {
      MOTOR_ZU();
      REEDSTAT_UNTEN = digitalRead(REED_UNTEN); // Pruefung ob REED_UNTEN "offen" oder "geschlossen" ist
    }

    MOTOR_STOP();
    //**Serial.println("Programm befindet sich in Bedingung TASTENDRUCK ZU");		//(fuer Diagnose-Zwecke)
  }

  //#################################################################################################################//
  //////////////////////////////////////////////
  // BEDINGUNGEN : ENTWEDER LDR ODER ZEITSTRG //
  //////////////////////////////////////////////
  //#################################################################################################################//

  //######  BEDINGUNG LDR/ZEITSTRG AUF  ####### #####################################################################//

  if ((LDR_WERT >= LDR_SW_MORGEN) || (UHRZEIT.hour == ZEIT_AUF_STD && UHRZEIT.min == ZEIT_AUF_MIN))
  {
    while (REEDSTAT_OBEN) // Schleife die soloange durchlaufen wird bis REED_OBEN schließt
    {
      LCD.backlight();
      MOTOR_AUF();
      REEDSTAT_OBEN = digitalRead(REED_OBEN); // Pruefung ob REED_OBEN "offen" oder "geschlossen" ist
    }
    MOTOR_STOP();
    //**Serial.println("Programm befindet sich in Bedingung LDR/ZEITRG-AUF");		//(fuer Diagnose-Zwecke)
  }

  //######  BEDINGUNG LDR/ZEITSTRG ZU  ####### ######################################################################//

  else if ((LDR_WERT <= LDR_SW_ABEND) || (UHRZEIT.hour == ZEIT_ZU_STD && UHRZEIT.min == ZEIT_ZU_MIN))
  {
    while (REEDSTAT_UNTEN) // Schleife die soloange durchlaufen wird bis REED_UNTEN schließt
    {
      LCD.backlight();
      MOTOR_ZU();
      REEDSTAT_UNTEN = digitalRead(REED_UNTEN); // Pruefung ob REED_UNTEN "offen" oder "geschlossen" ist
    }
    MOTOR_STOP();
    //**Serial.println("Programm befindet sich in Bedingung LDR/ZEITRG-ZU");		//(fuer Diagnose-Zwecke)
  }

  //#################################################################################################################//
  //////////////////////////////////////////////
  // BEDINGUNGEN : RGB-STATUS-ANZEIGE         //
  //////////////////////////////////////////////
  //#################################################################################################################//

  //######  BEDINUNG RGB-STATUS-AUF  ################################################################################//

  if ((UHRZEIT.sec <= ZEIT_RGB) && (REEDSTAT_OBEN == 0))
  {
    digitalWrite(LED_ROT, HIGH);
    digitalWrite(LED_GRUEN, LOW);
    digitalWrite(LED_BLAU, LOW);
    //**Serial.println("Programm befindet sich in der Bedingung RGB-STATUS-AUF");   //(fuer Diagnose-Zwecke)
  }

  else
  {
    digitalWrite(LED_ROT, LOW);
  }

  //######  BEDINUNG RGB-STATUS-ZU  #################################################################################//

  if ((UHRZEIT.sec <= ZEIT_RGB) && (REEDSTAT_UNTEN == 0))
  {
    digitalWrite(LED_ROT, LOW);
    digitalWrite(LED_GRUEN, HIGH);
    digitalWrite(LED_BLAU, LOW);
    //**Serial.println("Programm befindet sich in der Bedingung RGB-STATUS-ZU");    //(fuer Diagnose-Zwecke)
  }

  else
  {
    digitalWrite(LED_GRUEN, LOW);
  }

  //#################################################################################################################//
  //////////////////////////////////////////////
  // BEDINGUNGEN : RELAIS-STEUERUNG           //
  //////////////////////////////////////////////
  //#################################################################################################################//

  //######  ZEIT-BEDINUNG-1 RELAIS EIN  #############################################################################//

  if ((ZEIT_1000er_FORMAT >= REL_ZEIT_1_EIN) && (ZEIT_1000er_FORMAT < REL_ZEIT_1_AUS))
  {
    digitalWrite(RELAIS, HIGH);
    //**Serial.println("Programm befindet sich in der Bedingung RELAISZEIT 1 AKTIV");    //(fuer Diagnose-Zwecke)
  }

  //######  RELAIS-ZEIT-BEDINUNG-2  #################################################################################//

  else if ((ZEIT_1000er_FORMAT >= REL_ZEIT_2_EIN) && (ZEIT_1000er_FORMAT < REL_ZEIT_2_AUS))
  {
    digitalWrite(RELAIS, HIGH);
    //**Serial.println("Programm befindet sich in der Bedingung RELAISZEIT 2 AKTIV");    //(fuer Diagnose-Zwecke)
  }

  //EINFÜGEMARKE => Hier unterhalb werden die zusaetzlichen RELAIS-ZEIT-BEDINGUNGEN eingefuegt  #####################//

  //######  RELAIS-ZEIT-UND-LDR-BEDINGUNG  #################################################################################//
  else if ((ZEIT_1000er_FORMAT >= LEGELICHT_ZEITFENSTER_BEGINN_1000er_FORMAT) && (ZEIT_1000er_FORMAT < LEGELICHT_ZEITFENSTER_ENDE_1000er_FORMAT) && (LDR_WERT <= LDR_SW_LEGELICHT) && (LEGELICHT_ENDZEIT_1000er_FORMAT < 0))
  {
    if (LEGELICHT_START_COUNTER > LEGELICHT_VERZÖGERUNG_SEK)
    {
      LEGELICHT_ENDZEIT_1000er_FORMAT = ZEIT_1000er_FORMAT + LEGELICHT_LEUCHTDAUER_1000er_FORMAT;
      // Serial.println("Legelicht endzeit wurde berechnet.")
    }

    else
    {
      LEGELICHT_START_COUNTER++; // ++ bedeutet um 1 erhöhen
      // Serial.println("Legelicht Verzögerungscounter wurde um 1 erhöht.")
    }
  }

  else if (ZEIT_1000er_FORMAT < LEGELICHT_ENDZEIT_1000er_FORMAT)
  {
    digitalWrite(RELAIS, HIGH);
    //**Serial.println("Legelicht wird eingeschaltet");    //(fuer Diagnose-Zwecke)
  }

  //######  RELAIS-ZEIT-BEDINUNG INAKTIV  ###########################################################################//

  else
  {
    digitalWrite(RELAIS, LOW);
    LEGELICHT_START_COUNTER = 0;
    //**Serial.println("Programm befindet sich in der Bedingung RELAISZEIT INAKTIV");    //(fuer Diagnose-Zwecke)
  }

  if ((ZEIT_1000er_FORMAT > LEGELICHT_ZEITFENSTER_ENDE_1000er_FORMAT) && (LEGELICHT_ENDZEIT_1000er_FORMAT >= 0))
  {
    LEGELICHT_ENDZEIT_1000er_FORMAT = -1;
  }

  delay(1000); // PROGRAMM-Durchlaufzeit
}

//#################################################################################################################//
//////////////////
// FUNKTIONEN : //
//////////////////
//#################################################################################################################//

void WELCOME()
{
  LCD.init();
  LCD.clear();
  LCD.backlight();
  LCD.setCursor(0, 0);
  LCD.print("  Arduino-CCC   ");
  LCD.setCursor(0, 1);
  LCD.print("V1.24.12 iNerds");
  Serial.println("Arduino-CCC V1.24.12  Release 27.10.2021 by iNerds");
  delay(3000);
  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.print("  Beta Version  ");
  LCD.setCursor(0, 1);
  LCD.print("   RGB/RELAIS   ");
  Serial.println("RGB- & RELAIS-Version");
  delay(3000);
  LCD.noBacklight();
}

void INTERRUPT_TASTER_AUF()
{
  ISR_FLAG_AUF = 1;
}

void INTERRUPT_TASTER_ZU()
{
  ISR_FLAG_ZU = 1;
}

void MOTOR_AUF()
{
  digitalWrite(LED_ROT, LOW);                                             // Um Mischfarben zu verhindern
  digitalWrite(LED_GRUEN, LOW);                                           // Um Mischfarben zu verhindern
  digitalWrite(LED_BLAU, (millis() % (BLINK_ON + BLINK_OFF)) < BLINK_ON); //Blinkintervall
  digitalWrite(MOTOR_1, HIGH);
  digitalWrite(MOTOR_2, LOW);
  LCD.setCursor(0, 0);
  LCD.print("  HKS OEFFNET   ");
  LCD.setCursor(0, 1);
  LCD.print(" CCC-STRG AKTIV ");
  //**Serial.println("CCC STATUS ON - KLAPPE OEFFNET");			//(fuer Diagnose-Zwecke)
}

void MOTOR_ZU()
{
  digitalWrite(LED_ROT, LOW);                                             // Um Mischfarben zu verhindern
  digitalWrite(LED_GRUEN, LOW);                                           // Um Mischfarben zu verhindern
  digitalWrite(LED_BLAU, (millis() % (BLINK_ON + BLINK_OFF)) < BLINK_ON); //Blinkintervall
  digitalWrite(MOTOR_1, LOW);
  digitalWrite(MOTOR_2, HIGH);
  LCD.setCursor(0, 0);
  LCD.print("  HKS SCHLIESST ");
  LCD.setCursor(0, 1);
  LCD.print(" CCC-STRG AKTIV ");
  //**Serial.println("CCC STATUS ON - KLAPPE SCHLIESST");		//(fuer Diagnose-Zwecke)
}

void MOTOR_STOP()
{
  digitalWrite(LED_BLAU, LOW);
  digitalWrite(MOTOR_1, LOW);
  digitalWrite(MOTOR_2, LOW);
  AUSGABE_LCD();
  //**Serial.println("MOTORSTOP wird ausgefuehrt");				//(fuer Diagnose-Zwecke)
}

void AUSGABE_LCD()
{
  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.print("LDR-Wert: ");
  LCD.print(LDR_WERT);
  LCD.setCursor(0, 1);
  LCD.print(RTC.getTemp());
  LCD.print("C    ");
  if (UHRZEIT.hour < 10)
    LCD.print("0");
  LCD.print(UHRZEIT.hour, DEC);
  LCD.print(":");
  if (UHRZEIT.min < 10)
    LCD.print("0");
  LCD.print(UHRZEIT.min, DEC);
}

void AUSGABE_SERIAL()
{
  Serial.print("LDR: ");
  Serial.println(LDR_WERT);
  Serial.print("ZEIT: ");
  Serial.println(RTC.getTimeStr());
  Serial.print("DATUM: ");
  Serial.println(RTC.getDateStr());
  Serial.print("QUARZ-TEMP: ");
  Serial.print(RTC.getTemp());
  Serial.println(" C");
  Serial.print("ZEIT 1000er-Format Relais: ");
  Serial.println(ZEIT_1000er_FORMAT);
  Serial.println("****************************************************");
  Serial.println();
}

//#################################################################################################################//
//#################################################################################################################//
//#################################################################################################################//
