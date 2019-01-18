#include <FastIO.h>
#include <I2CIO.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal_SR.h>
#include <LiquidCrystal_SR2W.h>
#include <LiquidCrystal_SR3W.h>
#include <EEPROM.h>
#include <RTClib.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
RTC_DS3231 RTC;

Servo myservo;  // create servo object to control a servo


//-----------------------//
//------Variables--------//
//-----------------------//

// Melody notes
int melody[] = {
        262, 196, 196, 220, 196, 0, 247, 262
};

// note duration: 4 = quarter note, 8 = eighth note etc.
int noteDurations[] = {
        4, 8, 8, 4, 4, 4, 4, 4
};

//------First Timer
byte onhour1;
byte onmin1;
byte onsec1;

byte offhour1;
byte offmin1;
byte offsec1;
//------Pages or menus
int page_counter=1;
int subpage1_counter=0;
int subpage2_counter=0;

//-------To convert clock into single number
unsigned long Time;
unsigned long Hour;
unsigned long Min;
unsigned long Sec;
//------To convert first timer into Single number
unsigned long on_Time1;
unsigned long on_hour1;
unsigned long on_min1;
unsigned long on_sec1;

unsigned long off_Time1;
unsigned long off_hour1;
unsigned long off_min1;
unsigned long off_sec1;

//-------Push buttons current/last state
boolean current_up = LOW;
boolean last_up=LOW;
boolean current_sel = LOW;
boolean last_sel = LOW;
boolean last_down = LOW;
boolean current_down = LOW;
//-------Pins
byte Relay =12;//Relay to pin 12
byte LED =13;//LED to pin 13
byte SERVOPIN =7;//Servo to pin 7
byte Buzzer =12;//Buzzer to pin 12 same as relay, can add a speaker here and edit pin number
int up=2;      //Up button to pin 2
int sel=3;     //Select button to pin 3
int down=4;   //Down button to pin 4

//Custom return char
byte back[8] = {
        0b00100,
        0b01000,
        0b11111,
        0b01001,
        0b00101,
        0b00001,
        0b00001,
        0b11111
};

//Custom arrow char
byte arrow[8] = {
        0b01000,
        0b00100,
        0b00010,
        0b11111,
        0b00010,
        0b00100,
        0b01000,
        0b00000
};

void setup() {
    Serial.begin(9600);

    myservo.attach(SERVOPIN);  //the pin for the servo control
    pinMode(Buzzer, OUTPUT);

    Wire.begin();
    RTC.begin();
    lcd.begin(16,2);
    lcd.backlight();
    lcd.clear();
    lcd.createChar(1, back);//Custom chars
    lcd.createChar(2, arrow);
//--------eePROM  read values-------//
//------First Timer
    onhour1=EEPROM.read(0);
    onmin1=EEPROM.read(1);
    onsec1=EEPROM.read(2);

    offhour1=EEPROM.read(3);
    offmin1=EEPROM.read(4);
    offsec1=EEPROM.read(5);

    myservo.write(180);
    delay(1000);
    myservo.write(0); //set initial servo position to mid
    delay(1000);
    myservo.write(180);
    delay(1000);
    myservo.write(0);
    delay(1000);
    myservo.write(90);
    delay(1000);

}

int nextPage(int page) {
    if(page <3){                   //Page counter never higher than 3(total of pages)
        page ++;                       //Page up
    }
    else{
        page= 1;                       //If higher than 3 (last page)go to main page
    }
    return page;
}

int previousPage(int page) {
    if(page >1){                      //Page counter never lower than 1
        page --;                          //Page down
    }
    else{
        page= 3;                 //If lower than 1(first page)go to last page
    }
    return page;
}

int nextSubpageCounterForPageTwo(int counter) {
    if(counter <7){                    // subpage counter never higher than 7 (total of items)
        counter ++;                         //subcounter to move beetwen submenu
    }
    else{                                       //If subpage higher than 7 (total of items) return to first item
        counter=1;
    }
    return counter;
}

//---- De-bouncing function for all buttons----//
boolean debounce(boolean last, int pin)
{
    boolean current = digitalRead(pin);
    if (last != current)
    {
        delay(10);
        current = digitalRead(pin);
    }
    return current;
}

