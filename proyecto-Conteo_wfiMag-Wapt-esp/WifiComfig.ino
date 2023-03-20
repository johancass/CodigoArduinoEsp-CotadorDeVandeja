/**************************************************************************
**ESTE CODIGO FUE ESCRITO JOHAN CASTILLO LOPEZ - CALI COLOMBIA - V2
**Hardware de Fuentes Abiertas (OSHW en inglés) es aquel hardware cuyo
**diseño se hace disponible públicamente para que cualquier persona lo
**pueda estudiar, modificar, distribuir, materializar y vender, tanto 
**el original como otros objetos basados en ese diseño.
**NOM  |  ESP32  |   
**------------------------------------------------------
**SDA  | GPIO21  |    PANTALLA-SDA     |  RTC-SDA
**SCL  | GPIO22  |    PANTALLA-SCL     |  RTC-SCL
**-------------------------------------------------------
**       ESP32           ENTRADAS-todoas las entradas esta cmo pullup asi que se activan conpulso bajo
**--------------------------------------------------------
**RST  | GPIO15  |   BTN1 & GND           
**CONF | GPIO4   |   BTN2 & GND
**RST  | GPIO2   |MICRI SWTCH & GND     //contador de vandeja
***************************************************************************/

#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include <Arduino.h>
#include <EEPROM.h>


#define SETUP_PIN 0
#define PosTel 0
#define PosMaquina 20
#define PosPeso 30
#define PosTime 35
#define PosAlarma1 50
#define PosAlarma2 52
WiFiManager wm;
WiFiManagerParameter ParametroAlarma1;
WiFiManagerParameter ParametroAlarma2;
String HOST_NAME = "http://api.callmebot.com"; // change to your PC's IP address
String PATH_NAME   = "/whatsapp.php";

class IPAddressParameter : public WiFiManagerParameter {
public:
  IPAddressParameter(const char *id, const char *placeholder, IPAddress address)
    : WiFiManagerParameter("") {
    init(id, placeholder, address.toString().c_str(), 16, "", WFM_LABEL_BEFORE);
  }

  bool getValue(IPAddress &ip) {
    return ip.fromString(WiFiManagerParameter::getValue());
  }
};

class IntParameter : public WiFiManagerParameter {
public:
  IntParameter(const char *id, const char *placeholder, long value, const uint8_t length = 5)
    : WiFiManagerParameter("") {
    init(id, placeholder, String(value).c_str(), length, "", WFM_LABEL_BEFORE);
  }

  long getValue() {
    return String(WiFiManagerParameter::getValue()).toInt();
  }
};

class FloatParameter : public WiFiManagerParameter {
public:
  FloatParameter(const char *id, const char *placeholder, float value, const uint8_t length = 10)
    : WiFiManagerParameter("") {
    init(id, placeholder, String(value).c_str(), length, "", WFM_LABEL_BEFORE);
  }

  float getValue() {
    return String(WiFiManagerParameter::getValue()).toFloat();
  }
};
String getParam(String name) {
  //read parameter from server, for customhmtl input
  String value;
  if (wm.server->hasArg(name)) {
    value = wm.server->arg(name);
  }
  return value;
}

