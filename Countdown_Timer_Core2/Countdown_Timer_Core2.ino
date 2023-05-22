/*================================================================================*
   Core2 Animated Countdown Timer                       Version 1.01 - 21 May 2023

   Flexible and affordable Pinewood Derby timer that interfaces with the 
   following software:
     - PD Test/Tune/Track Utility
     - Grand Prix Race Manager software

   Refer to the Readme document for setup and usage instructions.


   Copyright (C) 2023-     Damien Norris

   This work is licensed under the Creative Commons Attribution-NonCommercial-
   ShareAlike 3.0 Unported License. To view a copy of this license, visit 
   http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to 
   Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 
   94041, USA.
 *================================================================================*/

/*================================================================================*
   SETUP FOR APPLICATION -
 *================================================================================*/
#include <M5Core2.h>

#include "timer_tones.h"
#include "timerLogo.h"

#define LGFX_M5STACK_CORE2        // M5Stack M5Stack Core2
#define LGFX_AUTODETECT           // Automatic recognition (D-duino-32 XS, PyBadge seems to be excluded from automatic recognition because it cannot read the panel ID)

#include <LovyanGFX.hpp>          // Prepare header for lovyanGFX
#include <LGFX_AUTODETECT.hpp>    // Prepare class "LGFX"

static LGFX lcd;                  // Create an instance of LGFX (use the class LGFX to do things with the lcd command) 
static LGFX_Sprite ttext(&lcd);   // Create an instance of LGFX_Sprite when using sprites
static LGFX_Sprite arc(&lcd);

static auto transpalette = 0;

/*================================================================================*
   - END SETUP FOR APPLICATION -
 *================================================================================*/

////////////////////////////////////////////////////////////////////////////////////

/*================================================================================*
   - VARIABLES & CONSTANTS -
 *================================================================================*/
enum Mode {
  SET_NUM_REPS,
  SET_WORK_TIME,
  SET_TIMER_READY,
  RUN_WORK_TIME,
  PAUSE_WORK_TIME,
  END_WORK_TIME,
  SET_REST_TIME,
  RUN_REST_TIME,
  PAUSE_REST_TIME,
  RESET_TIMERS
};

enum Mode mode;

// Set the variables for the Number of Reps
int set_reps         = 1;
int pset_reps        = 0;
int remainingReps    = 1;

int numReps;

int c_pos = 0;
int p_pos = 0;

int rep_pos[] = { 120, 152};
int rep_inc[] = { 10, 1 };

// Set the variables for the Workout Timer
unsigned int set_time   = 60;
unsigned int pset_time  =  0;
unsigned int remainingTime;

int workMins = 0;
int workSecs = 0;

unsigned long p_millis;
unsigned long c_millis;

// Set the variables for the Rest Period Timer
unsigned int set_rest_time   = 60;
unsigned int pset_rest_time  =  0;
unsigned int remainingRestTime;

int restMins = 0;
int restSecs = 0;

unsigned long w_millis;
unsigned long x_millis;

// Set the shared variables for both Timers
unsigned long lastBeepTime  =  0;

int sound = 0;

int pos[] = { 91, 123, 167, 199 };
int inc[] = { 600, 60, 10, 1 };

byte buttonCounter   = 0;

/*================================================================================*
   - END VARIABLES & CONSTANTS -
 *================================================================================*/

 ////////////////////////////////////////////////////////////////////////////////////

/*================================================================================*
   - AUDIO SETUP -
 *================================================================================*/
//                                0,       1,      
const unsigned char *wavList[] = {sec_beep, countdownTimer};
const size_t wavSize[] = {sizeof(sec_beep), sizeof(countdownTimer)};

#define CONFIG_I2S_BCK_PIN      12
#define CONFIG_I2S_LRCK_PIN     0
#define CONFIG_I2S_DATA_PIN     2
#define CONFIG_I2S_DATA_IN_PIN  34

#define SPEAKER_I2S_NUMBER      I2S_NUM_0

#define MODE_MIC                0
#define MODE_SPK                1

