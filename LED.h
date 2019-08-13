#ifndef _LED
#define _LED

class LED {
  public:
    LED();

    void Beginn();

    typedef enum _Signal {
      nix,
      kurz,
      an,
	  blinken
    } _Signal_t;

    _Signal_t Status();

    void Signal(_Signal_t welches);
	
	void Tick();

  private:

  void An_Aus(bool an);
  
  _Signal_t _modus;
  bool _an;
  ulong _next_switch;
};

#endif // _LED
