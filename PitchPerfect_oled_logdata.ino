

/*
   This sketch sends data via HTTP GET requests to data.sparkfun.com service.

   You need to get streamId and privateKey at data.sparkfun.com and paste them
   below. Or just customize this script to talk to other HTTP servers.

*/

#include <ESP8266WiFi.h>
#include <Wire.h>
//#include <Adafruit_L3GD20.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_9DOF.h>
#include <Adafruit_LSM9DS0.h>
//#include <Adafruit_LSM303.h>
#include <Adafruit_LSM303_U.h>
#include <SPI.h>
#include<SD.h>
#include "Adafruit_LiquidCrystal.h"
//#include "LiquidCrystal.h"
//#include<LiquidCrystal_I2C.h>
//#include<LiquidTWI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 0
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2


#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000
};

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


int Acc_Threshold = 52;
int Gyr_Threshold = 200;

const char* ssid     = "rlMotog3wifi";//"APT305-2.4";//"HP-setup-south";
const char* password = "funkymonkey";//"walnut305";//"detkinlabsouth101999";
const byte IP[] = {192, 168, 43, 47}; //192.168.1.58   10.0.0.247 158.130.164.148 192.168.43.47
char* IP_char =  "192.168.1.58";

//File myFile;
const int button = 15;



//LiquidTWI lcd(0);
//LiquidCrystal lcd(0);
Adafruit_LiquidCrystal lcd(0);

/*
  const char* host = "data.sparkfun.com";
  const char* streamId   = "....................";
  const char* privateKey = "....................";*/

sensors_event_t accel_event;
sensors_vec_t   orientation;
sensors_vec_t gyro;
Adafruit_9DOF dof   = Adafruit_9DOF();
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);

double x, y, z, roll, pitch, heading, angle1, angle2;
double ax, ay, az, vx, vy, vz;
double sumAbs = 0, diff;

double arrayUpload[10][5];
bool uploadMode = true;
bool startUpload = false;
bool serverUpload = false;

double acqRate = 0;
long timeprev = 0, timenow, dt;

bool isVerbose = true;

bool FileWriteMode = false;
bool FileReadMode = false;
//File myFile1, myFile2;
int value = 0, bpress = 0;
bool DataRecordMode = false;    //initially false. The first button press will make it true and values get recorded.
static int data_pts = 0;
//static bool FileReadMode=false;
int tag = 0;
int pitch_count = 0;

void setup() {

  Serial.begin(115200);
  //Wire.begin();
  //Wire.setClock(400000L);

  pinMode(button, INPUT);
  delay(10);
  if (!accel.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println(F("Ooops, no LSM303 detected ... Check your wiring!"));
    while (1);
  }
  bool SDinit = false;
  while (!SDinit) {
    if (!SD.begin(2)) {
      Serial.println("SD initialization failed!");
      //return;

    }
    else {
      Serial.println("initialization done.");
      SDinit = true;
    }
  }

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //  myFile1 = SD.open("test2.txt", FILE_WRITE);
  //  SD.remove("test2.txt");
  //  SD.remove("test2.txt");

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 5; j++) {
      arrayUpload[i][j] = 0;
    }
  }

  timeprev = millis();
  /*
     Wire.begin(4,5);
    lcd.begin(16,2);
    lcd.print("HELLO WORLD");*/
  //lcd.begin(16, 2);
  // Print a message to the LCD.
  //lcd.print("PitchPerfect!");

  /*
    lcd.setCursor(0, 1);
    // print the number of seconds since reset:
    lcd.print(millis()/1000);

    lcd.setBacklight(HIGH);
    delay(500);
    lcd.setBacklight(LOW);
    delay(500);
  */

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.display();
  /*delay(2000);
    display.clearDisplay();
    display.setTextSize(1.5);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("Pitch Perfect!");
    display.setCursor(0, 10);
    display.println("Pitch after 2 seconds...");
    display.clearDisplay();
    display.display();
    delay(2000);*/
  SD.remove("test2.txt");
}

