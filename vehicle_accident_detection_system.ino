#include "Adafruit_FONA.h"
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#define FONA_RX 8
#define FONA_TX 7
#define FONA_RST 10
#define Powerkey 9
#define minVal -1.00
#define MaxVal 1.00
int trigger =0;
int LED_Pin = 13;
int vibr_Pin =3;

// this is a large buffer for replies
char replybuffer[255];
String baseURL = "api.thingspeak.com/update?api_key=96YEPIWLLFLGC1A7&field1=";
int firstRun = 1;
String lat;
String lon;
// We default to using software serial. If you want to use hardware serial
// (because softserial isnt supported) comment out the following three lines 
// and uncomment the HardwareSerial line
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

// Hardware serial is also possible!
//  HardwareSerial *fonaSerial = &Serial1;


Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

uint8_t type;
LiquidCrystal_I2C lcd(0x27, 16, 2);

int Xread;
int Xrest;

int Yread;
int Yrest;

int Zread;
int Zrest;

double Gx;
double Gy;
double Gz;

int zpin = 0;
int ypin = 1;
int xpin = 2;

int t1;

const int buttonPin = 4;



void setup() {
  while (!Serial);
  Serial.begin(115200);
  pinMode(LED_Pin, OUTPUT);
  pinMode(vibr_Pin, INPUT); //set vibr_Pin input for measurment
  Xrest=analogRead(xpin);
  
 Yrest=analogRead(ypin);
 
 Zrest=analogRead(zpin);
 
 digitalWrite(13,LOW);
 Serial.println(F("FONA basic test"));
 Serial.println(F("Initializing....(May take 3 seconds)"));
 lcd.begin();
 lcd.backlight();
 lcd.setCursor(0, 0);
 lcd.print("System Initiated");
 pinMode(Powerkey, OUTPUT);   // initialize the digital pin as an output.  
 power();
 fonaSerial->begin(4800);
 if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while (1);
  }
  type = fona.type();
  lcd.setCursor(0, 1);
  lcd.print("FONA is OK");
  
  // Print module IMEI number.
  char imei[16] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("Module IMEI: "); Serial.println(imei);
  }

  // Optionally configure a GPRS APN, username, and password.
  // You might need to do this to access your network's GPRS/data
  // network.  Contact your provider for the exact APN, username,
  // and password values.  Username and password are optional and
  // can be removed, but APN is required.
  //fona.setGPRSNetworkSettings(F("your APN"), F("your username"), F("your password"));
 //fona.setGPRSNetworkSettings(F("dialogbb"), F(""), F(""));

  // Optionally configure HTTP gets to follow redirects over SSL.
  // Default is not to follow SSL redirects, however if you uncomment
  // the following line then redirects over SSL will be followed.
  //fona.setHTTPSRedirect(true);
  
  logData();
  delay(2000); 
  lcd.setCursor(0, 1);
  lcd.print(" System Ready! ");
}
void calibrate(){
Serial.print("Time ");
t1=millis();
Serial.println(t1*0.001);

Xread=analogRead(xpin)-Xrest;
Yread=analogRead(ypin)-Yrest;
Zread=analogRead(zpin)-Zrest;

Gx=Xread/67.584;
Gy=Yread/67.584;
Gz=Zread/67.584;

Serial.print("Acceleration X :");
Serial.print(Gx);
Serial.print("Acceleration Y :");
Serial.print(Gy);
Serial.print("Acceleration Z :");
Serial.print(Gz);
Serial.print("\n");
/*
lcd.setCursor(0, 0);
lcd.print("gx:");
lcd.print(Gx);
lcd.setCursor(8, 0);
lcd.print("gy:");
lcd.print(Gy);
lcd.setCursor(0, 1);
lcd.print("gz:");
lcd.print(Gz);
delay(1000);
lcd.clear();
*/
if(Gx < minVal || Gx > MaxVal  || Gy < minVal || Gy > MaxVal  || Gz < minVal || Gz > MaxVal)
{
  trigger = 1;
}
else
  trigger = 0;
}



String printFloat(float value, int places) {
  // this is used to cast digits 
  int digit;
  float tens = 0.1;
  int tenscount = 0;
  int i;
  float tempfloat = value;

    // make sure we round properly. this could use pow from <math.h>, but doesn't seem worth the import
  // if this rounding step isn't here, the value  54.321 prints as 54.3209

  // calculate rounding term d:   0.5/pow(10,places)  
  float d = 0.5;
  if (value < 0)
    d *= -1.0;
  // divide by ten for each decimal place
  for (i = 0; i < places; i++)
    d/= 10.0;    
  // this small addition, combined with truncation will round our values properly 
  tempfloat +=  d;

  // first get value tens to be the large power of ten less than value
  // tenscount isn't necessary but it would be useful if you wanted to know after this how many chars the number will take

  if (value < 0)
    tempfloat *= -1.0;
  while ((tens * 10.0) <= tempfloat) {
    tens *= 10.0;
    tenscount += 1;
  }

String ll;
  // write out the negative if needed
  if (value < 0){
    Serial.print('-');
    ll+= "-";
  }
  if (tenscount == 0){
    Serial.print(0, DEC);
    ll+= "0";
  }
  for (i=0; i< tenscount; i++) {
    digit = (int) (tempfloat/tens);
    Serial.print(digit, DEC);
    ll+=String(digit,DEC);
    tempfloat = tempfloat - ((float)digit * tens);
    tens /= 10.0;
  }

  // if no places after decimal, stop now and return
  if (places <= 0)
    return;

  // otherwise, write the point and continue on
  Serial.print('.');  
  ll+=".";
  // now write out each decimal place by shifting digits one by one into the ones place and writing the truncated value
  for (i = 0; i < places; i++) {
    tempfloat *= 10.0; 
    digit = (int) tempfloat;
    Serial.print(digit,DEC);  
    ll+= String(digit,DEC);
    // once written, subtract off that digit
    tempfloat = tempfloat - (float) digit; 
  }
  //Serial.println(ll);
  return ll;
  
}