void loop() {

    current_up = debounce(last_up, up);         //Debounce for Up button
    current_sel = debounce(last_sel, sel);      //Debounce for Select  button
    current_down = debounce(last_down, down);   //Debounce for Down button

    DateTime now = RTC.now();        // Clock call
    now = RTC.now();

//-----Up/Down functions to move main pages------///

    if(subpage1_counter==0 && subpage2_counter==0){ //up/down buttons enabled if subpages counters are 0,Disabled if 1,2..etc to work on submenus

//Page Up
        if (last_up== LOW && current_up == HIGH){ //Up button pressed
            lcd.clear();                            //Clear lcd if page is changed to print new one
            page_counter=nextPage(page_counter);
        }
        last_up = current_up;                   //Save up button last state

//Page Down
        if (last_down== LOW && current_down == HIGH){//Down button pressed
            lcd.clear();                               //Clear lcd if page is changed to print new one
            page_counter=previousPage(page_counter);
        }
        last_down = current_down;         //Save down button last state
    }
//------------Pages and submenus display and control----------//
    switch (page_counter){
        case 1:                      //Content of main page
            last_sel=current_sel;  //Save last state of select button when we jump from the save screen
            setLcdForPageOne(now);
            //case 1
            break;

        case 2:                   //Content and functions of page 2
            setLcdLabelsForPageTwo();
            setLcdTimeForPageTwo(onhour1,onmin1, onsec1, 7,0); //Printing on values
            setLcdTimeForPageTwo(offhour1,offmin1, offsec1, 7,1); //Printing off values

//--------------Modifying on/off values-------//
            // Sub counter control
            if (last_sel== LOW && current_sel == HIGH){ //select button pressed
                subpage1_counter = nextSubpageCounterForPageTwo(subpage1_counter);
            }
            last_sel=current_sel;                      //Save last state of select button

            //First item control(subpage_counter =1) onhour1
            if(subpage1_counter==1){
                lcd.setCursor(0,1);         //Delete last arrow position (back)
                lcd.print(" ");
                lcd.setCursor(6,0);          //Place arrow in front of selected item
                lcd.write(byte(2));
                //Move item + or -
                if (last_up== LOW && current_up == HIGH){  //Up
                    if(onhour1 < 23){
                        onhour1 ++;
                    }
                    else{
                        onhour1 =0;
                    }
                }
                last_up=current_up;

                if(last_down== LOW && current_down == HIGH){//Down
                    if(onhour1 >0){
                        onhour1 --;
                    }
                    else{
                        onhour1=23;
                    }
                }
                last_down=current_down;
            }//subpage1_counter 1

            //Second item control(subpage_counter =2) onmin1
            if(subpage1_counter==2){
                lcd.setCursor(6,0);         //Delete last arrow position (onhour1)
                lcd.print(" ");
                lcd.setCursor(9,0);          //Place arrow in front of selected item
                lcd.write(byte(2));
                //Move item + or -
                if (last_up== LOW && current_up == HIGH){  //Up
                    if(onmin1 < 59){
                        onmin1 ++;
                    }
                    else{
                        onmin1 =0;
                    }
                }
                last_up=current_up;

                if(last_down== LOW && current_down == HIGH){//Down
                    if(onmin1 >0){
                        onmin1 --;
                    }
                    else{
                        onmin1=59;
                    }
                }
                last_down=current_down;
            }//subpage1_counter 2

            //Thirth item control(subpage_counter =3) onsec1
            if(subpage1_counter==3){
                lcd.setCursor(9,0);         //Delete last arrow position (onmin1)
                lcd.print(" ");
                lcd.setCursor(12,0);          //Place arrow in front of selected item
                lcd.write(byte(2));
                //Move item + or -
                if (last_up== LOW && current_up == HIGH){  //Up
                    if(onsec1 < 59){
                        onsec1 ++;
                    }
                    else{
                        onsec1 =0;
                    }
                }
                last_up=current_up;

                if(last_down== LOW && current_down == HIGH){//Down
                    if(onsec1 >0){
                        onsec1 --;
                    }
                    else{
                        onsec1=59;
                    }
                }
                last_down=current_down;
            }//subpage1_counter 3

            //fourth item control(subpage_counter =4) offhour1
            if(subpage1_counter==4){
                lcd.setCursor(12,0);         //Delete last arrow position (onsec1)
                lcd.print(" ");
                lcd.setCursor(6,1);          //Place arrow in front of selected item
                lcd.write(byte(2));
                //Move item + or -
                if (last_up== LOW && current_up == HIGH){  //Up
                    if(offhour1 < 23){
                        offhour1 ++;
                    }
                    else{
                        offhour1 =0;
                    }
                }
                last_up=current_up;

                if(last_down== LOW && current_down == HIGH){//Down
                    if(offhour1 >0){
                        offhour1 --;
                    }
                    else{
                        offhour1=23;
                    }
                }
                last_down=current_down;
            }//subpage1_counter 4

            //fifth item control(subpage_counter =5) offmin1
            if(subpage1_counter==5){
                lcd.setCursor(6,1);         //Delete last arrow position (offhour1)
                lcd.print(" ");
                lcd.setCursor(9,1);          //Place arrow in front of selected item
                lcd.write(byte(2));
                //Move item + or -
                if (last_up== LOW && current_up == HIGH){  //Up
                    if(offmin1 < 59){
                        offmin1 ++;
                    }
                    else{
                        offmin1 =0;
                    }
                }
                last_up=current_up;

                if(last_down== LOW && current_down == HIGH){//Down
                    if(offmin1 >0){
                        offmin1 --;
                    }
                    else{
                        offmin1=59;
                    }
                }
                last_down=current_down;
            }//subpage1_counter 5

            //sixth item control(subpage_counter =6) offsec1
            if(subpage1_counter==6){
                lcd.setCursor(9,1);         //Delete last arrow position (offmin1)
                lcd.print(" ");
                lcd.setCursor(12,1);          //Place arrow in front of selected item
                lcd.write(byte(2));
                //Move item + or -
                if (last_up== LOW && current_up == HIGH){  //Up
                    if(offsec1 < 59){
                        offsec1 ++;
                    }
                    else{
                        offsec1 =0;
                    }
                }
                last_up=current_up;

                if(last_down== LOW && current_down == HIGH){//Down
                    if(offsec1 >0){
                        offsec1 --;
                    }
                    else{
                        offsec1=59;
                    }
                }
                last_down=current_down;
            }//subpage1_counter 6


            //seventh item control(subpage_counter =7) back
            if(subpage1_counter==7){
                lcd.setCursor(12,1);         //Delete last arrow position (offsec1)
                lcd.print(" ");
                lcd.setCursor(0,1);          //Place arrow in front of selected item
                lcd.write(byte(2));
                //Move item + or -
                if (last_up== LOW && current_up == HIGH){  //Up
                    lcd.setCursor(0,1);         //Delete last arrow position (back) to exit
                    lcd.print(" ");
                    subpage1_counter=0;         //Exit submenu. Up/down butons enabled to move main pages
                }
                last_up=current_up;

                if(last_down== LOW && current_down == HIGH){//Down
                    lcd.setCursor(0,1);         //Delete last arrow position (back)
                    lcd.print(" ");
                    subpage1_counter=1;         //Go to first item (onhour1)
                }
                last_down=current_down;
            }//subpage1_counter 7
            //case 2
            break;

        case 3:                //Page 3 display and functions
            lcd.setCursor(4,0);
            lcd.print("PRESS SEL");
            lcd.setCursor(1,1);
            lcd.print("TO SAVE ALARM");
            if (last_sel== LOW && current_sel == HIGH){  //select button pressed.Save settings to eeprom
                EEPROM.write(0, onhour1);
                EEPROM.write(1, onmin1);
                EEPROM.write(2, onsec1);
                EEPROM.write(3, offhour1);
                EEPROM.write(4, offmin1);
                EEPROM.write(5, offsec1);
                lcd.clear();                 //Print message "SAVED!"
                lcd.setCursor(0,0);
                lcd.print("WE ARE WATCHING");
                lcd.setCursor(0,1);
                lcd.print("YOUR FOOD!");
                delay(2000);
                lcd.clear();               //Clear lcd and go to main page
                page_counter=1;
            }
            last_sel=current_sel;  //Save last state of select button

            //Case 3
            break;
    }//switch


//-------------Conversion----------//

//---------Converting clock time into single number

    Hour = now.hour();
    Min = now.minute();
    Sec = now.second();
    Time = (Hour*10000+ Min*100 +Sec*1);

//--------Converting firt timer on/off into single number
    on_hour1=onhour1;
    on_min1=onmin1;
    on_sec1=onsec1;
    on_Time1=(on_hour1*10000 + on_min1*100 + on_sec1);

    off_hour1=offhour1;
    off_min1=offmin1;
    off_sec1=offsec1;
    off_Time1=(off_hour1*10000 + off_min1*100 + off_sec1);



//----Relay/Buzzer Function----//
    if(onhour1 == offhour1 && onmin1==offmin1 && onsec1==offsec1){
        alarmOff();
    }

    if(on_Time1 < off_Time1){

        if(Time >= on_Time1 && Time < off_Time1){  //Start
            alarmOn();
        }
        else if(Time >= off_Time1) {
            alarmOff();
        }
        else{
            alarmOff();
        }
    }
    if (on_Time1 > off_Time1){
        if(Time >= on_Time1 && Time <= 235959){     //Start
            alarmOn();
        }
        else if(Time < off_Time1 ){
            if(Time >= on_Time1){     //Start
                alarmOn();
            }
            else {
                alarmOff();
            }
        }
        else if(Time >= off_Time1 && Time < on_Time1){
            alarmOff();
        }

    }
    unsigned long rem = on_Time1-Time;
    if(on_Time1<Time) {
        rem = 240000+on_Time1-Time;
    }
    myservo.write(map(rem, 0, 235959, 0, 180));
}//void loop

