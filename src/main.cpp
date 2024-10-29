// Motor Controller for Loop Antenna
// K3DIY - Sam Bell

//  Begin General Init //
int debounce=150;

#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include <LiquidCrystal.h>
#include "global.h"

//  Serial.begin(9600);


/* Added a library to work with AA-30 ZERO analyzer */

// for Zero
#define RX_PIN 2
#define TX_PIN 3
// for mega
//#define RX_PIN 12
//#define TX_PIN 13



// Establish global variables for printing frequency and SWR //
char SWRstr[30];
char FRQstr[30];
long ScanFrequency = 0;
float SWR = 999;
int ScanDelay = 10;


// End General Init //


// Begin Motor Init
unsigned turnsCounter = 0;
int step_size = 10;
String step_size_string;
String direction_str;
bool direction = 0; //0 is down, 1 is up
int speed = 500;
// End Motor Init

String menuItems[] = {"Zero", "Manual", "Presets", "Dump"};


unsigned TotalTurns = 35000;
int AntennaMemoryCount = 35;
unsigned AntennaMemory[36][2];
int NumberTurnsPerMemory = 1000;
long ScanWidth = 1000000;

float InterpolatedFrequency = 0.000; // Interpolated



int i;
int x;


// Navigation button variables
int readKey;
int savedDistance = 0;

// Menu control variables
int menuPage = 0;
int maxMenuPages = round(((sizeof(menuItems) / sizeof(String)) / 2) + .5);
int cursorPosition = 0;

// Creates 3 custom characters for the menu display
byte downArrow[8] = {
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b10101, // * * *
  0b01110, //  ***
  0b00100  //   *
};

byte upArrow[8] = {
  0b00100, //   *
  0b01110, //  ***
  0b10101, // * * *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100  //   *
};

byte menuCursor[8] = {
  B01000, //  *
  B00100, //   *
  B00010, //    *
  B00001, //     *
  B00010, //    *
  B00100, //   *
  B01000, //  *
  B00000  //
};

// Begin Display Init //
// Setting the LCD shields pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);


void setup() {
//  Initializes serial communication
//  Serial.begin(9600);
//  Serial3.end();

  // ZERO.startZero();
  delay(50);

  // Initializes and clears the LCD screen
  lcd.begin(16, 2);
  lcd.clear();

  // populate strings
  step_size_string.reserve(7);
  step_size_string = "10     ";

  direction_str.reserve(4);
  direction_str = "";
    
  
  // Creates the byte for the 3 custom characters
  lcd.createChar(0, menuCursor);
  lcd.createChar(1, upArrow);
  lcd.createChar(2, downArrow);
  
}

//  Draws the turns counter in the upper right hand corner of the display.  Passing true erases the counter prior to writing it.
void drawCounter(bool reset_flag) {

    if (reset_flag == 0)
    {
      lcd.setCursor(10, 0); // Set cursor to the top line
      lcd.print(turnsCounter);
    }
    else
    {
      lcd.setCursor(10, 0); // Set cursor to the top line
      lcd.print(F("      "));
      lcd.setCursor(10, 0); // Set cursor to the top line
      lcd.print(turnsCounter);
    };
}

//  Draws the frequency in the upper left hand corner of the display.  Passing true erases the frequency prior to writing it.
void drawFrequency(bool reset_flag) {

    
    if (reset_flag == 0)
    {
      lcd.setCursor(0, 0); // Set cursor to the top line
      lcd.print(InterpolatedFrequency);
    }
    else
    {
      lcd.setCursor(0, 0); // Set cursor to the top line
      lcd.print(F("      "));
      lcd.setCursor(0, 0); // Set cursor to the top line
      lcd.print(InterpolatedFrequency,3);
    };
}

//  Draws the frequency in the upper left hand corner of the display.  Passing true erases the frequency prior to writing it.
void drawPreset(int choice) {
  
      lcd.setCursor(0, 1); // Set cursor to the bottom line
      lcd.print(F("      "));
      lcd.setCursor(0, 1); // Set cursor to the bottom line
      lcd.print(float((AntennaMemory[choice][2]))/1000,3);
   
}

