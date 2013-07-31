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



static unsigned char brightness_256[256]={0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,
4,4,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,7,7,7,7,7,7,8,8,8,8,8,8,9,9,9,9,10,10,10,10,10,11,11,11,11,12,12,12,13,13,13,13,14,14,14,15,15,15,16,16,16,17,17,18,18,18,19,19,20,20,20,21,21,22,22,23,23,
24,24,25,26,26,27,27,28,29,29,30,31,31,32,33,33,34,35,36,36,37,38,39,40,41,42,42,43,44,45,46,47,48,50,51,52,53,54,55,57,58,59,60,62,63,64,66,67,69,70,72,74,75,77,79,80,82,84,86,88,90,91,94,
96,98,100,102,104,107,109,111,114,116,119,122,124,127,130,133,136,139,142,145,148,151,155,158,161,165,169,172,176,180,184,188,192,196,201,205,210,214,219,224,229,234,239,244,250,255};
static unsigned char brightness_768[768]={0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,
4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,8,8,8,8,8,8,8,8,8,8,8,8,8,9,9,9,9,9,9,9,
9,9,9,9,9,10,10,10,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,13,13,13,13,13,13,13,13,14,14,14,14,14,14,14,14,15,15,15,15,15,15,15,15,16,16,16,16,16
,16,16,17,17,17,17,17,17,18,18,18,18,18,18,18,19,19,19,19,19,19,20,20,20,20,20,21,21,21,21,21,21,22,22,22,22,22,23,23,23,23,23,24,24,24,24,25,25,25,25,25,26,26,26,26,27,27,27,27,28,28,28,28
,29,29,29,29,30,30,30,30,31,31,31,31,32,32,32,33,33,33,33,34,34,34,35,35,35,36,36,36,36,37,37,37,38,38,38,39,39,39,40,40,41,41,41,42,42,42,43,43,43,44,44,45,45,45,46,46,47,47,47,48,48,49,49
,50,50,50,51,51,52,52,53,53,54,54,55,55,56,56,57,57,58,58,59,59,60,60,61,61,62,62,63,63,64,64,65,66,66,67,67,68,68,69,70,70,71,72,72,73,73,74,75,75,76,77,77,78,79,79,80,81,82,82,83,84,84,85
,86,87,87,88,89,90,90,91,92,93,94,95,95,96,97,98,99,100,100,101,102,103,104,105,106,107,108,109,110,111,112,112,113,114,115,116,117,119,120,121,122,123,124,125,126,127,128,129,130,132,133,134
,135,136,137,139,140,141,142,143,145,146,147,149,150,151,152,154,155,157,158,159,161,162,163,165,166,168,169,171,172,174,175,177,178,180,181,183,185,186,188,189,191,193,194,196,198,200,201,203
,205,207,208,210,212,214,216,218,220,221,223,225,227,229,231,233,235,237,239,242,244,246,248,250,252,254,257,259,261,263,266,268,270,273,275,278,280,282,285,287,290,292,295,297,300,303,305,308
,311,313,316,319,322,324,327,330,333,336,339,342,345,348,351,354,357,360,363,366,369,373,376,379,383,386,389,393,396,399,403,406,410,414,417,421,424,428,432,436,439,443,447,451,455,459,463,467
,471,475,479,483,488,492,496,500,505,509,514,518,523,527,532,536,541,546,550,555,560,565,570,575,580,585,590,595,600,606,611,616,621,627,632,638,643,649,655,660,666,672,678,684,690,696,702,708,714,720,726,733,739,745,752,759,768};
static unsigned char brightness_1000[1000]={0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,
3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,10,10,10,10,10,10,10,10,10,10,
10,10,10,10,11,11,11,11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,12,12,12,13,13,13,13,13,13,13,13,13,13,13,14,14,14,14,14,14,14,14,14,14,14,15,15,15,15,15,15,15,15,15,16,16,16,16,
16,16,16,16,16,17,17,17,17,17,17,17,17,17,18,18,18,18,18,18,18,18,19,19,19,19,19,19,19,19,20,20,20,20,20,20,20,21,21,21,21,21,21,21,22,22,22,22,22,22,22,23,23,23,23,23,23,24,24,24,24,24,24,25,
25,25,25,25,25,26,26,26,26,26,26,27,27,27,27,27,28,28,28,28,28,28,29,29,29,29,29,30,30,30,30,30,31,31,31,31,32,32,32,32,32,33,33,33,33,33,34,34,34,34,35,35,35,35,36,36,36,36,37,37,37,37,38,38,
38,38,39,39,39,39,40,40,40,40,41,41,41,42,42,42,42,43,43,43,44,44,44,44,45,45,45,46,46,46,47,47,47,48,48,48,49,49,49,50,50,50,51,51,51,52,52,52,53,53,53,54,54,54,55,55,55,56,56,57,57,57,58,58,
59,59,59,60,60,61,61,61,62,62,63,63,63,64,64,65,65,66,66,67,67,67,68,68,69,69,70,70,71,71,72,72,73,73,74,74,75,75,76,76,77,77,78,78,79,79,80,80,81,81,82,82,83,84,84,85,85,86,86,87,88,88,89,89,
90,91,91,92,92,93,94,94,95,95,96,97,97,98,99,99,100,101,101,102,103,103,104,105,106,106,107,108,108,109,110,111,111,112,113,114,114,115,116,117,117,118,119,120,121,121,122,123,124,125,126,126,
127,128,129,130,131,132,132,133,134,135,136,137,138,139,140,141,142,143,144,144,145,146,147,148,149,150,151,152,153,154,156,157,158,159,160,161,162,163,164,165,166,167,168,170,171,172,173,174,175,177,178,179,180,181,183,184,185,186,187,189,190,191,193,194,195,196,198,199,200,202,203,204,206,207,209,210,211,213,214,216,217,219,220,221,223,224,226,227,229,231,232,234,235,237,238,240,242,243,245,246,248,250,251,253,255,256,258,260,262,263,265,267,269,271,272,274,276,278,280,282,283,285,287,289,291,293,295,297,299,301,303,305,307,309,311,313,315,317,320,322,324,326,328,330,333,335,337,339,342,344,346,348,351,353,355,358,360,363,365,368,370,372,375,377,380,383,385,388,390,393,395,398,401,403,406,409,412,414,417,420,423,426,428,431,434,437,440,443,446,449,452,455,458,461,464,467,470,473,477,480,483,486,489,493,496,499,503,506,509,513,516,520,523,527,530,534,537,541,544,548,552,555,559,563,567,570,574,578,582,586,590,594,598,602,606,610,614,618,622,626,630,635,639,643,647,652,656,660,665,669,674,678,683,687,692,697,701,706,711,715,720,725,730,735,740,745,750,755,760,765,7768};
#define TEST_MODE

