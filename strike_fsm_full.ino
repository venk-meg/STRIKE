// STRIKE FSM + Gesture Detectors Example for Arduino
// --------------------------------------------------
// This file demonstrates the full pipeline:
// Simulated sensor data -> Gesture Detectors -> FSM -> Actions
// All actions and detector firings print to Serial.

#include <Arduino.h>
#include <functional>

// --- Mock SensorValue struct (replace with your real sensor struct) ---
struct SensorValue {
    float linearMagnitude; // e.g., for fist detection
    float flickValue;      // e.g., for flick detection
    float handToEarValue;  // e.g., for hand-to-ear detection
};

// --- State Definitions ---
enum class State {
    Sleep,
    Idle,
    Recording,
    Listening,
    Slicing,
    Listening_RecHarm // Composite: Listening + Recording Harmony
};

// --- Event/Action Definitions ---
enum class Event {
    ListenStart,
    ListenStop,
    RecordStart,
    RecordStop,
    Delete,
    NextRecording,
    PrevRecording,
    Slice,
    SelectFront,
    SelectBack,
    Timeout,
    NextHarm,
    PrevHarm,
    Record // For 'record over from here' in Slicing
};

// --- Condition Function Stubs ---
bool atLeastOneTrackExists() { Serial.println("[Condition] At least one track exists (simulated: true)"); return true; }
bool harmExists() { Serial.println("[Condition] Harmony exists (simulated: true)"); return true; }
bool sliceMarked() { Serial.println("[Condition] Slice is marked (simulated: true)"); return true; }

// --- Action Function Stubs ---
void wakeUpStartPlayback() { Serial.println("[Action] Wake up and start playback. State: Sleep -> Listening"); }
void wakeUpStartRecording() { Serial.println("[Action] Wake up and start recording. State: Sleep -> Recording"); }
void stopRecording() { Serial.println("[Action] Stop recording. State: Recording -> Idle"); }
void stopRecordingSaveStartListen() { Serial.println("[Action] Stop recording, save track, start listening. State: Recording -> Listening"); }
void discardStopRecording() { Serial.println("[Action] Discard/stop recording due to timeout. State: Recording -> Idle"); }
void startHarmonyRecording() { Serial.println("[Action] Start harmony recording. State: Listening -> Listening+RecHarm"); }
void playNextRecording() { Serial.println("[Action] Play next recording. State: Listening -> Listening"); }
void playPrevRecording() { Serial.println("[Action] Play previous recording. State: Listening -> Listening"); }
void pausePlaybackStartSlice() { Serial.println("[Action] Pause playback, start slice mode. State: Listening -> Slicing"); }
void switchToNextHarmonyLayer() { Serial.println("[Action] Switch to next harmony layer. State: Listening -> Listening"); }
void switchToPrevHarmonyLayer() { Serial.println("[Action] Switch to previous harmony layer. State: Listening -> Listening"); }
void markSlice() { Serial.println("[Action] Mark slice. State: Slicing -> Slicing"); }
void deleteSliceResume() { Serial.println("[Action] Delete slice, resume playback. State: Slicing -> Listening"); }
void exitSliceMode() { Serial.println("[Action] Exit slice mode due to timeout. State: Slicing -> Idle"); }
void goToSleep() { Serial.println("[Action] Go to sleep. State: Idle -> Sleep"); }
void startRecording() { Serial.println("[Action] Start recording. State: Idle -> Recording"); }
void startPlayback() { Serial.println("[Action] Start playback. State: Idle -> Listening"); }
void recordOverFromSlicePoint() { Serial.println("[Action] Record over current track from slice point. State: Slicing -> Recording"); }
void stopListening() { Serial.println("[Action] Stop listening. State: Listening -> Idle"); }

// --- Transition Struct ---
struct Transition {
    State from;
    Event trigger;
    bool (*condition)(); // nullptr if always allowed
    State to;
    void (*action)();    // nullptr if no action
};

// --- Transition Table ---
Transition transitions[] = {
    // Sleep
    {State::Sleep, Event::ListenStart, atLeastOneTrackExists, State::Listening, wakeUpStartPlayback},
    {State::Sleep, Event::RecordStart, nullptr, State::Recording, wakeUpStartRecording},
    // Recording
    {State::Recording, Event::RecordStop, nullptr, State::Idle, stopRecording},
    {State::Recording, Event::ListenStart, atLeastOneTrackExists, State::Listening, stopRecordingSaveStartListen},
    {State::Recording, Event::Timeout, nullptr, State::Idle, discardStopRecording},
    // Listening
    {State::Listening, Event::RecordStart, nullptr, State::Listening_RecHarm, startHarmonyRecording},
    {State::Listening, Event::NextRecording, nullptr, State::Listening, playNextRecording},
    {State::Listening, Event::PrevRecording, nullptr, State::Listening, playPrevRecording},
    {State::Listening, Event::Slice, nullptr, State::Slicing, pausePlaybackStartSlice},
    {State::Listening, Event::NextHarm, harmExists, State::Listening, switchToNextHarmonyLayer},
    {State::Listening, Event::PrevHarm, harmExists, State::Listening, switchToPrevHarmonyLayer},
    {State::Listening, Event::ListenStop, nullptr, State::Idle, stopListening},
    // Slicing
    {State::Slicing, Event::SelectFront, nullptr, State::Slicing, markSlice},
    {State::Slicing, Event::SelectBack, nullptr, State::Slicing, markSlice},
    {State::Slicing, Event::Delete, sliceMarked, State::Listening, deleteSliceResume},
    {State::Slicing, Event::Timeout, nullptr, State::Idle, exitSliceMode},
    {State::Slicing, Event::Record, nullptr, State::Recording, recordOverFromSlicePoint}, // After, return to Listening
    // Idle
    {State::Idle, Event::Timeout, nullptr, State::Sleep, goToSleep},
    {State::Idle, Event::RecordStart, nullptr, State::Recording, startRecording},
    {State::Idle, Event::ListenStart, nullptr, State::Listening, startPlayback},
};
const int numTransitions = sizeof(transitions) / sizeof(Transition);