void InitI2SSpeakerOrMic(int mode) {
  esp_err_t err = ESP_OK;

  i2s_driver_uninstall(SPEAKER_I2S_NUMBER);
  i2s_config_t i2s_config = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER),
    .sample_rate          = 16000,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format       = I2S_CHANNEL_FMT_ALL_RIGHT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = 6,
    .dma_buf_len          = 60,
    .use_apll             = false,
    .tx_desc_auto_clear   = true,
    .fixed_mclk           = 0
  };
  if (mode == MODE_MIC) {
    i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM);
  } else {
    i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
  }

  err += i2s_driver_install(SPEAKER_I2S_NUMBER, &i2s_config, 0, NULL);

  i2s_pin_config_t tx_pin_config = {
    .bck_io_num           = CONFIG_I2S_BCK_PIN,
    .ws_io_num            = CONFIG_I2S_LRCK_PIN,
    .data_out_num         = CONFIG_I2S_DATA_PIN,
    .data_in_num          = CONFIG_I2S_DATA_IN_PIN,
  };
  err += i2s_set_pin(SPEAKER_I2S_NUMBER, &tx_pin_config);

  if (mode != MODE_MIC) {
    err += i2s_set_clk(SPEAKER_I2S_NUMBER, 16000, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
  }

  i2s_zero_dma_buffer(SPEAKER_I2S_NUMBER);
}

/*================================================================================*
   - END AUDIO SETUP -
 *================================================================================*/

////////////////////////////////////////////////////////////////////////////////////

/*================================================================================*
   - TIMER DISPLAY FUNCTION -
 *================================================================================*/
void disp_num_reps(int num_reps) {              //  Display Set Reps Digits
  ttext.setCursor(0, 0);
  ttext.setTextColor(GREEN, 0);
  ttext.printf("%02d", num_reps);
  ttext.pushRotateZoom(190, 115, 0, 1, 1);
}

void disp_time(int d_min, int d_sec) {  //  Display Timer
  arc.pushRotateZoom(160, 120, 0, 1.5, 1);
  ttext.setCursor(0, 0);
  ttext.setTextColor(DARKCYAN, 0);
  ttext.printf("%02d:%02d", d_min, d_sec);
  ttext.pushRotateZoom(160, 115, 0, 1, 1);
}

void disp_rest_time(int d_min, int d_sec) {  //  Display Timer
  arc.pushRotateZoom(160, 120, 0, 1.5, 1);
  ttext.setCursor(0, 0);
  ttext.setTextColor(MAROON, 0);
  ttext.printf("%02d:%02d", d_min, d_sec);
  ttext.pushRotateZoom(160, 115, 0, 1, 1);
}

/*================================================================================*
   - END TIMER DISPLAY FUNCTION -
 *================================================================================*/

////////////////////////////////////////////////////////////////////////////////////

/*================================================================================*
   - MAIN SETUP FUNCTION -
 *================================================================================*/
void setup() {
  M5.begin();

  lcd.init();

  Serial.begin(115200);

  lcd.setBrightness(255);
  lcd.setColorDepth(16);
  lcd.clear();

  ttext.setFont(&fonts::Font7);
  lcd.fillScreen(BLACK);

  arc.setColorDepth(16);
  ttext.setColorDepth(16);

  arc.createSprite(240, 200);
  ttext.createSprite(140, 60);

  arc.fillScreen(transpalette);
  ttext.fillScreen(transpalette);
  lcd.fillScreen(transpalette);

  arc.fillArc(80, 80, 75, 80, 0, 360, GREEN);

  arc.setPivot(80, 80);
  ttext.setPivot(70, 15);

  lcd.setTextColor(WHITE, 0);
  lcd.setTextSize(2);

/************************** Display Logo ***************************/
  lcd.startWrite();

  lcd.pushImage(0, 0, timerLogoWidth, timerLogoHeight, timerLogo);
/*******************************************************************/

  delay(5000);

  lcd.clear();

  mode = SET_NUM_REPS;
}

/*================================================================================*
   - END MAIN SETUP FUNCTION -
 *================================================================================*/

////////////////////////////////////////////////////////////////////////////////////

/*================================================================================*
   - MAIN LOOP FUNCTION -
 *================================================================================*/
