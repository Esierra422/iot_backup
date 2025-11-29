#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <AccelStepper.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "ESGalaxy"
#define WIFI_PASSWORD "tyxj1443"
#define API_KEY "AIzaSyC46BgxFQAMZo9Ibco7ZgtrDIn02ojc6Wg"
#define DATABASE_URL "https://smart-blinds-rtdb-default-rtdb.firebaseio.com/"

#define IN1 19
#define IN2 18
#define IN3 5
#define IN4 15
AccelStepper stepper (AccelStepper::FULL4WIRE, IN1, IN3, IN2, IN4);

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

// Motor control variables (read from Firebase)
bool isTurnCounterCW = false;
bool isTurnCW = false; 
bool isManual = true;

void setup() {
  // wifi & firebase setup - begin
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  while(WiFi.status() != WL_CONNECTED){
    Serial.print("."); delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  //setup firebase connection
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if(Firebase.signUp(&config, &auth, "", "")){
    Serial.println("signUp OK");
    signupOK = true;
  } else {
    Serial.println("ERROR: COULD NOT SIGN UP");
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  // wifi & firebase setup - end

  stepper.setMaxSpeed(5000);
  stepper.setAcceleration(100);
}

void loop() {
  stepper.run();

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    // Read DB value
    if (Firebase.RTDB.getBool(&fbdo, "/Motor/turnCounterCW")) {
      isTurnCounterCW = fbdo.boolData();
    }
    if (Firebase.RTDB.getBool(&fbdo, "/Motor/turnCW")) {
      isTurnCW = fbdo.boolData();
    }
  }
  
  if (isTurnCounterCW) {
    turnMotorCCW();
  }

  if (isTurnCW) {
    turnMotorCW();
  }

}

void turnMotorCCW() {
  stepper.moveTo(6144);

  if (stepper.distanceToGo() == 0) {
    isTurnCounterCW = false;
    Firebase.RTDB.setBool(&fbdo, "/Motor/turnCounterCW", false);
  }
}

void turnMotorCW(){
  stepper.moveTo(-6144);

  if (stepper.distanceToGo() == 0) {
    isTurnCW = false;
    Firebase.RTDB.setBool(&fbdo, "/Motor/turnCW", false);
  }
}