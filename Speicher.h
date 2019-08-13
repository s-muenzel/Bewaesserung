#ifndef _SPEICHER
#define _SPEICHER

class Speicher {
  public:
    Speicher();

    void Beginn();

    void speichern();

    time_t   lese_Uhrzeit();
    uint16_t lese_Dauer();
    float    lese_Feuchte();
    bool     lese_Aktiv();

    void setze_Uhrzeit(time_t t);
    void setze_Dauer(uint16_t d);
    void setze_Feuchte(float f);
    void setze_Aktiv(bool a);

    const char *lese_Hostname();
    void setze_Hostname(const char* n);

    bool jetztBewaessern(time_t Jetzt);

  private:
    time_t   _UZ;         // zu welcher Uhrzeit wird bewässert
    uint16_t _Dauer;      // [s] wie lange wird bewässert
    float    _Feuchte;    // [%] Grenzfeuchte, bei Überschreitung wird nicht bewässert
    bool     _Aktiv;      // globaler Schalter, ob Bewaesserungen laufen oder nicht

    char _Hostname[64];   // Unter welchem Namen meldet sich der MC im Netz an
};

#endif // _SPEICHER
