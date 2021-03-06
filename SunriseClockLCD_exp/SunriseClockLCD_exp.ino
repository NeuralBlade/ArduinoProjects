#define TEST_MODE


//#include <LiquidCrystal_8x2.h>
#include <LiquidCrystal.h>

#define BUTTON_PIN A1
#define ERROR_WINDOW 75  // +/- this value, for button input
#define BUTTONDELAY 20

#define NOISE_THRESHOLD    6
#define SECONDS_BUFFER     60
#define SAMPLE_TIME       100 
#define AUDIO_TRIGGER_VAL  30
#define MIN_SAMPLE_SIZE    10 * (1000/SAMPLE_TIME)

#define MODE_OFF 0
#define MODE_WAKING 1
#define MODE_ON 2
#define MODE_SLEEP 3
#define MODE_IGNORE_TRIGGER_DELAY 4
#define MODE_IGNORE_TRIGGER 5

#define STRIP1_PIN  5
#define STRIP2_PIN  10
#define STRIP3_PIN  11

#define LIGHT_PIN  A0
#define SWITCH_PIN A1
#define AUDIO_PIN  A2
#define POT_PIN    A3




/*byte brightness_1[256]={
 0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,
 4,4,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,7,7,7,7,7,7,8,8,8,8,8,8,9,9,9,9,10,10,10,10,10,11,11,11,11,12,12,12,13,13,13,13,14,14,14,15,15,15,16,16,16,17,17,18,18,18,19,19,20,20,20,21,21,22,22,23,23,
 24,24,25,26,26,27,27,28,29,29,30,31,31,32,33,33,34,35,36,36,37,38,39,40,41,42,42,43,44,45,46,47,48,50,51,52,53,54,55,57,58,59,60,62,63,64,66,67,69,70,72,74,75,77,79,80,82,84,86,88,90,91,94,
 96,98,100,102,104,107,109,111,114,116,119,122,124,127,130,133,136,139,142,145,148,151,155,158,161,165,169,172,176,180,184,188,192,196,201,205,210,214,219,224,229,234,239,244,250,255};
/**/
int br_100[100];

//#define MAX_COUNTER 768
#define MAX_COUNTER 100

int brightnessEXP = 0;

#ifdef TEST_MODE
unsigned long  DELAY_TOTAL_TIME = 2;  //minutes
unsigned long DELAY_ENABLE_TIME = 5; //in seconds
//unsigned int ledBrightnessDelay = 0;//(DELAY_TOTAL_TIME*60*1000)/MAX_COUNTER; //in miliseconds
unsigned int mode = 1;
#else
unsigned long  DELAY_TOTAL_TIME = 30;  //minutes
unsigned long DELAY_ENABLE_TIME = 2.5*60*60; //in seconds
unsigned int mode = 0;
#endif


unsigned long ledBrightnessDelay = 0;//(DELAY_TOTAL_TIME*60*1000)/MAX_COUNTER; //in miliseconds
unsigned int inputThresholdLevel = 15;
unsigned int inputThresholdTime = 20;  //in seconds
unsigned long maxAudio = 0;
unsigned int str1 = 0;
unsigned int str2 = 0;
unsigned int str3 = 0;
unsigned long count = 0;
unsigned long timeElapsed = 0;

unsigned int lightValue = 0;
int audioValue = 0;
unsigned int potValue   = 0;
unsigned int switchValue= 0;

unsigned long buttonLastChecked = 0;
unsigned long lastButtonPressedTime = 0;
int audioHistory[SECONDS_BUFFER*(1000/SAMPLE_TIME)];
unsigned int audioHistoryPointer = 0;
unsigned long lastAudioSampleTime = 0;

//8 bit: RS, RW, EN, , D4, D5, D6, D7
//RS-->PC0
//RW-->PC1
//EN-->PC2
LiquidCrystal lcd(2,3,4,13,12,8,7);

/*****************************************
 *       Filtering Code Starts Here       * 
 ******************************************/
void wait_for_tick(void) {
  unsigned long timestamp = millis();
  while(millis() == timestamp);
}

void wait_for_five_ticks(void) {
  wait_for_tick();
  wait_for_tick();
  wait_for_tick();
  wait_for_tick();
  wait_for_tick();
}