// --- FSM Class ---
class FSM {
public:
    State currentState;
    FSM(State initial) : currentState(initial) {}

    void handleEvent(Event e) {
        for (int i = 0; i < numTransitions; ++i) {
            if (transitions[i].from == currentState && transitions[i].trigger == e) {
                if (!transitions[i].condition || transitions[i].condition()) {
                    if (transitions[i].action) transitions[i].action();
                    currentState = transitions[i].to;
                    Serial.print("[FSM] Transitioned to state: ");
                    Serial.println(stateToString(currentState));
                    return;
                }
            }
        }
        Serial.println("[FSM] No valid transition for this event in current state.");
    }

    const char* stateToString(State s) {
        switch (s) {
            case State::Sleep: return "Sleep";
            case State::Idle: return "Idle";
            case State::Recording: return "Recording";
            case State::Listening: return "Listening";
            case State::Slicing: return "Slicing";
            case State::Listening_RecHarm: return "Listening+RecHarm";
            default: return "Unknown";
        }
    }
};

FSM fsm(State::Sleep);

// --- Gesture Detector Classes ---
#define THRESH_FIST 0.5
#define THRESH_FLICK 1.5
#define THRESH_HAND_TO_EAR 0.8

class FistDetector {
    bool lastState = false;
public:
    std::function<void()> onFistMade;
    std::function<void()> onFistReleased;
    void update(const SensorValue &sv) {
        bool nowFist = (sv.linearMagnitude < THRESH_FIST);
        if (!lastState && nowFist)  { Serial.println("[Detector] Fist made"); if(onFistMade) onFistMade(); }
        if ( lastState && !nowFist) { Serial.println("[Detector] Fist released"); if(onFistReleased) onFistReleased(); }
        lastState = nowFist;
    }
};

class FlickDetector {
    bool lastState = false;
public:
    std::function<void()> onFlick;
    void update(const SensorValue &sv) {
        bool nowFlick = (sv.flickValue > THRESH_FLICK);
        if (!lastState && nowFlick) { Serial.println("[Detector] Flick detected"); if(onFlick) onFlick(); }
        lastState = nowFlick;
    }
};

class HandToEarDetector {
    bool lastState = false;
public:
    std::function<void()> onHandUp;
    std::function<void()> onHandDown;
    void update(const SensorValue &sv) {
        bool nowUp = (sv.handToEarValue > THRESH_HAND_TO_EAR);
        if (!lastState && nowUp)  { Serial.println("[Detector] Hand to ear (up)"); if(onHandUp) onHandUp(); }
        if ( lastState && !nowUp) { Serial.println("[Detector] Hand from ear (down)"); if(onHandDown) onHandDown(); }
        lastState = nowUp;
    }
};

// --- Instantiate Detectors ---
FistDetector fist;
FlickDetector flick;
HandToEarDetector handEar;

// --- Arduino Setup & Loop ---
void setup() {
    Serial.begin(9600);
    delay(1000);
    Serial.println("\n--- STRIKE FSM + Gesture Detectors Demo Start ---");
    Serial.print("Initial state: ");
    Serial.println(fsm.stateToString(fsm.currentState));

    // Wire up detectors to FSM events
    fist.onFistMade     = [](){ fsm.handleEvent(Event::RecordStart); };
    fist.onFistReleased = [](){ fsm.handleEvent(Event::RecordStop); };
    flick.onFlick       = [](){ fsm.handleEvent(Event::Delete); };
    handEar.onHandUp    = [](){ fsm.handleEvent(Event::ListenStart); };
    handEar.onHandDown  = [](){ fsm.handleEvent(Event::ListenStop); };
}

// --- Simulated Sensor Data Sequence ---
SensorValue simulatedData[] = {
    {0.6, 0.0, 0.0}, // nothing
    {0.4, 0.0, 0.0}, // fist made
    {0.4, 0.0, 0.0}, // fist held
    {0.6, 0.0, 0.0}, // fist released
    {0.6, 1.6, 0.0}, // flick
    {0.6, 0.0, 0.9}, // hand to ear (up)
    {0.6, 0.0, 0.9}, // hand held up
    {0.6, 0.0, 0.0}, // hand down
    {0.4, 0.0, 0.0}, // fist made again
    {0.4, 0.0, 0.0}, // fist held
    {0.6, 0.0, 0.0}, // fist released
};
const int numSimulated = sizeof(simulatedData)/sizeof(SensorValue);

void loop() {
    static int idx = 0;
    if (idx < numSimulated) {
        SensorValue sv = simulatedData[idx];
        Serial.print("\n[Loop] Simulated sensor step "); Serial.println(idx);
        fist.update(sv);
        flick.update(sv);
        handEar.update(sv);
        idx++;
        delay(2000);
    } else {
        Serial.println("\n--- End of simulation ---");
        while(1);
    }
} 