void time()
{
int8_t stat;
 // check GPS fix
 stat = fona.GPSstatus();
 lcd.setCursor(0, 0);
 if (stat < 0){
 lcd.print(F("Failed to query"));
 }
 if (stat == 0) lcd.println(F("     GPS off    "));
 if (stat == 1) lcd.println(F("   No GPS fix   "));
 if (stat == 2) lcd.println(F("   2D GPS fix   "));
 if (stat == 3) lcd.println(F("   3D GPS fix   "));

}

void logData(){
  if(firstRun == 1){
  // turn GPS off
  if (!fona.enableGPS(false))
    Serial.println(F("Failed to turn off"));
    firstRun = 0;
  delay(1000);
  // turn GPS on
  if (!fona.enableGPS(true))
    Serial.println(F("Failed to turn on"));
  delay(1000);
  firstRun = 0;
  }
  while (fona.GPSstatus() < 2);
  time();
  float latitude, longitude, speed_kph, heading, altitude;
  bool gpsFix = fona.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude);
  //lcd.clear();
  Serial.print(F("Latitude: "));
  lat = printFloat(latitude, 5);
  Serial.println(F(""));
  lcd.setCursor(0, 1);
  lcd.print("   Speed:");
  lcd.print(speed_kph);
  Serial.print(F("Longitude: "));
  lon = printFloat(longitude, 5);

  Serial.println(F(""));
  // Wait 5 secs
  delay(5000);

 
}

void isCollided(){
     //http request !
     if (!fona.enableGPRS(true))
          Serial.println(F("Failed to turn on"));
          
  String sendd = baseURL + lat+"&field2=" + lon + "&field3=" + Gx + "&field4=" + Gy + "&field5=" + Gz + "&status=" + t1;
 
  uint16_t statuscode;
  int16_t length;
  unsigned int ln = sendd.length();
  char url[ln];
  sendd.toCharArray(url,ln);
  Serial.println(F("****"));
  if (!fona.HTTP_GET_start(url, &statuscode, (uint16_t *)&length)) {
    Serial.println("Failed!");
    lcd.setCursor(0, 1);
    lcd.print("  http Failed! ");
  }else {
    Serial.println("Success!");
    lcd.setCursor(0, 1);
    lcd.print(" http Success! ");
  }
  lcd.setCursor(0, 1);
  lcd.print(" System Ready! ");
  trigger = 0;
  if (!fona.enableGPRS(false))
          Serial.println(F("Failed to turn off"));
}

//************************************************************************************************
void loop() {
  if (digitalRead(buttonPin) == LOW)
  {
      logData();
      time();
      isCollided();
  }else if(trigger == 1){
      logData();
      time();
      isCollided();
  }
  else{
    shock();
    calibrate();

  }
    //isCollided()
    delay(50);
}
//************************************************************************************************
void shock(){
    
   long measurement =TP_init();
   delay(50);
   Serial.print("measurment = ");
   Serial.println(measurement);
   if (measurement > 20000){
    lcd.setCursor(0, 0);
    lcd.println(" Shock detected ");
    calibrate();
    trigger = 1;
    }
}
long TP_init(){
  delay(10);
  long measurement=pulseIn (vibr_Pin, HIGH);  //wait for the pin to get HIGH and returns measurement
  return measurement;
}

void flushSerial() {
  while (Serial.available())
    Serial.read();
}

char readBlocking() {
  while (!Serial.available());
  return Serial.read();
}
uint16_t readnumber() {
  uint16_t x = 0;
  char c;
  while (! isdigit(c = readBlocking())) {
    //Serial.print(c);
  }
  Serial.print(c);
  x = c - '0';
  while (isdigit(c = readBlocking())) {
    Serial.print(c);
    x *= 10;
    x += c - '0';
  }
  return x;
}

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout) {
  uint16_t buffidx = 0;
  boolean timeoutvalid = true;
  if (timeout == 0) timeoutvalid = false;

  while (true) {
    if (buffidx > maxbuff) {
      //Serial.println(F("SPACE"));
      break;
    }

    while (Serial.available()) {
      char c =  Serial.read();

      //Serial.print(c, HEX); Serial.print("#"); Serial.println(c);

      if (c == '\r') continue;
      if (c == 0xA) {
        if (buffidx == 0)   // the first 0x0A is ignored
          continue;

        timeout = 0;         // the second 0x0A is the end of the line
        timeoutvalid = true;
        break;
      }
      buff[buffidx] = c;
      buffidx++;
    }

    if (timeoutvalid && timeout == 0) {
      //Serial.println(F("TIMEOUT"));
      break;
    }
    delay(1);
  }
  buff[buffidx] = 0;  // null term
  return buffidx;
}
void power(void)
{
  digitalWrite(Powerkey, LOW); 
  delay(1000);               // wait for 1 second
  digitalWrite(Powerkey, HIGH);
}
