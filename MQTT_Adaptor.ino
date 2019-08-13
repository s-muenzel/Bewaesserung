#define USE_ARDUINO_MQTT
//#define USE_PUBSUBBLIENT

#include "MQTT_Adaptor.h"

#ifdef USE_PUBSUBCLIENT
#include <PubSubClient.h>
//WiFiClient __Wifi_Client;
PubSubClient client(__Wifi_Client);
#endif // USE_PUBSUBCLIENT
#ifdef USE_ARDUINO_MQTT
#include <MQTT.h>
#include <MQTTClient.h>
WiFiClient __Wifi_Client;
MQTTClient client;
#endif // USE_ARDUINO_MQTT


///////////////////////////////////////////////////
// MQTT Part als Adator Pattern

//void MQTT_Callback::Callback(const char*t, const char*n, int l) {
//  D_PRINTF("Aufruf Basisklasse (sollte nicht passieren)\n");
//}
MQTT_Callback *__MeinCallbackObjekt = NULL;

#ifdef USE_PUBSUBBLIENT
void Nachricht_Erhalten(char* topic, byte* payload, unsigned int length) {
  D_PRINTF("Nachricht: <%s><%s><%d>\n", topic, payload, length);
  if (__MeinCallbackObjekt != NULL) {
    D_PRINTF("Rufe Callback auf\n");
    __MeinCallbackObjekt->Callback(topic, payload, length);

  }
}
#endif // USE_PUBSUBBLIENT
#ifdef USE_ARDUINO_MQTT
void Nachricht_Erhalten(String &topic, String &payload) {
  D_PRINTF("Nachricht: <%s><%s><%d>\n", topic.c_str(), payload.c_str(), payload.length());
  if (__MeinCallbackObjekt != NULL) {
    D_PRINTF("Rufe Callback auf\n");
    __MeinCallbackObjekt->Callback(topic.c_str(), payload.c_str(), payload.length());
  }
}
#endif // USE_ARDUINO_MQTT


MQTT_Adaptor::MQTT_Adaptor() {
}

void MQTT_Adaptor::Beginn() {
#ifdef USE_PUBSUBBLIENT
  client.setServer(mqtt_server, 1883);
  client.setCallback(Nachricht_Erhalten);
#endif // USE_PUBSUBBLIENT
#ifdef USE_ARDUINO_MQTT
  client.begin(mqtt_server, 1883, __Wifi_Client);
  client.onMessage(Nachricht_Erhalten);
#endif // USE_ARDUINO_MQTT
}

void MQTT_Adaptor::Ende() {
  if (client.connected())
    client.disconnect();
}

bool MQTT_Adaptor::Verbunden() {
  return client.connected();
}

bool MQTT_Adaptor::Verbinde(const char*ClientID, const char* User, const char* PW) {
  return client.connect(ClientID, User, PW);
}

bool MQTT_Adaptor::Verbinde(const char*ClientID, const char* User, const char* PW, const char* WillTopic, int WillQoS, int WillRetain, const char*WillMessage) {
#ifdef USE_PUBSUBBLIENT
  client.connect (ClientID, User, PW, WillTopic, WillQoS, WillRetain, WillMessage);
#endif // USE_PUBSUBBLIENT
#ifdef USE_ARDUINO_MQTT
  client.connect (ClientID, User, PW);
  client.setWill(WillTopic, WillMessage, WillRetain, WillQoS);
#endif // USE_ARDUINO_MQTT
}

void MQTT_Adaptor::Tick() {
  yield();
  client.loop();
  yield();
}

bool MQTT_Adaptor::Publish(const char *Thema, const char *Nachricht, bool retained, int qos) {
#ifdef USE_PUBSUBBLIENT
  yield();
  if (client.publish(Thema, Nachricht, retained)) {
    delay(10);
    client.loop();
    return true;
  } else
    return false;
#endif // USE_PUBSUBBLIENT
#ifdef USE_ARDUINO_MQTT
  yield();
  if (client.publish(Thema, Nachricht, retained, qos)) {
    delay(10);
    client.loop();
    return true;
  } else
    return false;
#endif // USE_ARDUINO_MQTT
}

bool MQTT_Adaptor::Subscribe(const char *Thema) {
  if (client.connected()) {
#ifdef USE_PUBSUBBLIENT
    bool ergebnis = client.subscribe(Thema, 1);
#endif // USE_PUBSUBBLIENT
#ifdef USE_ARDUINO_MQTT
    bool ergebnis = client.subscribe(Thema, 1);
#endif // USE_ARDUINO_MQTT
    yield();
    client.loop();
    yield();
    return ergebnis;
  } else {
    return true;
  }
}

bool MQTT_Adaptor::Unsubscribe(const char *Thema) {
  if (client.connected()) {
    bool ergebnis = client.unsubscribe(Thema);
    yield();
    client.loop();
    yield();
    return ergebnis;
  } else {
    return true;
  }
}

int MQTT_Adaptor::Status() {
#ifdef USE_PUBSUBBLIENT
  return client.state();
#endif // USE_PUBSUBBLIENT
#ifdef USE_ARDUINO_MQTT
  return client.lastError();
#endif // USE_ARDUINO_MQTT
}

void MQTT_Adaptor::SetCallback(MQTT_Callback*cb_objekt) {
  __MeinCallbackObjekt = cb_objekt;
}
