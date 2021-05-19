#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define RST_PIN D3
#define SS_PIN D4
#define buzzer D0
LiquidCrystal_I2C lcd(20, 2);
unsigned long previousMillis = 0;
const long interval = 200;
MFRC522 mfrc522(SS_PIN, RST_PIN);
const char *ssid = "Asus_X01BDA";
const char *pwd = "heri12345";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", 28200, 60000);
String weekDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

void setup()
{
    Serial.begin(9600);
    Serial.print("Connecting to wifi ");
    Serial.println(ssid);
    WiFi.begin(ssid, pwd);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");

        lcd.backlight();
        lcd.print("Connecting to wifi..");
        delay(100);
    }
    timeClient.begin();
    timeClient.setTimeOffset(25200);
    SPI.begin();
    mfrc522.PCD_Init();
    mfrc522.PCD_DumpVersionToSerial();
    Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

    pinMode(buzzer, OUTPUT);

    lcd.backlight();
    lcd.print("Welcome to HANDY");
    delay(1200);
    lcd.clear();
}

String strID, ok;
void loop()
{

    unsigned long currentMillis = millis();
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    {
        waktu();
        return;
        digitalWrite(buzzer, LOW);
    }
    else
    {
        if (currentMillis - previousMillis >= interval)
        {
            previousMillis = currentMillis;
            digitalWrite(buzzer, HIGH);
            delay(200);
            digitalWrite(buzzer, LOW);
        }
    }
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    /*if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }*/
    strID = "";
    for (byte i = 0; i < 4; i++)
    {
        strID +=
            (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "") +
            String(mfrc522.uid.uidByte[i], HEX) +
            (i != 3 ? ":" : "");
    }
    strID.toUpperCase();
    Serial.print("Tap card key: ");
    Serial.println(strID);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(strID);

    HTTPClient http;
    String url = "http://192.168.43.189/coba_nodemcu/mage2020/rfidlog.php?uid=" + String(strID);
    Serial.println(url);
    http.begin(url);

    //GET method
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        Serial.printf("[HTTP] GET...code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK)
        {
            String payload = http.getString();
            Serial.println(payload);
            Serial.println();
            lcd.setCursor(0, 1);
            lcd.print(payload);
        }
    }
    else
    {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        Serial.println();
    }
    http.end();
    delay(1000);
    lcd.clear();
}
void waktu()
{
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();
    String formattedTime = timeClient.getFormattedTime();
    Serial.print("Formatted Time: ");
    Serial.println(formattedTime);
    int currentHour = timeClient.getHours();
    int currentMinute = timeClient.getMinutes();
    int currentSecond = timeClient.getSeconds();

    String weekDay = weekDays[timeClient.getDay()];
    struct tm *ptm = gmtime((time_t *)&epochTime);
    int monthDay = ptm->tm_mday;
    int currentMonth = ptm->tm_mon + 1;
    String currentMonthName = months[currentMonth - 1];
    int currentYear = ptm->tm_year + 1900;
    String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
    String hari = String(weekDay) + "-" + String(currentMonth) + "-" + String(monthDay) + "-" + String(currentYear);
    Serial.print("Current date: ");
    Serial.println(currentDate);

    lcd.setCursor(6, 0);
    lcd.backlight();
    lcd.print(formattedTime);
    lcd.setCursor(0, 1);
    lcd.print(hari);
    lcd.setCursor(3, 2);
    lcd.print("Tempelkan Untuk");
    lcd.setCursor(7, 3);
    lcd.print("Absent");
}