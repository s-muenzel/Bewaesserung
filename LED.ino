#include "LED.h"

#define LED_PIN LED_BUILTIN	// Sonoff LED (13)
#define KURZ	  1000			// [ms] wie lange ist ein "kurz" Signal
#define BLINK	   500			// [ms] wie lange ist ein "Blink" (jeweils an und aus)


LED::LED() {
  _modus = nix;
}

void LED::Beginn() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // LOW == aus
}

LED::_Signal_t LED::Status() {
  return _modus;
}

void LED::Signal(_Signal_t welches) {
  ulong _t = millis();
  _modus = welches;
  switch (_modus) {
    default:
    case nix:
      if (_an) {
        An_Aus(false);
      }
      _next_switch = 0;
      break;
    case kurz:
      if (!_an)
        An_Aus(true);
      _next_switch = _t + KURZ;
      break;
    case an:
      if (!_an)
        An_Aus(true);
      _next_switch = 0;
      break;
    case blinken:
      if (!_an)
        An_Aus(true);
      _next_switch = _t + BLINK;
      break;
  }
}

void LED::Tick() {
  ulong _t = millis();
  switch (_modus) {
    default:
    case nix:
      if (_an) {
        An_Aus(false);
      }
      _next_switch = 0;
      break;
    case kurz:
      if (!_an)
        An_Aus(true);
      if (_t > _next_switch) {
        An_Aus(false);
        _next_switch = 0;
      }
      break;
    case an:
      if (!_an)
        An_Aus(true);
      break;
    case blinken:
      if (_t > _next_switch) {
        An_Aus(!_an);
        _next_switch = _t + BLINK;
      }
      break;
  }
}

void LED::An_Aus(bool an) {
  if (_an != an) {
    digitalWrite(LED_PIN, an ? LOW : HIGH);
    _an = an;
  }
}
