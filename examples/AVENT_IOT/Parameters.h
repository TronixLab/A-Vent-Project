float airFlow = 0.00;
float tidalVolume = 0.00;
float pressureCMH2O = 0.00;

static volatile float filteredFlow = 0.00;
static volatile float scaledVol = 0.00;
static volatile float filteredPres = 0.00;

static unsigned long previousMillis = 0;
static unsigned long last_interval_ms = 0;
int zeroCount = 0;

bool inhalation = false;
bool lastBreath = false;

int respiration_rate = 0;
unsigned long int respiratoryPeriod = 0;
unsigned long int lastPeriod = 0;

// To classify 1 frame of data you need EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE values
static float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
// Keep track of where we are in the feature array
size_t feature_ix = 0;

String machineStatus = "";
String pvaStatus = "";
String pressureStatus = "";
String respirationStatus = "";
String volumeStatus = "";
String anomalyStatus = "";

static int asynchronyPositiveAlarm = 0;
static int pthdPositiveAlarm = 0;
static int fthdPositiveAlarm = 0;
static int vthdPositiveAlarm = 0;

int alarmSignal = 0;
float anomalyScore = 0;

String alarmType = "";
bool asynchronyAlarm = false;
bool thresholdAlarm = false;

int displayMode = 0;

unsigned long lastReconnectAttempt = 0;
unsigned long lastMsg = 0;
