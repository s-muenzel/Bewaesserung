#ifndef _VENTIL
#define _VENTIL

#define RELAIS_PIN 12        // Sonoff Basic: GPIO 12

class Ventil {
  public:
    Ventil();

    void Beginn();

    void Start(uint16_t d);  // d[s]: Dauer der Bewaesserung
    void Stop();

    bool Laeuft();

    void Tick();

  private:

    uint32_t _Stopzeit;      // [ms] - läuft, bzw. seit wann läuft eine Bewaesserung
    bool _status_Relais;     // Wasser an / aus
};

#endif // _Ventil
