#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <DHT.h>

// 🔹 WiFi & Firebase Credentials
#define WIFI_SSID "Redmi1"
#define WIFI_PASSWORD "26262626"

#define FIREBASE_HOST "soilmoisture-3abe7-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "V8HSNOkBU03Y5jlgjmVIl8MOSZUNNssydm6xIrFr"

// 🔹 Firebase Setup
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

// 🔹 DHT22 Setup (Temperature & Humidity)
#define DHTPIN 2       // GPIO2 (D4)
#define DHTTYPE DHT22   
DHT dht(DHTPIN, DHTTYPE);

// 🔹 Analog Sensors using Time-Division Multiplexing (TDM)
#define SOIL_MOISTURE_POWER 14  // GPIO14 (VCC Control for Soil Sensor)
#define RAIN_DROP_POWER 12     // GPIO12 (VCC Control for Rain Sensor)
#define SENSOR_PIN A0           // Shared Analog Input

void setup() {
  Serial.begin(115200);

  // 🔹 Setup WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnected to WiFi!");

  // 🔹 Configure Firebase
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // 🔹 Sensor Initialization
  dht.begin();
  pinMode(SOIL_MOISTURE_POWER, OUTPUT);
  pinMode(RAIN_DROP_POWER, OUTPUT);

  digitalWrite(SOIL_MOISTURE_POWER, HIGH);
  digitalWrite(RAIN_DROP_POWER, HIGH);
}

void loop() {
  // Read DHT22 (Temperature & Humidity)
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Read Soil Moisture Sensor
  int soilMoisture = readSoilMoisture();

  // Read Rain Drop Sensor
  int rainDrop = readRainDrop();

  // Print values to Serial Monitor
  Serial.print("Temperature: "); Serial.print(temperature); Serial.println("°C");
  Serial.print("Humidity: "); Serial.print(humidity); Serial.println("%");
  Serial.print("Soil Moisture: "); Serial.println(soilMoisture);
  Serial.print("Rain Drop: "); Serial.println(rainDrop);

  // Generate a Unique Timestamp for Each Record
  String timestamp = String(millis());

  // Firebase Path: Storing as a Unique Entry
  String path = "/SmartIrrigation/" + timestamp;  

  // 🔹 Send Data to Firebase
  if (Firebase.setFloat(firebaseData, path + "/Temperature", temperature)) {
    Serial.println("Temperature sent to Firebase!");
  } else {
    Serial.print("Failed to send Temperature: ");
    Serial.println(firebaseData.errorReason());
  }
  
  if (Firebase.setFloat(firebaseData, path + "/Humidity", humidity)) {
    Serial.println("Humidity sent to Firebase!");
  } else {
    Serial.print("Failed to send Humidity: ");
    Serial.println(firebaseData.errorReason());
  }
  
  if (Firebase.setInt(firebaseData, path + "/SoilMoisture", soilMoisture)) {
    Serial.println("Soil Moisture sent to Firebase!");
  } else {
    Serial.print("Failed to send Soil Moisture: ");
    Serial.println(firebaseData.errorReason());
  }

  if (Firebase.setInt(firebaseData, path + "/RainDrop", rainDrop)) {
    Serial.println("Rain Drop sent to Firebase!");
  } else {
    Serial.print("Failed to send Rain Drop: ");
    Serial.println(firebaseData.errorReason());
  }

  delay(5000); // Wait for 5 seconds before next reading
}

// 🔹 Function to Read Soil Moisture Using GPIO Switching
int readSoilMoisture() {
    digitalWrite(SOIL_MOISTURE_POWER, LOW);  // Turn ON Soil Sensor
    delay(100);  // Wait for stable reading
    int value = analogRead(SENSOR_PIN);
    digitalWrite(SOIL_MOISTURE_POWER, HIGH);  // Turn OFF Soil Sensor
    return value;
}

int readRainDrop() {
    digitalWrite(RAIN_DROP_POWER, LOW);  // Turn ON Rain Sensor
    delay(100);  // Wait for stable reading
    int value = analogRead(SENSOR_PIN);
    digitalWrite(RAIN_DROP_POWER, HIGH);  // Turn OFF Rain Sensor
    return value;
}
