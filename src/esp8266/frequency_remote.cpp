#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
}

#define LED_1 15
#define LED_2 13
#define LED_3 12
#define LED_4 14

// Define the struct for mic data (must match sender)
typedef struct struct_message {
  int deviceID;
  double Freq_1;  float dB_1;
  double Freq_2;  float dB_2;
  double Freq_3;  float dB_3;
  double Freq_4;  float dB_4;
} struct_message;

struct_message myData;

LiquidCrystal_I2C lcd(0x27, 20, 4);

// To print MAC address nicely
void printMAC(const uint8_t *mac) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr),
           "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print(macStr);
}

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print(myData.deviceID);
  Serial.print(",");
  printMAC(mac);
  Serial.print(" || Mic_1 Freq: ");
  Serial.print(myData.Freq_1);    Serial.print(",");    Serial.print(myData.dB_1);
  Serial.print(" || Mic_2 Freq: ");
  Serial.print(myData.Freq_2);    Serial.print(",");    Serial.print(myData.dB_2);
  Serial.print(" || Mic_3 Freq: ");
  Serial.print(myData.Freq_3);    Serial.print(",");    Serial.print(myData.dB_3);
  Serial.print(" || Mic_4 Freq: ");
  Serial.print(myData.Freq_4);    Serial.print(",");    Serial.print(myData.dB_4);
  Serial.println();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Device: "); lcd.print(myData.deviceID);
  lcd.setCursor(0, 1);
  lcd.print("M1:"); lcd.print(myData.dB_1, 1); lcd.print(","); lcd.print(myData.Freq_1, 1);
  lcd.setCursor(0, 2);
  lcd.print("M2:"); lcd.print(myData.dB_2, 1); lcd.print(","); lcd.print(myData.Freq_2, 1);
  lcd.setCursor(0, 3);
  lcd.print("M3:"); lcd.print(myData.dB_3, 1); lcd.print(","); lcd.print(myData.Freq_3, 1);

  // For 20x4 LCD, use scrolling or trim display if you want all 4 lines
  // Here, using Serial for M4:
  // Print M4 info to Serial as well
  Serial.print("M4: "); Serial.print(myData.dB_4, 1); Serial.print(", "); Serial.println(myData.Freq_4, 1);

  // LED logic for loudest mic
  if (myData.dB_1 > myData.dB_2 && myData.dB_1 > myData.dB_3 && myData.dB_1 > myData.dB_4) {
    digitalWrite(LED_1, HIGH); digitalWrite(LED_2, LOW); digitalWrite(LED_3, LOW); digitalWrite(LED_4, LOW);
  } else if (myData.dB_2 > myData.dB_1 && myData.dB_2 > myData.dB_3 && myData.dB_2 > myData.dB_4) {
    digitalWrite(LED_1, LOW); digitalWrite(LED_2, HIGH); digitalWrite(LED_3, LOW); digitalWrite(LED_4, LOW);
  } else if (myData.dB_3 > myData.dB_1 && myData.dB_3 > myData.dB_2 && myData.dB_3 > myData.dB_4) {
    digitalWrite(LED_1, LOW); digitalWrite(LED_2, LOW); digitalWrite(LED_3, HIGH); digitalWrite(LED_4, LOW);
  } else if (myData.dB_4 > myData.dB_1 && myData.dB_4 > myData.dB_2 && myData.dB_4 > myData.dB_3) {
    digitalWrite(LED_1, LOW); digitalWrite(LED_2, LOW); digitalWrite(LED_3, LOW); digitalWrite(LED_4, HIGH);
  } else {
    digitalWrite(LED_1, LOW); digitalWrite(LED_2, LOW); digitalWrite(LED_3, LOW); digitalWrite(LED_4, LOW);
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  pinMode(LED_1, OUTPUT); pinMode(LED_2, OUTPUT); pinMode(LED_3, OUTPUT); pinMode(LED_4, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("ESP-NOW Receiver");

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    lcd.setCursor(0, 1); lcd.print("ESP-NOW Init Fail");
    while (true) { delay(1000); }
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO); // Accept connection from any peer

  esp_now_register_recv_cb(OnDataRecv);

  lcd.setCursor(0, 1); lcd.print("ESP-NOW Ready");
  delay(1000);
  lcd.clear();
}

void loop() {
  // Nothing needed here, all work is interrupt-driven by ESP-NOW callback
}
