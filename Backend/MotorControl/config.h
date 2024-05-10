#ifndef CONFIG_H
#define CONFIG_H
// Struktur für Config
typedef struct Config {
    char* address;
    char* clientId;
    char* topic;
    char* stopTopic;
    int qos;
    int pulseWidth;
    int pauseBetweenPulses;
    int directionChangeDelay;
    int* motor_gpios;
    int* dir_gpios;
    int* enb_gpios;
    int motor_count;
} Config;
// Funktion zum Laden der Konfiguration aus einer JSON-Datei
Config load_config(const char* filename);

#endif