//  Draws the estimated frequency in the upper left hand corner of the display.
void setFrequency() {


// Variables from SetFrequency
int minTurns;
int maxTurns;
int minFreq;
int maxFreq;
int gapTurns;
int gapFreq;
float gapRatio;
int deltaTurns;
float deltaFreq;

i =0;
minTurns = (AntennaMemory[0][1]);
maxTurns = (AntennaMemory[AntennaMemoryCount][1]);
minFreq = (AntennaMemory[0][2]);
maxFreq = (AntennaMemory[AntennaMemoryCount][2]);
gapTurns = 0;
gapFreq = 0;
gapRatio = 0.0;
deltaTurns = 0;
deltaFreq = 0;

   for (i=0; i<AntennaMemoryCount; i++)
   {    
    if (turnsCounter >= (AntennaMemory[i][1]))
    {
      if (turnsCounter <= (AntennaMemory[i+1][1]))
      {
        minTurns = (AntennaMemory[i][1]);
        maxTurns = (AntennaMemory[i+1][1]);
        minFreq = (AntennaMemory[i][2]);
        maxFreq = (AntennaMemory[i+1][2]);
        break;
      }
    }
   }

   gapTurns = maxTurns-minTurns;
   gapFreq = maxFreq-minFreq;
   gapRatio = float(gapFreq)/float(gapTurns);
   deltaTurns = turnsCounter - minTurns;   
   deltaFreq = float(deltaTurns) * float(gapRatio);
   InterpolatedFrequency = (float(minFreq) + deltaFreq)/1000;
   drawFrequency(true);  
}


