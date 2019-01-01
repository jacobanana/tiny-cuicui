/*
 * Adrien Fauconnet - 2018
 * Cuicui for AtTiny85
 *
 * Pin 1 - PB5 = Disconnected
 * Pin 2 - PB3 = LDR / 3k3 voltage divider
 * Pin 3 - PB4 = Speaker (pin 1)
 * Pin 4       = GND
 * Pin 5 - PB0 = LEDs
 * Pin 6 - PB1 = Speaker (pin 2)
 * Pin 7 - PB2 = Momentary switch to GND - using internal pull up
 * Pin 8       = 5V
*/

#include "Arduino.h"
#include "wavetables.h"

#define WAVE SINE
#define SWEEP_LENGTH 50
#define THRESHOLD 768
#define N_MODES 3

int wt_counter = 0; // wavetable position
int s = 1; // step in the wavetable (controls frequency)

int ldr_counter, pitch_counter, led_counter = 0; // counters for LDR read and pitch refresh and LED
boolean output_enable, led_state, current_sw, previous_sw = 1; // states
int mode = 0; // current mode (0 = sweep / 1 = pitch)

int ldr; // LDR voltage divider ADC value

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
// the setup routine runs once when you press reset:
void setup() {
  // Enable 64 MHz PLL and use as source for Timer1
  PLLCSR = 1<<PCKE | 1<<PLLE;

  // Set up Timer/Counter1 for PWM output
  TIMSK = 0;                              // Timer interrupts OFF
  TCCR1 = 1<<PWM1A | 2<<COM1A0 | 1<<CS10; // PWM A, clear on match, 1:1 prescale
  GTCCR = 1<<PWM1B | 2<<COM1B0;           // PWM B, clear on match
  OCR1A = 128; //OCR1B = 128;               // 50% duty at start

  // Set up Timer/Counter0 for 8kHz interrupt to output samples.
  TCCR0A = 3<<WGM00;                      // Fast PWM
  TCCR0B = 1<<WGM02 | 2<<CS00;            // 1/8 prescale
  TIMSK = 1<<OCIE0A;                      // Enable compare match
  OCR0A = 124;                            // Divide by 1000

  pinMode(PB1, OUTPUT);
  pinMode(PB4, OUTPUT);
  pinMode(PB0, OUTPUT); // LED
  pinMode(PB2, INPUT_PULLUP); // on/off mom switch
}

// the loop routine runs over and over again forever:
void loop() { }

ISR (TIMER0_COMPA_vect) {
  current_sw = digitalRead(PB2);
  if (current_sw != previous_sw){
    if(current_sw == HIGH){
      mode = (mode + 1) % N_MODES;
    }
    previous_sw = current_sw;
  }

  ldr_counter += 1;
  if (ldr_counter >= 80){
    // read LDR value
    ldr_counter = 0;
    ldr = 1024-analogRead(PB3);
  }

  switch (mode){
    case 0:
      pitch_counter += 1;
      if (pitch_counter >= ldr/2){
        // update pitch according to LDR value
        pitch_counter = 0;
        s = (s + 1) % SWEEP_LENGTH + 1;
      }
      break;

    case 1:
      s = map(ldr, 1024, 0, 0, SWEEP_LENGTH);
      break;

    default:
      output_enable = !output_enable;
      mode = 0;
      break;

  }

  wt_counter = (wt_counter + s) % N;

  if (ldr <= THRESHOLD){
    // LED Control
    led_counter += 1;
    if (led_counter >= ldr*4){
      led_counter = 0;
      led_state = !led_state;
      digitalWrite(PB0, led_state);
    }

    // Audio output
    if (output_enable==1){
      OCR1A = WAVE[wt_counter];
      OCR1B = WAVE[wt_counter] ^ 255; // PWM values, ACR1B is complementary to OCR1A
    }
  } else {
    // The bird is resting ...
    OCR1A = 0;
    digitalWrite(PB0, 0);
  }

}
