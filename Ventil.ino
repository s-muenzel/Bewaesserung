#include "Ventil.h"

Ventil::Ventil() {
  pinMode(RELAIS_PIN, OUTPUT);
  D_PRINTF("RELAIS AUS");
  digitalWrite(RELAIS_PIN, LOW);
  _status_Relais    = false;
}

void Ventil::Beginn() {
  D_PRINTF("RELAIS AUS");
  digitalWrite(RELAIS_PIN, LOW);
  _status_Relais = false;
}

void Ventil::Start(uint16_t d) {
  // Start rechnet die Zeit aus, wann die Bewaesserung wieder aufhören soll.
  // Wenn Stopzeit > 0 ist, läuft eine Bewaesserung
  // Sollte eine Bewaesserung bereits laufen --> von neuem anfangen

  _Stopzeit = millis() + d * 1000l;
  D_PRINTF("Starte Ventil bei %ld, stoppe bei %ld \n", millis(), _Stopzeit);
  if (!_status_Relais) {
    D_PRINTF("RELAIS AN");
    digitalWrite(RELAIS_PIN, HIGH);
    _status_Relais = true;
  }
}

void Ventil::Stop() {
  D_PRINTF("Stoppe Ventil bei %lu\n", _Stopzeit);
  // Stop schliesst das Ventil und setzt _Stopzeit wieder auf 0
  D_PRINTF("RELAIS AUS");
  digitalWrite(RELAIS_PIN, LOW);
  _status_Relais = false;
  _Stopzeit = 0;
}

bool Ventil::Laeuft() {
  return _Stopzeit > 0;
}

void Ventil::Tick() {
  // Sollte eine Bewässerung laufen, schauen ob wir wieder aufhören
  if (_Stopzeit > 0) {
    if ( millis() > _Stopzeit) {
      Stop();
    }
  } else {
    if (_status_Relais) {
      D_PRINTF("RELAIS AUS");
      digitalWrite(RELAIS_PIN, LOW);
      _status_Relais = false;
    }
  }
}
