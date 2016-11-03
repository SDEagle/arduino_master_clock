#include "MIDIUSB.h"

#define BOUNCE_LOCK_OUT
#include <Bounce2.h>

#include <TimerOne.h>

#define PORT_START_STOP 2
#define PORT_NEXT 3
#define PORT_PREV 4
#define BOUNCE_TIME 5

boolean running = false;

Bounce start_stop_bouncer = Bounce();
Bounce next_bouncer = Bounce();
Bounce prev_bouncer = Bounce();

// Beats per thousand minitues, BPM with three decimal places
unsigned long bptm = 60000;

void setup() {
  pinMode(PORT_START_STOP, INPUT);
  pinMode(PORT_NEXT, INPUT);
  pinMode(PORT_PREV, INPUT);
  digitalWrite(PORT_START_STOP, INPUT_PULLUP);
  digitalWrite(PORT_NEXT, INPUT_PULLUP);
  digitalWrite(PORT_PREV, INPUT_PULLUP);
  start_stop_bouncer.attach(PORT_START_STOP);
  next_bouncer.attach(PORT_NEXT);
  prev_bouncer.attach(PORT_PREV);
  start_stop_bouncer.interval(BOUNCE_TIME);
  next_bouncer.interval(BOUNCE_TIME);
  prev_bouncer.interval(BOUNCE_TIME);

  Timer1.initialize(calculateIntervalMicroSecs(bptm));
  Timer1.attachInterrupt(sendClockPulse);
}

void loop() {
  start_stop_bouncer.update();
  next_bouncer.update();
  prev_bouncer.update();

  if (start_stop_bouncer.fell()) {
    onStartStopPressed();
  }
  if (next_bouncer.fell()) {
    onNextPressed();
  }
  if (prev_bouncer.fell()) {
    onPrevPressed();
  }
}

void onStartStopPressed() {
  if (running) {
    sendStop();
  } else {
    sendStart();
  }
  running = !running;
}

void onNextPressed() {
  if (running) {
  } else {
  }
}

void onPrevPressed() {
  if (running) {
  } else {
  }
}

void updateTimer() {
  Timer1.setPeriod(calculateIntervalMicroSecs(bptm));
}

unsigned long calculateIntervalMicroSecs(unsigned long bptm) {
  // 60 * 1000 * 1000 * 1000 / 24 (PULSES PER BEAT) /bptm
  return 2500000000ul / bptm;
}

void sendClockPulse() {
  uint8_t data[2];
  data[0] = 0x0F;
  data[1] = 0xF8;
  MidiUSB.write(data, 2);
  MidiUSB.flush();
}

void sendStart() {
  uint8_t data[2];
  data[0] = 0x0F;
  data[1] = 0xFA;
  MidiUSB.write(data, 2);
  MidiUSB.flush();
}

void sendStop() {
  uint8_t data[2];
  data[0] = 0x0F;
  data[1] = 0xFC;
  MidiUSB.write(data, 2);
  MidiUSB.flush();
}

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}
