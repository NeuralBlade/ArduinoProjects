//#define TEST_MODE

#define LCD_LENGTH 8.0
#define LCD_ROWS 2

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



byte brightness_1[256]={
  0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,
  4,4,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,7,7,7,7,7,7,8,8,8,8,8,8,9,9,9,9,10,10,10,10,10,11,11,11,11,12,12,12,13,13,13,13,14,14,14,15,15,15,16,16,16,17,17,18,18,18,19,19,20,20,20,21,21,22,22,23,23,
  24,24,25,26,26,27,27,28,29,29,30,31,31,32,33,33,34,35,36,36,37,38,39,40,41,42,42,43,44,45,46,47,48,50,51,52,53,54,55,57,58,59,60,62,63,64,66,67,69,70,72,74,75,77,79,80,82,84,86,88,90,91,94,
  96,98,100,102,104,107,109,111,114,116,119,122,124,127,130,133,136,139,142,145,148,151,155,158,161,165,169,172,176,180,184,188,192,196,201,205,210,214,219,224,229,234,239,244,250,255};

byte p1[8]={
  0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10};
byte p2[8]={
  0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18};
byte p3[8]={
  0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C};
byte p4[8]={
  0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E};
byte p5[8]={
  0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F};

#define MAX_COUNTER 768


#ifdef TEST_MODE
unsigned long  DELAY_TOTAL_TIME = 1;  //seconds
unsigned long DELAY_ENABLE_TIME = 5; //in seconds
//unsigned int ledBrightnessDelay = 0;//(DELAY_TOTAL_TIME*60*1000)/MAX_COUNTER; //in miliseconds
unsigned int mode = 1;
#else
unsigned long  DELAY_TOTAL_TIME = 30*60;  //minutes
unsigned long DELAY_ENABLE_TIME = 2.5*60*60; //in seconds
unsigned int mode = 0;
#endif


unsigned long button2Hold = 0;

unsigned long ledBrightnessDelay = (DELAY_TOTAL_TIME*1000)/MAX_COUNTER; //in miliseconds
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
 /*
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
/**/
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
int prevPotValue;
int onModeBrightness = 0;
float brightnessEXP = 0;
int backlit = 30;
void setup(){


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



  initLCD();
#ifdef TEST_MODE
  lcd.print("TESTMODE");
  lcd.setCursor(0,1);
  lcd.print("ENABLED");
  mode=4;

#else
  lcd.print("Starting");
  lcd.setCursor(0,1);
  lcd.print("  v0.1.2");
  for(int i = 150; i >= 0; i--){
    delay(10);  
    adjustLed(6, i); //sets medium intensity for LCD
  }  
  for(int i = 0; i >= 150; i--){
    delay(10);  
    adjustLed(6, i); //sets medium intensity for LCD
  }  
#endif
  delay(1000);
  lcd.print(ledBrightnessDelay);
  initMode(mode);
  prevPotValue = onModeBrightness;
  adjustLed(6, backlit);
} //***************   END OF SETUP METHOD

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
unsigned long lastAdjusted = 0;
int adjustedBrightness = 0;