void loop() {
  M5.update();

  switch (mode) {
    case SET_NUM_REPS:
      set_num_reps();
      break;
    case SET_WORK_TIME:
      set_work_timer();
      break;
    case SET_REST_TIME:
      set_rest_timer();
      break;
    case SET_TIMER_READY:
      run_timer_ready();
      break;
    case RUN_WORK_TIME:
      run_work_timer();
      break;
    case PAUSE_WORK_TIME:
      work_timer_paused();
      break;
    case RUN_REST_TIME:
      run_rest_timer();
      break;
    case PAUSE_REST_TIME:
      rest_timer_paused();
      break;
    case RESET_TIMERS:
      reset_timers();
      break;
  }

  // Uses long press on the same button to navigate functions
  if (M5.BtnC.pressedFor(500)) {    
    delay(1000);

    buttonCounter++;

    Serial.println(buttonCounter);

    // set the number of reps -> set the workout timer
    if (mode == SET_NUM_REPS && set_reps >= 1 && buttonCounter == 1) {
      arc.fillArc(80, 80, 75, 80, 0, 360, GREEN);
      mode = SET_WORK_TIME;
    }
    // set the workout timer -> set the rest timer
    else if (mode == SET_WORK_TIME && buttonCounter == 2) {
      arc.fillArc(80, 80, 75, 80, 0, 360, RED);
      mode = SET_REST_TIME;
    } 
    // set the rest timer -> load the timers ready function
    else if (mode == SET_REST_TIME && buttonCounter == 3) {
      lcd.clear();

      arc.fillArc(80, 80, 75, 80, 0, 360, GREEN);
      workMins = set_time / 60;
      workSecs = set_time % 60;
      disp_time(workMins, workSecs);
      remainingTime = set_time;
      mode = SET_TIMER_READY;
    } 
  }
}  

/*================================================================================*
   - END MAIN LOOP FUNCTION -
 *================================================================================*/

////////////////////////////////////////////////////////////////////////////////////

/*================================================================================*
   - SET NUMBER OF REPS FUNCTION -
 *================================================================================*/
void set_num_reps() {

  lcd.setFont(&fonts::Font4);
  lcd.drawString("SET NUM REPS:", 15, 5, 1);
  lcd.drawString("DOWN", 15, 220, 1);
  lcd.drawString("UP", 155, 220, 1);
  lcd.drawString("MOVE", 255, 220, 1);
  lcd.drawString("NEXT/", 255, 190, 1);

  if (M5.BtnC.wasPressed()) {                       // move set up cursor next
    lcd.drawFastHLine(rep_pos[c_pos], 148, 30, 0);
    c_pos++;
    if (c_pos > 1) c_pos = 0;
    lcd.drawFastHLine(rep_pos[c_pos], 148, 30, GREEN);
  }

  /*if (M5.BtnC.wasPressed()) {                       // move set up cursor previous
    lcd.drawFastHLine(pos[c_pos], 148, 30, 0);
    c_pos--;
    if (c_pos < 0) c_pos = 3;
    lcd.drawFastHLine(pos[c_pos], 148, 30, WHITE);
  }*/

  if (M5.BtnB.wasPressed()) {                       // increase figure on cursor
    set_reps += rep_inc[c_pos];
    if (set_reps > 99) {
      set_reps -= rep_inc[c_pos];
    }
  }

  if (M5.BtnA.wasPressed()) {                       // decrease figure on cursor
    set_reps -= rep_inc[c_pos];
    if (set_reps == 99) {
      set_reps += rep_inc[c_pos];
    }
  }

  if (set_reps != pset_reps) {
    numReps = set_reps;
    disp_num_reps(numReps);
    lcd.drawFastHLine(rep_pos[c_pos], 148, 30, GREEN);
  }
  pset_reps = set_reps;
}

 /*================================================================================*
   - END NUMBER OF REPS FUNCTION -
 *================================================================================*/

////////////////////////////////////////////////////////////////////////////////////

/*================================================================================*
   - SET UP TIMER FUNCTION -
 *================================================================================*/
