#include <MIDI.h>
#include "DaisyDuino.h"
#include "basicsynth.h"

DaisyHardware hw;
// listen to all channels
#define LISTEN_TO_ALL_CHANNELS 1

MIDI_CREATE_DEFAULT_INSTANCE();

/// SYNTH
// synth

#define VOICES_MAX 4

Oscillator osc_0_[VOICES_MAX];
Oscillator osc_1_[VOICES_MAX];
Oscillator osc_2_[VOICES_MAX];
Oscillator osc_3_[VOICES_MAX];
Adsr envAdsr_[VOICES_MAX];

int notes[VOICES_MAX];
float notes_freq[VOICES_MAX];

int next_note = 0;
int notes_played = 0;

SynthSettings *settings = new SynthSettings();

void SetupSynth(float samplerate) {
  for (int i = 0; i < VOICES_MAX; i++) {
    osc_0_[i].Init(samplerate);
    osc_0_[i].SetWaveform(Oscillator::WAVE_SAW);
    osc_0_[i].SetFreq(220 * (i + 1) / 5.0);

    osc_1_[i].Init(samplerate);
    osc_1_[i].SetWaveform(Oscillator::WAVE_SAW);
    osc_1_[i].SetFreq(220 * (i + 1) / 5.0);

    osc_2_[i].Init(samplerate);
    osc_2_[i].SetWaveform(Oscillator::WAVE_SAW);
    osc_2_[i].SetFreq(220 * (i + 1) / 5.0);

    osc_3_[i].Init(samplerate);
    osc_3_[i].SetWaveform(Oscillator::WAVE_SAW);
    osc_3_[i].SetFreq(220 * (i + 1) / 5.0);
  }


  for (int i = 0; i < VOICES_MAX; i++) {
    envAdsr_[i].Init(samplerate);
    envAdsr_[i].SetTime(ADSR_SEG_ATTACK, settings->eg_a_attack);
    envAdsr_[i].SetTime(ADSR_SEG_DECAY, settings->eg_a_decay);
    envAdsr_[i].SetTime(ADSR_SEG_RELEASE, settings->eg_a_release);
    envAdsr_[i].SetSustainLevel(settings->eg_a_sustain);
  }
}

/// SYNTH
static void AudioCallback(float **in, float **out, size_t size) {
  float osc1, osc2, osc3, osc4, sum, ea;
  bool note_on;
  for (int i = 0; i < size; i++) {
    sum = 0.0;
    for (int j = 0; j < VOICES_MAX; j++) {
      note_on = (notes[j] != 0);
      ea = envAdsr_[j].Process(note_on);

      osc_0_[j].SetFreq(notes_freq[j]);
      osc_1_[j].SetFreq(notes_freq[j] * 1.01);
      //osc_2_[j].SetFreq(notes_freq[j] * 1.02);
      //osc_3_[j].SetFreq(notes_freq[j] * 1.03);
      //if (note_on) {
      osc1 = osc_0_[j].Process();
      osc2 = osc_1_[j].Process();
      //osc3 = osc_2_[j].Process();
      //osc4 = osc_3_[j].Process();
      //sum += (synth / 2.0) * env_[j].Process();
      sum += ((osc1 + osc2 /* + osc3  + osc4*/) / 4.0) * ea;
      //}
    }
    sum = sum * 0.05;
    out[0][i] = out[1][i] = sum;
  }
}