void menuItem1() { // Zero Function - executes when you select the 1st item from main menu
  int activeButton = 0;
  char choice = 'N';

  lcd.clear();
  drawCounter(true);
  
  // Take a scan and provide results
  Scan(10000000,12000000);
  lcd.setCursor(0, 0);
  PrintFRQ();
  lcd.setCursor(0, 1);
  PrintSWR();
  delay(2000);

  // Ask if set
  lcd.setCursor(0, 0);
  lcd.print(F("Set Zero?       "));
  lcd.setCursor(10, 0);
  lcd.print(choice);

  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(debounce);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);

    switch (button) {
      case 1:  // This case will execute if the "right" button is pressed
        button = 0;
        if(choice == 'Y')
        {
          turnsCounter = 0;  // Reset the motor turns counter
          activeButton = 0;
  
          AntennaMemory[0][2]= ScanFrequency/1000;
          AntennaMemory[0][1]= 0;  // store zero for turns
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(AntennaMemory[0][1]);
            lcd.setCursor(0, 1);
            lcd.print(AntennaMemory[0][2]);
            delay(2000);
            
          // and loop for the rest...
          for (i=1; i<AntennaMemoryCount; i++)
          {
            TurnMotor(NumberTurnsPerMemory,0);
            turnsCounter = turnsCounter + NumberTurnsPerMemory;
            Scan(ScanFrequency-1000000,ScanFrequency + 1000000);
            AntennaMemory[i][2]= ScanFrequency/1000;
            AntennaMemory[i][1]= turnsCounter;  // store a value every NumberTurnsPerMemory pulses
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(AntennaMemory[i][1]);
            lcd.setCursor(0, 1);
            lcd.print(AntennaMemory[i][2]);
           }
          
          // echo back the array
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(F("Dumping array"));
            delay(2000);
            lcd.clear();
            lcd.setCursor(0, 0);
          
          for (i=0; i<AntennaMemoryCount; i++)
          {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(F("*"));
            lcd.print(i);
            lcd.print(F("*"));
            lcd.print(AntennaMemory[i][1]);
            lcd.setCursor(0, 1);
            lcd.print(AntennaMemory[i][2]);
            delay(1500);
           }




          
          //drawCounter(true);
          break;
        }
        else
        {
          activeButton = 0;
          drawCounter(true);
          break;
        };           
      case 2:  // This case will execute if the "up" button is pressed
        button = 0;
        choice = 'Y';
        lcd.setCursor(10, 0);
        lcd.print(choice);
        activeButton = 0;
        break;
      case 3:  // This case will execute if the "down" button is pressed
        button = 0;
        choice = 'N';
        lcd.setCursor(10, 0);
        lcd.print(choice);
        activeButton = 0;
        break;
      case 5:  // This case will execute if the "select (back)" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem2() { // Manual Tune - Function executes when you select the 2nd item from main menu
  int activeButton = 0;
  char choice = 'N';

  lcd.clear();
  drawCounter(true);
  ManualDisplay(direction_str);
  setFrequency();
  
  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(debounce);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);

    switch (button) {
      case 1:  // This case will execute if the "right" button is pressed
        button = 0;
        activeButton = 0; 
        direction_str = F("Up  ");
        direction = 0; //1 is tune down, 0 is up     
        turnsCounter = turnsCounter - step_size;
        drawCounter(true);
        ManualDisplay(direction_str);
        TurnMotor (step_size,1); // 1 is up
        setFrequency();
     break;
               
      case 2:  // This case will execute if the "up" button is pressed
        button = 0;
        activeButton = 0;    
        switch(step_size)
           {
            case 10:  {step_size=100;step_size_string=F("100    ");break;}
            case 100: {step_size=1000;step_size_string=F("1000   ");break;}
            case 1000: {step_size=10000;step_size_string=F("10000  ");break;}
            case 10000: {step_size=10;step_size_string=F("10     ");break;}
           };
        ManualDisplay(direction_str);
        break;
      case 3:  // This case will execute if the "down" button is pressed
        button = 0;
        activeButton = 0;
        switch(step_size)
          {
            case 10000: {step_size=1000;step_size_string=F("1000   ");break;}
            case 1000:  {step_size=100;step_size_string=F("100    ");break;}
            case 100: {step_size=10;step_size_string=F("10     ");break;}
            case 10: {step_size=10000 ;step_size_string=F("10000   ");break;}
           };  
          ManualDisplay(direction_str);
          break;
      
      case 4:  // This case will execute if the "left" button is pressed      
        button = 0;
        activeButton = 0;
        direction_str = F("Down");
        direction = 1; //1 is tune down, 0 is up
        turnsCounter = turnsCounter + step_size;
        drawCounter(true);
        ManualDisplay(direction_str);
        TurnMotor (step_size,0);  //0 is down  

        setFrequency();
        
        break;
        
      case 5:  // This case will execute if the "select (back)" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem3() { // Presets - Function executes when you select the 3rd item from main menu

  int activeButton = 0;
  int row = 0;
  int button;
  int choice = 0;
  
  lcd.clear();
  lcd.setCursor(0, 0);
  
  drawCounter(true);
  setFrequency();
  drawPreset(choice);
  

//  lcd.print("Presets");

  while (activeButton == 0) {
   
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(debounce);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 1:  // This case will execute if the "right" button is pressed
        button = 0;
        if (turnsCounter < (AntennaMemory[choice][1]))
        {
          TurnMotor ((AntennaMemory[choice][1])-turnsCounter, 0);
          turnsCounter = (AntennaMemory[choice][1]);            
        }
        else
        {
          TurnMotor (turnsCounter-(AntennaMemory[choice][1]), 1);
          turnsCounter = (AntennaMemory[choice][1]);
        }  
        setFrequency();
        drawCounter(true);
        drawPreset(choice);
        break;
                
      case 2:  // This case will execute if the "up" button is pressed
        button = 0;
        if (choice != 0)
          {choice = choice-1;}
        drawPreset(choice);
        activeButton = 0;
        break;
      case 3:  // This case will execute if the "down" button is pressed
        button = 0;
        if (choice != AntennaMemoryCount-1)
         {choice = choice +1;}
        drawPreset(choice);
        activeButton = 0;
        break;
      case 5:  // This case will execute if the "select (back)" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}

void menuItem4() { // Function executes when you select the 4th item from main menu
  int activeButton = 0;
  drawCounter(true);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Array Dump"));

          // echo back the array
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(F("Dumping array"));
            delay(2000);
            lcd.clear();
            lcd.setCursor(0, 0);
          
          for (i=0; i<AntennaMemoryCount; i++)
          {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(F("*"));
            lcd.print(i);
            lcd.print(F("*"));
            lcd.print(AntennaMemory[i][1]);
            lcd.setCursor(0, 1);
            lcd.print(AntennaMemory[i][2]);
            delay(1500);
           }


  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(debounce);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    drawCounter(false);
    switch (button) {
      case 5:  // This case will execute if the select (back)" button is pressed
        button = 0;
        activeButton = 1;
        break;
    }
  }
}



// This function will generate the 2 menu items that can fit on the screen. They will change as you scroll through your menu. Up and down arrows will indicate your current menu position.
void mainMenuDraw() {
//  Serial.print(menuPage);
  lcd.clear();
  drawCounter(false);
  lcd.setCursor(1, 0);
  lcd.print(menuItems[menuPage]);
  lcd.setCursor(1, 1);
  lcd.print(menuItems[menuPage + 1]);
  if (menuPage == 0) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
  } else if (menuPage > 0 and menuPage < maxMenuPages) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  } else if (menuPage == maxMenuPages) {
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  }
}