#ifdef TEST_MODE

unsigned int ledBrightnessDelay = 500; //in miliseconds
unsigned long DELAY_ENABLE_TIME = 5; //in seconds
#else
unsigned int ledBrightnessDelay = 2352; //in miliseconds
unsigned long DELAY_ENABLE_TIME = 2.6*60*60; //in seconds
#endif


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
unsigned int mode = 0;
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
  #ifdef TEST_MODE
  Serial.begin(9600); 

  #endif
  for(int i = 0; i < 255; i++){
    delay(10);  
    adjustLed(6, i); //sets medium intensity for LCD
  }

  lcd.begin();
  lcd.clear();
  lcd.print("Starting");
  lcd.setCursor(0,1);
  lcd.print("  v0.1");
  delay(1000);
  lcd.clear();
  onModeBrightness = readAnalog(POT_PIN); 

  initMode(mode);
  delay(6000);
   Serial.write("Starting");
}

int lastButtonPressed = 0;

int readAudioPin(){
  int readAudioValue = analogRead(AUDIO_PIN); 
  readAudioValue = abs(readAudioValue-512)-NOISE_THRESHOLD;      
  if(readAudioValue < 0) readAudioValue = 0;
  if(readAudioValue > 250) readAudioValue = 250;
  return readAudioValue;
}

