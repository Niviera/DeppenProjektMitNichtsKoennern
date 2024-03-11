/* Libarys */
#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <WebServer.h>
#include "Website.h"

/* */
// start your defines for pins for sensors, outputs etc.
#define PIN_OUTPUT 26 // connected to nothing but an example of a digital write from the web page
#define PIN_FAN 27    // pin 27 and is a PWM signal to control a fan speed
#define PIN_LED 2     // On board LED
#define PIN_A0 34     // some analog input sensor
#define PIN_A1 35     // some analog input sensor

/* DEFINES */
#define DHTPIN 32
#define DHTTYPE DHT11
#define WLPIN 35
#define MPIN 33
#define LPIN 34
#define RELAIPINLED 0
#define RELAIPINPUMPE 4
#define RELAIPINCOOLER 16

#define WATER_MAX 4095

#define AP_SSID "AGW-Webinterface"
#define AP_PASS "0123456789"
/* Allgemeine Variablen */

/* Alle Funktionen & Variablen für DHT11 */
// Variablen'
DHT_Unified dht(DHTPIN, DHTTYPE);
sensors_event_t event;
double temp = 18.00;
double humi = 20.00;
double soll_bodenfeuchte = 40.00;

bool auto_light = true;
bool enable_light = false;
bool on_light = false;

bool auto_luefter = true;
bool on_luefter = false;
bool enable_luefter = false;

bool auto_pumpe = true;
bool on_pumpe = false;
bool enable_pumpe = false;

double kuehl_temp = 18.00;

double wasserLvlProzent = 20.00;
double bodenfeuchteProzent = 20.00;

IPAddress Actual_IP;
char XML[2048];
IPAddress PageIP(192, 168, 113, 200);
IPAddress gateway(192, 168, 113, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress ip;
WebServer server(80);
char buf[32];

// Funktionen
void read_Tempi();
void read_Humi();
void read_Waterlevel();
void read_Moisture();
void read_light();

void ProcessButton_0();
void ProcessButton_2();
void ProcessButton_3();
void ProcessButton_4();
void ProcessButton_5();
void ProcessButton_6();
void handleSubmitTest();
void handleSubmitBoden();
void SendWebsite();
void SendXML();

/* Alle für Wasserlevel */
// Variablen

//

void setup()
{

  // pinMode( 'Welcher Pin', INPUT OUTPUT )
  // analogRead(PinNumber) --> return int
  // digitalWrite( 'Welcher Pin', High Low)
  pinMode(WLPIN, INPUT);
  pinMode(MPIN, INPUT);
  pinMode(LPIN, INPUT);

  pinMode(RELAIPINLED, OUTPUT);
  pinMode(RELAIPINPUMPE, OUTPUT);
  pinMode(RELAIPINCOOLER, OUTPUT);

  /* Setup Serialport für debuggen */
  Serial.begin(9600);

  /* DHT11 Setup */
  dht.begin();

  /* Webserver start */
  WiFi.softAP(AP_SSID, AP_PASS);
  delay(100);
  WiFi.softAPConfig(PageIP, gateway, subnet);
  delay(100);
  Actual_IP = WiFi.softAPIP();
  Serial.print("IP address: ");
  Serial.println(Actual_IP);

  server.on("/", SendWebsite);
  server.on("/xml", SendXML);
  server.on("/BUTTON_0", ProcessButton_0);
  server.on("/BUTTON_2", ProcessButton_2);
  server.on("/BUTTON_3", ProcessButton_3);
  server.on("/BUTTON_4", ProcessButton_4);
  server.on("/BUTTON_5", ProcessButton_5);
  server.on("/BUTTON_6", ProcessButton_6);
  server.on("/BUTTON_7", handleSubmitTest);
  server.on("/BUTTON_8", handleSubmitBoden);

  server.begin();
}

void loop()
{
  // put your main code here, to run repeatedly:

  /* DHT auswerten */
  read_Tempi();
  read_Humi();
  read_Waterlevel();
  read_Moisture();
  read_light();

  server.handleClient();
  // Serial.print("IP address: ");
  // Serial.println(Actual_IP);

  // delay(1000);
}

void read_Tempi()
{
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature))
  {
    /* Fehlerbehandlung Sensor nicht gefunden */
    Serial.println(F("Error reading temperature!"));
  }
  else
  {
    /* Sensor gefunden Temp ausgelesen */
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature - 2);
    Serial.println(F("°C"));
    temp = event.temperature - 2;

    if (enable_luefter || (temp >= kuehl_temp && auto_luefter))
    {
      digitalWrite(RELAIPINCOOLER, HIGH);
      on_luefter = true;
    }
    else
    {
      digitalWrite(RELAIPINCOOLER, LOW);
      on_luefter = false;
    }
  }
}

