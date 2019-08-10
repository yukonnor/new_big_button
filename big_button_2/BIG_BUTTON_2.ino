// Clock Input ... Pin 2 (available for "Interupts")
// Clear Button .. Pin 4 (clears the entire loop sequence for the current channel and bank)
// Delete Button . Pin 7 (deletes the current step for the current channel and bank)
// Bank Button ... Pin 3 (each channel has 2 banks where alternative patterns can be recored and stored. this button toggles the bank for the current channeL)
// Big Button .... Pin 19 (A5) (this is the record button!)
// Reset Button .. Pin 6 (resets the current channel and back to step 1)
// Fill Button ... Pin 5 (while held, it will continuously play the channel your on)

// Channel Select Switch .. Pin (A0) (selects the current channel, 1-6, to modify)
// Step Length Knob ....... Pin (A1) (sets the amount of steps for all channels, 1-32)
// Shift Knob ............. Pin (A2) (set the amount of steps to shift the output of the current channel)

// Output 1 ... Pin 8
// Output 2 ... Pin 9
// Output 3 ... Pin 10
// Output 4 ... Pin 11
// Output 5 ... Pin 12
// Output 6 ... Pin 13

const byte    debug = 0;           // if 1, run the Serial Monitor and debug program

unsigned long time = 0;            
const byte    debounce = 200;

// CLOCK IN 
// Declared as volatile as it is shared between the ISR and the main program.
const byte    clkInPin = 2;
volatile byte clkInState;       

// BIG BUTTON aka Record Button
const byte recordButtonPin = 19;
byte       recordButtonState = 0;       

// DELETE BUTTON
const byte deleteButtonPin = 7;
byte       deletebuttonState = 0;

// RESET BUTTON
const byte resetButtonPin = 6; 
byte       resetButtonState = 0;  

// FILL BUTTON
const byte fillButtonPin = 5;
byte       fillButtonState = 0;
byte       fillState[6] = {0,0,0,0,0,0};   // store whether the fill is on or off for each channel

// CLEAR BUTTON
const byte clearButtonPin = 4; 
byte       clearButtonState = 0;

// BANK BUTTON
const byte bankButtonPin = 3;
byte       bankButtonState = 0;            
byte       bankState[6] = {0,0,0,0,0,0};   // stores whether the bank is off or on (0 or 6) for each channel (which then selects the pattern sequence). 

// CLOCK STUFF
byte       currentStep  = 0;    // sets the shared global 'current step', used in the 'delete' function. Previously 'looper'
byte       currentStep1 = 0;                      
byte       currentStep2 = 0;
byte       currentStep3 = 0;
byte       currentStep4 = 0;
byte       currentStep5 = 0;
byte       currentStep6 = 0;

// SHIFT STUFF 
const byte shiftPin = A2;
int        shiftPotRead = 0;    
byte       shiftValue = 0;       // Value of how many beats to shift (0-8) - prev "KnobValue"
byte       lastShiftValue = 0;      
byte       shiftValue1 = 0;         
byte       shiftValue2 = 0;
byte       shiftValue3 = 0;
byte       shiftValue4 = 0;
byte       shiftValue5 = 0;
byte       shiftValue6 = 0;

// PATERN LENGTH STUFF
const byte patLengthPin = A1;   
int        patLengthPotRead = 0; 
byte       steps = 0;             // length of sequence (1 - 32 steps)

// CHANNEL SELECT STUFF
const byte channelPotReadPin = A0; 
int        channelPotRead = 0;
byte       currentChannel = 0;    // current channel (0-5)
byte       lastChannel = 0;          
          
// OUTPUTS
const byte outPin1 = 8;           // pins for trigger and LED outputs
const byte outPin2 = 9;
const byte outPin3 = 10;
const byte outPin4 = 11;
const byte outPin5 = 12;
const byte outPin6 = 13;
const byte bankLEDPin;               // pin TBD  for bank LED 

// SEQUENCE ARRAY
/*  First dimension is the Channel / Bank combo (6 channels, each with 2 banks = 12 sequences) 
 *  Second dimension is if the beat is on or off within that sequence. Each sequence can have up to 32 beats.
 *  Sequence[0]:  Channel 1, Bank Off
 *  Sequence[1]:  Channel 2, Bank Off  
 *  Sequence[2]:  Channel 3, Bank Off 
 *  Sequence[3]:  Channel 4, Bank Off  
 *  Sequence[4]:  Channel 5, Bank Off 
 *  Sequence[5]:  Channel 6, Bank Off  
 *  Sequence[6]:  Channel 1, Bank On  //Note: bank ON is "+ 6" as 'current channel' (0-5) is used to determine sequence #
 *  Sequence[7]:  Channel 2, Bank On
 *  Sequence[8]:  Channel 3, Bank On
 *  Sequence[9]:  Channel 4, Bank On
 *  Sequence[10]: Channel 5, Bank On
 *  Sequence[11]: Channel 6, Bank On
 */