void loop(){
    Serial.println("=======================================");  
    Serial.println("STARTING TEST #1 768 straight");
    Serial.println("=======================================");      
    int numberOfIntervals = 768;
    int number_of_minutes_for_fading = 30;
    int delay_between_intervals = (number_of_minutes_for_fading*60*1000)/numberOfIntervals;
    lcd.print(delay_between_intervals);
    for(int i = 0; i < numberOfIntervals; i++){
          delay(delay_between_intervals);
      

            int b = i / 256;
          int diff = i % 256;
          if(b == 0){ 
            str1=diff;
          }
          else if (b == 1){
            str1=255;
            str2=diff;
          } 
          else if (b == 2){
            str1=str2=255;
            str3=diff;
          } 
          else if (b == 3){
            str1=str2=str3=255;
          }           
      
          Serial.print(millis());
          Serial.print(",");
          Serial.print(i);
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
          adjustLed(STRIP1_PIN, str1);        
          adjustLed(STRIP2_PIN, str2);        
          adjustLed(STRIP3_PIN, str3);     
    }
    Serial.println("=======================================");  
    Serial.println("STARTING TEST #2 - 256 Parrallel");
    Serial.println("=======================================");  
    
    numberOfIntervals = 256;
    number_of_minutes_for_fading = 30;
    delay_between_intervals = (number_of_minutes_for_fading*60*1000)/numberOfIntervals;
    
    for(int i = 0; i < numberOfIntervals; i++){
          delay(delay_between_intervals);
      
          str1=str2=str3=brightness_256[i];
          Serial.print(millis());
          Serial.print(",");
          Serial.print(i);
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
          adjustLed(STRIP1_PIN, str1);        
          adjustLed(STRIP2_PIN, str2);        
          adjustLed(STRIP3_PIN, str3);     
    }
    Serial.println("=======================================");  
    Serial.println("STARTING TEST #3a 1000 Serial");
    Serial.println("=======================================");  
      numberOfIntervals = 1000;
      number_of_minutes_for_fading = 30;
      delay_between_intervals = (number_of_minutes_for_fading*60*1000)/numberOfIntervals;
      
      for(int i = 0; i < numberOfIntervals; i++){
          delay(delay_between_intervals);      
          int brightness  = brightness_1000[i];
            int b = brightness / 256;
            
            int diff = brightness % 256;
            if(b == 0){ 
              str1=diff;
            }
            else if (b == 1){
              str1=255;
              str2=diff;
            } 
            else if (b == 2){
              str1=str2=255;
              str3=diff;
            } 
            else if (b == 3){
              str1=str2=str3=255;
            } 
          Serial.print(millis());
          Serial.print(",");
          Serial.print(i);
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
          adjustLed(STRIP1_PIN, str1);        
          adjustLed(STRIP2_PIN, str2);        
          adjustLed(STRIP3_PIN, str3);     
    }    
    Serial.println("=======================================");  
    Serial.println("STARTING TEST #3b 1000 Parrallel Inc");
    Serial.println("=======================================");  
      numberOfIntervals = 1000;
      number_of_minutes_for_fading = 30;
      delay_between_intervals = (number_of_minutes_for_fading*60*1000)/numberOfIntervals;
      
      for(int i = 0; i < numberOfIntervals; i++){
          delay(delay_between_intervals);      
          int brightness  = brightness_1000[i];
            int b = brightness / 3;
            int diff = brightness % 3;
            str1=str2=str3=b;
             if (b == 1){
              str1++;

            } 
            else if (b == 2){
              str1++;
              str2++;
            } 
          Serial.print(millis());
          Serial.print(",");
          Serial.print(i);
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
          adjustLed(STRIP1_PIN, str1);        
          adjustLed(STRIP2_PIN, str2);        
          adjustLed(STRIP3_PIN, str3);     
    }     
}

void setBrightness(int brightness){


/*
  int b = brightness / 3;
  int diff = brightness % 3;
  str1=str2=str3=b;
  /**/
  /*
    int b = brightness / 256;
  int diff = brightness % 256;
  if(b == 0){ 
    str1=diff;
  }
  else if (b == 1){
    str1=255;
    str2=diff;
  } 
  else if (b == 2){
    str1=str2=255;
    str3=diff;
  } 
  else if (b == 3){
    str1=str2=str3=255;
  } 
  /**/
  int average = brightness / 3;
   int mod = brightness % 3;
   str1=str2=str3=average;
   if(mod == 1){
   str1++;
   }
   else if (mod == 2){
   str1++;
   str2++;
   }/**/
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


void initMode(int newMode){

  for (int i = 0; i < SECONDS_BUFFER*(1000/SAMPLE_TIME); i++) {
    audioHistory[0] = 0;
  }   

  if(newMode == MODE_OFF){
    setBrightness(0);
  } 
  else if(newMode == MODE_WAKING){
    //timeElapsed = millis();
    count = 0;  
  } 
  else if(newMode == MODE_ON){
    setBrightness(1000);       
  } 
  else if(newMode == MODE_SLEEP){
    count = 765;
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
 // analogWrite(pin,exp_map[value]);
  analogWrite(pin,value);
}

unsigned long lastUpdated = 0;


void updateDisplay(){

  if(abs(millis()-lastUpdated) > 333){
    unsigned int time_in_mode = (millis() - timeElapsed) / 1000 / 60; //time in seconds)
    lcd.clear();
    lcd.print(maxAudio);
    lcd.print( " "  );
    lcd.print(count);    
    lcd.print( " "  );
    lcd.print(lightValue/10);   
    lcd.setCursor(0,1);
    lcd.print("M");
    lcd.print(mode);
    lcd.print(" ");
    lcd.print(usedVal);
    lcd.print(" ");    
    if(time_in_mode < 100) {
      lcd.print(time_in_mode);      
    } 
    else {
      lcd.print(time_in_mode/60);      
    }
    //lcd.print(lightValue);      
    maxAudio = 0;
    lastUpdated = millis();  
  }
}