int readAnalog(int sensorPin){
  int total;
  // Filter out any 50 Hz hum by using a 4-point moving average
  // Each sample takes about 0.1ms, so we have to wait a relatively 
  // long a while between samples.
  wait_for_tick();
  total = analogRead(sensorPin); 
  wait_for_five_ticks();
  total += analogRead(sensorPin);    
  wait_for_five_ticks();
  total += analogRead(sensorPin);    
  wait_for_five_ticks();
  total += analogRead(sensorPin);    
  total  /= 4;
  return total;
}

/*****************************************
 *       Filtering Code Ends Here         * 
 ******************************************/

/*****************************************
 *       Multi Button Code Starts Here    * 
 ******************************************/
int usedVal = 0;
int buttonPushed(int pinNum) {
  int val = 0;         // variable to store the read value
  digitalWrite((14+pinNum), HIGH); // enable the 20k internal pullup
  val = analogRead(pinNum);   // read the input pin

  // we don't use the upper position because that is the same as the
  // all-open switch value when the internal 20K ohm pullup is enabled.
  //if( val >= 923 and val <= 1023 )

  if ( val >= (400-ERROR_WINDOW) and val <= (400+ERROR_WINDOW) ) { // 430
    return 2;
  }
  else if ( val >= (200-ERROR_WINDOW) and val <= (200+ERROR_WINDOW) ) { // 230
    return 1;
  }
  else{
    return 0;  // no button found to have been pushed
  }
}

/*****************************************
 *       Multi Button Code Ends Here      * 
 ******************************************/

int onModeBrightness = 0;

void setup(){

  ledBrightnessDelay = (DELAY_TOTAL_TIME*60*1000)/MAX_COUNTER; //in miliseconds
#ifdef TEST_MODE
  Serial.begin(9600); 
  Serial.write("");
  Serial.write("");
#endif

  if(MAX_COUNTER == 768){


    brightnessEXP = 115.595;
  }
  else{
    brightnessEXP = 46;
  }

  for(int i = 0; i < 255; i++){
    delay(10);  
    adjustLed(6, i); //sets medium intensity for LCD
  }
  float adjVal = 1;
  br_100[0] = 0;
  for(int i = i; i < 100;i++){

    br_100[i] = round(adjVal);
    adjVal *= 1.06941;

  }


  lcd.begin(8,2);
  lcd.clear();
#ifdef TEST_MODE
  lcd.print("TESTMODE");
  lcd.setCursor(0,1);
  lcd.print("ENABLED");
  mode=1;
#elif


  lcd.print("Starting");
  lcd.setCursor(0,1);
  lcd.print("  v0.1");
#endif
  delay(1000);
  lcd.clear();
  lcd.print(DELAY_TOTAL_TIME);
  lcd.print(" ");
  lcd.print(MAX_COUNTER);
lcd.setCursor(0,1);
lcd.print(DELAY_TOTAL_TIME*60*1000);
  delay(3000);
  lcd.clear();
  lcd.print(ledBrightnessDelay);
  delay(3000);
  lcd.clear();
  onModeBrightness = readAnalog(POT_PIN); 

  initMode(mode);



}

int lastButtonPressed = 0;

int readAudioPin(){
  int readAudioValue = analogRead(AUDIO_PIN); 
  readAudioValue = abs(readAudioValue-512)-NOISE_THRESHOLD;      
  if(readAudioValue < 0) readAudioValue = 0;
  if(readAudioValue > 250) readAudioValue = 250;
  return readAudioValue;
}

int inputOff = 0;
int lastRef = 0;

