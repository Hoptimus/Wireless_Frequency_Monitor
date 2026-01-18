#include <LiquidCrystal_I2C.h>
#include "arduinoFFT.h"
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>

// Mic pins for ESP32 ADC
#define MIC_1 35
#define MIC_2 34
#define MIC_3 39
#define MIC_4 36

#define LED_1 15  // Status PWM LED

// FFT Config
#define SAMPLES 512
#define SAMPLING_FREQUENCY 8000 // Hz

ArduinoFFT<double> FFT = ArduinoFFT<double>();

double vReal[SAMPLES];
double vImag[SAMPLES];

LiquidCrystal_I2C lcd(0x27, 20, 4);

// MAC address of master ESP32 receiving data
uint8_t masterAddress[] = {0xC4, 0xD8, 0xD5, 0x3C, 0xA6, 0x52};

// Struct to send over ESP-NOW
typedef struct struct_message {
  int deviceID;
  double Freq_1;  float dB_1;
  double Freq_2;  float dB_2;
  double Freq_3;  float dB_3;
  double Freq_4;  float dB_4;
} struct_message;

struct_message myData;

// Callback on send
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    ledcWrite(0, 255); // LED ON for success
  } else {
    ledcWrite(0, 0);   // LED OFF for failure
  }
}

// Struct to return both Frequency & dB
struct FreqDbResult {
  double freq;  double dB;
};

// Convert peak amplitude to decibels
float peakToDb(double rms) {
  return 20.0 * log10(rms + 1e-6);
}

// Measure for one mic
FreqDbResult readFrequencyForMic(int micPin) {
  double sumSq = 0;

  // Sample the signal
  for (int i = 0; i < SAMPLES; i++) {
    unsigned long microsStart = micros();
    double val = analogRead(micPin);
    sumSq += val * val;
    vReal[i] = val;
    vImag[i] = 0;
    while (micros() - microsStart < (1000000 / SAMPLING_FREQUENCY)) {
      // Wait for correct sample time
    }
  }

  // RMS then dB
  double rms = sqrt(sumSq / SAMPLES);
  double dB = peakToDb(rms);

  // Perform FFT
  FFT.windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.complexToMagnitude(vReal, vImag, SAMPLES);

  // Find peak frequency
  double peakFreq = FFT.majorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);

  return { peakFreq, dB };
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  // PWM LED setup (5000 Hz, 8-bit)
  ledcSetup(0, 5000, 8);
  ledcAttachPin(LED_1, 0);

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Audio Freq Monitor");

  // WiFi STA mode required for ESP-NOW
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    lcd.setCursor(0, 1); lcd.print("ESP-NOW Init Fail");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, masterAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    lcd.setCursor(0, 2); lcd.print("Failed to add peer");
    return;
  }
}

void loop() {
  FreqDbResult freq1 = readFrequencyForMic(MIC_1);
  FreqDbResult freq2 = readFrequencyForMic(MIC_2);
  FreqDbResult freq3 = readFrequencyForMic(MIC_3);
  FreqDbResult freq4 = readFrequencyForMic(MIC_4);

  // Display on LCD
  lcd.setCursor(0, 0); lcd.print("M1:"); lcd.print(freq1.freq, 1); lcd.print("Hz  ");
  lcd.setCursor(0, 1); lcd.print("M2:"); lcd.print(freq2.freq, 1); lcd.print("Hz  ");
  lcd.setCursor(0, 2); lcd.print("M3:"); lcd.print(freq3.freq, 1); lcd.print("Hz  ");
  lcd.setCursor(0, 3); lcd.print("M4:"); lcd.print(freq4.freq, 1); lcd.print("Hz  ");

  // Prepare data
  myData.deviceID = 1;
  myData.Freq_1 = freq1.freq;
  myData.dB_1 = freq1.dB;
  myData.Freq_2 = freq2.freq;
  myData.dB_2 = freq2.dB;
  myData.Freq_3 = freq3.freq;
  myData.dB_3 = freq3.dB;
  myData.Freq_4 = freq4.freq;
  myData.dB_4 = freq4.dB;

  // Send via ESP-NOW
  esp_err_t result = esp_now_send(masterAddress, (uint8_t *) &myData, sizeof(myData));
  if (result != ESP_OK) {
    Serial.println("Error sending data");
    ledcWrite(0, 0); // Off on failure
  }

  delay(50); // Small gap before the next cycle
}
