#include "Speicher.h"
#include <EEPROM.h>

#define POS_UZ            0
#define POS_DAUER         sizeof(_UZ)
#define POS_FEUCHTE       POS_DAUER       + sizeof(_Dauer)
#define POS_AKTIV         POS_FEUCHTE     + sizeof(_Feuchte)
#define POS_HOSTNAME      POS_AKTIV       + sizeof(_Aktiv)
#define GROESSE_ALLES     POS_HOSTNAME    + sizeof(_Hostname)

Speicher::Speicher() {
}

void Speicher::Beginn() {
  EEPROM.begin(GROESSE_ALLES);
  EEPROM.get(POS_UZ,          _UZ);
  EEPROM.get(POS_DAUER,       _Dauer);
  EEPROM.get(POS_FEUCHTE,     _Feuchte);
  EEPROM.get(POS_AKTIV,       _Aktiv);
  EEPROM.get(POS_HOSTNAME,    _Hostname);
  if ((strnlen(_Hostname, 63) == 0) || (strnlen(_Hostname, 63) >= 63)) {
    strcpy(_Hostname, host_name);
    EEPROM.put(POS_HOSTNAME, _Hostname);
  }
}

void Speicher::speichern() {
  EEPROM.commit();
}

bool Speicher::jetztBewaessern(time_t Jetzt) {
  if (_Aktiv) {
    return (hour(Jetzt) == hour(_UZ)) && (minute(Jetzt) == minute(_UZ)) && (second(Jetzt) == second(_UZ));
  } else
    return false;
}

time_t Speicher::lese_Uhrzeit() {
  EEPROM.get(POS_UZ, _UZ);
  return _UZ;
}

uint16_t Speicher::lese_Dauer() {
  EEPROM.get(POS_DAUER, _Dauer);
  return _Dauer;
}

float Speicher::lese_Feuchte() {
  EEPROM.get(POS_FEUCHTE, _Feuchte);
  return _Feuchte;
}

bool Speicher::lese_Aktiv() {
  EEPROM.get(POS_AKTIV, _Aktiv);
  return _Aktiv;
}

void Speicher::setze_Uhrzeit(time_t t) {
  _UZ = t;
  EEPROM.put(POS_UZ, _UZ);
}

void Speicher::setze_Dauer(uint16_t d) {
  _Dauer = d;
  EEPROM.put(POS_DAUER, _Dauer);
}
void Speicher::setze_Feuchte(float f) {
  _Feuchte = f;
  EEPROM.put(POS_FEUCHTE, _Feuchte);
}

void Speicher::setze_Aktiv(bool a) {
  _Aktiv = a;
  EEPROM.put(POS_AKTIV, _Aktiv);
}

const char *Speicher::lese_Hostname() {
  return _Hostname;
}

void Speicher::setze_Hostname(const char* n) {
  strncpy(_Hostname, n, 63);
  _Hostname[63] = '\n';
  D_PRINTF("Hostname: %s (size: %d)\n", _Hostname, sizeof(_Hostname));
  EEPROM.put(POS_HOSTNAME, _Hostname);
}
