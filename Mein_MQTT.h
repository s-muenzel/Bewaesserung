#ifndef _INCLUDE_MQTT
#define _INCLUDE_MQTT

#include "MQTT_Adaptor.h"

#define MAX_NACHRICHTEN	40
#define MAX_NACHRICHT   40
#define MAX_THEMA       40
#define MAX_THEMEN      6


class Mein_MQTT : public MQTT_Callback {
  public:
    Mein_MQTT();

    void Beginn();

    void Tick();

    float Feuchte();
    float Feuchte_am_Tag();
    
    void Callback(const char*t, const char*n, int l);

  private:
    void reconnect();
};

#endif // _INCLUDE_MQTT
