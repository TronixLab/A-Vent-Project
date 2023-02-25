#define SAMPLE_RATE 50
#define ATTACK_TIME 0.1F
#define RELEASE_TIME 12.0F
#define SMOOTH_COEFF 0.5F
#define THRESH_RATIO 1.5F;
#define THRESH_DIFF 3.0F;

float ATTACK_COEFF = 0.00F;
float RELEASE_COEFF = 0.00F;

float v_high_pressure = 0.00F;
float v_low_pressure = 0.00F;
int t_high_pressure = 0;
int t_low_pressure = 0;
float v_max_pressure = 0.00F;
float v_min_pressure = 0.00F;
int t_peak_pressure = 0;

float v_high_volume = 0.00F;
float v_low_volume = 0.00F;
int t_high_volume = 0;
int t_low_volume = 0;
float v_max_volume = 0.00F;
float v_min_volume = 0.00F;
int t_peak_volume = 0;

float pip = 0;
float peep = 0;

float maxTV = 0;
float minTV = 0;

int THRESH_NC = 10;  // s
int THRESH_LP = 10;  // cm H20
int THRESH_HP = 30;  // cm H20
int THRESH_LR = 15;  // breaths/min
int THRESH_HR = 40;  // breaths/min
int THRESH_LV = 120; // mL
int THRESH_HV = 230; // mL

struct pressure {
  float min;
  float max;
};

struct volume {
  float max;
};

struct pressure pressureCurve;
struct volume volumeCurve;

struct pressure pressureTracker(float signal) {
  struct pressure pcurve;

  float p = signal;

  t_peak_pressure++;
  // if current sample is PIP attack
  if (p > v_high_pressure) {
    v_high_pressure = ATTACK_COEFF * v_high_pressure + (1 - ATTACK_COEFF) * p;
    v_max_pressure = p;
    t_high_pressure = 0;

    if (!inhalation) {
      // smooth peep display
      peep = SMOOTH_COEFF * peep + (1 - SMOOTH_COEFF) * v_min_pressure;
    }
  }
  // if current sample is PIP release
  else {
    v_high_pressure = RELEASE_COEFF * v_high_pressure + (1 - RELEASE_COEFF) * p;
    t_high_pressure++;
  }
  // if current sample is PEEP attack
  if (p < v_low_pressure) {
    v_low_pressure = ATTACK_COEFF * v_low_pressure + (1 - ATTACK_COEFF) * p;
    v_min_pressure = p;
    t_low_pressure = 0;

    if (inhalation && (t_peak_pressure > 1)) {

      // smooth pip display
      pip = SMOOTH_COEFF * pip + (1 - SMOOTH_COEFF) * v_max_pressure;

      // calculate breathing rate
      t_peak_pressure = t_high_pressure;
    }
  }
  // if current sample is PEEP release
  else {
    v_low_pressure = RELEASE_COEFF * v_low_pressure + (1 - RELEASE_COEFF) * p;
    t_low_pressure++;
  }

  pcurve.min = peep;
  pcurve.max = pip;
  return pcurve;
}

struct volume volumeTracker(float signal) {
  struct volume vcurve;

  float v = signal;

  t_peak_volume++;
  // if current sample is PIP attack
  if (v > v_high_volume) {
    v_high_volume = ATTACK_COEFF * v_high_volume + (1 - ATTACK_COEFF) * v;
    v_max_volume = v;
    t_high_volume = 0;

    if (!inhalation) {
      // smooth peep display
      minTV = SMOOTH_COEFF * minTV + (1 - SMOOTH_COEFF) * v_min_volume;
    }
  }
  // if current sample is PIP release
  else {
    v_high_volume = RELEASE_COEFF * v_high_volume + (1 - RELEASE_COEFF) * v;
    t_high_volume++;
  }
  // if current sample is PEEP attack
  if (v < v_low_volume) {
    v_low_volume = ATTACK_COEFF * v_low_volume + (1 - ATTACK_COEFF) * v;
    v_min_volume = v;
    t_low_volume = 0;

    if (inhalation && (t_peak_volume > 1)) {

      // smooth pip display
      maxTV = SMOOTH_COEFF * maxTV + (1 - SMOOTH_COEFF) * v_max_volume;

      // calculate breathing rate
      t_peak_volume = t_high_volume;
    }
  }
  // if current sample is PEEP release
  else {
    v_low_volume = RELEASE_COEFF * v_low_volume + (1 - RELEASE_COEFF) * v;
    t_low_volume++;
  }

  vcurve.max = maxTV;
  return vcurve;
}