void handleControlChange(byte inChannel, byte d1, byte d2) {
  Serial.print("cc: ");
  Serial.print(inChannel);
  Serial.print(", ");
  Serial.print(d1);
  Serial.print(", ");
  Serial.print(d2);

  int change = 0;


  if (d1 == 48) {  //reset all
    float value = lin2log(d2);
    Serial.println("RESET");
    for (int j = 0; j < VOICES_MAX; j++) {
      notes[j] = 0;
      notes_freq[j] = 0;
      change++;
    }
  }

  if (d1 == 20) {  //A_Attack
    float value = lin2log(d2);
    Serial.print("Attack: ");
    Serial.println(value);
    settings->eg_a_attack = value;
    change++;
  }

  if (d1 == 21) {  //A_Decay
    float value = lin2log(d2);
    Serial.print("Decay: ");
    Serial.println(value);
    settings->eg_a_decay = value;
    change++;
  }

  if (d1 == 22) {  //A_Sustain
    float value = d2 / 128.0;
    Serial.print("Sustain: ");
    Serial.println(value);
    settings->eg_a_sustain = value;
    change++;
  }

  if (d1 == 23) {  //A_Release
    float value = lin2log(d2);
    Serial.print("Release: ");
    Serial.println(value);
    settings->eg_a_release = value;
    change++;
  }

  if (change) {
    updateSettings();
    change = 0;
  }
}

float lin2log(int d2) {
  if (d2 == 0) return 0.01;
  float extended = 10000 - map128(d2, 2, 10000);
  float value = (9.3 - log(extended)) / 4.0;
  value = max(0.01f, value);
  return value;
}

float map128(int i, float m1, float m2) {
  float diff = m2 - m1;
  float factor = diff / 128;
  return factor * i;
}

void updateSettings() {
  for (int i = 0; i < VOICES_MAX; i++) {
    envAdsr_[i].SetTime(ADSR_SEG_ATTACK, settings->eg_a_attack);
    envAdsr_[i].SetTime(ADSR_SEG_DECAY, settings->eg_a_decay);
    envAdsr_[i].SetTime(ADSR_SEG_RELEASE, settings->eg_a_release);
    envAdsr_[i].SetSustainLevel(settings->eg_a_sustain);
  }
}

void handleNoteOn(byte inChannel, byte inNote, byte inVelocity) {
  Serial.print("midi on: ");
  Serial.print(inChannel);
  Serial.print(", ");
  Serial.print(inNote);
  Serial.print(", ");
  Serial.println(inVelocity);

  if (LISTEN_TO_ALL_CHANNELS) {
    for (uint8_t i = 0; i < VOICES_MAX; i++) {
      next_note = (next_note + 1) % VOICES_MAX;

      if (notes[next_note] == 0) {
        notes[next_note] = inNote;
        //env_[next_note].Trigger();
        //envAdsr_[next_note].Retrigger(false);
        notes_freq[next_note] = mtof(inNote);
        break;
      }
    }
  }
}

void handleNoteOff(byte inChannel, byte inNote, byte inVelocity) {
  Serial.println("midi off ...");
  Serial.print("midi on: ");
  Serial.print(inChannel);
  Serial.print(", ");
  Serial.print(inNote);
  Serial.print(", ");
  Serial.println(inVelocity);

  for (int i = 0; i < VOICES_MAX; i++) {
    if (notes[i] == inNote) {
      notes[i] = 0;
      notes_played = max(notes_played - 1, 0);
    }
  }
}

void InitSynthSettings() {
  settings->detune = 1.0;
  settings->eg_a_attack = 0.01;
  settings->eg_a_decay = 0.5;
  settings->eg_a_release = 0.5;
  settings->eg_a_sustain = 0.1;

  settings->eg_f_attack = 0.01;
  settings->eg_f_attack = 0.5;
  settings->eg_f_release = 1.0;
  settings->eg_a_sustain = 0.5;
  settings->eg_f_amount = 1.0;

  settings->filter_cutoff = 5000.0;
  settings->filter_res = 1.0;
}

void setup() {
  hw = DAISY.init(DAISY_POD, AUDIO_SR_48K);
  float samplerate = DAISY.get_samplerate();

  InitSynthSettings();
  SetupSynth(samplerate);

  hw.leds[0].Set(0, 0, 1);
  hw.leds[1].Set(1, 0, 0);

  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandleControlChange(handleControlChange);

  MIDI.begin(MIDI_CHANNEL_OMNI);  // Listen to all incoming messages
  delay(1000);
  Serial.begin(9600);
  delay(1000);
  Serial.print("Samplerate: ");
  Serial.println(samplerate);
  DAISY.begin(AudioCallback);
}

void loop() {
  // Read incoming messages
  MIDI.read();
}
