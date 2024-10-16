// Define the notes' frequencies (e.g., A4 = 440 Hz, C5 = 523 Hz)
#define NOTE_A4 440
#define NOTE_C5 523
#define NOTE_E5 659

//setting pwm resolution to 12 bit 
#define PWM_RESOLUTION 4096

#define PWM_CHANNEL  0  // Use channel 0 for PWM
#define SPEAKER_PIN 44

//frequency to set the pwm to in terms of Hz
#define PWM_FREQUENCY 5000

// idk the only 2 free pins is 44 and 43

void playTone(int frequency, int duration) {
    // Set the PWM frequency for the desired note
    ledcWriteTone(PWM_CHANNEL, frequency);
    delay(duration);  // Play the note for the specified duration
    ledcWriteTone(PWM_CHANNEL, 0);  // Turn off sound
}

void setup() {
    // Same setup as before
    ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(SPEAKER_PIN, PWM_CHANNEL);
}

void loop() {
    playTone(NOTE_A4, 500);  // Play A4 for 500ms
    delay(100);              // Short delay between notes
    playTone(NOTE_C5, 500);  // Play C5 for 500ms
    delay(100);
    playTone(NOTE_E5, 500);  // Play E5 for 500ms
    delay(100);
}