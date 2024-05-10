#include "config.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
Config load_config(const char* filename) {
    // Datei öffnen
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Cannot open config file\n");
        exit(1);
    }
    // Dateigröße ermitteln
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    // Speicher für die Daten reservieren und Dateiinhalt lesen
    char* data = (char*)malloc(length + 1);
    fread(data, 1, length, file);
    fclose(file);
    data[length] = '\0';
    // JSON-Daten parsen
    cJSON* json = cJSON_Parse(data);
    free(data);
    // Konfigurationsstruktur initialisieren
    Config config;

    // MQTT-Einstellungen aus JSON extrahieren
    cJSON* mqtt = cJSON_GetObjectItemCaseSensitive(json, "mqtt");
    config.address = strdup(cJSON_GetObjectItemCaseSensitive(mqtt, "address")->valuestring);
    config.clientId = strdup(cJSON_GetObjectItemCaseSensitive(mqtt, "clientId")->valuestring);
    config.topic = strdup(cJSON_GetObjectItemCaseSensitive(mqtt, "topic")->valuestring);
    config.stopTopic = strdup(cJSON_GetObjectItemCaseSensitive(mqtt, "stopTopic")->valuestring);
    config.qos = cJSON_GetObjectItemCaseSensitive(mqtt, "qos")->valueint;

    // Standardwerte aus JSON extrahieren
    cJSON* defaults = cJSON_GetObjectItemCaseSensitive(json, "defaults");
    config.pulseWidth = cJSON_GetObjectItemCaseSensitive(defaults, "pulseWidth")->valueint;
    config.pauseBetweenPulses = cJSON_GetObjectItemCaseSensitive(defaults, "pauseBetweenPulses")->valueint;
    config.directionChangeDelay = cJSON_GetObjectItemCaseSensitive(defaults, "directionChangeDelay")->valueint;

     // Motoranzahl aus JSON extrahieren (falls vorhanden, ansonsten Standardwert 3 verwenden)
    cJSON* motor_count_json = cJSON_GetObjectItemCaseSensitive(json, "motor_count");
    config.motor_count = motor_count_json ? motor_count_json->valueint : 3;  // Default auf 3
    // Speicher für GPIO-Pins reservieren
    config.motor_gpios = malloc(sizeof(int) * config.motor_count);
    config.dir_gpios = malloc(sizeof(int) * config.motor_count);
    config.enb_gpios = malloc(sizeof(int) * config.motor_count);
     // GPIO-Einstellungen aus JSON extrahieren
    cJSON* gpio = cJSON_GetObjectItemCaseSensitive(json, "gpio");
    for (int i = 0; i < config.motor_count; i++) {
        config.motor_gpios[i] = cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(gpio, "motor_gpios"), i)->valueint;
        config.dir_gpios[i] = cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(gpio, "dir_gpios"), i)->valueint;
        config.enb_gpios[i] = cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(gpio, "enb_gpios"), i)->valueint;
    }
    // JSON-Objekt freigeben und Konfiguration zurückgeben
    cJSON_Delete(json);
    return config;
}