void set_work_timer() {

  lcd.setFont(&fonts::Font4);
  lcd.drawString("SET WORK TIMER:", 15, 5, 1);
  lcd.drawString("DOWN", 15, 220, 1);
  lcd.drawString("UP", 155, 220, 1);
  lcd.drawString("MOVE", 255, 220, 1);
  lcd.drawString("NEXT/", 255, 190, 1);

  if (M5.BtnC.wasPressed()) {                       // move set up cursor next
    lcd.drawFastHLine(pos[c_pos], 148, 30, 0);
    c_pos++;
    if (c_pos > 3) c_pos = 0;
    lcd.drawFastHLine(pos[c_pos], 148, 30, WHITE);
  }

  /*if (M5.BtnC.wasPressed()) {                       // move set up cursor previous
    lcd.drawFastHLine(pos[c_pos], 148, 30, 0);
    c_pos--;
    if (c_pos < 0) c_pos = 3;
    lcd.drawFastHLine(pos[c_pos], 148, 30, WHITE);
  }*/

  if (M5.BtnB.wasPressed()) {                       // increase figure on cursor
    set_time += inc[c_pos];
    if (set_time > 5999) {
      set_time -= inc[c_pos];
      set_time = set_time % inc[c_pos];
    }
  }

  if (M5.BtnA.wasPressed()) {                       // decrease figure on cursor
    set_time -= inc[c_pos];
    if (set_time > 5999) {
      set_time += inc[c_pos];
      set_time = set_time % inc[c_pos];
    }
  }

  if (set_time != pset_time) {
    workMins = set_time / 60;
    workSecs = set_time % 60;
    disp_time(workMins, workSecs);
    lcd.drawFastHLine(pos[c_pos], 148, 30, WHITE);
  }
  pset_time = set_time;
}

void set_rest_timer() {

  lcd.setFont(&fonts::Font4);
  lcd.drawString("SET REST TIMER:", 15, 5, 1);
  lcd.drawString("DOWN", 15, 220, 1);
  lcd.drawString("UP", 155, 220, 1);
  lcd.drawString("MOVE", 255, 220, 1);
  lcd.drawString("NEXT/", 255, 190, 1);

  if (M5.BtnC.wasPressed()) {                       // move set up cursor next
    lcd.drawFastHLine(pos[c_pos], 148, 30, 0);
    c_pos++;
    if (c_pos > 3) c_pos = 0;
    lcd.drawFastHLine(pos[c_pos], 148, 30, WHITE);
  }

  /*if (M5.BtnC.wasPressed()) {                       // move set up cursor previous
    lcd.drawFastHLine(pos[c_pos], 148, 30, 0);
    c_pos--;
    if (c_pos < 0) c_pos = 3;
    lcd.drawFastHLine(pos[c_pos], 148, 30, WHITE);
  }*/

  if (M5.BtnB.wasPressed()) {                       // increase figure on cursor
    set_rest_time += inc[c_pos];
    if (set_rest_time > 5999) {
      set_rest_time -= inc[c_pos];
      set_rest_time = set_rest_time % inc[c_pos];
    }
  }

  if (M5.BtnA.wasPressed()) {                       // decrease figure on cursor
    set_rest_time -= inc[c_pos];
    if (set_rest_time > 5999) {
      set_rest_time += inc[c_pos];
      set_rest_time = set_rest_time % inc[c_pos];
    }
  }

  if (set_rest_time != pset_rest_time) {
    restMins = set_rest_time / 60;
    restSecs = set_rest_time % 60;
    disp_rest_time(restMins, restSecs);
    lcd.drawFastHLine(pos[c_pos], 148, 30, WHITE);
  }
  pset_rest_time = set_rest_time;
}

/*================================================================================*
   - END SET UP TIMER FUNCTION -
 *================================================================================*/

////////////////////////////////////////////////////////////////////////////////////

/*================================================================================*
   - TIMER READY FUNCTION -
 *================================================================================*/