void setupInit() {
  WiFi.mode(WIFI_STA);  // explicitly set mode, esp defaults to STA+AP
  pinMode(SETUP_PIN, INPUT_PULLUP);
  Serial.begin(115200);  // initialize the lcd
  delay(500);
  lcd.clear();
  //Delay to push SETUP button
  Serial.println("Press setup button");
  lcd.setCursor(0, 0);
  lcd.print("Press Config button");
  lcd.setCursor(2, 3);
  lcd.print("Inicando");
  int i = 0;
  for (int sec = 3; sec > 0; sec--) {
    Serial.print(sec);
    Serial.print("..");
    lcd.setCursor(10 + i, 3);
    lcd.print(".");
    delay(1000);
    i++;
  }

  // warning for example only, this will initialize empty memory into your vars
  // always init flash memory or add some checksum bits
  EEPROM.begin(512);
  EEPROM.get(PosMaquina, MonMaqui);
  EEPROM.get(PosTel, Tel);
  EEPROM.get(PosPeso, Pesp);
  EEPROM.get(PosTime, Seg);
  EEPROM.get(PosAlarma1, BoolAlarma1);
  EEPROM.get(PosAlarma2, BoolAlarma2);
  Serial.println("Settings loaded");

  if (digitalRead(SETUP_PIN) == LOW) {
    lcd.clear();
    // Button pressed
    Serial.println("SETUP");
    lcd.setCursor(0, 0);
    lcd.print("CONFIG");
    lcd.setCursor(0, 1);
    lcd.print("BUSCA EN REDES WIFI");

    int LargoAmarma = 50;
    String TxOuptAlarma1;
    String TxOuptAlarma2;
    if (BoolAlarma1) {
      new (&ParametroAlarma1) WiFiManagerParameter("PosAlarma1", ">Alarma Vandeja", "1", LargoAmarma, "placeholder=\"PosAlarma1\" type=\"checkbox\" checked><br>");
    } else {
      new (&ParametroAlarma1) WiFiManagerParameter("PosAlarma1", ">Alarma Vandeja", "1", LargoAmarma, "placeholder=\"PosAlarma1\" type=\"checkbox\"><br>");
    }
    if (BoolAlarma2) {
      new (&ParametroAlarma2) WiFiManagerParameter("Pos", "Alarma Rst", "1", LargoAmarma, "placeholder=\"PosAlarma1\" type=\"checkbox\" checked");  // custom html type

    } else {
      new (&ParametroAlarma2) WiFiManagerParameter("Pos", "Alarma Rst", "1", LargoAmarma, "placeholder=\"PosAlarma1\" type=\"checkbox\"");  // custom html type
    }

    // custom html type




    lcd.setCursor(2, 2);
    lcd.print(MonMaqui);
    Tel[19] = '\0';       //add null terminator at the end cause overflow
    MonMaqui[19] = '\0';  //add null terminator at the end cause overflow
    WiFiManagerParameter param_str("str", "Nom Dispositivo:", MonMaqui, 20);
    WiFiManagerParameter param_str2("str2", "Telefono:", Tel, 20);
    IntParameter param_int("int", "Peso X cubo:", Pesp);
    IntParameter param_int2("int2", "Segundos de alarma:", Seg);
    WiFiManagerParameter custom_html_Titule("<h2 style=\"color='blue'\"><center>PARAMETROS</center></h2></br>");  // only custom html

    wm.addParameter(&custom_html_Titule);

    wm.addParameter(&param_str);
    wm.addParameter(&param_str2);
    wm.addParameter(&param_int);
    wm.addParameter(&param_int2);
    wm.addParameter(&ParametroAlarma1);
    wm.addParameter(&ParametroAlarma2);
    /*int Seg;
    int Pesp;
    char Tel[20];
    char MonMaqui[20];*/

    //SSID & password parameters already included
    wm.startConfigPortal(MonMaqui);

    strncpy(Tel, param_str2.getValue(), 20);
    Tel[19] = '\0';



    strncpy(MonMaqui, param_str.getValue(), 20);
    MonMaqui[19] = '\0';
    char TxInAlaema1[2];
    char TxInAlaema2[2];
    strncpy(TxInAlaema1, ParametroAlarma1.getValue(), 2);
    TxInAlaema1[1] = '\0';

    strncpy(TxInAlaema2, ParametroAlarma2.getValue(), 2);
    TxInAlaema2[1] = '\0';

    Pesp = param_int.getValue();
    Seg = param_int2.getValue();

    // BoolAlarma2 = custom_radio_str.getValue();
    Serial.print("Telefono: ");
    Serial.println(Tel);
    Serial.print("Peso X: ");
    Serial.println(Pesp, DEC);
    Serial.print("Tiempo: ");
    Serial.println(Seg, DEC);
    Serial.print("Bool1: ");
    Serial.write(TxInAlaema1);
    Serial.print("Bool2: ");
    Serial.write(TxInAlaema2);
    Serial.println();
    if (String(TxInAlaema1) == "1") {
      BoolAlarma1 = 1;
    } else {
      BoolAlarma1 = 0;
    }
    if (String(TxInAlaema2) == "1") {
      BoolAlarma2 = 1;
    } else {
      BoolAlarma2 = 0;
    }
    Serial.print("Bool1: ");
    Serial.println(BoolAlarma1);
    Serial.print("Bool2: ");
    Serial.println(BoolAlarma2);
    delay(100);

    EEPROM.put(PosMaquina, MonMaqui);
    EEPROM.put(PosTel, Tel);
    EEPROM.put(PosPeso, Pesp);
    EEPROM.put(PosTime, Seg);
    EEPROM.put(PosAlarma1, BoolAlarma1);
    EEPROM.put(PosAlarma2, BoolAlarma2);
    delay(10);
    if (EEPROM.commit()) {
      Serial.println("Settings saved");
    } else {
      Serial.println("EEPROM error");
    }
  } else {
    Serial.println("WORK");

    //connect to saved SSID
    WiFi.begin();

    //do smth
    Serial.print("Telefono: ");
    Serial.println(Tel);
    Serial.print("Peso X: ");
    Serial.println(Pesp, DEC);
    Serial.print("Tiempo: ");
    Serial.println(Seg, DEC);
    Serial.print("Bool1: ");
    Serial.println(BoolAlarma1);
    Serial.print("Bool2: ");
    Serial.println(BoolAlarma2);
  }
}

void EnviHttFun(String TelWassatp,String MsmWassatp) {

  if ((WiFi.status() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;
     String Get = "?";
  Get = Get + "phone=" + TelWassatp + "&text=" + MsmWassatp + "&apikey=9970997";

    String DirUlr = HOST_NAME + PATH_NAME + Get;
    Serial.print("[HTTP] begin...\n");
    Serial.print(DirUlr);
    if (http.begin(client, DirUlr)) { // HTTP


      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);
        }
        else {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());


          http.end();
        }
      } else {
        Serial.printf("[HTTP} Unable to connect\n");
      }

    }
  }
}
