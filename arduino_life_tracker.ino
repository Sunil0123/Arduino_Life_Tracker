#include <ESP8266HTTPClient.h>   //webhook fire 
#include <ESP8266WiFi.h>   ///esp inbuilt
#include <FirebaseArduino.h>   //connect to firebase   
#include<NTPClient.h>  //NTP time server
#include<WiFiUdp.h>    //Support lib for ntp

#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#define REPORTING_PERIOD_MS     1000


// Set these to run example.
#define FIREBASE_HOST "tcs-life-tracker-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "JKacUHLpDk8YJNUDh0mXghm6fnTDV7jFt5IE1RpU"
#define WIFI_SSID "hitman"
#define WIFI_PASSWORD "virat188"

String hotspot = "virat188";
const long utcOffsetInSeconds = 5*3600+1800;
String currenttime,uid,email,token,note,inshort;
 String key_hook = "mY7jCcdWqsn5QuOu1eJo2x_m0QJEhlPt4yhn4_3fX1h"; //your webhooks key  cTJMYE7LvsjIM9l7nfO3ms-KUuYi7TKX0jSFEz3vKke
String event_name = "notify"; //your webhooks event name
String value1="sunil"; //value1 that you want to send when the event is triggered
String value2="2" ; //value2 that you want to send when the event is triggered
String value3="3" ; //value3 that you want to send when the event is triggered

/////////Temperature sensor
int ThermistorPin = A0;
int Vo;
float R1 = 10000;
float logR2, R2, T;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;







//*******************************************************************************************************************************************************************************************//
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);


PulseOximeter pox;
uint32_t tsLastReport = 0;
// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
    Serial.println("Beat!");
}

//*******************************************************************************************************************************************************************************************//
//Setup Programs//

void setup() {
  Serial.begin(9600);
  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
      Serial.print("Initializing pulse oximeter..");
    // Initialize the PulseOximeter instance
    // Failures are generally due to an improper I2C wiring, missing power supply
    // or wrong target chip
    if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }
    // The default current for the IR LED is 50mA and it could be changed
    //   by uncommenting the following line. Check MAX30100_Registers.h for all the
    //   available options.
    // pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
    // Register a callback for the beat detection
    pox.setOnBeatDetectedCallback(onBeatDetected);

    
  timeClient.begin();
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
   //getting user id
    uid = Firebase.getString("users/"+hotspot+"/UID");
     email = Firebase.getString("users/"+hotspot+"/email");
      token = Firebase.getString("users/"+hotspot+"/Firebase_Token");


   timeClient.begin(); 

}




//*******************************************************************************************************************************************************************************************//

bool critical;
String conclusion;


//****************************************************************************************************************************************************************************************//
//Main Programs//

void loop() {
    // Make sure to call update as fast as possible
    note = "";
    inshort="";
timeClient.update();
 unsigned long epochTime = timeClient.getEpochTime();
 struct tm *ptm = gmtime ((time_t *)&epochTime); 
  String formattedTime = timeClient.getFormattedTime(); 
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;
  int currentYear = ptm->tm_year+1900;
  int currentHour = timeClient.getHours();
    int currentMinute = timeClient.getMinutes();
   int currentSecond = timeClient.getSeconds();
  String Hour;
  (currentHour-12)>=10 && currentHour ?Hour=String(currentHour-12):Hour="0"+String(abs(currentHour-12));
  if(currentHour<10){
    Hour="0"+String(currentHour);
    }else if(currentHour-12<10){
       Hour="0"+String(abs(currentHour-12));
      }else{
            Hour=String(abs(currentHour-12));
        }
  String Minute;
  (currentMinute)>=10?Minute=String(currentMinute):Minute="0"+String(currentMinute);
   String Second;
  (currentSecond)>=10?Second=String(currentSecond):Second="0"+String(currentSecond);
    String Day;
  (monthDay)>=10?Day=String(monthDay):Day="0"+String(monthDay);
   String Month;
  (currentMonth)>=10?Month=String(currentMonth):Month="0"+String(currentMonth);

  if(currentHour>12 && currentHour<24){
      currenttime=Hour+":"+Minute+":"+Second+" PM";
     }else if(currentHour<12 &&currentHour>0){
      currenttime=Hour+":"+Minute+":"+Second+" AM";
     }else if(currentHour==24){
      currenttime=Hour+":"+Minute+":"+Second+" AM";
    }else if(currentHour==0){
      currenttime="12:"+Minute+":"+Second+" AM";
    }

  String currentDate = Day+ "-" + Month + "-" + String(currentYear);

  Serial.print("Current date: ");
  Serial.println(currentDate);
   Serial.print("Current time: ");
  Serial.println(currenttime);
   critical=false;
  conclusion="";

 //make json object of data for sending in firebase
StaticJsonBuffer<1000> jsonBuffer;
JsonObject& data = jsonBuffer.createObject();
JsonObject& notify = jsonBuffer.createObject();

String timestamp = (String)millis();
//
//


 pox.update();
   

/////////temperature
      Vo = analogRead(ThermistorPin);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  T = T - 273.15+55;
 // T = (T * 9.0)/ 5.0 + 32.0; 

  Serial.print("Temperature: "); 
  Serial.print(T);
  Serial.println(" c");
  critical = false; 
  if(T>38 || T<36){
    critical = true;
    }else if(pox.getHeartRate()>100 || pox.getHeartRate()<60){
      
      }else if (pox.getSpO2()<=90){
       
        }
        if(T<36){
          note="Low-Body-Temperature-Detected";
            inshort="Low-Body-Temperature-Detected";
          }else if(T>=38){
              note="High-Body-Temperature-Detected";
                inshort="High-Body-Temperature-Detected";
            }
   data["Temperature"] = code(T);
     data["Heart_Rate"] = code(pox.getHeartRate());
  data["Oxygen_Level"] = code(pox.getSpO2());
    data["time"] = currentDate+" "+currenttime;
  /////////temperature


  if(critical){
     data["remark"] = "critical";
        String output = email+"/"+note+"/"+inshort+"/"+token;
  triggerWebhook(output);
        Firebase.set("Life_Tracker/"+uid+"/User_Details/output","abnormal");
    }else{
       data["remark"] = "normal";
        Firebase.set("Life_Tracker/"+uid+"/User_Details/output","normal");
      }

  Firebase.set("Life_Tracker/"+uid+"/Sensor_Value/"+currentDate+"/"+currenttime+"/",data);

  delay(30000);
}

void triggerWebhook(String doc){
   //triggerEvent takes an Event Name and then you can optional pass in up to 3 extra Strings
  HTTPClient http;
  http.begin("http://maker.ifttt.com/trigger/"+event_name+"/with/key/"+key_hook+"?value1="+doc+"&value2="+value2+"&value3="+value3);
  http.GET();
  http.end();
  Serial.print("done ");
  Serial.println(doc);
  }

//*******************************************************************************************************************************************************************************************//
//Encryption Programs//
String code(int data){
  int key = random(10,20);
  data = data*key;
  int first = key/10;
  int last = key%10;
  return (String)first+(String)data+(String)last;
  }
