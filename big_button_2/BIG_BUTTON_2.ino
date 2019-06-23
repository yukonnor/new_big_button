//the premise of this is a simple performance sequencer. 
//it is used in synth bike mk2 to sequencer the drums.
//i figured whats the point in not sharing it!!!
//dont be a pleb and steal it for a product or some shit. Build it and
//enjoy it as a musical instrument :)

// Clock in is pin 2 ... Pin 2 is available for "Interupts"
// Clear Button is pin 4 .... this clears the whole loop sequence
// Button Delete is pin 7 This deletes the step your on
// Bank select Pin 3 .... each channel has 2 banks of patterns, so you can record 2 alternative patterns and go between with this button
// BIG BUTTON is pin 19 (A5), this is the performance button!
// Reset in is pin 6 you can add a button or a jack!
// FILL BUTTON pin 5, push this and it will continuously play the channel your on, doesnt record, push it and twist the select knob to make a fill!
// STEP LENGTH analog pin 1 (A1)
// Channel select Analog pin (A0) 
// SHIFT KNOB Analog pin (A2)
// LED (big button LED) pin 20 (A5)

// OUTPUT 1 ... pin 8
// OUTPUT 2 ... pin 9
// OUTPUT 3 ... pin 10
// OUTPUT 4 ... pin 11
// OUTPUT 5 ... pin 12
// OUTPUT 6 ... pin 13

const byte debug = 0;
long time = 0;
long debounce = 300;

// clock stuff
// To make sure variables shared between an ISR and the main program are updated correctly, declare them as volatile.
const byte clkInPin = 2;
volatile byte clkInState;          // current state of the Clock input, prev "buttonState"
byte lastClkInState = 0;           // previous state of the button, prev "lastButtonState"

const byte BigButtonPin = 19;

// Readings of the buttons
byte RecordButtonState = 0;        // BIG button
byte LastRecordButtonState = 0;
byte deletebuttonState = 0;
byte ClearButtonState = 0;




//RESET BUTTON
const byte ResetButton = 6; // Reset button pin
byte ResetButtonState = 0;  // Reading of the reset button
byte LastResetButtonState = 0;

const byte channelSelectPin = A0; 

//FILL BUTTON
int FillButton = 5;
int FillButtonState = 0;
byte FillState[7] = {0,0,0,0,0,0,0};       // store whether the fill is on or off for each channel

//CLEAR BUTTON
//int ClearState = 0;

//BANK BUTTON
byte BankState[7] = {0,0,0,0,0,0,0};        // stores whether the bank is on or off for each channel (which then selects the pattern). "7" as it uses currentChannel (1-6) as an input, so never hits "0"
byte BankButtonState;                       // reads the state of the bank button

byte currentStep1 = 0;                      // previously "BankPush1"
byte currentStep2 = 0;
byte currentStep3 = 0;
byte currentStep4 = 0;
byte currentStep5 = 0;
byte currentStep6 = 0;

// PATTERN BEAT SHIFT 
const byte shiftPin = A2;
int  shiftPotRead = 0;       // Reading from the Shift Pot Value - prev "KnobVal"
byte shiftValue = 0;         // Value of how many beats to shift (0-8) - prev "KnobValue"
byte oldShiftValue = 0;      // prev "OldKnobValue"
byte newShiftValue = 0;      // prev "NewKnobValue"
byte newShiftValue1 = 0;     // prev "NewKnobValue1"
byte newShiftValue2 = 0;
byte newShiftValue3 = 0;
byte newShiftValue4 = 0;
byte newShiftValue5 = 0;
byte newShiftValue6 = 0;

#define TOLERANCE 0

//Clock Reset Keepers
//int ClockKeep = 0;
byte looper = 0;                // sets the shared 'current step', used in the 'delete' function  

// PATERN LENGTH STUFF
const byte patLengthPin = A1;   // prev stepLengthPin
int patLengthPotRead = 0;       // pot reading for step length pot, prev StepLength
byte steps = 0;                 // beginning number of the steps in the sequence adjusted by StepLength


const byte clearButtonPin = 4;     
const byte deleteButtonPin = 7;
const byte bankButtonPin = 3;


byte currentPattern = 0;       // current sequence (0-11 for the 6 channels, 2 banks each)
byte currentChannel = 1;       // current channel (1-6)
byte lastChannel = 1;          
int  CHANNELSELECT = 0 ;        // reading of channel pot