void run_timer_ready() {

  display_rep_count();

  lcd.setFont(&fonts::Font4);
  lcd.drawString("____", 15, 220, 1);
  lcd.drawString("START", 135, 220, 1);
  lcd.drawString("____", 255, 220, 1);

  if (M5.BtnB.wasPressed()) {
    lcd.clear();
    mode = RUN_WORK_TIME;
  }
}

/*================================================================================*
   - END TIMER READY FUNCTION -
 *================================================================================*/

////////////////////////////////////////////////////////////////////////////////////

/*================================================================================*
   - RUN TIMER FUNCTION -
 *================================================================================*/
void run_work_timer() {

  display_rep_count();

  lcd.setFont(&fonts::Font4);
  lcd.drawString("____", 15, 220, 1);
  lcd.drawString("PAUSE", 135, 220, 1);
  lcd.drawString("____", 255, 220, 1);

  if (M5.BtnB.wasPressed()) {
    mode = PAUSE_WORK_TIME;
  }

  c_millis = millis();
  if (c_millis - p_millis >= 1000) {
    p_millis = c_millis;
    remainingTime--;
    if (set_time > 5999) remainingTime = 0;

    arc.clear();
    if ((360 * remainingTime) / set_time < 90) arc.fillArc(80, 80, 75, 80, 270, 270 + 360 * remainingTime / set_time, GREEN);
    else arc.fillArc(80, 80, 75, 80, 270, (360 * remainingTime) / set_time - 90, GREEN);
    arc.pushRotateZoom(160, 120, 0, 1.5, 1);

    workMins = remainingTime / 60;
    workSecs = remainingTime % 60;
    disp_time(workMins, workSecs);

  } 

  if (remainingTime <= 5) {
    if (millis() - lastBeepTime >= 1000) {
	    size_t bytes_written;
      
      M5.Axp.SetSpkEnable(true);
      InitI2SSpeakerOrMic(MODE_SPK);
  
	    i2s_write(SPEAKER_I2S_NUMBER, wavList[0], wavSize[0], &bytes_written, portMAX_DELAY);
      i2s_zero_dma_buffer(SPEAKER_I2S_NUMBER);
	  
	    // Set Mic Mode
      InitI2SSpeakerOrMic(MODE_MIC);
      M5.Axp.SetSpkEnable(false);
  
      lastBeepTime = millis();
    }
  }

  if (remainingTime < 1) {
    lastBeepTime = 0;
    
    lcd.clear();

    delay(1000);

    restMins = set_rest_time / 60;
    restSecs = set_rest_time % 60;
    disp_rest_time(restMins, restSecs);
    remainingRestTime = set_rest_time;
    mode = RUN_REST_TIME;
  }
}

void run_rest_timer() {

  display_rep_count();

  lcd.setFont(&fonts::Font4);
  lcd.drawString("____", 15, 220, 1);
  lcd.drawString("PAUSE", 135, 220, 1);
  lcd.drawString("____", 255, 220, 1);

  if (M5.BtnB.wasPressed()) {
    mode = PAUSE_REST_TIME;
  }

  x_millis = millis();
  if (x_millis - w_millis >= 1000) {
    w_millis = x_millis;
    remainingRestTime--;
    if (set_rest_time > 5999) remainingRestTime = 0;

    arc.clear();
    if ((360 * remainingRestTime) / set_rest_time < 90) arc.fillArc(80, 80, 75, 80, 270, 270 + 360 * remainingRestTime / set_rest_time, RED);
    else arc.fillArc(80, 80, 75, 80, 270, (360 * remainingRestTime) / set_rest_time - 90, RED);
    arc.pushRotateZoom(160, 120, 0, 1.5, 1);

    restMins = remainingRestTime / 60;
    restSecs = remainingRestTime % 60;
    disp_rest_time(restMins, restSecs);

  }

  if (remainingRestTime <= 5) {
    if (millis() - lastBeepTime >= 1000) {
	    size_t bytes_written;
      
      M5.Axp.SetSpkEnable(true);
      InitI2SSpeakerOrMic(MODE_SPK);
  
	    i2s_write(SPEAKER_I2S_NUMBER, wavList[0], wavSize[0], &bytes_written, portMAX_DELAY);
      i2s_zero_dma_buffer(SPEAKER_I2S_NUMBER);
	  
	    // Set Mic Mode
      InitI2SSpeakerOrMic(MODE_MIC);
      M5.Axp.SetSpkEnable(false);
  
      lastBeepTime = millis();
    }
  }

  if (remainingRestTime < 1 && remainingReps < set_reps) {
    remainingReps++;

    lastBeepTime = 0;
    
    lcd.clear();

    delay(1000);

    workMins = set_time / 60;
    workSecs = set_time % 60;
    disp_time(workMins, workSecs);
    remainingTime = set_time;
    mode = RUN_WORK_TIME;
  }

  else if (remainingRestTime < 1 && remainingReps <= set_reps) {
    lcd.clear();

    set_rest_time = 0;

    restMins = set_rest_time / 60;
    restSecs = set_rest_time % 60;
    disp_rest_time(restMins, restSecs);
    remainingRestTime = set_rest_time;

    mode = RESET_TIMERS;
  }
}

