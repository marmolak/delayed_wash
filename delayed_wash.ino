#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>>

static const byte button = 13;
static const byte warn_led = 12;
static bool error = false;

// this is something like panic state so we can delay in interrupt routine
void blink_warn_led(void)
{
  static byte state = HIGH;
  
  wdt_disable();
  for (byte p = 0; p < 10; ++p) {
    digitalWrite(warn_led, state);
    state = HIGH ? LOW : HIGH;
    delay(200);
    digitalWrite(warn_led, state);
    state = HIGH ? LOW : HIGH;
    delay(200);
  }
}

ISR (WDT_vect)
{
  blink_warn_led();
}

void set_watchdog()
{
  noInterrupts ();   // timed sequence below

  MCUSR = 0;                          // reset various flags
  WDTCSR |= 0b00011000;               // see docs, set WDCE, WDE
  WDTCSR =  0b01000000 | 0b00100001;    // set WDIE, set delay 8 secs
  wdt_reset();

  byte adcsra_save = ADCSRA;
  ADCSRA = 0;  // disable ADC
  power_all_disable ();   // turn off all modules
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);   // sleep mode is set here
  sleep_enable();

  interrupts ();
  sleep_cpu ();            // now goes to Sleep and waits for the interrupt

  ADCSRA = adcsra_save;  // stop power reduction
  power_all_enable ();   // turn on all modules
}

void setup() {
  pinMode (warn_led, OUTPUT);
  pinMode (button, INPUT);

  if (false) {
    error = true;
    return;
  }
}

void loop() {
  if (error) {
    set_watchdog();
    return;
  }
 

}
