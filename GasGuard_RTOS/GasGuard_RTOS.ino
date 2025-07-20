#define BLYNK_TEMPLATE_ID   "TMPL65b8IezMD"
#define BLYNK_TEMPLATE_NAME "Gas Guard"
#define BLYNK_AUTH_TOKEN    "08S0ukSdChO378ejVJUzPStYeCypqjDk"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <BlynkSimpleEsp32.h>

#define NTP_SERVER          "id.pool.ntp.org"
#define UTC_OFFSET          7 * 3600
#define UTC_OFFSET_DST      0
#define dhtpin              14
#define dhttype             DHT22
#define relay               32
#define MQ2_ANA             34
#define buzzer              18
#define Threshold           30

const char* ssid = "Wokwi-GUEST";
const char* pass = "";

#define VPIN_RELAY    V0
// #define VPIN_BUZZER   V1
#define VPIN_GAS      V1
#define VPIN_TEMP     V2
#define VPIN_HUMI     V3

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(dhtpin, dhttype);
BlynkTimer timer;

SemaphoreHandle_t lcdMutex;
bool relayState = LOW;
bool isWifiConnected = false;
int value = 0;

BLYNK_WRITE(VPIN_RELAY) {
  int value = param.asInt();
  relayState = value;
  digitalWrite(relay, value);
  Serial.print("Relay control received from Blynk. Value: ");
  Serial.println(value ? "ON" : "OFF");
}

// void printLocalTime() {
//   if (xSemaphoreTake(lcdMutex, portMAX_DELAY)) {
//     struct tm timeinfo;
//     if (!getLocalTime(&timeinfo)) {
//       lcd.setCursor(0, 0);
//       lcd.print("Time Error     ");
//       Serial.println("Failed to obtain time");
//     } else {
//       char timeString[20];
//       strftime(timeString, sizeof(timeString), "%H:%M:%S", &timeinfo);
//       lcd.setCursor(0, 0);
//       lcd.print(&timeinfo, "     %H:%M:%S   ");
//       Serial.print("Current time: ");
//       Serial.println(timeString);
//     }
//     xSemaphoreGive(lcdMutex);
//   }
// }

String printLocalTime() {
  struct tm timeinfo;
  char timeString[20];
  
  if (getLocalTime(&timeinfo)) {
    strftime(timeString, sizeof(timeString), "%H:%M:%S", &timeinfo);
    // Serial.print("Current time: ");
    // Serial.println(timeString);
    return String(timeString);
  } else {
    Serial.println("Failed to obtain time");
    return String("Time Error");
  }
}

void connectWiFi() {
  Serial.println("\nAttempting to connect to WiFi...");
  Serial.printf("SSID: %s\n", ssid);
  
  WiFi.begin(ssid, pass);
  int attempts = 0;
  
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    isWifiConnected = true;
    Serial.println("WiFi connected successfully!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal Strength (RSSI): ");
    Serial.println(WiFi.RSSI());
    
    if (xSemaphoreTake(lcdMutex, portMAX_DELAY)) {
      lcd.setCursor(0, 1);
      lcd.print("WiFi Connected ");
      xSemaphoreGive(lcdMutex);
    }
  } else {
    Serial.println("WiFi connection failed!");
    if (xSemaphoreTake(lcdMutex, portMAX_DELAY)) {
      lcd.setCursor(0, 1);
      lcd.print("WiFi Failed    ");
      xSemaphoreGive(lcdMutex);
    }
  }
}

void checkAndControlRelay(float gas_val) {
  static bool hasSentNotification = false;

  if (gas_val > Threshold || relayState) {
    if (!relayState) {
      digitalWrite(relay, HIGH);
      relayState = true;
      value = 1;
      Blynk.virtualWrite(VPIN_RELAY, 1); 
      Serial.println("Relay: ON");
    }
    if (!hasSentNotification) {
      String message = "Warning: Level gas sudah melewati batas! (" + String(gas_val, 1) + "%)";
      Blynk.logEvent("alarm_gas", message);
      Serial.println("Notification sent to Blynk: " + message);
      hasSentNotification = true;
    }
    tone(buzzer, 2000, 100);
  } else {
    if (relayState) {
      digitalWrite(relay, LOW);
      relayState = false;
      Blynk.virtualWrite(VPIN_RELAY, 0); 
      Serial.println("Relay: OFF");
    }
    noTone(buzzer);
    hasSentNotification = false;
  }
}

