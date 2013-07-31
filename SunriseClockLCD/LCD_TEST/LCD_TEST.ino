// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(2,3,4,13,12,8,7);

float d = 0;
float oldit = 0;
int analogPin = 3; // The main input
/*
void setup() {
  analogWrite(6,150);
  // set up the LCD's number of columns and rows: 
  lcd.begin(8, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis()/1000);
}

/**/



void setup() {
  analogWrite(6,150);
  lcd.begin(8, 2); 
  lcd.setCursor(8, 0); // show what pin we're using for input
  lcd.println("Analog "); // delete this and use whatever you want
  lcd.setCursor(15, 0); // for the top row caption
  lcd.print(analogPin);
}

void loop() {
  float num = analogRead(analogPin); // get our input
  float it = map(num, 0, 1023, 0, 16);  // take our input and break it down to 16
  lcd.setCursor(0, 0); // print the actual number we're using.
  lcd.print(num);

  if (it > oldit) { // check if the number changes to a higher number
    for (d = 0; it >= d; d++) { // count up from 0 to 15
      lcd.setCursor(d, 1); // start at the bottom left and work forward
      lcd.write(1023); // show a block
    }
  }

  if (it <= oldit) { // check if the number changed to a smaller number
    for (d = 15; it <= d; d--) { //count down from 15 to 0
      lcd.setCursor(d, 1); // start at the bottom right and work back
      lcd.write(1022); // show blank
    }
  }

  oldit = it;  
}
/**/
