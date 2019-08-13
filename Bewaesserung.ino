/*
  Steuerung der Bewässerung.
  HW:
  - ein Sonoff Basic (ESP8266 Microcontroller)
  - ein AC 230V - DC 12V Netzteil
  - ein 12V gesteuertes Ventil
  SW:
  - NTP: Zeit über NTP holen
  - OTA: SW-Update über Netzwerk
  - WebS: Webserver als UI um die Steuerung einzustellen
  - Speicher: Konfiguration abspeichern / lesen. weiss, wann das Ventil geöffnet werden muss.
  - Knopf: Taster, zur "Vorort-Steuerung"
  - Ventil: Ansteuerung des Ventils

  Allgemeine Konventionen: Sprache für Variablen und Methoden etc. ist Englisch
  // Globale Variablen starten immer mit __[a-z]
  // lokale Variable starten mit _[a-z]
  // Argumente starten mit [a-z]



*/

#include "Ventil.h"
#include "Knopf.h"
#include "LED.h"
#include "NTP.h"
#include "Speicher.h"
#include "OTA.h"
#include "WebS.h"
#include "Mein_MQTT.h"

#define DEBUG_SERIAL
#ifdef DEBUG_SERIAL
#define D_BEGIN(speed)   Serial.begin(speed)
#define D_PRINT(...)     Serial.print(__VA_ARGS__)
#define D_PRINTLN(...)   Serial.println(__VA_ARGS__)
#define D_PRINTF(...)    Serial.printf(__VA_ARGS__)
#else
#define D_BEGIN(speed)
#define D_PRINT(...)
#define D_PRINTLN(...)
#define D_PRINTF(...)
#endif

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include <Time.h>
#include <TimeLib.h>

#include "Zugangsinfo.h"

Speicher      __WZ;
Ventil        __WV;
Knopf         __KN;
NTP_Helfer    __NTP;
OTA           __OTA;
WebS          __WebS;
LED           __Led;
Mein_MQTT     __MQTT;

void setup() {
  // Seriellen Output enablen
  D_BEGIN(115200);
  D_PRINT("Starte...");

  // LED konfigurieren
  __Led.Beginn();
  D_PRINT(" LED initialisiert");

  // EEPROM "Speicher" auslesen
  __WZ.Beginn();
  D_PRINT(" gespeicherte Werte ok");

  // Wasserventil Objekt initialisieren
  __WV.Beginn();
  D_PRINT(" WV_Objekt");

  // Wifi
  WiFi.hostname(__WZ.lese_Hostname());
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int retries = 0;
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    D_PRINTLN("Keine Wifi-Verbindung! Neustart in 5 Sekunden...");
    delay(1000);
    retries++;
    if (retries > 20)
      ESP.restart();
  }
  D_PRINTF(" Wifi %s (IP Address %s)", ssid, WiFi.localIP().toString().c_str());

  if (MDNS.begin(__WZ.lese_Hostname())) {
    D_PRINT(" MDNS responder");
  }

  // Zeitserver konfigurieren und starten
  __NTP.Beginn();
  D_PRINT(" Zeitservice");

  // Webserver konfigurieren
  __WebS.Beginn();
  D_PRINT(" Webserver laeuft");

  // MQTT initialisieren
  __MQTT.Beginn();


  // Wert vom Knopf setzen
  __KN.Beginn();
  D_PRINT(" Taster");

  // OTA Initialisieren
  __OTA.Beginn();
  D_PRINT(" OTA vorbereitet");

  D_PRINTLN("Fertig");
}

void loop() {
  time_t t = now(); // Zeit holen

#ifdef DEBUG_SERIAL
  // Nur noch für Testzwecke
  if (Serial.available() > 0) {
    // read the incoming byte:
    while (Serial.available() > 0) {
      /*char _c  =*/ Serial.read();
    }
    if (__WV.Laeuft()) {
      D_PRINTF("Stoppe Ventil um %d:%02d:02d\n", hour(t), minute(t), second(t));
      __WV.Stop();
    } else {
      D_PRINTF("Starte Ventil um %d:%02d:02d\n", hour(t), minute(t), second(t));
      __WV.Start(__WZ.lese_Dauer());
    }
  }
#endif

  switch (__KN.Status()) {
    case Knopf::nix:
      break;
    case Knopf::kurz: // Kurz heisst, wenn läuft, aufhören.
      D_PRINTF("Knopf kurz um %d:%02d:%02d\n", hour(t), minute(t), second(t));
      if (!__WV.Laeuft()) {
        D_PRINTF("stoppe Bewaesserung um %d:%02d:%02d\n", hour(t), minute(t), second(t));
        __WV.Stop();
      }
      break;
    case Knopf::lang: // Lang: Bewaesserung generell an- oder abschalten. Ausnahme: OTA einschalten
      D_PRINTF("Knopf lang - 24h Pause %d:%02d:%02d\n", hour(t), minute(t), second(t));
      break;
  }

  if (__WZ.jetztBewaessern(t)) {
    if (!__WV.Laeuft()) {
      D_PRINTF("Startzeit erreicht, starte Bewaesserung um %d:%02d:%02d\n", hour(t), minute(t), second(t));
      __WV.Start(__WZ.lese_Dauer());
    }
  }

  __WV.Tick();
  __OTA.Tick();
  __WebS.Tick();
  __Led.Tick();
  __MQTT.Tick();

  delay(20);
}