// When called, this function will erase the current cursor and redraw it based on the cursorPosition and menuPage variables.
void drawCursor() {
  for (x = 0; x < 2; x++) {     // Erases current cursor
    lcd.setCursor(0, x);
    lcd.print(" ");
  }

  // The menu is set up to be progressive (menuPage 0 = Item 1 & Item 2, menuPage 1 = Item 2 & Item 3, menuPage 2 = Item 3 & Item 4), so
  // in order to determine where the cursor should be you need to see if you are at an odd or even menu page and an odd or even cursor position.
  if (menuPage % 2 == 0) {
    if (cursorPosition % 2 == 0) {  // If the menu page is even and the cursor position is even that means the cursor should be on line 1
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
    }
    if (cursorPosition % 2 != 0) {  // If the menu page is even and the cursor position is odd that means the cursor should be on line 2
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
    }
  }
  if (menuPage % 2 != 0) {
    if (cursorPosition % 2 == 0) {  // If the menu page is odd and the cursor position is even that means the cursor should be on line 2
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
    }
    if (cursorPosition % 2 != 0) {  // If the menu page is odd and the cursor position is odd that means the cursor should be on line 1
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
    }
  }
}


void operateMainMenu() {
  int activeButton = 0;
  while (activeButton == 0) {
    int button;
    readKey = analogRead(0);
    if (readKey < 790) {
      delay(debounce);
      readKey = analogRead(0);
    }
    button = evaluateButton(readKey);
    switch (button) {
      case 0: // When button returns as 0 there is no action taken
        break;
      case 1:  // This case will execute if the "forward" button is pressed
        button = 0;
        switch (cursorPosition) { // The case that is selected here is dependent on which menu page you are on and where the cursor is.
          case 0:
            menuItem1();
            break;
          case 1:
            menuItem2();
            break;
          case 2:
            menuItem3();
            break;
          case 3:
            menuItem4();
            break;

        }
        activeButton = 1;
        mainMenuDraw();
        drawCursor();
        break;
      case 2:
        button = 0;
        if (menuPage == 0) {
          cursorPosition = cursorPosition - 1;
          cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));
        }
        if (menuPage % 2 == 0 and cursorPosition % 2 == 0) {
          menuPage = menuPage - 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        if (menuPage % 2 != 0 and cursorPosition % 2 != 0) {
          menuPage = menuPage - 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        cursorPosition = cursorPosition - 1;
        cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));

        mainMenuDraw();
        drawCursor();
        activeButton = 1;
        break;
      case 3:
        button = 0;
        if (menuPage % 2 == 0 and cursorPosition % 2 != 0) {
          menuPage = menuPage + 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        if (menuPage % 2 != 0 and cursorPosition % 2 == 0) {
          menuPage = menuPage + 1;
          menuPage = constrain(menuPage, 0, maxMenuPages);
        }

        cursorPosition = cursorPosition + 1;
        cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));
        mainMenuDraw();
        drawCursor();
        activeButton = 1;
        break;
    }
  }
}

// This function is called whenever a button press is evaluated. The LCD shield works by observing a voltage drop across the buttons all hooked up to A0.
int evaluateButton(int x) {
  int result = 0;
  if (x < 50) {
    result = 1; // right
  } else if (x < 195) {
    result = 2; // up
  } else if (x < 380) {
    result = 3; // down
  } else if (x < 555) {
    result = 4; // left
  } else if (x < 790) {
    result = 5; // select
  }
  return result;
}

void TurnMotor (int step_size, int direction)
{
  // Begin Motor Init //
  // Create the motor shield object with the default I2C address
  Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
  // Connect a stepper motor with 200 steps per revolution (1.8 degree)
  // to motor port #2 (M3 and M4)
  Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 2);

  
  ;
