typedef struct
{
  // config
  uint8_t waveform;
  float osc_mix;
  float detune;
  float osc_pw;

  uint8_t osc2_waveform;
  float osc2_detune;
  float osc2_transpose;
  float osc2_pw;

  float eg_a_attack;
  float eg_a_decay;
  float eg_a_sustain;  // level
  float eg_a_release;
  float eg_f_attack;
  float eg_f_decay;
  float eg_f_sustain;  // level
  float eg_f_release;

  uint8_t lfo_waveform;
  float lfo_freq;
  float lfo_amp;

  uint8_t pwmlfo_waveform;
  float pwmlfo_freq;
  float pwmlfo_amp;

  uint8_t pwm2lfo_waveform;
  float pwm2lfo_freq;
  float pwm2lfo_amp;

  uint8_t vcavcflfo_waveform;
  float vcavcflfo_freq;
  float vcavcflfo_amp;

  float vcf_kbd_follow;
  float env_kbd_follow;

  float filter_res;
  float filter_cutoff;
  float eg_f_amount;
  uint8_t vel_select;
  uint8_t midi_channel;
} SynthSettings;
