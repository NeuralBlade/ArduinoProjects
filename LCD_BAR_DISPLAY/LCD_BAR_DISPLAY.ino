#include <LiquidCrystal.h>

#define LCD_LENGTH 8.0
#define LCD_ROWS 2

LiquidCrystal lcd(2,3,4,13,12,8,7);

double percent=100.0;
unsigned char b;
unsigned int peace;

#define POT_PIN    A3

// custom charaters

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

void setup()   {
  analogWrite(6,255);
  initLCD();
  /*
    lcd.print((char)0);
    delay(1000);
    lcd.print((char)1);
    delay(1000);    
    lcd.print((char)2);
    delay(1000);    
    lcd.print((char)3);
    delay(1000);    
    lcd.print((char)4);
    delay(1000);
    lcd.clear();   
   /**/ 
}

void loop(){

  //ADC conversion
  unsigned int value = analogRead(POT_PIN);

  printBar(map(value,0,1024,0,101));
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

void printBar(int percentage){

  double ratio = 100/(LCD_LENGTH*5);
  double numberOfBars = percentage/ratio;
  
  int blocks      = floor(numberOfBars/5);
  int remainder   = numberOfBars - (blocks*5);
  int emptyBlocks = LCD_LENGTH-ceil(numberOfBars/5); 
  
  lcd.setCursor(0, 0);  
  lcd.print(percentage);
  lcd.print("% ");
  lcd.setCursor(0, 1);    
  
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