int OUT1 = 8;                  // pins for outputs
int OUT2 = 9;
int OUT3 = 10;
int OUT4 = 11;
int OUT5 = 12;
int OUT6 = 13;
int bankLEDPin ;               // pin for bank LED


// Sequence Array
// First dimension is which Channel / Bank combo (6 channels, each with 2 banks = 12 sequences)
// Second dimension is if the beat is on or off within that sequence. Each sequence can have up to 32 beats, but 43 kept due to shift stuff
byte Sequence[12][43] = {
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
 }; 
 

// Sequence[0]:  Channel 1, Bank Off or 1A or pattern 0
// Sequence[1]:  Channel 1, Bank On  or 1B or pattern 1
// Sequence[2]:  Channel 2, Bank Off or 2A or pattern 2
// Sequence[3]:  Channel 2, Bank On  or 2B or pattern 3
// Sequence[4]:  Channel 3, Bank Off or 3A or pattern 4
// Sequence[5]:  Channel 3, Bank On  or 3B or pattern 5
// Sequence[6]:  Channel 4, Bank Off or 4A or pattern 6
// Sequence[7]:  Channel 4, Bank On  or 4B or pattern 7
// Sequence[8]:  Channel 5, Bank Off or 5A or pattern 8
// Sequence[9]:  Channel 5, Bank On  or 5B or pattern 9
// Sequence[10]: Channel 6, Bank Off or 6A or pattern 10
// Sequence[11]: Channel 6, Bank On  or 6B or pattern 11

void startupLEDSequence(){
  digitalWrite(bankLEDPin , HIGH); //this could go in a function, be redesigned for channel LEDs
  delay(200);
  digitalWrite(bankLEDPin , LOW);
  delay(180);
  digitalWrite(bankLEDPin , HIGH);
  delay(160);
  digitalWrite(bankLEDPin , LOW);
  delay(140);
  digitalWrite(bankLEDPin , HIGH);
  delay(120);
  digitalWrite(bankLEDPin , LOW);
  delay(100);
  digitalWrite(bankLEDPin , HIGH);
  delay(80);
  digitalWrite(bankLEDPin , LOW);
  delay(60);
  digitalWrite(bankLEDPin , HIGH);
  delay(40);
  digitalWrite(bankLEDPin , LOW);
  delay(20);
  digitalWrite(bankLEDPin , HIGH);
  delay(60);
  digitalWrite(bankLEDPin , LOW);
}

//  isr() - or Interupt Service Routine - quickly handle interrupts from the clock input
//  If clock input is triggered, set the reading of clock in to be HIGH
void isr()
{
  clkInState = HIGH;
}

void debuger(){
  if (debug == 1){
  Serial.print("Clock In = ");
  Serial.println(clkInState);
  
  //Serial.print("Big Button = ");
  //Serial.print(RecordButtonState);
  //Serial.print("  Bank Button = ");
  //Serial.print(ButtonBankSelectState[BankArrayNumber]);
  //Serial.print("  Clear Button = ");
  //Serial.print(ClearButtonState);
  //Serial.print("  Fill Button = ");
  //Serial.print(FillButtonState);
  //Serial.print("  Reset Button = ");
  //Serial.print(ResetButtonState);
  //Serial.print("  Delete Button = ");
  //Serial.println(DeleteButtonState);

  // POT TESTING
  Serial.print("Current Channel = ");
  Serial.print(currentChannel);  
  Serial.print("  Beat Shift = ");
  Serial.print(shiftValue);           // Should be between 0 - 8
  Serial.print("  Steps = ");
  Serial.println(steps);             // Should be between 1 - 32
  } 
}


