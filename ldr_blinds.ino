#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define ldr 34
#define errorLED 14
#define wifiLED 12

#define SSID "Brayden (2)"
#define WIFI_PASS "12345678"
#define API_KEY "AIzaSyC46BgxFQAMZo9Ibco7ZgtrDIn02ojc6Wg"
#define DATABASE_URL "https://smart-blinds-rtdb-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;


int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

int lastRecord = 0;

void setup() {
  Serial.begin(115200);
  pinMode(ldr, INPUT);
  pinMode(errorLED, OUTPUT);
  pinMode(wifiLED, OUTPUT);

  //initialize the lcd
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);

  //Ensure wifiLED is off until a connection begins
  digitalWrite(wifiLED,LOW);

  lcd.print(String("connecting to: ") + SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  digitalWrite(wifiLED,HIGH);
  lcd.clear();

  //set up database
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if(Firebase.signUp(&config, &auth, "", "")) {
      signupOK = true;
  } else{
    digitalWrite(errorLED, HIGH);
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}


void loop() {
  lcd.clear();
  lcd.setCursor(0,0);
  int ldr_data = analogRead(ldr);
  lcd.print(String("ldr_data: ") + ldr_data);


  delay(100);
  if(Firebase.ready() && signupOK) {
    if(Firebase.RTDB.setInt(&fbdo, "Sensor/ldr_data", ldr_data)) {
      Serial.print(ldr_data);
      Serial.println(" - saved to the database");
    }
  }
  delay(1000);

}