void setLcdForPageOne(DateTime now) {
    lcd.setCursor(0,0);
    lcd.print("FOOD-ALARM");
    lcd.setCursor(0,1);
    lcd.print("TIME");

//--------Show  Time On LCD

    lcd.setCursor(7,1);
    if(now.hour() < 10)
    {
        lcd.print("0");
    }
    lcd.print(now.hour(), DEC); //Print hour
    lcd.print(':');
    if(now.minute() < 10)
    {
        lcd.print("0");
    }
    lcd.print(now.minute(), DEC); //Print min
    lcd.print(':');
    if(now.second() < 10)
    {
        lcd.print("0");
    }
    lcd.print(now.second(), DEC); //Print sec
}

void setLcdLabelsForPageTwo() {
    lcd.setCursor(0,0);
    lcd.print("ALARM");
    // lcd.setCursor(3,0);
    //lcd.print("ON");
    lcd.setCursor(1,1);
    lcd.write(byte(1));
    lcd.setCursor(3,1);
    lcd.print("OFF");
}

void setLcdTimeForPageTwo(int hour, int min, int sec, int x, int y) {
    lcd.setCursor(x,y);
    setLcdTimeDigit(hour);
    lcd.setCursor(x+3,y);
    setLcdTimeDigit(min);
    lcd.setCursor(x+6,y);
    setLcdTimeDigit(sec);
}
void setLcdTimeDigit(int digit) {
    if(digit<10){
        lcd.print("0");
    }
    lcd.print(digit);
}

void playMelody() {
    tone(Buzzer, 50);
    // iterate over all notes in melody[]
    TODO Sync with loop of arduino upon alarm
    /* for (int i = 0; i <= 8; i++) {

       // In order to calculate the correct note length we take 1 second and divide it by the type of note
       // for example: a quarter note is 1000 / 4, an eighth note 1000 / 8 etc.
       int noteDuration = 1000 / noteDurations[i];

       // Call tone() with the current note of the melody with the calculated duration of the note.
       tone(Buzzer, melody[i], noteDuration);

       // in order to distinguish the notes, we set a minimal pause between two notes
       // it turned out the note length plus 30% works fine.
       int pauseBetweenNotes = noteDuration * 1.30;

       // pause for the calculated duration
       delay(pauseBetweenNotes);

       // stop playing the current note
       noTone(Buzzer);
     }*/
}

void loopservo(){
    // myservo.write(90);
    //delay(20);
}

void alarmOn(){
    digitalWrite(Relay, HIGH);
    digitalWrite(LED, HIGH);
    playMelody();
    loopservo();
}

void alarmOff(){
    digitalWrite(Relay, LOW);
    digitalWrite(LED, LOW);
    noTone(Buzzer);
}
