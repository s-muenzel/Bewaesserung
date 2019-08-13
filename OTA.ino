#include "OTA.h"
#include <ArduinoOTA.h>
#include <WiFiUdp.h>

OTA::OTA() {
	_OTA_An = false;
}

void OTA::Beginn() {
  ArduinoOTA.setHostname(__WZ.lese_Hostname()); // überschreibt wohl den Aufruf von MDNS.begin()
  ArduinoOTA.setPasswordHash(ota_hash);
  ArduinoOTA.setRebootOnSuccess(true);
  ArduinoOTA.onStart([]() {
    D_PRINTLN("Start updating ");
    __WV.Stop();
    __Led.Signal(LED::blinken);
  });
  ArduinoOTA.onEnd([]() {
    D_PRINTLN("\nEnd");
    __WV.Stop();
    __Led.Signal(LED::nix);
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    __Led.Tick(); // Tick wird sonst nicht gerufen, ohne Tick kein Licht
    D_PRINTF("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    D_PRINTF("Error[%u]: ", error);
    __Led.Signal(LED::blinken);
    if (error == OTA_AUTH_ERROR) {
      D_PRINTLN("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      D_PRINTLN("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      D_PRINTLN("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      D_PRINTLN("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      D_PRINTLN("End Failed");
    }
  });
	
}

void OTA::Bereit() {
	_OTA_An = true;
    ArduinoOTA.begin();
};

void OTA::Tick() {
  if (_OTA_An) {
    ArduinoOTA.handle();
  }
};
