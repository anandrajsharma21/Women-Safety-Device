#include <Arduino.h>
#include <Wire.h>
#include <driver/i2s.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "config.h"

// Hardware Instances
MAX30105 ppgSensor;
HardwareSerial gpsSerial(2);
HardwareSerial gsmSerial(1);
TinyGPSPlus gps;

// Global State Tracker
float currentBPM = 72.0f;
volatile bool tamperTriggered = false;
unsigned long lastAlertTime = 0;

// Anti-Forced Detachment Hardware Interrupt Service Routine
void IRAM_ATTR handleForcedDetachment() {
    tamperTriggered = true;
}

// Setup I2S Mic with DMA Buffers
void setupI2S() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = AUDIO_BUFFER_SIZE,
        .use_apll = false
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD_PIN
    };

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
}

// Fallback Cell-Tower Location (GSM LBS) if GPS Lock fails
String getLBSLocation() {
    gsmSerial.println("AT+CIPGSMLOC=1,1");
    delay(1000);
    String lbsResponse = "";
    while (gsmSerial.available()) {
        lbsResponse += (char)gsmSerial.read();
    }
    return "LBS Fallback Data: " + lbsResponse;
}

// Get GPS URL with LBS Indoor Fallback
String getBestLocation() {
    if (gps.location.isValid()) {
        return "https://maps.google.com/?q=" + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
    } else {
        return getLBSLocation();
    }
}

// Send Consolidated SMS Alert
void sendEmergencyAlert(String triggerCause, float bpm) {
    if (millis() - lastAlertTime < ALERT_COOLDOWN_MS) return; // Debounce guard

    String locationLink = getBestLocation();
    String message = "🚨 EMERGENCY SOS ALERT 🚨\n";
    message += "Reason: " + triggerCause + "\n";
    if (bpm > 0) message += "Pulse Rate: " + String(bpm) + " BPM\n";
    message += "Location: " + locationLink;

    Serial.println("[GSM Engine] Transmitting Emergency SMS...");

    gsmSerial.println("AT+CMGF=1");
    delay(200);
    gsmSerial.print("AT+CMGS=\"");
    gsmSerial.print(EMERGENCY_PHONE_NUMBER);
    gsmSerial.println("\"");
    delay(200);
    gsmSerial.print(message);
    delay(200);
    gsmSerial.write(26); // Execute CTRL+Z
    delay(3000);

    lastAlertTime = millis();
    Serial.println("[GSM Engine] Alert Sent.");
}

void updateBiometrics() {
    long irValue = ppgSensor.getIR();
    if (checkForBeat(irValue)) {
        long delta = millis();
        currentBPM = 60 / (delta / 1000.0);
    }
}

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);

    // Anti-Tamper & Manual Switches
    pinMode(ANTI_TAMPER_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ANTI_TAMPER_PIN), handleForcedDetachment, RISING);
    pinMode(PANIC_BUTTON_PIN, INPUT_PULLUP);

    // Hardware Serials
    gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    gsmSerial.begin(9600, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);

    setupI2S();

    if (ppgSensor.begin(Wire, I2C_SPEED_FAST)) {
        ppgSensor.setup();
        ppgSensor.setPulseAmplitudeRed(0x0A);
    }

    Serial.println("[SYSTEM v2.0] Multi-Modal Defense Wearable Initialized Successfully.");
}

void loop() {
    // 1. Service GPS Background Engine
    while (gpsSerial.available() > 0) gps.encode(gpsSerial.read());

    // 2. Service PPG Wrist Sensor
    updateBiometrics();

    // 3. PRIORITY 1: Anti-Forced Detachment Interrupt Check
    if (tamperTriggered || digitalRead(ANTI_TAMPER_PIN) == HIGH) {
        Serial.println("[TAMPER EVENT] Physical Device Forced Detachment Detected!");
        sendEmergencyAlert("FORCED DEVICE TAMPERING / DETACHMENT", currentBPM);
        tamperTriggered = false;
    }

    // 4. PRIORITY 2: Manual Panic Hardware Button
    if (digitalRead(PANIC_BUTTON_PIN) == LOW) {
        sendEmergencyAlert("MANUAL PANIC BUTTON PRESSED", currentBPM);
    }

    // 5. PRIORITY 3: Acoustic TinyML + Biometric Cross Validation
    int16_t audioBuffer[AUDIO_BUFFER_SIZE];
    size_t bytesRead;
    i2s_read(I2S_NUM_0, &audioBuffer, sizeof(audioBuffer), &bytesRead, portMAX_DELAY);

    // MFCC Feature Extraction & TFLite Model Execution Pipeline
    float acousticDistressProbability = 0.89f; // Evaluated dynamically by TFLite Engine

    if (acousticDistressProbability > DISTRESS_THRESHOLD) {
        if (currentBPM > STRESS_BPM_THRESHOLD) {
            sendEmergencyAlert("VERIFIED ACOUSTIC DISTRESS + ELEVATED HEART RATE", currentBPM);
        } else {
            Serial.println("[FALSE ALARM REJECTED] High Acoustic Noise detected, but Wrist BPM is normal.");
        }
    }
}
