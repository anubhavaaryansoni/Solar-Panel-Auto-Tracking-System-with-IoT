#include <ESP32Servo.h>
#include <WiFi.h>
#include <ThingSpeak.h>
#include <NTPClient.h>
#include <time.h>
Servo solarPanel;
#define LDR_PIN 36
const float GAMMA = 0.7;
const float RL10 = 50;
float angle;
const int servoPin = 13;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";
WiFiClient client;
unsigned long myChannelNumber = 2500621;
const char *myWriteAPIKey = "0AB37B1GFE9D4S0R";
int statusCode;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000);

void setup()
{
    solarPanel.attach(servoPin);
    Serial.begin(115200);
    pinMode(LDR_PIN, INPUT);
    WiFi.mode(WIFI_STA);
    ThingSpeak.begin(client);
    wificonnection();
    timeClient.begin();
}
void wificonnection()
{
    Serial.println("Attempting to connect");
      while (WiFi.status() != WL_CONNECTED)
    {
        WiFi.begin(ssid, pass);
        Serial.print(".");
        delay(5000);
    }
}

void channelupdation()
{
    if (statusCode == 200)
    { // successful writing code
        Serial.println("Channel update successful.");
    }
    else
    {
        Serial.println("Problem Writing data. HTTP error code :" +
                       String(statusCode));
    }
}

void loop()
{
    int analogValue = analogRead(36);
    float voltage = analogValue / 4099. * 5;
    float resistance = 2000 * voltage / (1 - voltage / 5);
    float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));

    timeClient.update();

    time_t now = timeClient.getEpochTime();
    setenv("TZ", "Asia/Kolkata", 1);
    tzset();

    struct tm *timeinfo = localtime(&now);
    int hour = timeinfo->tm_hour;
    int minute = timeinfo->tm_min;
    if (hour >= 7 && hour < 11)
    {
        angle = 45;
    }
    else if (hour >= 11 && hour < 14)
    {
        angle = 90;
    }
    else if (hour >= 14 && hour < 17)
    {
        angle = 135;
    }
    else if (hour >= 17 && hour <= 18)
    {
        angle = 179;
    }
    else
    {
        angle = 0;
    }
    solarPanel.write(angle);
    Serial.println("Time:" + String(hour) + ":" + String(minute));

    Serial.println("Angle:" + String(angle));
    Serial.println("Lux: ");
    Serial.print(lux);
    Serial.println("Voltage:" + String(voltage));
    Serial.println("<-------------------------------->");

    ThingSpeak.setField(5, voltage);
    ThingSpeak.setField(1, angle);
    ThingSpeak.setField(4, lux);
    ThingSpeak.setField(2, hour);
    ThingSpeak.setField(3, minute);
    statusCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    channelupdation();
    delay(15000);
}
