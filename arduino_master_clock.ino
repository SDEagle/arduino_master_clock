#define BOUNCE_LOCK_OUT
#include <Bounce2.h>
#include "MIDIUSB.h"
#include <TimerOne.h>

#define PORT_START_STOP 2
#define PORT_NEXT 3
#define PORT_PREV 4
#define BOUNCE_TIME 5

Bounce start_stop_bouncer = Bounce();
Bounce next_bouncer = Bounce();
Bounce prev_bouncer = Bounce();

boolean running = false;

// In beats per thousand minutes, BPM with three decimal places
typedef unsigned long tempo;
int current_song_index = 0;
int current_track_selection = 0;
tempo song_tempos[] = {180000, 140000, 143000, 150000, 162000, 111000, 160000};
#define song_count() (sizeof(song_tempos) / sizeof(tempo))

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

  Timer1.initialize(calculateIntervalMicroSecs(song_tempos[current_song_index]));
  Timer1.attachInterrupt(sendClockPulseEvent);
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
    sendStopEvent();
  } else {
    sendStartEvent();
  }
  running = !running;
}

void onNextPressed() {
  change_song(1);
}

void onPrevPressed() {
  change_song(-1);
}

void sendStartEvent() {
  Timer1.stop();
  createStartEvent();
  MidiUSB.flush();
  Timer1.restart();
}

void sendStopEvent() {
  createStopEvent();
  MidiUSB.flush();
}

void change_song(int offset) {
  if (running) {
    Timer1.stop();
    createStopEvent();
    current_song_index = (current_song_index + offset) % song_count();
    updateTimer();
    updateTrackSelection();
    createStartEvent();
    MidiUSB.flush();
    Timer1.restart();
  } else {
    current_song_index = (current_song_index + offset) % song_count();
    updateTimer();
    updateTrackSelection();
    MidiUSB.flush();
  }
}

void updateTimer() {
  Timer1.setPeriod(calculateIntervalMicroSecs(song_tempos[current_song_index]));
}

unsigned long calculateIntervalMicroSecs(tempo bptm) {
  // 60 * 1000 * 1000 * 1000 / 24 (PULSES PER BEAT) /bptm
  return 2500000000ul / bptm;
}

void updateTrackSelection() {
  int offset = current_song_index - current_track_selection;
  while(offset > 0) {
    createCCEventForNextTrack();
    offset--;
  }
  while(offset < 0) {
    createCCEventForPrevTrack();
    offset++;
  }
}

void sendClockPulseEvent() {
  createClockPulseEvent();
  MidiUSB.flush();
}


void createClockPulseEvent() {
  uint8_t data[2];
  data[0] = 0x0F;
  data[1] = 0xF8;
  MidiUSB.write(data, 2);
}

void createStartEvent() {
  uint8_t data[2];
  data[0] = 0x0F;
  data[1] = 0xFA;
  MidiUSB.write(data, 2);
}

void createStopEvent() {
  uint8_t data[2];
  data[0] = 0x0F;
  data[1] = 0xFC;
  MidiUSB.write(data, 2);
}

void createCCEventForNextTrack() {
  createControlChangeEvent(0, 80, 127);
  createControlChangeEvent(0, 80, 0);
}

void createCCEventForPrevTrack() {
  createControlChangeEvent(0, 81, 127);
  createControlChangeEvent(0, 81, 0);
}

void createControlChangeEvent(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