void loop() {

  // Show the display buffer on the hardware.
  // NOTE: You _must_ call display after making any drawing commands
  // to make them visible on the display hardware!
  static int countprint = 0;
  static int pitch_count = 0;

  //display.display();
  //display.
  if (countprint == 500) {
    Serial.println("OLED PRINTS NOW");
    display.display();
    delay(2000);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.setCursor(0, 0);
    display.println("Pitch Perfect!");
    display.setCursor(0, 10);
    display.println("MISFITS");
    display.display();
    //countprint = 0;
  }
  countprint += 1;
  Serial.print("COUNT");
  Serial.println(countprint);

  int val = 0;
  static File myFile1, myFile;
  val = digitalRead(button);
  if (val) {
    delay(200);
    tag++;
    val = 0;
  }
  if (serverUpload) //data_pts>300)              // START UPLOAD CODE
  {

    display.display();
    delay(2000);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    String text = "Uploading " + String(data_pts) + " data-pts";
    display.println(text);
    display.setCursor(0, 20);
    display.println("Please wait.....");
    display.display();
    data_pts = 0;
    delay(5000);
    if (isVerbose) {
      Serial.print("connecting to ");
      Serial.println(IP_char);
    }
    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 5005;

    if (!client.connect(IP, httpPort)) {
      if (isVerbose) {
        Serial.println("connection failed");
      }
      return;
    }
    if (myFile1) {
      if (isVerbose) {
        Serial.println("Exiting ReadMode");
      }
      myFile1.close();
    }
    /// Serial.println("Entering ReadMode");
    myFile = SD.open("test2.txt", FILE_READ);
    if (myFile) {
      FileReadMode = true;
    }
    else {
      FileReadMode = false;
    }


    while (client.connected())
    {
      if (!FileReadMode)
      {

        myFile = SD.open("test2.txt", FILE_READ);
        FileReadMode = true;

      }

      if (!myFile.available())
      {
        // Serial.println("Data upload Complete");
        //DataRecordMode = true; //Switch to data record mode
        //DataUploadMode = false;
        break;

      }
      String strToSend = myFile.readStringUntil('\n'); //String(myFile.read());
      // Serial.print("Uploading Data: ");
      // Serial.println(strToSend);

      client.print(strToSend);


      // Read all the lines of the reply from server and print them to Serial
      while (client.available()) {
        String line = client.readStringUntil('\r');
        lcd.setCursor(0, 1);
        // print the number of seconds since reset:
        pitch_count = pitch_count + (line).toInt();
        lcd.print(pitch_count);



        //Serial.print(line);
      }

    }

    //Serial.println();
    // Serial.println("closing connection");
    client.stop();
    myFile.close();
    //Serial.println("Deleting uploaded File.");
    SD.remove("test2.txt");

    //FileReadMode = false;
    serverUpload = false;
    display.display();
    delay(2000);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("Upload Complete");
    display.setCursor(0, 10);
    display.println(pitch_count);
    display.display();
  }   //                                                                              END UPLOAD CODE




  //Serial.println("CP9\n");
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  x = 0;
  y = 0;
  z = 0;


  accel.getEvent(&accel_event);
  if (accel_event.acceleration.x && accel_event.acceleration.y && accel_event.acceleration.z)
  {
    timenow = millis();
    dt = timenow - timeprev;
    timeprev = timenow;
    x = accel_event.acceleration.x;
    y = accel_event.acceleration.y;
    z = accel_event.acceleration.z;


    //PREPROCESSING
    ax = x / 0.27;
    ay = y / 0.27;
    az = z / 0.27;

    vx = ax * dt;
    vy = ay * dt;
    vz = az * dt;
    String Vels = "Vels: " + String(x) + " " + String(y) + " " + String(z) + " Time: " + String(dt) + "\n";


    // IT WAS HERE

    //}

    // Serial.print(Vels);
    // Serial.print("DIFF");
    // Serial.println(diff);


  }
  if (dof.accelGetOrientation(&accel_event, &gyro))
  {
    /* 'orientation' should have valid .roll and .pitch fields */
    roll = gyro.roll;
    pitch = gyro.pitch;
    heading = gyro.heading;

    angle1 = 0.98 * (angle1 + roll) + 0.02 * x;
    angle2 = 0.98 * (angle2 + pitch) + 0.02 * y;

    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 5; j++) {
        arrayUpload[i][j] = arrayUpload[i + 1][j];
      }
    }

    arrayUpload[9][0] = x;
    arrayUpload[9][1] = y;
    arrayUpload[9][2] = z;
    arrayUpload[9][3] = angle1;
    arrayUpload[9][4] = angle2;

    String send_string = String(millis()) + "," + String(x) + "," + String(y) + "," + String(z) + "," + String(roll) + "," + String(pitch) + " , " + String(pitch_count) + "\r\n";
    char* char_data;
    send_string.toCharArray(char_data, send_string.length());
    if (isVerbose) {
      Serial.print(send_string);
    }

    if ((pitch_count ) && (pitch_count % 5 == 0)) {

      serverUpload = true;
      //uploadMode = true;
      //startUpload = false;//true;
      //Serial.println("Pitch Data");
    }

  }
  if (uploadMode) {
    if (startUpload) {
      for (int i = 0; i < 10; i++) {
        String send_string = "F," + String(arrayUpload[i][0]) + "," + String(arrayUpload[i][1]) + "," + String(arrayUpload[i][2]) + "," + String(arrayUpload[i][3]) + "," + String(arrayUpload[i][4]) + " , " + String(tag) + "\r\n";
        char* char_data;
        send_string.toCharArray(char_data, send_string.length());
        //Serial.println(send_string);
        if (x != 0 || y != 0 || z != 0 || roll != 0 || pitch != 0 || heading != 0)
        {
          // if the file opened okay, write to it:
          if (myFile1) {
            // Serial.println("CP5\n");
            FileWriteMode = true;
            //Serial.println("CP4\n");
            myFile1.print(send_string);
            data_pts += 1;
            myFile1.close();
          }
          else {
            myFile1 = SD.open("test2.txt", FILE_WRITE);
            // if the file didn't open, print an error:
            //      Serial.println("error opening test2.txt");
            //      FileWriteMode = false;
            if (myFile1) {
              //    Serial.println("CP15\n");
              FileWriteMode = true;
              //    Serial.println("CP14\n");
              myFile1.print(send_string);
              data_pts += 1;
              myFile1.close();
            }
            else {
              // Serial.println("error opening test2.txt");
            }
          }

        }
      }

      startUpload = false;





    }


    if (false) { //data_pts > 300) { !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      uploadMode = false;
      serverUpload = true;
    }

    String send_string = String(millis()) + "," + String(x) + "," + String(y) + "," + String(z) + "," + String(roll) + "," + String(pitch) + " , " + String(pitch_count) + "\r\n";
    diff = abs(x) + abs(y) + abs(z);// - sumAbs;
    sumAbs = x + y + z; //abs(x) + abs(y) + abs(z)+abs(roll)+abs(pitch);
    //diff = sumAbs;
    int diff2 = abs(roll) + abs(pitch);

    static bool GyrThFound = false;
    static int threshCount = 0;


    int restDays = 0;
    if (pitch_count >= 3) {
      restDays = 3;
    }
    if (GyrThFound)
    { threshCount += 1;
      if (threshCount > 200) {
        threshCount = 0;
        GyrThFound = false;
      }
      if (diff > Acc_Threshold)
      {
        pitch_count++;
        display.display();
        delay(2000);
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        String str1, str2;
        str1 = "Pitch Count = " + String(pitch_count);
        display.println(str1);
        display.setCursor(0, 10);
        str2 = "Suggested Rest days:";
        display.println(str2);
        display.setCursor(0, 20);
        display.println(restDays);
        display.display();
      }
    }

    if ((diff2 > Gyr_Threshold)) { //&&(diff>=59)){ //&& (diff2<250)){
      GyrThFound = true;
    }



    char* char_data;
    send_string.toCharArray(char_data, send_string.length());
    Serial.println(send_string);
    if (x != 0 || y != 0 || z != 0 || roll != 0 || pitch != 0 || heading != 0)
    {
      // if the file opened okay, write to it:
      if (myFile1) {
        // Serial.println("CP5\n");
        FileWriteMode = true;
        //Serial.println("CP4\n");
        myFile1.print(send_string);
        data_pts += 1;
        myFile1.close();
      }
      else {
        myFile1 = SD.open("test2.txt", FILE_WRITE);
        // if the file didn't open, print an error:
        //      Serial.println("error opening test2.txt");
        //      FileWriteMode = false;
        if (myFile1) {
          //    Serial.println("CP15\n");
          FileWriteMode = true;
          //    Serial.println("CP14\n");
          myFile1.print(send_string);
          data_pts += 1;
          myFile1.close();
        }
        else {
          // Serial.println("error opening test2.txt");
        }
      }

    }
  }
  //client.print(send_string);
  //myFile1.close();
  // myFile2.close();

}