/*================================================================================*
   - END RUN TIMER FUNCTION -
 *================================================================================*/

////////////////////////////////////////////////////////////////////////////////////

/*================================================================================*
    TIMER PAUSED FUNCTION -
 *================================================================================*/
void work_timer_paused() {

  display_rep_count();

  lcd.setFont(&fonts::Font4);
  lcd.drawString("RESET", 15, 220, 1);
  lcd.drawString("RESUME", 135, 220, 1);
  lcd.drawString("____", 255, 220, 1);

  if (M5.BtnB.wasPressed()) {
    mode = RUN_WORK_TIME;
  }

  if (M5.BtnA.pressedFor(500)) {
    reset_variables();
  } 
}

void rest_timer_paused() {

  display_rep_count();

  lcd.setFont(&fonts::Font4);
  lcd.drawString("RESET", 15, 220, 1);
  lcd.drawString("RESUME", 135, 220, 1);
  lcd.drawString("____", 255, 220, 1);

  if (M5.BtnB.wasPressed()) {
    mode = RUN_REST_TIME;
  }

  if (M5.BtnA.pressedFor(500)) {
    reset_variables();
  }  
}

/*================================================================================*
   - END TIMER PAUSED FUNCTION -
 *================================================================================*/

////////////////////////////////////////////////////////////////////////////////////

/*================================================================================*
   - TIMER FINISHED FUNCTION -
 *================================================================================*/
void reset_timers() {

  display_rep_count();

  lcd.setFont(&fonts::Font4);
  lcd.drawString("____", 15, 220, 1);
  lcd.drawString("RESET", 135, 220, 1);
  lcd.drawString("____", 255, 220, 1);

  if (M5.BtnB.pressedFor(500)) {
    reset_variables();
  }
}

/*================================================================================*
   - END TIMER FINISHED FUNCTION -
 *================================================================================*/

////////////////////////////////////////////////////////////////////////////////////

/*================================================================================*
   - REP COUNT DISPLAY FUNCTION -
 *================================================================================*/
void display_rep_count() {
  lcd.setFont(&fonts::Font2);

  if (!(mode == RESET_TIMERS)) {
    lcd.setCursor (260, 15);
    lcd.printf("%d/%d", remainingReps, set_reps);
  } else {
    lcd.drawString("DONE!", 260, 15, 1);
  }
}

/*================================================================================*
   - END REP COUNT DISPLAY FUNCTION -
 *================================================================================*/

////////////////////////////////////////////////////////////////////////////////////

/*================================================================================*
   - RESET VARIABLES FUNCTION -
 *================================================================================*/
void reset_variables() {
  lcd.clear();
  ttext.clear();

  set_reps        =  1;
  pset_reps       =  0;
  set_time        = 60;
  pset_time       =  0;
  set_rest_time   = 60;
  pset_rest_time  =  0;
  c_pos           =  0;
  p_pos           =  0;
  remainingReps   =  1;
  buttonCounter   =  0;

  mode = SET_NUM_REPS;
}

 /*================================================================================*
   - END RESET VARIABLES FUNCTION -
 *================================================================================*/

////////////////////////////////////////////////////////////////////////////////////