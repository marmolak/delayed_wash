#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <SoftwareSerial.h>

#define SECOND 1000
#define MINUTE (SECOND * 60)

static const byte button = 2;
static const byte warn_led = 13;

// changed in interrupt routines
bool error = false;

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
}


void go_idle()
{
  noInterrupts ();   // timed sequence below
  MCUSR = 0;                          // reset various flags
  byte adcsra_save = ADCSRA;
  ADCSRA = 0;  // disable ADC
  power_all_disable ();   // turn off all modules
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);   // sleep mode is set here
  sleep_enable();
  EIFR |= (1 << INTF0);
  attachInterrupt(0, run_program_impl, FALLING);
  interrupts ();

  sleep_cpu ();            // now goes to Sleep and waits for the interrupt
  detachInterrupt (0);

  ADCSRA = adcsra_save;  // stop power reduction
  power_all_enable ();   // turn on all modules

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
    EIFR |= (1 << INTF0);
    attachInterrupt(0, intr_routine, FALLING);

  }
  interrupts ();
  sleep_cpu ();            // now goes to Sleep and waits for the interrupt
  detachInterrupt (0);

  ADCSRA = adcsra_save;  // stop power reduction
  power_all_enable ();   // turn on all modules
}


void setup() {
  Serial.begin(9600);
  pinMode (warn_led, OUTPUT);
  pinMode (button, INPUT);
  digitalWrite(button, HIGH);

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

  // don't go idle now. Wait for pulse by X minutes.
  if (pulseIn(button, LOW, MINUTE * 5) == 0) {
     go_idle();
  }

  // Wait for 5 seconds
  // TODO: FIX this nasty overflow and also check for 
  // milis() turnaround
  unsigned long futureMilis = millis() + (SECOND * 5);

  byte local_click;
  do {
    local_click = digitalRead(button);
  } while ((futureMilis > millis()));



  if (local_click == 0) {

    blink_warn_led();
    blink_warn_led();
  } else {
    digitalWrite(13, HIGH);
    delay(3000);
    digitalWrite(13, LOW);
  }
}