void setup() { 
  pinMode(OUT1,OUTPUT);
  pinMode(OUT2,OUTPUT);
  pinMode(OUT3,OUTPUT);
  pinMode(OUT4,OUTPUT);
  pinMode(OUT5,OUTPUT);
  pinMode(OUT6,OUTPUT);
  pinMode(bankLEDPin , OUTPUT);
  pinMode(clkInPin, INPUT);     //SHOULDNT USE PULLUP as Clock input is high (use hardware pulldown)
  pinMode(BigButtonPin,INPUT_PULLUP);
  pinMode(deleteButtonPin,INPUT_PULLUP);
  pinMode(clearButtonPin,INPUT_PULLUP);
  pinMode(bankButtonPin, INPUT_PULLUP);
  pinMode(ResetButton, INPUT_PULLUP);
  pinMode(FillButton, INPUT_PULLUP);
  pinMode(patLengthPin, INPUT);  // pot to analog pin
  pinMode(shiftPin, INPUT);      // pot to analog pin

  // read pattern length pot value on start up
  // PUT THIS IN A FUNCTION
  patLengthPotRead = analogRead(patLengthPin);
  if(0<patLengthPotRead){steps= 1;}
  if(150<patLengthPotRead){steps= 2;}
  if(300<patLengthPotRead){steps= 4;}
  if(500<patLengthPotRead){steps= 8;}
  if(750<patLengthPotRead){steps=16;}
  if(1000<patLengthPotRead){steps=32;}    

  startupLEDSequence();  // LED flashing start up sequence

  // Interupt Service Routine for Clock Input
  // Syntax: attachInterrupt(digitalPinToInterrupt(pin), ISR, mode)
  // ISR: the interupt service routine to call when the interupt occurs (when clock pin goes high)
  // Rising: trigger when the pin goes from low to high
  attachInterrupt(0, isr, RISING);

  // if debug mode is on, run the serial monitor
  if(debug == 1){Serial.begin(9600);}
  
} //END SETUP


