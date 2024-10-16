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

//SETUP TASK TO RUN IN CORE 1
TaskHandle_t Task1;


void playTone(int frequency, int duration) {
    // Set the PWM frequency for the desired note
    ledcWriteTone(PWM_CHANNEL, frequency);
    delay(duration);  // Play the note for the specified duration
    ledcWriteTone(PWM_CHANNEL, 0);  // Turn off sound
}

//function that will be run by core1
void Task1code(void * pvParameters) {
  for(;;) {
    playTone(NOTE_A4, 500);  // Play A4 for 500ms
    delay(100);              // Short delay between notes
    playTone(NOTE_C5, 500);  // Play C5 for 500ms
    delay(100);
    playTone(NOTE_E5, 500);  // Play E5 for 500ms
    delay(100);

  }
}



void setup() {
  //Setup for running stuff specifically on core 1 
  xTaskCreatePinnedToCore(
      Task1code, /* Function to implement the task */
      "Task1", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &Task1,  /* Task handle. */
      1); /* Core where the task should run */



    // Same setup as before
    ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(SPEAKER_PIN, PWM_CHANNEL);
}

void loop() {
    
}