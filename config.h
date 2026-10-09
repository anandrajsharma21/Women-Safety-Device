#ifndef CONFIG_H
#define CONFIG_H

// ==========================================
// Emergency Communication Configurations
// ==========================================
#define EMERGENCY_PHONE_NUMBER "+515909190890"
#define APN_NAME "www" // Local Telecom APN for Cell-Tower LBS

// ==========================================
// Hardware Pin Allocations
// ==========================================
#define I2S_WS_PIN         25
#define I2S_SCK_PIN        33
#define I2S_SD_PIN         32
#define GPS_RX_PIN         16
#define GPS_TX_PIN         17
#define GSM_RX_PIN         26
#define GSM_TX_PIN         27
#define PANIC_BUTTON_PIN   4
#define ANTI_TAMPER_PIN    5

// ==========================================
// Audio & MFCC Feature Pipeline Parameters
// ==========================================
#define SAMPLE_RATE        16000
#define AUDIO_BUFFER_SIZE  512
#define MFCC_NUM_BANKS     13
#define DISTRESS_THRESHOLD 0.85f  // 85% Model confidence score threshold

// ==========================================
// Biometric & Hysteresis Thresholds
// ==========================================
#define STRESS_BPM_THRESHOLD 110.0f
#define ALERT_COOLDOWN_MS    15000  // 15 seconds lockout to prevent duplicate SMS

#endif
