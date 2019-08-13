#include "Zugangsinfo.h"
#include "Mein_MQTT.h"

#define THEMA "Sensor/WZTuF/EG/WZ//F"

#define CLIENT_ID "Bewaesserung"

MQTT_Adaptor __MQTT_Adaptor;

Mein_MQTT::Mein_MQTT() {
}

void Mein_MQTT::Beginn() {
  D_PRINTLN("Mein_MQTT.Beginn");
  __MQTT_Adaptor.Beginn();
  __MQTT_Adaptor.SetCallback(this);
  reconnect();
}

void Mein_MQTT::Tick() {
  if (!__MQTT_Adaptor.Verbunden()) {
    reconnect();
  }
  __MQTT_Adaptor.Tick();
}

void Mein_MQTT::Callback(const char*t, const char*n, int l) {
  D_PRINTF("Neue Nachricht [%s] [%s]\n", t, n);
//  NeuerEintrag(t, n, l);
}

//void Mein_MQTT::NeuerEintrag(const char*t, const char *n, int l) {
//  //  digitalWrite(LED_BUILTIN, LED_AN);   // Turn the LED on (Note that LOW is the voltage level
//  D_PRINTF("Neue Nachricht [%s] [%s]\n", t, n);
//}

void Mein_MQTT::reconnect() {
  while (!__MQTT_Adaptor.Verbunden()) {
    D_PRINTF("Verbinde mit MQTT server als <%s> ", CLIENT_ID);
    if (__MQTT_Adaptor.Verbinde(CLIENT_ID, device_user, device_pw)) {
      D_PRINTLN("verbunden");
      if (__MQTT_Adaptor.Subscribe(THEMA)) {
        D_PRINTLN("erfolgreich");
      } else {
        D_PRINTLN("nicht erfolgreich");
      }
    } else {
      D_PRINTF("nicht verbunden, rc=%d, in 5 Sekunden neuer Versuch\n", __MQTT_Adaptor.Status());
      delay(5000);
    }
  }
}