void loop() {

  // Get button readings
  RecordButtonState = digitalRead(BigButtonPin);
  deletebuttonState = digitalRead(deleteButtonPin);
  ClearButtonState = digitalRead(clearButtonPin); 
  ResetButtonState = digitalRead(ResetButton);
  FillButtonState = digitalRead(FillButton);
  BankButtonState = digitalRead(bankButtonPin);


  if(clkInState == HIGH) {
    looper    = (looper+1);
    //ClockKeep = (ClockKeep+1);
    currentStep1 = (currentStep1+1);
    currentStep2 = (currentStep2+1);
    currentStep3 = (currentStep3+1);
    currentStep4 = (currentStep4+1);
    currentStep5 = (currentStep5+1);
    currentStep6 = (currentStep6+1);
    
    
    // Write each channels sequence
    digitalWrite(OUT1,Sequence[0+BankState[1]] [currentStep1 + newShiftValue1] || FillState[1]); // if currentFillChannel == 1, OUT1 should be HIGH
    digitalWrite(OUT2,Sequence[2+BankState[2]] [currentStep2 + newShiftValue2] || FillState[2]); // Logical OR results in a true if either of the two operands is true.
    digitalWrite(OUT3,Sequence[4+BankState[3]] [currentStep3 + newShiftValue3] || FillState[3]); 
    digitalWrite(OUT4,Sequence[6+BankState[4]] [currentStep4 + newShiftValue4] || FillState[4]);
    digitalWrite(OUT5,Sequence[8+BankState[5]] [currentStep5 + newShiftValue5] || FillState[5]);
    digitalWrite(OUT6,Sequence[10+BankState[6]][currentStep6 + newShiftValue6] || FillState[6]);
    delay(10);
    digitalWrite(OUT1,LOW);
    digitalWrite(OUT2,LOW);
    digitalWrite(OUT3,LOW);
    digitalWrite(OUT4,LOW);
    digitalWrite(OUT5,LOW);
    digitalWrite(OUT6,LOW);
    clkInState = LOW;
  } 

  
  // RECORD BUTTON - record the sequence of the current pattern
  if(RecordButtonState != LastRecordButtonState){
       if((RecordButtonState == LOW) && (currentChannel == 1)) {Sequence[0+BankState[currentChannel]] [currentStep1+1+newShiftValue1] = 1;}
  else if((RecordButtonState == LOW) && (currentChannel == 2)) {Sequence[2+BankState[currentChannel]] [currentStep2+1+newShiftValue2] = 1;}
  else if((RecordButtonState == LOW) && (currentChannel == 3)) {Sequence[4+BankState[currentChannel]] [currentStep3+1+newShiftValue3] = 1;}
  else if((RecordButtonState == LOW) && (currentChannel == 4)) {Sequence[6+BankState[currentChannel]] [currentStep4+1+newShiftValue4] = 1;}
  else if((RecordButtonState == LOW) && (currentChannel == 5)) {Sequence[8+BankState[currentChannel]] [currentStep5+1+newShiftValue5] = 1;}
  else if((RecordButtonState == LOW) && (currentChannel == 6)) {Sequence[10+BankState[currentChannel]][currentStep6+1+newShiftValue6] = 1;}                                                                                                                  
  delay (5); // seems sketchy?
  }  

  // Turn on Bank LED if bank is on for the current channel
  digitalWrite(bankLEDPin, BankState[currentChannel]);

 
  // If the bank button is pressed and it's been debounced, then:
  // Set the bank state for the current channel to high if low and vice versa
  if(BankButtonState == LOW && millis() - time > debounce){
     if(BankState[currentChannel] == HIGH){
        BankState[currentChannel] = LOW;
        }
     else {
       BankState[currentChannel] = HIGH;
       }
     time = millis();
  }                                                                                               
    

  // Read the beat shift pot and determine how many beats to shift the pattern for the current channel
  shiftPotRead = analogRead(shiftPin);  // readinging of the beat shift pot
    if(0 < shiftPotRead)   { shiftValue = 0;} // should use an incrementer here and a function
    if(127 < shiftPotRead) { shiftValue = 1;}
    if(254 < shiftPotRead) { shiftValue = 2;}
    if(383 < shiftPotRead) { shiftValue = 3;}
    if(511 < shiftPotRead) { shiftValue = 4;}
    if(638 < shiftPotRead) { shiftValue = 5;}
    if(767 < shiftPotRead) { shiftValue = 6;}
    if(895 < shiftPotRead) { shiftValue = 7;}
    if(1000 < shiftPotRead){ shiftValue = 8;}
 
  // not sure what this does
  int diff = abs(shiftValue - oldShiftValue);

  if((diff > TOLERANCE) && (currentChannel == 1))
  { newShiftValue1 = shiftValue;
    oldShiftValue = shiftValue;
    }     
  if((diff > TOLERANCE) && (currentChannel == 2))
  { newShiftValue2 = shiftValue;
    oldShiftValue = shiftValue;
    }  
  if((diff > TOLERANCE) && (currentChannel  == 3))
  { newShiftValue3 = shiftValue;
    oldShiftValue = shiftValue;
    }
  if((diff > TOLERANCE) && (currentChannel  == 4))
  { newShiftValue4 = shiftValue;
    oldShiftValue = shiftValue;
    }
  if((diff > TOLERANCE) && (currentChannel  == 5))
  { newShiftValue5 = shiftValue;
    oldShiftValue = shiftValue;
    }
  if((diff > TOLERANCE) && (currentChannel  == 6))
  { newShiftValue6 = shiftValue;
    oldShiftValue = shiftValue;
    } //lots of the shit with that shift knob!!!! MEMORY STUFF
   

  // This should go in a function
  CHANNELSELECT= analogRead(channelSelectPin);
  if(20>CHANNELSELECT)  {currentChannel = 1;}
  if(170<CHANNELSELECT) {currentChannel = 2;}
  if(240<CHANNELSELECT) {currentChannel = 3;}
  if(420<CHANNELSELECT) {currentChannel = 4;}
  if(750<CHANNELSELECT) {currentChannel = 5;}
  if(1000<CHANNELSELECT){currentChannel = 6;}

    
  // "switch for selecting between both channels and banks"
  // If bank state for channel 1 is low and channel select state for channel 1 is high: ???
  // It seems like sequences are in order:
  // 1:A1, 2:A2, 3:A3, 4:B1, 5:B2, 6:B3, 7:A4, 8:A5, 9:A6, 10:B4, 11:B5, 12:B6
       if(BankState[currentChannel] == 0 && currentChannel == 1) {currentPattern = 0;}
  else if(BankState[currentChannel] == 0 && currentChannel == 2) {currentPattern = 2;}
  else if(BankState[currentChannel] == 0 && currentChannel == 3) {currentPattern = 4;} 
  else if(BankState[currentChannel] == 0 && currentChannel == 4) {currentPattern = 6;} 
  else if(BankState[currentChannel] == 0 && currentChannel == 5) {currentPattern = 8;} 
  else if(BankState[currentChannel] == 0 && currentChannel == 6) {currentPattern = 10;} 
  
  else if(BankState[currentChannel] == 1 && currentChannel == 1) {currentPattern = 1; }
  else if(BankState[currentChannel] == 1 && currentChannel == 2) {currentPattern = 3; }
  else if(BankState[currentChannel] == 1 && currentChannel == 3) {currentPattern = 5; } 
  else if(BankState[currentChannel] == 1 && currentChannel == 4) {currentPattern = 7; } 
  else if(BankState[currentChannel] == 1 && currentChannel == 5) {currentPattern = 9; }
  else if(BankState[currentChannel] == 1 && currentChannel == 6) {currentPattern = 11;}

  // This is the clear current pattern button
  // If the clear button is pressed, remove all triggers from the current pattern
  if(ClearButtonState == LOW){ 
    for(int i = 1; i<42; i++){
    Sequence[currentPattern][i] = 0;
    }
  } 
        
// "THIS MAKES SURE WHEN THE KNOB SHIFTS THE PATTERN IT DOESNT LOSE A LOAD OF IT!"
// It was commented out in his code, but may give a clue as to why there are 42 beats instead of 32 beats
// in the sequence array
//for(int ii = 1; ii<14; ii++){
//Sequence[ii][33]=Sequence[ii][1];
//Sequence[ii][34]=Sequence[ii][2];
//Sequence[ii][35]=Sequence[ii][3];
//Sequence[ii][36]=Sequence[ii][4];
//Sequence[ii][37]=Sequence[ii][5];
//Sequence[ii][38]=Sequence[ii][6];
//Sequence[ii][39]=Sequence[ii][7];
//Sequence[ii][40]=Sequence[ii][8];
//Sequence[ii][41]=Sequence[ii][9]; 
//}      

  
  // FILL BUTTON
  // If the fill button pressed, the current channel should go in to fill mode
  // Fill can only be on one channel at a time (when the channel switches, the last channel should have fill = 0)
  if(FillButtonState == LOW){
    FillState[currentChannel] = 1;
    if(currentChannel != lastChannel){
        FillState[lastChannel] = 0;
       }
    } 
  else {FillState[currentChannel] = 0;}

  // DELETE BUTTON
  // If the delete button is pressed, do set step of the current pattern to be 0
  if(deletebuttonState == LOW && millis() - time > debounce){
    Sequence[currentPattern][looper+1] = 0;
    time = millis();  
    }                            

  // RESET BUTTON
  // If the reset button is pressed, set the current steps to 0 (start from step 1 of the patterns)
  if(ResetButtonState == LOW && millis() - time > debounce) {
    looper = 0;  
    //ClockKeep = 0;
    currentStep1 = 0;
    currentStep2 = 0;
    currentStep3 = 0;
    currentStep4 = 0;
    currentStep5 = 0;
    currentStep6 = 0;
    time = millis();   
   } 

  // Determine how many steps the looping pattern is
  patLengthPotRead = analogRead(patLengthPin);
  if(0<patLengthPotRead){steps= 1;}
  if(150<patLengthPotRead){steps= 2;}
  if(300<patLengthPotRead){steps= 4;}
  if(500<patLengthPotRead){steps= 8;}
  if(750<patLengthPotRead){steps=16;}
  if(1000<patLengthPotRead){steps=32;}                                    

  if(looper >= steps) {looper = 0;}   //this bit starts the sequence over again
  //if(ClockKeep >= 32) {looper = 0; ClockKeep = 0;}
  if((currentStep1  + newShiftValue1) >= steps) {currentStep1 = 0;}
  if((currentStep2  + newShiftValue2) >= steps) {currentStep2 = 0;}
  if((currentStep3  + newShiftValue3) >= steps) {currentStep3 = 0;}
  if((currentStep4  + newShiftValue4) >= steps) {currentStep4 = 0;}
  if((currentStep5  + newShiftValue5) >= steps) {currentStep5 = 0;}
  if((currentStep6  + newShiftValue6) >= steps) {currentStep6 = 0;}


  lastClkInState = clkInState;
  LastRecordButtonState = RecordButtonState;
  LastResetButtonState = ResetButtonState;   //sectoion is for the state change (debounce?) detections
  lastChannel = currentChannel;              //update the lastChannel, used for the Fill button
  
  debuger();


} // END LOOP
 