void loop(){

  int inputPin = POT_PIN;
  int newReading = analogRead(inputPin);
  int diff = (newReading -lastRef);
  
  if(abs(diff) < NOISE_THRESHOLD){ //input changing too much, cant use it
    if(diff != 0){
      diff=diff/4;
      adjustLed(STRIP1_PIN, str1+diff);        
      adjustLed(STRIP2_PIN, str2+diff);        
      adjustLed(STRIP3_PIN, str3+diff); 
    }

  } 
  lastRef = newReading;
/**/
  potValue = analogRead(POT_PIN); 
  lightValue = analogRead(LIGHT_PIN); 
  //  potValue = readAnalog(POT_PIN); 
  //  lightValue = readAnalog(LIGHT_PIN); 
  //  adjustLed(6, (potValue-(lightValue/3))/4); // need to work on adjusting LCD brightness when its dark
  adjustLed(6, map(potValue,0,1024,0,256)); // need to work on adjusting LCD brightness when its dark


  if(mode == MODE_ON){
    adjustLed(STRIP1_PIN, str1);        
    adjustLed(STRIP2_PIN, str2);        
    adjustLed(STRIP3_PIN, str3);  

    if(abs(potValue-onModeBrightness) > NOISE_THRESHOLD ){
      onModeBrightness = potValue;
      count = potValue * 0.7470703125;
      setBrightness(count);
      if(count > MAX_COUNTER){
        initMode(MODE_ON);
      }
      /**
       * if(count < 255) {
       * str1 = count;
       * str2=0;
       * str3=0;
       * } 
       * else if(count < 510){
       * str1=255;
       * str2=count-255;
       * str3=0;        
       * } 
       * else if(count < 765){
       * str1=255;
       * str2=255;
       * str3=count-510;
       * } 
       * else {
       * str1=255;
       * str2=255;
       * str3=255;
       * mode=MODE_ON;
       * count = 0;
       }/**/
    }
  } 
  else if(mode == MODE_OFF){
    if( (millis() - lastAudioSampleTime) > SAMPLE_TIME){
      setBrightness(0);  

      audioHistory[audioHistoryPointer ++] = readAudioPin();
      if(audioHistoryPointer > (SECONDS_BUFFER*(1000/SAMPLE_TIME))) audioHistoryPointer = 0;

      unsigned long averageLevel = 0;
      unsigned int levelCount = 0;
      for (int i = 0; i < SECONDS_BUFFER*(1000/SAMPLE_TIME); i++) {
        int value = audioHistory[i];

        if(value > 0){
          averageLevel += value;
          levelCount ++;
        }
      }
      averageLevel = averageLevel / (1000/SAMPLE_TIME);
      maxAudio=averageLevel;
      lastAudioSampleTime = millis();
      count = levelCount;

      if(averageLevel >= AUDIO_TRIGGER_VAL && levelCount >= MIN_SAMPLE_SIZE){
        initMode(MODE_WAKING);
      }
    }

  } 
  else if(mode == MODE_WAKING){
    //    debug();
    if(abs(millis () - timeElapsed) > ledBrightnessDelay){
      timeElapsed = millis();

      setBrightness(count++);
      if(count > MAX_COUNTER) initMode(MODE_ON);
    }
  } 
  else if(mode == MODE_SLEEP){
    if(abs(millis () - timeElapsed) > ledBrightnessDelay){
      timeElapsed = millis();
      count--;
      if(count <= 0){

        initMode(MODE_IGNORE_TRIGGER_DELAY);

      }
      else {
        setBrightness(count);
      }
    } 
  }  
  else if(mode == MODE_IGNORE_TRIGGER_DELAY){
    setBrightness(0);  
    count = (DELAY_ENABLE_TIME -(abs(millis () - timeElapsed)/1000) )   ;
    if((abs(millis () - timeElapsed)/1000) > DELAY_ENABLE_TIME){
      initMode(MODE_OFF);
    }
  }
  /**/

  //*******************  Check if buttons were pressed
  if( buttonLastChecked == 0 ) // see if this is the first time checking the buttons
    buttonLastChecked = millis()+BUTTONDELAY;  // force a check this cycle
  if( millis() - buttonLastChecked > BUTTONDELAY ) { // make sure a reasonable delay passed
    if( int buttNum = buttonPushed(BUTTON_PIN) ) {
      if(lastButtonPressed != buttNum){
        lastButtonPressed = buttNum;
        lastButtonPressedTime = millis();
      }

    } 
    else {
      if(lastButtonPressed){
        unsigned long buttonPressedTime = millis()/lastButtonPressedTime;
        buttonHandler(lastButtonPressed,buttonPressedTime);             
      }         


      lastButtonPressed = 0;
    }

    //        if(lastButtonPressed != 0){
    buttonLastChecked = millis(); // reset the lastChecked value
  }
  //*******************  END Check if buttons were pressed


  updateDisplay(); 
}

int prevBrightness =0;
int prevStr1 = 0;
int prevStr2 = 0;
int prevStr3 = 0;