byte Sequence[12][32] = {
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
 };
 
// FUN BLINKY ON STARTUP
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
//  See "attachInterrupt" in Setup
//  If clock input is triggered (rising edge), set the reading of clock in to be HIGH
void isr()
{
  clkInState = HIGH;
}

void debuger(){
  if (debug == 1){
  Serial.print("Clock In = ");
  Serial.println(clkInState);
  
  /*
   * Serial.print("Big Button = ");
   * Serial.print(recordButtonState);
   * Serial.print("  Bank Button = ");
   * Serial.print(ButtonBankSelectState[BankArrayNumber]);
   * Serial.print("  Clear Button = ");
   * Serial.print(clearButtonState);
   * Serial.print("  Fill Button = ");
   * Serial.print(fillButtonState);
   * Serial.print("  Reset Button = ");
   * Serial.print(resetButtonState);
   * Serial.print("  Delete Button = ");
   * Serial.println(DeleteButtonState);
   */

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
  pinMode(outPin1,OUTPUT);
  pinMode(outPin2,OUTPUT);
  pinMode(outPin3,OUTPUT);
  pinMode(outPin4,OUTPUT);
  pinMode(outPin5,OUTPUT);
  pinMode(outPin6,OUTPUT);
  pinMode(bankLEDPin , OUTPUT);
  pinMode(clkInPin, INPUT);     //SHOULDNT USE PULLUP as Clock input is high (use hardware pulldown)
  pinMode(recordButtonPin,INPUT_PULLUP);
  pinMode(deleteButtonPin,INPUT_PULLUP);
  pinMode(clearButtonPin,INPUT_PULLUP);
  pinMode(bankButtonPin, INPUT_PULLUP);
  pinMode(resetButtonPin, INPUT_PULLUP);
  pinMode(fillButtonPin, INPUT_PULLUP);
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
  recordButtonState = digitalRead(recordButtonPin);
  deletebuttonState = digitalRead(deleteButtonPin);
  clearButtonState = digitalRead(clearButtonPin); 
  resetButtonState = digitalRead(resetButtonPin);
  fillButtonState = digitalRead(fillButtonPin);
  bankButtonState = digitalRead(bankButtonPin);


  if(clkInState == HIGH) {
    currentStep  = (currentStep+1);
    currentStep1 = (currentStep1+1);
    currentStep2 = (currentStep2+1);
    currentStep3 = (currentStep3+1);
    currentStep4 = (currentStep4+1);
    currentStep5 = (currentStep5+1);
    currentStep6 = (currentStep6+1); 
    
    // Output each channels sequence
    // when I moved out of if statement, it created longer duty cycles (fill would run into itself for each beat, leading to constant ON state)
    digitalWrite(outPin1,Sequence[0 + bankState[0]] [currentStep1 + shiftValue1] || fillState[0]); // Logical OR results in a true if either of the two operands is true.
    digitalWrite(outPin2,Sequence[1 + bankState[1]] [currentStep2 + shiftValue2] || fillState[1]); 
    digitalWrite(outPin3,Sequence[2 + bankState[2]] [currentStep3 + shiftValue3] || fillState[2]); 
    digitalWrite(outPin4,Sequence[3 + bankState[3]] [currentStep4 + shiftValue4] || fillState[3]);
    digitalWrite(outPin5,Sequence[4 + bankState[4]] [currentStep5 + shiftValue5] || fillState[4]);
    digitalWrite(outPin6,Sequence[5 + bankState[5]] [currentStep6 + shiftValue6] || fillState[5]);
    delay(10);              //do this with a time diff?
    digitalWrite(outPin1,LOW);
    digitalWrite(outPin2,LOW);
    digitalWrite(outPin3,LOW);
    digitalWrite(outPin4,LOW);
    digitalWrite(outPin5,LOW);
    digitalWrite(outPin6,LOW);

    clkInState = LOW;      // isr triggers on rising edge of clock signal. isr sets clkInState to be HIGH, we so need to set to LOW here.   
  } 
    
  
  // RECORD BUTTON - record the sequence of the current pattern
  if(recordButtonState == LOW && millis() - time > debounce) {
    // Sequence[currentChannel + bankState[currentChannel]] [currentStep1 + 1 + newShiftValue1] = 1;  -- tried it this way, but doesn't work with "shift" (currentStep is dif for each channel)
    if(currentChannel == 0) {Sequence[currentChannel + bankState[currentChannel]] [currentStep1 + 1 + shiftValue1] = 1;}
    else if(currentChannel == 1) {Sequence[currentChannel + bankState[currentChannel]] [currentStep2 + 1 + shiftValue2] = 1;} 
    else if(currentChannel == 2) {Sequence[currentChannel + bankState[currentChannel]] [currentStep3 + 1 + shiftValue3] = 1;}
    else if(currentChannel == 3) {Sequence[currentChannel + bankState[currentChannel]] [currentStep4 + 1 + shiftValue4] = 1;}
    else if(currentChannel == 4) {Sequence[currentChannel + bankState[currentChannel]] [currentStep5 + 1 + shiftValue5] = 1;}
    else if(currentChannel == 5) {Sequence[currentChannel + bankState[currentChannel]] [currentStep6 + 1 + shiftValue6] = 1;}
                                                                                    
    time = millis();    
  }

  // Turn on Bank LED if bank is on for the current channel
  digitalWrite(bankLEDPin, bankState[currentChannel]);

 
  // BANK BUTTON
  // If the bank button is pressed set the bank state for the current channel to 6 if 0 and vice versa
  // Bank is 0 or 6 to add 6 to get the correct pattern (current channel + 6)
  if(bankButtonState == LOW && millis() - time > debounce){
     if(bankState[currentChannel] == 6){
        bankState[currentChannel] = 0;
        }
     else {
       bankState[currentChannel] = 6;
       }
     time = millis();
  }                                                                                               
    
  // SHIFT POT
  // Read the beat shift pot and determine how many beats to shift the pattern for the current channel
  shiftPotRead = analogRead(shiftPin);         
    if(0 < shiftPotRead)   { shiftValue = 0;}  
    if(127 < shiftPotRead) { shiftValue = 1;}
    if(254 < shiftPotRead) { shiftValue = 2;}
    if(383 < shiftPotRead) { shiftValue = 3;}
    if(511 < shiftPotRead) { shiftValue = 4;}
    if(638 < shiftPotRead) { shiftValue = 5;}
    if(767 < shiftPotRead) { shiftValue = 6;}
    if(895 < shiftPotRead) { shiftValue = 7;}
    if(1000 < shiftPotRead){ shiftValue = 8;}

  // if the shift value changes, set the new shift value for the current channel
  if(shiftValue != lastShiftValue){
    if(currentChannel == 0) {shiftValue1 = shiftValue;} 
    if(currentChannel == 1) {shiftValue2 = shiftValue;}   
    if(currentChannel == 2) {shiftValue3 = shiftValue;}   
    if(currentChannel == 3) {shiftValue4 = shiftValue;}   
    if(currentChannel == 4) {shiftValue5 = shiftValue;}   
    if(currentChannel == 5) {shiftValue6 = shiftValue;}     
    } 
   

  // CHANNEL SELECT POT
  channelPotRead= analogRead(channelPotReadPin);
  if(20>channelPotRead)  {currentChannel = 0;}
  if(170<channelPotRead) {currentChannel = 1;}
  if(240<channelPotRead) {currentChannel = 2;}
  if(420<channelPotRead) {currentChannel = 3;}
  if(750<channelPotRead) {currentChannel = 4;}
  if(1000<channelPotRead){currentChannel = 5;}


  // CLEAR PATTERN BUTTON
  // If the clear button is pressed, remove all triggers from the current pattern
  if(clearButtonState == LOW){ 
    for(int i = 1; i<32; i++){
    Sequence[currentChannel + bankState[currentChannel]][i] = 0;
    }
  } 
  
  // FILL BUTTON
  // If the fill button pressed, the current channel should go in to fill mode
  // Fill can only be on one channel at a time
  if(fillButtonState == LOW){
    fillState[currentChannel] = 1;
    if(currentChannel != lastChannel){
        fillState[lastChannel] = 0;
        }
      } 
  else {fillState[currentChannel] = 0;}

  // DELETE BUTTON
  // If the delete button is pressed, do set step of the current pattern to be 0
  if(deletebuttonState == LOW && millis() - time > debounce){
    Sequence[currentChannel + bankState[currentChannel]][currentStep+1] = 0;
    time = millis();  
    }                            

  // RESET BUTTON
  // If the reset button is pressed, set the current steps to 0 (start from step 1 of the patterns)
  if(resetButtonState == LOW && millis() - time > debounce) {
    currentStep = 0;  
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

  //this bit starts the sequence over again
  if(currentStep >= steps) {currentStep = 0;}   
  if((currentStep1  + shiftValue1) >= steps) {currentStep1 = 0;}
  if((currentStep2  + shiftValue2) >= steps) {currentStep2 = 0;}
  if((currentStep3  + shiftValue3) >= steps) {currentStep3 = 0;}
  if((currentStep4  + shiftValue4) >= steps) {currentStep4 = 0;}
  if((currentStep5  + shiftValue5) >= steps) {currentStep5 = 0;}
  if((currentStep6  + shiftValue6) >= steps) {currentStep6 = 0;}

  lastChannel = currentChannel;              //update the lastChannel, used for the Fill button
  lastShiftValue = shiftValue;
  
  debuger();
  
} // END LOOP