// End Motor Init //

 // Initialize Motor Control Card
  AFMS.begin();  // create with the default frequency 1.6KHz
  //AFMS.begin(1000);  // OR with a different frequency, say 1KHz

  // Set motor speed
  myMotor->setSpeed(speed);

  switch (direction)
  {
  case 0:
   {
    myMotor->step(step_size, BACKWARD, DOUBLE);
    break;
   }
  case 1:
   {
    myMotor->step(step_size, FORWARD, DOUBLE);
    break;
   }
  }
  myMotor->release();
}

int ManualDisplay (String direction)
{
     
     lcd.setCursor(0,1);            // move to the begining of the second line
     lcd.print("                ");
     lcd.setCursor(0,1);            // move to the begining of the second line
     lcd.print(step_size_string);
     lcd.print(" ");
     lcd.print(direction);
}


void Scan(long Start, long Stop)
{
// Variables from Scan
float SWRmin=100000;
long Fmin=100000000;
long f=0;
  
SWRmin=100000;
Fmin=100000000;
 
f=0;
// Slice 1
    for (f=Start; f<Stop; f=f+100000)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(f);
      // ZERO.startMeasure(f);             // start measurement
      delay(ScanDelay);
      // SWR = ZERO.getSWR();            // get SWR value
      if (SWR < SWRmin)
      {
        SWRmin = SWR;
        Fmin = f;
      }
    }

// Slice 2
    for (f=Fmin-100000; f<Fmin+100000; f=f+10000)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(f);
      // ZERO.startMeasure(f);             // start measurement
      delay(ScanDelay);
      // SWR = ZERO.getSWR();            // get SWR value
      if (SWR < SWRmin)
      {
        SWRmin = SWR;
        Fmin = f;
      }
    }

// Slice 3
    for (f=Fmin-10000; f<Fmin+10000; f=f+1000)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(f);
      // ZERO.startMeasure(f);             // start measurement
      delay(ScanDelay);
      // SWR = ZERO.getSWR();            // get SWR value
      if (SWR < SWRmin)
      {
        SWRmin = SWR;
        Fmin = f;
      }
    }


//  Pass to Global Variables //
  SWR = SWRmin;
  ScanFrequency = Fmin;
       
}


void PrintSWR()
{
        // Since we can not print floats we get int & fraction as INT's
      // if swr is 4.12 then v1=4 and v2=12 -- 1.04 then 1 and 4 printed as 1.04 using %d.%02d
      int swr1 = SWR;
      int swr2 = (SWR - swr1) * 100;
      if (swr1 < 0)
      {
        swr1 = 99;
        swr2 = 0;
      }
      sprintf(SWRstr, "SWR = %d.%02d", swr1, swr2); // comput swr as string
      lcd.print(SWRstr); 
}

void PrintFRQ()
{
        // Since we can not print floats we get int & fraction as INT's
      // if swr is 4.12 then v1=4 and v2=12 -- 1.04 then 1 and 4 printed as 1.04 using %d.%02d
     
      long frq1 = ScanFrequency/1000000;
      long frq2 = ScanFrequency-(frq1*1000000);       

      sprintf(FRQstr, "FRQ = %ld.%02ld", frq1, frq2); // comput frq as string
      lcd.print(FRQstr); 
}

void loop() {
  mainMenuDraw();
  drawCursor();
  operateMainMenu();
}



// This function will generate the 2 menu items that can fit on the screen. They will change as you scroll through your menu. Up and down arrows will indicate your current menu position.

// When called, this function will erase the current cursor and redraw it based on the cursorPosition and menuPage variables.
/***************************************************************************************
    Name    : LCD Button Shield Menu - Courtesy of:
    Author  : Paul Siewert
    Created : June 14, 2016
    Last Modified: June 14, 2016
    Version : 1.0
    Notes   : This code is for use with an Arduino Uno and LCD/button shield. The
              intent is for anyone to use this program to give them a starting
              program with a fully functional menu with minimal modifications
              required by the user.
    License : This program is free software. You can redistribute it and/or modify
              it under the terms of the GNU General Public License as published by
              the Free Software Foundation, either version 3 of the License, or
              (at your option) any later version.
              This program is distributed in the hope that it will be useful,
              but WITHOUT ANY WARRANTY; without even the implied warranty of
              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
              GNU General Public License for more details.
 ***************************************************************************************/