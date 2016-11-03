/*
   This examples shows how to make a simple seven keys MIDI keyboard with volume control

   Created: 4/10/2015
   Author: Arturo Guadalupi <a.guadalupi@arduino.cc>
   
   http://www.arduino.cc/en/Tutorial/MidiDevice
*/

#include "MIDIUSB.h"
#include "PitchToNote.h"

#define BOUNCE_LOCK_OUT
#include <Bounce2.h>
#include <Keyboard.h>

#define PORT_START_STOP 2
#define PORT_NEXT 3
#define PORT_PREV 4
#define BOUNCE_TIME 5

#define ABLETON_NEXT_SCENE KEY_DOWN_ARROW
#define ABLETON_PREV_SCENE KEY_UP_ARROW
#define ABLETON_LAUNCH_SCENE 'j'
#define ABLETON_STOP_TRANSPORT 'k'
#define ABLETON_STOP_CLIPS 'l'

boolean running = false;

Bounce start_stop_bouncer = Bounce();
Bounce next_bouncer = Bounce();
Bounce prev_bouncer = Bounce();

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

  Keyboard.begin();
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
    Keyboard.write(ABLETON_NEXT_SCENE);
    delay(1);
    Keyboard.write(ABLETON_LAUNCH_SCENE);
    delay(1);
    Keyboard.write(ABLETON_PREV_SCENE);
  } else {
    Keyboard.write(ABLETON_NEXT_SCENE);
  }
}

void onPrevPressed() {
  if (running) {
    Keyboard.write(ABLETON_PREV_SCENE);
    delay(1);
    Keyboard.write(ABLETON_LAUNCH_SCENE);
    delay(1);
    Keyboard.write(ABLETON_PREV_SCENE);
  } else {
    Keyboard.write(ABLETON_PREV_SCENE);
  }
}

void loop() {
//  playNotes();
  sendClock();
//  noteOff(0, pitchC3, 0);
//  MidiUSB.flush();
  delay(42);

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

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

void playNotes() {
  noteOn(0, pitchC3, 100);
  MidiUSB.flush();
  delay(500);
  noteOff(0, pitchC3, 0);
  MidiUSB.flush();
  delay(500);
}

void sendClock() {
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
