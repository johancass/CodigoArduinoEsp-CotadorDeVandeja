#include <ETH.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiSTA.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiType.h>
#include <WiFiUdp.h>

//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#if defined(ARDUINO) && ARDUINO >= 100
#include "RTClib.h"

#define printByte(args) write(args);
#else
#define printByte(args) print(args, BYTE);
#endif

RTC_DS3231 rtc;

byte Hielo2_1[] = {
  B01000,
  B01011,
  B01010,
  B01010,
  B01111,
  B00010,
  B00011,
  B00000
};
byte Hielo2_2[] = {
  B01000,
  B11110,
  B01010,
  B01010,
  B11010,
  B00010,
  B11110,
  B00000
};

uint8_t Clock[8] = { 0x0, 0xe, 0x15, 0x17, 0x11, 0xe, 0x0 };


#if defined(ESP32)
const byte VandejaInpin = 22, Btn2 = 21, Btn1 =24 ;  //pin gpio Esp32
#elif defined(ESP8266)
const byte VandejaInpin = 2, Btn2 = 0, Btn1 = 14;  //pin gpio Esp8266
#else

LiquidCrystal_I2C lcd(0x27, 20, 4);                // set the LCD address to 0x27 for a 16 chars and 2 line display
long TiempoAlarma = 5000, tiempo1;
long pulsos = 0, NmeroMult = 3, ContPduc = 0;
boolean BanderaAlarma = 0, BanderraMenu = 0, PulsoAnterior;
int cambios = 0, SegundosAlarma = 0, DsparadorAlarma = 10;
//variables cronometro//
int horas = 0;
int minutos = 0;
int segundos = 0;
int decimas = 0;
long milisegundos = 0;
// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
int SegundoAnterior;
///////
boolean AlarmaVandeja= 0;
///////

int Seg;
int Pesp;
char Tel[20];
char MonMaqui[20];
byte BoolAlarma1,BoolAlarma2;
String Mensaje[2]={"no%20se%20detecta%20movimiento%20en%20la%20bandeja","El dispositivonoha podido iniciar"};
void setup() {

  pinMode(VandejaInpin, INPUT_PULLUP);
  pinMode(Btn1, INPUT_PULLUP);
  pinMode(Btn2, INPUT_PULLUP);

  lcd.begin();  // initialize the lcd
  delay(500);
  lcd.backlight();

  lcd.createChar(0, Hielo2_1);
  lcd.createChar(1, Hielo2_2);
  lcd.createChar(2, Clock);

  lcd.home();
  delay(500);
  lcd.clear();
  setupInit();
  delay(500);
  DsparadorAlarma = Seg;
  lcd.clear();
  lcd.print("Hello world...");
  lcd.setCursor(0, 1);
  lcd.print(" i ");
  lcd.setCursor(2, 3);

  lcd.print(" arduinos!");
  delay(1000);

  lcd.clear();
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
}

// display all keycode
void Pantall_1() {
  DateTime now = rtc.now();


  int SegundoActual = now.second();
  if (BanderaAlarma == 0) {

    if (SegundoActual != SegundoAnterior) {
      SegundoAnterior = SegundoActual;
      segundos++;
      SegundosAlarma++;
      if (segundos == 60) {  //When it has passed 60 seconds it count one minute
        minutos++;
        segundos = 0;
      }
      if (minutos == 60) {  //When it has passed 60 minutes it count one hour
        minutos = 0;
        horas++;
      }
      if (horas == 24) {  //When it has passed 60 minutes it count one hour
        horas = 0;
      }
    }
    /////////////////////////////////////
    ContPduc = pulsos * Pesp;

    lcd.setCursor(2, 0);
    lcd.print(MonMaqui);
    lcd.setCursor(0, 1);
    lcd.print("Tiempo: ");

    lcd.setCursor(7, 1);
    if (horas < 10) {
      lcd.print("0");
    }
    lcd.print(horas);
    lcd.print(":");
    //lcd.setCursor(3, 0);
    if (minutos < 10) {
      lcd.print("0");
    }
    lcd.print(minutos);
    lcd.print(":");
    //lcd.setCursor(6, 0);
    if (segundos < 10) {
      lcd.print("0");
    }
    lcd.print(segundos);
    //lcd.print(":");
    // lcd.setCursor(9, 0);
    //lcd.print(decimas);

    lcd.print(" ");
    lcd.setCursor(0, 2);
    lcd.print("Apertura: ");
    lcd.print(pulsos);
    lcd.print(" ");
    lcd.setCursor(0, 3);
    lcd.print("Poducido: ");
    lcd.print(ContPduc);
    lcd.print(" Kg");
  } else {
    if(AlarmaVandeja==0)  {
      AlarmaVandeja=1;
      String TexWasap="*";
      TexWasap=TexWasap + "ALARMA-" + String(MonMaqui) + "*" + Mensaje[0];
       EnviHttFun(String(Tel), TexWasap);      
    }
    lcd.noBacklight();
    delay(100);
    lcd.backlight();
  }
  boolean pulsoActual = digitalRead(VandejaInpin);
  if (pulsoActual != PulsoAnterior) {
    cambios++;
    PulsoAnterior = pulsoActual;
    if (cambios == 2) {
      pulsos++;
      cambios = 0;
      SegundosAlarma = 0;
      BanderaAlarma = 0;
    }
    delay(50);
  }

  lcd.setCursor(17, 1);
  lcd.print(DsparadorAlarma - SegundosAlarma);
  lcd.print(" ");

  if (SegundosAlarma == Seg) {
    BanderaAlarma = 1;
  }
}

void PantallRst_2() {

  lcd.setCursor(0, 1);
  lcd.print("Reseteando....");
  lcd.setCursor(0, 3);
  lcd.print("Login...");
}


void loop() {
  if (digitalRead(VandejaInpin) == 0) {
    delay(50);
    int Cont = 0;
    while (digitalRead(VandejaInpin) == 0 and Cont < 100) {
      Cont++;
      delay(50);
    }
    if (Cont >= 100) {
      BanderraMenu = 1;
      lcd.clear();

    } else {
      pulsos++;
      delay(50);
      BanderaAlarma = 0;
      tiempo1 = millis();
    }
  } else if (digitalRead(Btn1) == 0) {
    horas = 0;
    minutos = 0;
    segundos = 0;
    decimas = 0;
    milisegundos = 0;
    ContPduc = 0;
    lcd.clear();
    PantallRst_2();
    delay(800);
    lcd.clear();
  }
  Pantall_1();

  /////////////////////////////////////
  /*
    if (millis() > (tiempo1 + TiempoAlarma)) {  //Si ha pasado 1 segundo ejecuta el IF
    tiempo1 = millis();
    BanderaAlarma = 1;
    lcd.clear();
    }*/
}
///////////////////////////////////////////
