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


int buttonPushCounter = 0;          // counter for the number of button presses
// Typically global variables are used to pass data between an ISR and the main program. 
// To make sure variables shared between an ISR and the main program are updated correctly, declare them as volatile.
volatile byte clkInState;          // current state of the Clock input, prev "buttonState"
byte lastClkInState = 0;           // previous state of the button, prev "lastButtonState"
byte RecordButtonState = 0;        // current state of the BIG button
byte LastRecordButtonState = 0;
byte DeleteButtonState = 0;
byte LastDeleteButtonState = 0;
byte ClearButtonState = 0;

//clock state stuff
int ClockState = 0;            
const byte clkInPin = 2;

//Clock Reset Keepers
int ClockKeep = 0;
int ResetSteps = 33;

//RESET BUTTON
const byte ResetButton = 6; // Reset button pin
byte ResetButtonState = 0;
byte LastResetButtonState = 0;

const byte channelSelectPin = A0; 

//FILL BUTTON
int FillButton = 5;
int FillButtonState = 0;
int Fill1 = 0;
int Fill2 = 0;
int Fill3 = 0;
int Fill4 = 0;
int Fill5 = 0;
int Fill6 = 0;

//CLEAR

int ClearState = 0;

//Bank Button Latching
long time = 0;
long debounce = 150;
int  ButtonBankSelectState[7];
bool BankState[7] = {LOW,LOW,LOW,LOW,LOW,LOW,LOW};
bool Bankprevious[7] = {HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
byte BankPush1 = 0;
byte BankPush2 = 0;
byte BankPush3 = 0;
byte BankPush4 = 0;
byte BankPush5 = 0;
byte BankPush6 = 0;

int BankArrayShift1 = 0;
int BankArrayShift2 = 0;
int BankArrayShift3 = 0;
int BankArrayShift4 = 0;
int BankArrayShift5 = 0;
int BankArrayShift6 = 0;
int BankClear = 0;          //not sure what this does
int BankRecord = 0;         //this notes which bank is playing for each channel (Bank A or B) - should be converted to bool
int BankArrayNumber = 0;    //ranges from 1-6 in the code (i think used in an array to note if the bank is on or off for each channel
// int ButtonBankSelectButton = 0; //dont think this does anything

// PATTERN BEAT SHIFT 
const byte shiftPin = A2;
int  shiftPotRead = 0;      // Reading from the Shift Pot Value - prev "KnobVal"
byte shiftValue = 0;        // Value of how many beats to shift (0-8) - prev "KnobValue"
byte oldShiftValue = 0;      // prev "OldKnobValue"
byte newShiftValue = 0;      // prev "NewKnobValue"
byte newShiftValue1 = 0;     // prev "NewKnobValue1"
byte newShiftValue2 = 0;
byte newShiftValue3 = 0;
byte newShiftValue4 = 0;
byte newShiftValue5 = 0;
byte newShiftValue6 = 0;

#define TOLERANCE 0


byte looper = 0;          
const byte BigButtonPin = 19;

// PATERN LENGTH STUFF
const byte patLengthPin = A1;   //prev stepLengthPin
int patLength = 0;              //Pot reading for step length pot, prev StepLength
byte steps = 0;                 //beginning number of the steps in the sequence adjusted by StepLength


const byte clearButtonPin = 4;     
const byte deleteButtonPin = 7;
const byte bankButtonPin = 3;


byte Channel = 1;               // not sure what this does
byte currentChannel = 1; 
int CHANNELSELECT = 0 ;        //not sure what this does
int ChannelSelectState1 = 0;
int ChannelSelectState2 = 0;
int ChannelSelectState3 = 0;
int ChannelSelectState4 = 0;
int ChannelSelectState5 = 0;
int ChannelSelectState6 = 0;


int OUT1 = 8;
int OUT2 = 9;
int OUT3 = 10;
int OUT4 = 11;
int OUT5 = 12;
int OUT6 = 13;
int bankLEDPin ; //pin for bank LED


// Sequence Array
// First dimension is which Channel / Bank combo (6 channels, each with 2 banks = 12 sequences)
// Second dimension is if the beat is on or off within that sequence. Each sequence can have up to 32 beats
int Sequence[14][43] = {
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
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
 }; 




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

  //read step length pot value on start up
  patLength = analogRead(patLengthPin);
  if(0<patLength){steps= 2;}   //this could likely go in a function
  if(200<patLength){steps= 4;}
  if(500<patLength){steps= 8;}
  if(800<patLength){steps=16;}
  if(1000<patLength){steps=32;}   

  //startupLEDSequence();
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
  DeleteButtonState = digitalRead(deleteButtonPin);
  ClearButtonState = digitalRead(clearButtonPin); 
  ResetButtonState = digitalRead(ResetButton);
  FillButtonState = digitalRead(FillButton);
  ButtonBankSelectState[BankArrayNumber] = digitalRead(bankButtonPin);//These setup the states of the buttons


  {if(clkInState == HIGH) {
    looper    = (looper+1);
    BankPush1 = (BankPush1+1);
    BankPush2 = (BankPush2+1);
    BankPush3 = (BankPush3+1);
    BankPush4 = (BankPush4+1);
    BankPush5 = (BankPush5+1);
    BankPush6 = (BankPush6+1);
    ClockKeep = (ClockKeep+1);
    
    digitalWrite(OUT1,Sequence[1+BankArrayShift1][BankPush1 + newShiftValue1] || (Fill1));
    digitalWrite(OUT2,Sequence[2+BankArrayShift2][BankPush2 + newShiftValue2] || (Fill2));
    digitalWrite(OUT3,Sequence[3+BankArrayShift3][BankPush3 + newShiftValue3] || (Fill3));
    digitalWrite(OUT4,Sequence[7+BankArrayShift4][BankPush4 + newShiftValue4] || (Fill4));
    digitalWrite(OUT5,Sequence[8+BankArrayShift5][BankPush5 + newShiftValue5] || (Fill5));
    digitalWrite(OUT6,Sequence[9+BankArrayShift6][BankPush6 + newShiftValue6] || (Fill6));
    delay(10);
    digitalWrite(OUT1,LOW);
    digitalWrite(OUT2,LOW);
    digitalWrite(OUT3,LOW);
    digitalWrite(OUT4,LOW);
    digitalWrite(OUT5,LOW);
    digitalWrite(OUT6,LOW);
    clkInState = LOW;
  } 
  else 
  {
    looper = looper;
    ClockKeep = ClockKeep;
  }}



if(RecordButtonState != LastRecordButtonState){
     if((RecordButtonState == LOW) && (Channel == 1)) {Sequence[Channel+BankRecord][BankPush1+1+newShiftValue1] = 1;}
else if((RecordButtonState == LOW) && (Channel == 2)) {Sequence[Channel+BankRecord][BankPush2+1+newShiftValue2] = 1;}
else if((RecordButtonState == LOW) && (Channel == 3)) {Sequence[Channel+BankRecord][BankPush3+1+newShiftValue3] = 1;}
else if((RecordButtonState == LOW) && (Channel == 7)) {Sequence[Channel+BankRecord][BankPush4+1+newShiftValue4] = 1;}
else if((RecordButtonState == LOW) && (Channel == 8)) {Sequence[Channel+BankRecord][BankPush5+1+newShiftValue5] = 1;}
else if((RecordButtonState == LOW) && (Channel == 9)) {Sequence[Channel+BankRecord][BankPush6+1+newShiftValue6] = 1;}                                                                                                                  


     else { 
  }
  delay (5); 
}   // END RECORD BUTTON



 //This bit is the clock in and step advance stuff

 {if((ClockKeep == 1)  || (ClockKeep == 5) || (ClockKeep == 9)  || (ClockKeep == 13) ||  (ClockKeep == 17)  || (ClockKeep == 21) || (ClockKeep == 25) || (ClockKeep == 29) && (BankRecord == 0)) {digitalWrite(bankLEDPin ,HIGH);} }
 {if((ClockKeep == 1)  || (ClockKeep == 5) || (ClockKeep == 9)  || (ClockKeep == 13) ||  (ClockKeep == 17)  || (ClockKeep == 21) || (ClockKeep == 25) || (ClockKeep == 29) && (BankRecord == 3)) {digitalWrite(bankLEDPin ,LOW);}
 else {digitalWrite(bankLEDPin, BankState[BankArrayNumber]);}}

 
  // Bank button state making it latch and stuff
  if(ButtonBankSelectState[BankArrayNumber] == HIGH && Bankprevious[BankArrayNumber] == LOW && millis() - time > debounce){
     if(BankState[BankArrayNumber] == HIGH){
        BankState[BankArrayNumber] = LOW;
        }
     else {
       BankState[BankArrayNumber] = HIGH;
       time = millis();
       }
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
 
int diff = abs(shiftValue - oldShiftValue);

if((diff > TOLERANCE) && (ChannelSelectState1 == HIGH))
{ newShiftValue1 = shiftValue;
  oldShiftValue = shiftValue;
  }     
if((diff > TOLERANCE) && (ChannelSelectState2 == HIGH))
{ newShiftValue2 = shiftValue;
  oldShiftValue = shiftValue;
  }  
if((diff > TOLERANCE) && (ChannelSelectState3  == HIGH))
{ newShiftValue3 = shiftValue;
  oldShiftValue = shiftValue;
  }
if((diff > TOLERANCE) && (ChannelSelectState4  == HIGH))
{ newShiftValue4 = shiftValue;
  oldShiftValue = shiftValue;
  }
if((diff > TOLERANCE) && (ChannelSelectState5  == HIGH))
{ newShiftValue5 = shiftValue;
  oldShiftValue = shiftValue;
  }
if((diff > TOLERANCE) && (ChannelSelectState6  == HIGH))
{ newShiftValue6 = shiftValue;
  oldShiftValue = shiftValue;
  } //lots of the shit with that shift knob!!!! MEMORY STUFF
   


{CHANNELSELECT= analogRead(channelSelectPin);
if(20>CHANNELSELECT)   {(ChannelSelectState1 = HIGH); (ChannelSelectState2 = LOW); (ChannelSelectState3 = LOW); (ChannelSelectState4 = LOW); (ChannelSelectState5 = LOW);  (ChannelSelectState6 = LOW);}
if(170<CHANNELSELECT) {(ChannelSelectState1 = LOW); (ChannelSelectState2 = HIGH); (ChannelSelectState3 = LOW); (ChannelSelectState4 = LOW); (ChannelSelectState5 = LOW);  (ChannelSelectState6 = LOW);}
if(240<CHANNELSELECT) {(ChannelSelectState1 = LOW); (ChannelSelectState2 = LOW); (ChannelSelectState3 = HIGH); (ChannelSelectState4 = LOW); (ChannelSelectState5 = LOW);  (ChannelSelectState6 = LOW);}
if(420<CHANNELSELECT) {(ChannelSelectState1 = LOW); (ChannelSelectState2 = LOW); (ChannelSelectState3 = LOW);  (ChannelSelectState4 = HIGH);(ChannelSelectState5 = LOW);  (ChannelSelectState6 = LOW);}
if(750<CHANNELSELECT) {(ChannelSelectState1 = LOW); (ChannelSelectState2 = LOW); (ChannelSelectState3 = LOW);  (ChannelSelectState4 = LOW); (ChannelSelectState5 = HIGH); (ChannelSelectState6 = LOW);}
if(1000<CHANNELSELECT){(ChannelSelectState1 = LOW); (ChannelSelectState2 = LOW); (ChannelSelectState3 = LOW);  (ChannelSelectState4 = LOW); (ChannelSelectState5 = LOW);  (ChannelSelectState6 = HIGH);}
}

if(ChannelSelectState1 == HIGH){currentChannel = 1;}
if(ChannelSelectState2 == HIGH){currentChannel = 2;}
if(ChannelSelectState3 == HIGH){currentChannel = 3;}
if(ChannelSelectState4 == HIGH){currentChannel = 4;}
if(ChannelSelectState5 == HIGH){currentChannel = 5;}
if(ChannelSelectState6 == HIGH){currentChannel = 6;}

  
  // "switch for selecting between both channels and banks"
  // If bank state for channel 1 is low and channel select state for channel 1 is high: ???
  // It seems like sequences are in order:
  // 1:A1, 2:A2, 3:A3, 4:B1, 5:B2, 6:B3, 7:A4, 8:A5, 9:A6, 10:B4, 11:B5, 12:B6
  if(BankState[1] == LOW && ChannelSelectState1  == HIGH)     {Channel = 1; BankArrayNumber = 1;  BankArrayShift1 = 0;  BankRecord = 0;  ClearState = 1;}
  else if(BankState[2] == LOW && ChannelSelectState2  == HIGH){Channel = 2; BankArrayNumber = 2;  BankArrayShift2 = 0;  BankRecord = 0;  ClearState = 2;}
  else if(BankState[3] == LOW && ChannelSelectState3  == HIGH){Channel = 3; BankArrayNumber = 3;  BankArrayShift3 = 0;  BankRecord = 0;  ClearState = 3;} 
  else if(BankState[4] == LOW && ChannelSelectState4  == HIGH){Channel = 7; BankArrayNumber = 4;  BankArrayShift4 = 0;  BankRecord = 0;  ClearState = 7;} 
  else if(BankState[5] == LOW && ChannelSelectState5  == HIGH){Channel = 8; BankArrayNumber = 5;  BankArrayShift5 = 0;  BankRecord = 0;  ClearState = 8;} 
  else if(BankState[6] == LOW && ChannelSelectState6  == HIGH){Channel = 9; BankArrayNumber = 6;  BankArrayShift6 = 0;  BankRecord = 0;  ClearState = 9;} 
  
  
  else if(BankState[1] == HIGH && ChannelSelectState1  == HIGH){Channel = 1; BankArrayNumber = 1;  BankArrayShift1 = 3;  BankRecord = 3;  ClearState = 1;}
  else if(BankState[2] == HIGH && ChannelSelectState2  == HIGH){Channel = 2; BankArrayNumber = 2;  BankArrayShift2 = 3;  BankRecord = 3;  ClearState = 2;}
  else if(BankState[3] == HIGH && ChannelSelectState3  == HIGH){Channel = 3; BankArrayNumber = 3;  BankArrayShift3 = 3;  BankRecord = 3;  ClearState = 3;} 
  else if(BankState[4] == HIGH && ChannelSelectState4  == HIGH){Channel = 7; BankArrayNumber = 4;  BankArrayShift4 = 3;  BankRecord = 3;  ClearState = 7;} 
  else if(BankState[5] == HIGH && ChannelSelectState5  == HIGH){Channel = 8; BankArrayNumber = 5;  BankArrayShift5 = 3;  BankRecord = 3;  ClearState = 8;}
  else if(BankState[6] == HIGH && ChannelSelectState6  == HIGH){Channel = 9; BankArrayNumber = 6;  BankArrayShift6 = 3;  BankRecord = 3;  ClearState = 9;}

  // This is the clear current pattern button
  if(ClearButtonState == LOW){ 
    for(int i = 1; i<42; i++){
    Sequence[ClearState+BankRecord][i] = 0;
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



// Fill button logic
// If fill button pressed, the current channel should go in to fill mode
     if((FillButtonState == LOW) && (Channel == 1)) {Fill1 = 1; Fill2 = 0; Fill3 = 0; Fill4 = 0;  Fill5 = 0;  Fill6 = 0;} 
else if((FillButtonState == LOW) && (Channel == 2)) {Fill1 = 0; Fill2 = 1; Fill3 = 0; Fill4 = 0;  Fill5 = 0;  Fill6 = 0;}
else if((FillButtonState == LOW) && (Channel == 3)) {Fill1 = 0; Fill2 = 0; Fill3 = 1; Fill4 = 0;  Fill5 = 0;  Fill6 = 0;}
else if((FillButtonState == LOW) && (Channel == 7)) {Fill1 = 0; Fill2 = 0; Fill3 = 0; Fill4 = 1;  Fill5 = 0;  Fill6 = 0;}
else if((FillButtonState == LOW) && (Channel == 8)) {Fill1 = 0; Fill2 = 0; Fill3 = 0; Fill4 = 0;  Fill5 = 1;  Fill6 = 0;}
else if((FillButtonState == LOW) && (Channel == 9)) {Fill1 = 0; Fill2 = 0; Fill3 = 0; Fill4 = 0;  Fill5 = 0;  Fill6 = 1;}

else if((FillButtonState == HIGH)  && (Channel == 1)) {Fill1 = 0; Fill2 = 0; Fill3 = 0; Fill4 = 0;  Fill5 = 0;  Fill6 = 0;} 
else if((FillButtonState == HIGH)  && (Channel == 2)) {Fill1 = 0; Fill2 = 0; Fill3 = 0; Fill4 = 0;  Fill5 = 0;  Fill6 = 0;}
else if((FillButtonState == HIGH)  && (Channel == 3)) {Fill1 = 0; Fill2 = 0; Fill3 = 0; Fill4 = 0;  Fill5 = 0;  Fill6 = 0;}
else if((FillButtonState == HIGH)  && (Channel == 7)) {Fill1 = 0; Fill2 = 0; Fill3 = 0; Fill4 = 0;  Fill5 = 0;  Fill6 = 0;} 
else if((FillButtonState == HIGH)  && (Channel == 8)) {Fill1 = 0; Fill2 = 0; Fill3 = 0; Fill4 = 0;  Fill5 = 0;  Fill6 = 0;} 
else if((FillButtonState == HIGH)  && (Channel == 9)) {Fill1 = 0; Fill2 = 0; Fill3 = 0; Fill4 = 0;  Fill5 = 0;  Fill6 = 0;} 


// if the delete button is pressed, do XYZ
if(DeleteButtonState == LOW){
Sequence[Channel+BankRecord][looper+1] = 0;}                            


// if the reset button does not equal last reset button state (debounce attempt?)
// and if the reset button is pressed, do XYZ
  if(ResetButtonState != LastResetButtonState){
        if(ResetButtonState == LOW) {
        looper = 0;  
        ClockKeep = 0;
        BankPush1 = 0;
        BankPush2 = 0;
        BankPush3 = 0;
        BankPush4 = 0;
        BankPush5 = 0;
        BankPush6 = 0;
        }
   } 

  

  // Determine how many steps the looping pattern is
  patLength = analogRead(patLengthPin);
  if(0<patLength){steps= 1;}
  if(150<patLength){steps= 2;}
  if(300<patLength){steps= 4;}
  if(500<patLength){steps= 8;}
  if(750<patLength){steps=16;}
  if(1000<patLength){steps=32;}                                    

  if(looper >= steps) {looper = 0;}   //this bit starts the sequence over again
  if(ClockKeep >= 32) {looper = 0; ClockKeep = 0;}
  if((BankPush1  + newShiftValue1) >= steps) {BankPush1 = 0;}
  if((BankPush2  + newShiftValue2) >= steps) {BankPush2 = 0;}
  if((BankPush3  + newShiftValue3) >= steps) {BankPush3 = 0;}
  if((BankPush4  + newShiftValue4) >= steps) {BankPush4 = 0;}
  if((BankPush5  + newShiftValue5) >= steps) {BankPush5 = 0;}
  if((BankPush6  + newShiftValue6) >= steps) {BankPush6 = 0;}


  lastClkInState = clkInState;
  LastRecordButtonState = RecordButtonState;
  LastResetButtonState = ResetButtonState;   //sectoion is for the state change (debounce?) detections
  Bankprevious[BankArrayNumber] = ButtonBankSelectState[BankArrayNumber]; 

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

  } // end debug


} // END LOOP
 
  
  

//  isr() - or Interupt Service Routine - quickly handle interrupts from the clock input
//  If clock input is triggered, set the reading of clock in to be HIGH
void isr()
{
  clkInState = HIGH;
}