void read_Humi()
{
  // werte Humidity aus.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity))
  {
    /* Fehlerbehandlung Sensor nicht gefunden */
    Serial.println(F("Error reading humidity!"));
  }
  else
  {
    /* Sensor gefunden Humi ausgelesen */
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
    humi = event.relative_humidity;
  }
}

void read_Waterlevel()
{

  int wlv = analogRead(WLPIN);
  Serial.print("Wasserlevel bei:");
  Serial.println(wlv);
  /* Unnnötig komplexes Mapping */
  wasserLvlProzent = ((-1.00) * ((wlv / (double)WATER_MAX) * 100)) + 100;
  Serial.println(wasserLvlProzent);
}

void read_Moisture()
{
  int mlv = analogRead(MPIN);
  Serial.print("Bodenfeuchte bei:");
  Serial.println(mlv);
  /* Unnnötig komplexes Mapping */
  bodenfeuchteProzent = ((-1.00) * ((mlv / 4096.00) * 100)) + 100;
  Serial.println(bodenfeuchteProzent);
  if ((enable_pumpe || (bodenfeuchteProzent >= soll_bodenfeuchte && auto_pumpe)) && wasserLvlProzent >= 5)
  {
    digitalWrite(RELAIPINPUMPE, HIGH);
    on_pumpe = true;
  }
  else
  {
    digitalWrite(RELAIPINPUMPE, LOW);
    on_pumpe = false;
  }
}

void read_light()
{
  int light = analogRead(LPIN);
  Serial.println(light);
  if (enable_light || (light >= 2000 && auto_light))
  {
    digitalWrite(RELAIPINLED, HIGH);
    on_light = true;
  }
  else
  {
    digitalWrite(RELAIPINLED, LOW);
    on_light = false;
  }
}

/* Alles fürs Webinterface quatsch blabla yolo */

void ProcessButton_0()
{
  if (!enable_light)
  {
    enable_light = true;
    on_light = true;
  }
  else
  {
    enable_light = false;
    on_light = false;
  }
  server.send(200, "text/plain", ""); // Send web page
}

void ProcessButton_2()
{
  if (!enable_luefter)
  {
    enable_luefter = true;
    on_luefter = true;
  }
  else
  {
    enable_luefter = false;
    on_luefter = false;
  }
  server.send(200, "text/plain", ""); // Send web page
}

void ProcessButton_3()
{
  if (!enable_pumpe)
  {
    enable_pumpe = true;
    on_pumpe = true;
  }
  else
  {
    enable_pumpe = false;
    on_pumpe = false;
  }
  server.send(200, "text/plain", ""); // Send web page
}
// Automode Buttons

// Auto Licht:
void ProcessButton_4()
{
  if (auto_light)
  {
    auto_light = false;
  }
  else
  {
    auto_light = true;
  }
  server.send(200, "text/plain", ""); // Senden an Web Oberfläche
}

// Auto Lüfter:
void ProcessButton_5()
{
  if (auto_luefter)
  {
    auto_luefter = false;
  }
  else
  {
    auto_luefter = true;
  }
  server.send(200, "text/plain", ""); // Senden an Web Oberfläche
}

void ProcessButton_6()
{
  if (auto_pumpe)
  {
    auto_pumpe = false;
  }
  else
  {
    auto_pumpe = true;
  }
  server.send(200, "text/plain", ""); // Senden an Web Oberfläche
}

void handleSubmitTest()
{
  if (server.hasArg("temp_schwellwert"))
  {
    Serial.println(server.arg("temp_schwellwert"));
    String inputValue = server.arg("temp_schwellwert");
    kuehl_temp = inputValue.toDouble();
    Serial.print("Eingabewert empfangen: ");
    Serial.println(inputValue);
    server.send(200, "text/plain", "Daten empfangen");
  }
  else
  {
    server.send(400, "text/plain", "Fehler: Kein Wert empfangen");
  }
}

