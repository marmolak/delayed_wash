#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>>

static const byte button = 2;
static const byte warn_led = 13;

// changed in interrupt routines
static volatile bool error = false;
static volatile bool run_program = false;

void blink_warn_led(void)
{

  for (byte p = 0; p < 10; ++p) {
    digitalWrite(warn_led, LOW);
    delay(200);
    digitalWrite(warn_led, HIGH);
    delay(200);
  }
  digitalWrite(warn_led, LOW);
}

ISR (WDT_vect)
{
  wdt_disable();
}

/* Just delay work to main loop */
void run_program_impl(void)
{
  wdt_disable();
  run_program = true;
}


void set_idle_watchdog(void (*intr_routine)(void))
{
  noInterrupts ();   // timed sequence below

  MCUSR = 0;                          // reset various flags
  WDTCSR |= 0b00011000;               // see docs, set WDCE, WDE
  WDTCSR =  0b01000000 | 0b100001;    // set WDIE, set delay 8 secs
  wdt_reset();

  byte adcsra_save = ADCSRA;
  ADCSRA = 0;  // disable ADC
  power_all_disable ();   // turn off all modules
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);   // sleep mode is set here
  sleep_enable();

  if (intr_routine != NULL) {
    attachInterrupt(0, intr_routine, LOW);
  }

  interrupts ();

  sleep_cpu ();            // now goes to Sleep and waits for the interrupt

  detachInterrupt (0);
  ADCSRA = adcsra_save;  // stop power reduction
  power_all_enable ();   // turn on all modules
}


void setup() {
  pinMode (warn_led, OUTPUT);
  pinMode (button, INPUT);
  digitalWrite(2, HIGH);

  /* initialize RTC here */
  if (!false) {
    error = false;
    return;
  }
}

void loop() {
  /* Unable to detect RTC or
     other error - like RTC stop work
   */
  if (error) {
    blink_warn_led();
    set_idle_watchdog(NULL);
    return;
  }

  set_idle_watchdog(&run_program_impl);
  if (run_program) {
    // now wait for another button press without blocking
    unsigned long currentMillis = millis();
    // Wait for 5 seconds
    unsigned long futureMilis = currentMillis + 5000;
    int second_click;
    do {
      second_click = digitalRead(button);
      if ((second_click == HIGH)
          || (futureMilis - currentMillis <= 0))
      {
        break;
      }
    } while (true);

    // skip timecheck if second_click == HIGH

    /* check time and date
     if (date is weekend) {
      if (hour >= 16 && minutes >= 45
    } else {
     if ( (hour >= 15 && minutes >= 45) && (hour    }
    */


    // start relay and start water

    // go idle and check time each 8 seconds..

    // shut down water and relay
    run_program = false;
  }
}