void sensorTask(void *pvParameters) {
  const TickType_t xDelay = pdMS_TO_TICKS(1000);
  Serial.println("Sensor task started");

  int lcdState = 0;

  while (true) {
    float MQ2_value = analogRead(MQ2_ANA);
    float temp = dht.readTemperature();
    float humi = dht.readHumidity();
    float gas_val = MQ2_value * 100 / 4095;

    String currentTime = printLocalTime();

    Serial.println("\n----- Sensor Readings -----");
    Serial.printf("Waktu      : %s\n", currentTime.c_str());
    Serial.printf("Suhu       : %.2f°C\n", temp);
    Serial.printf("Kelembapan : %.2f%%\n", humi);
    Serial.printf("Gas value  : %.2f%%\n", gas_val);
    Serial.println("--------------------------");

    // if (!isnan(temp) && !isnan(humi) && isWifiConnected) {
    //   if (gas_val >= Threshold || ) {
    //     digitalWrite(relay, LOW);
    //     tone(buzzer, 2000, 100);
    //     Blynk.virtualWrite(VPIN_RELAY, 1);
    //     // Blynk.virtualWrite(VPIN_BUZZER, 1);
    //   } else {
    //     digitalWrite(relay, HIGH);
    //     noTone(buzzer);
    //     Blynk.virtualWrite(VPIN_RELAY, 0);
    //     // Blynk.virtualWrite(VPIN_BUZZER, 0);
    //   }
    checkAndControlRelay(gas_val);

    if (xSemaphoreTake(lcdMutex, portMAX_DELAY)) {
        lcd.clear();
        if (lcdState == 0) {
          lcd.setCursor(0, 0);
          lcd.print(currentTime);
          lcd.setCursor(0, 1);
          lcd.print("Suhu   : ");
          lcd.print(temp, 1);
          lcd.print(" C");
        } else if (lcdState == 1) {
          lcd.setCursor(0, 0);
          lcd.print("Humid. : ");
          lcd.print(humi, 1);
          lcd.print(" %");
          lcd.setCursor(0, 1);
          lcd.print("Gas    : ");
          lcd.print(gas_val, 1);
          lcd.print(" %");
        }
      lcdState = (lcdState + 1) % 2;
      xSemaphoreGive(lcdMutex);
    }
    if (!isnan(temp) && !isnan(humi) && isWifiConnected) {
      Blynk.virtualWrite(VPIN_TEMP, temp);
      Blynk.virtualWrite(VPIN_HUMI, humi);
      Blynk.virtualWrite(VPIN_GAS, gas_val);
    } else {
      Serial.println("Error: Invalid sensor readings or WiFi disconnected");
    }
    vTaskDelay(xDelay);
  }
}

void timeTask(void *pvParameters) {
  const TickType_t xDelay = pdMS_TO_TICKS(1000);
  Serial.println("Time task started");
  
  while (true) {
    if (isWifiConnected) {
      printLocalTime();
    } else {
      Serial.println("Time task: WiFi not connected");
    }
    vTaskDelay(xDelay);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n----- Gas Guard System Starting -----");
  Serial.println("Initializing components...");
  
  lcdMutex = xSemaphoreCreateMutex();
  if (lcdMutex == NULL) {
    Serial.println("Error: Failed to create LCD mutex!");
  }
  
  Serial.println("Initializing LCD...");
  lcd.init();
  lcd.backlight();
  
  Serial.println("Initializing DHT sensor...");
  dht.begin();
  
  Serial.println("Setting up GPIO pins...");
  pinMode(MQ2_ANA, INPUT);
  // pinMode(MQ2_DIG, INPUT);
  pinMode(relay, OUTPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(relay, HIGH);
  
  Serial.println("Hardware initialization complete");

  if (xSemaphoreTake(lcdMutex, portMAX_DELAY)) {
    lcd.setCursor(0, 0);
    lcd.print("    Gas Guard   ");
    lcd.setCursor(0, 1);
    lcd.print(" Initializing...");
    xSemaphoreGive(lcdMutex);
  }
  delay(2000);

  connectWiFi();

  if (isWifiConnected) {
    Serial.println("Configuring time...");
    configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
    
    Serial.println("Connecting to Blynk...");
    Blynk.config(BLYNK_AUTH_TOKEN);
    if (!Blynk.connect()) {
      Serial.println("Error: Blynk connection failed!");
    } else {
      Serial.println("Blynk connected successfully!");
    }
  }

  Serial.println("Creating tasks...");
  BaseType_t sensorTaskCreated = xTaskCreatePinnedToCore(
    sensorTask,
    "Sensor Task",
    4096,
    NULL,
    2,
    NULL,
    1
  );
  
  if (sensorTaskCreated != pdPASS) {
    Serial.println("Error: Failed to create sensor task!");
  }

  BaseType_t timeTaskCreated = xTaskCreatePinnedToCore(
    timeTask,
    "Time Task",
    2048,
    NULL,
    1,
    NULL,
    1
  );
  
  if (timeTaskCreated != pdPASS) {
    Serial.println("Error: Failed to create time task!");
  }
  Serial.println("Setup complete!\n");
}

void loop() {
  if (isWifiConnected) {
    Blynk.run();
  } else {
    static unsigned long lastAttempt = 0;
    if (millis() - lastAttempt > 30000) {
      Serial.println("\nAttempting to reconnect WiFi...");
      connectWiFi();
      lastAttempt = millis();
    }
  }
  delay(10);
}