void handleSubmitBoden()
{
  if (server.hasArg("boden_schwellwert"))
  {
    Serial.println(server.arg("boden_schwellwert"));
    String inputValue = server.arg("boden_schwellwert");
    soll_bodenfeuchte = inputValue.toDouble();
    Serial.print("Eingabewert empfangen: ");
    Serial.println(inputValue);
    server.send(200, "text/plain", "Daten empfangen");
  }
  else
  {
    server.send(400, "text/plain", "Fehler: Kein Wert empfangen");
  }
}

// same notion for processing button_1

// code to send the main web page
// PAGE_MAIN is a large char defined in SuperMon.h
void SendWebsite()
{

  // Serial.println("sending web page");
  //  you may have to play with this value, big pages need more porcessing time, and hence
  //  a longer timeout that 200 ms
  server.send(200, "text/html", PAGE_MAIN);
}

// code to send the main web page
// I avoid string data types at all cost hence all the char mainipulation code
void SendXML()
{

  // Serial.println("sending xml");

  strcpy(XML, "<?xml version = '1.0'?>\n<Data>\n");

  // send Temperatur
  // send ist
  sprintf(buf, "<B0>%.2f°C</B0>\n", temp);
  strcat(XML, buf);
  // send soll
  sprintf(buf, "<V0>%.2f°C</V0>\n", kuehl_temp);
  strcat(XML, buf);

  // send Wasserstand
  // send ist
  sprintf(buf, "<B1>%0.2f</B1>\n", wasserLvlProzent);
  strcat(XML, buf);

  // send Bodenfeuchte
  //  send ist
  sprintf(buf, "<B2>%0.2f</B2>\n", bodenfeuchteProzent);
  strcat(XML, buf);
  // send soll
  sprintf(buf, "<V2>%0.2f</V2>\n", soll_bodenfeuchte);
  strcat(XML, buf);

  // send L
  //  send ist
  sprintf(buf, "<B3>%0.2f</B3>\n", humi);
  strcat(XML, buf);
  sprintf(buf, "<V3>%0.2f</V3>\n", soll_bodenfeuchte);
  strcat(XML, buf);

  // show led0 status
  if (on_light)
  {
    strcat(XML, "<LED>1</LED>\n");
    strcat(XML, "<SWITCH>1</SWITCH>\n");
  }
  else
  {
    strcat(XML, "<LED>0</LED>\n");       // Button
    strcat(XML, "<SWITCH>0</SWITCH>\n"); // Tabelleneintrag
  }
  // Luefter Tabelle
  if (on_luefter)
  {
    strcat(XML, "<LUEFTER>1</LUEFTER>\n"); // Tabelleneintrag
  }
  else
  {
    strcat(XML, "<LUEFTER>0</LUEFTER>\n"); // Tabelleneintrag
  }
  // Pumpe Tabelle
  if (on_pumpe)
  {
    strcat(XML, "<PUMPE>1</PUMPE>\n"); // Tabelleneintrag
  }
  else
  {
    strcat(XML, "<PUMPE>0</PUMPE>\n"); // Tabelleneintrag
  }

  // Auto Light
  if (auto_light)
  {
    strcat(XML, "<AUTOLIGHT>1</AUTOLIGHT>\n"); // Button bearbeiten
  }
  else
  {
    strcat(XML, "<AUTOLIGHT>0</AUTOLIGHT>\n"); // Button bearbeiten
  }

  // Auto Lüfter
  if (auto_luefter)
  {
    strcat(XML, "<AUTOLUEFTER>1</AUTOLUEFTER>\n"); // Button bearbeiten
  }
  else
  {
    strcat(XML, "<AUTOLUEFTER>0</AUTOLUEFTER>\n"); // Button bearbeiten
  }

  if (auto_pumpe)
  {
    strcat(XML, "<AUTOPUMPE>1</AUTOPUMPE>\n"); // Button bearbeiten
  }
  else
  {
    strcat(XML, "<AUTOPUMPE>0</AUTOPUMPE>\n"); // Button bearbeiten
  }

  strcat(XML, "</Data>\n");
  // wanna see what the XML code looks like?
  // actually print it to the serial monitor and use some text editor to get the size
  // then pad and adjust char XML[2048]; above
  // Serial.println(XML);

  // you may have to play with this value, big pages need more porcessing time, and hence
  // a longer timeout that 200 ms
  server.send(200, "text/xml", XML);
}