void setBrightness(int brightness){

  lcd.clear();
  lcd.print(brightness);
  lcd.print(" ");
  lcd.print(br_100[brightness]);

  if(brightness > MAX_COUNTER){
    brightness = MAX_COUNTER;
  }  



  if(brightness == 0 || brightness < 0){
    adjustLed(STRIP1_PIN, 0);        
    adjustLed(STRIP2_PIN, 0);      
    adjustLed(STRIP3_PIN, 0); 
  } 
  else  {

    //    int newBrightness=brightness_1[brightness/3];
    int newBrightness=br_100[brightness]/3;
    
    int scale = brightness % 3;
    if(newBrightness < 25){
      str1 = newBrightness;
      str2 = newBrightness;
      str3 = newBrightness;    
    } 
    else {
      str1=str2=str3=newBrightness;
      if(scale == 0) {
        str2=str3=newBrightness-1;         
      } 
      else if (scale == 1){
        str3=newBrightness-1;
      } 
    }





    prevBrightness = newBrightness;
#ifdef TEST_MODE
    Serial.print(millis());
    Serial.print(",");
    Serial.print(brightness);
    Serial.print(",");    
    Serial.print(str1);    
    Serial.print(",");
    Serial.print(str2);
    Serial.print(",");
    Serial.print(str3);
    Serial.print(",");
    Serial.print(str3);
    Serial.print(",");
    Serial.println(readAnalog(LIGHT_PIN));
#endif
    adjustLed(STRIP1_PIN, str1);        
    adjustLed(STRIP2_PIN, str2);      
    adjustLed(STRIP3_PIN, str3); 
  }
  #ifdef TEST_MODE
  delay(750);
  #endif
}


void initMode(int newMode){

  for (int i = 0; i < SECONDS_BUFFER*(1000/SAMPLE_TIME); i++) {
    audioHistory[i] = 0;
  }   

  if(newMode == MODE_OFF){
    setBrightness(0);
  } 
  else if(newMode == MODE_WAKING){
    //timeElapsed = millis();
    count = 0;  
  } 
  else if(newMode == MODE_ON){
    setBrightness(MAX_COUNTER);       
  } 
  else if(newMode == MODE_SLEEP){
    count = MAX_COUNTER;
  } 
  else if(newMode == MODE_IGNORE_TRIGGER_DELAY){
    str1=0;
    str2=0;
    str3=0;                    
    //timeElapsed = millis();          
  } 
  else if(newMode == MODE_IGNORE_TRIGGER){

  }

  timeElapsed = millis();
  mode = newMode;



}



void buttonHandler(unsigned int button, unsigned long timeHeld){
  if(button == 1){
    if(mode == MODE_OFF){
      initMode(MODE_ON);

    } 
    else if(mode == MODE_ON){
      initMode(MODE_SLEEP);
    } 
    else if(mode == MODE_SLEEP){
      initMode(MODE_IGNORE_TRIGGER_DELAY);
    } 
    else if(mode == MODE_IGNORE_TRIGGER_DELAY){
      initMode(MODE_ON);
    }

  }

  if(button == 2){
    if(mode == MODE_ON){
      initMode(MODE_OFF);        
    }
  }

}

void adjustLed(int pin,int value){
  if(value > 255) value = 255;
  if(value < 0)   value = 0;
  analogWrite(pin,value);
}

unsigned long lastUpdated = 0;


void updateDisplay(){

  if(abs(millis()-lastUpdated) > 333){
    unsigned int time_in_mode = (millis() - timeElapsed) / 1000 / 60; //time in seconds)
    lcd.clear();
    //lcd.print(maxAudio);
    lcd.print(str1);
    lcd.print( " "  );
    //lcd.print(count);
lcd.print(str2);    
    lcd.print( " "  );
    lcd.print(lightValue/100-1);   
    lcd.setCursor(0,1);
    lcd.print("M");
    lcd.print(mode);
    lcd.print(" ");
    lcd.print(usedVal);
    lcd.print(" ");    
    /*
    if(time_in_mode < 100) {
      lcd.print(time_in_mode);      
    } 
    else {
      lcd.print(time_in_mode/60);      
    }/**/
    lcd.print(count);
    //lcd.print(lightValue);      
    maxAudio = 0;
    lastUpdated = millis();  
  }
}