void adjustBacklit(int backVal, int lightVal){
  /*  
   int adj = map(lightVal,600,1024,0,150);
   if((backVal - adj) < 10) backVal = 10;
   else backVal-= adj;
  /**/
  adjustedBrightness = backVal;
  adjustLed(6, backVal);
}
/**/
void loop(){
  potValue = analogRead(POT_PIN); 
  lightValue = analogRead(LIGHT_PIN); 


  if(abs(millis()-button2Hold) > 200 && button2Hold > 0){
    backlit = analogRead(POT_PIN)/4;
    adjustBacklit(backlit,map(lightValue,500,1024,0,150));
  }  

  if(abs(millis()-lastAdjusted) > 200){
    adjustBacklit(backlit,map(lightValue,500,1024,0,150));
    lastAdjusted = millis();
  }

  /*
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
  //  potValue = readAnalog(POT_PIN); 
  //  lightValue = readAnalog(LIGHT_PIN); 
  //  adjustLed(6, (potValue-(lightValue/3))/4); // need to work on adjusting LCD brightness when its dark


  //adjustLed(6, map(potValue,0,1024,0,256)); // need to work on adjusting LCD brightness when its dark


  if(mode == MODE_ON){
    if(count == MAX_COUNTER){
      if(abs(analogRead(POT_PIN)-onModeBrightness) > 50 ){
        count=analogRead(POT_PIN);
      } 
      else{
        setBrightness(count-25); 
      }
    } 
    else {    
      onModeBrightness = analogRead(POT_PIN);
      count = potValue * 0.7470703125;
      setBrightness(count);
      lcd.setCursor(0,0);
      lcd.print("ON-Adjst");
      //delay(250);
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
    if(abs(onModeBrightness-analogRead(POT_PIN)) > 50){
      onModeBrightness = analogRead(POT_PIN);
      count = potValue * 0.7470703125;
    }
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
      if(buttNum == 2){
        button2Hold = lastButtonPressedTime;
      }
    }  
    else {
      if(lastButtonPressed){
        unsigned long buttonPressedTime = millis()/lastButtonPressedTime;
        buttonHandler(lastButtonPressed,buttonPressedTime);             
      }         


      lastButtonPressed = 0;
      button2Hold = 0;
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
  if(brightness > MAX_COUNTER){
    brightness = MAX_COUNTER;
  }  

  if(brightness <= 0 | 0){
    adjustLed(STRIP1_PIN, 0);        
    adjustLed(STRIP2_PIN, 0);      
    adjustLed(STRIP3_PIN, 0); 
  } 
  else  {
    int newBrightness=brightness_1[brightness/3];
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

}


void initMode(int newMode){
  onModeBrightness = analogRead(POT_PIN); 
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
    count = MAX_COUNTER;    
    setBrightness(count-10); 
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

  if(abs(millis()-lastUpdated) > 250){

    unsigned int time_in_mode = (millis() - timeElapsed) / 1000 / 60; //time in seconds)
    if(mode ==3){
      lcd.setCursor(0,0);
      lcd.print("M");
      lcd.print(mode);
      lcd.print(" ");
      //lcd.print(ledBrightnessDelay*count/1000);
      print_time(ledBrightnessDelay*count);
      lcd.print("    ");      
      printBar(map(count,0,MAX_COUNTER,0,100),1);
    } 
    else if(mode == 4){
      lcd.setCursor(0,0);
      lcd.print("M");
      lcd.print(mode);
      lcd.print(" ");
      //lcd.print(ledBrightnessDelay*count/1000);
      print_time(ledBrightnessDelay*count);
      lcd.print("    ");      
      printBar(map(count,DELAY_ENABLE_TIME*1000,0,0,100),1);
    } 
    else if(mode == 1){
      lcd.setCursor(0,0);
      lcd.print("M");
      lcd.print(mode);
      lcd.print(" ");
      print_time(ledBrightnessDelay*MAX_COUNTER-ledBrightnessDelay*count);

      printBar(map(count,0,MAX_COUNTER,100,0),1);
    } 
    else     if(mode == 2){
      lcd.setCursor(0,0);
      lcd.print("M");
      lcd.print(mode);
      lcd.print(" ");
      //      lcd.print(ledBrightnessDelay);
      lcd.print(count);
      lcd.print("    ");      
      printBar(map(count,0,MAX_COUNTER,0,100),1);
    } 
    else{


      lcd.clear();
      lcd.print(maxAudio);
      lcd.print( " "  );
      lcd.print(count);    
      lcd.print( " "  );
      lcd.print((lightValue/10)-25);   
      lcd.setCursor(0,1);
      lcd.print("M");
      lcd.print(mode);
      lcd.print(" ");
      lcd.print(adjustedBrightness);
      lcd.print(" ");    
      if(time_in_mode < 100) {
        lcd.print(time_in_mode);      
      } 
      else {
        lcd.print(time_in_mode/60);      
      }
    }
    /**/
    //lcd.print(lightValue);      
    maxAudio = 0;
    lastUpdated = millis();  
  }
}


void initLCD(){
  delay(100);
  lcd.createChar(0, p1);
  lcd.createChar(1, p2);
  lcd.createChar(2, p3);
  lcd.createChar(3, p4);
  lcd.createChar(4, p5);

  lcd.begin(LCD_LENGTH, LCD_ROWS);



}

void print_time(unsigned long t_milli)
{
  char buffer[20];
  int days, hours, mins, secs;
  int fractime;
  unsigned long inttime;

  inttime  = t_milli / 1000;
  fractime = t_milli % 1000;
  // inttime is the total number of number of seconds
  // fractimeis the number of thousandths of a second

  // number of days is total number of seconds divided by 24 divided by 3600
  days     = inttime / (24*3600);
  inttime  = inttime % (24*3600);

  // Now, inttime is the remainder after subtracting the number of seconds
  // in the number of days
  hours    = inttime / 3600;
  inttime  = inttime % 3600;

  // Now, inttime is the remainder after subtracting the number of seconds
  // in the number of days and hours
  mins     = inttime / 60;
  inttime  = inttime % 60;

  // Now inttime is the number of seconds left after subtracting the number
  // in the number of days, hours and minutes. In other words, it is the
  // number of seconds.
  secs = inttime;
  if(hours > 0){
    // Don't bother to print days
    sprintf(buffer, "%02dh%02d", hours, mins);
  }  else {
    sprintf(buffer, "%02dm%02d",mins, secs);
  }
  lcd.print(buffer);
}

void printBar(int percentage,int row){

  double ratio = 100/(LCD_LENGTH*5);
  double numberOfBars = percentage/ratio;

  int blocks      = floor(numberOfBars/5);
  int remainder   = numberOfBars - (blocks*5);
  int emptyBlocks = LCD_LENGTH-ceil(numberOfBars/5); 


  lcd.setCursor(0, row);    

  for(int i = 0; i < blocks;i++){
    lcd.print((char)4);
  }

  if(remainder > 0){
    // drawing charater's colums
    switch (remainder) {
    case 0:
      break;
    case 1:
      lcd.print((char)0);
      break;
    case 2:
      lcd.print((char)1);
      break;
    case 3:
      lcd.print((char)2);
      break;
    case 4:
      lcd.print((char)3);
      break;
    }
  }
  for(int i = 0; i < emptyBlocks;i++) lcd.print(" ");

}


















