/* Libarys */
#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

/* DEFINES */
#define DHTPIN 32
#define DHTTYPE DHT11
#define WLPIN 27
#define MPIN 14
#define LPIN 12
#define RELAIPINLED 0
#define RELAIPINPUMPE 4
#define RELAIPINCOOLER 16
/* Allgemeine Variablen */

/* Alle Funktionen & Variablen für DHT11 */
// Variablen'
DHT_Unified dht(DHTPIN, DHTTYPE);
sensors_event_t event;
double temp;
double humi;
double soll_bodenfeuchte = 40.00;
bool auto_light = true;
bool enable_light = false;
double kuehl_temp = 18.00;
// Funktionen
void read_Tempi();
void read_Humi();
void read_Waterlevel();
void read_Moisture();
void read_light();

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

  delay(2000);
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

    if (temp >= kuehl_temp)
    {
      digitalWrite(RELAIPINCOOLER, HIGH);
    }
    else
    {
      digitalWrite(RELAIPINCOOLER, LOW);
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
  double wasserLvlProzent = ((-1.00) * ((wlv / 4096.00) * 100)) + 100;
  Serial.println(wasserLvlProzent);
}

void read_Moisture()
{
  int mlv = analogRead(MPIN);
  Serial.print("Bodenfeuchte bei:");
  Serial.println(mlv);
  /* Unnnötig komplexes Mapping */
  double bodenfeuchteProzent = ((-1.00) * ((mlv / 4096.00) * 100)) + 100;
  Serial.println(bodenfeuchteProzent);
  if (bodenfeuchteProzent >= soll_bodenfeuchte)
  {
    digitalWrite(RELAIPINPUMPE, HIGH);
  }
  else
  {
    digitalWrite(RELAIPINPUMPE, LOW);
  }
}

void read_light()
{
  int light = analogRead(LPIN);
  Serial.println(light);
  if (enable_light || (light >= 2000 && auto_light))
  {
    digitalWrite(RELAIPINLED, HIGH);
  }
  else
  {
    digitalWrite(RELAIPINLED, LOW);
  }
}
