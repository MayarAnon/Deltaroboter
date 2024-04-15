#include "config.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Config load_config(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Cannot open config file\n");
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* data = (char*)malloc(length + 1);
    fread(data, 1, length, file);
    fclose(file);
    data[length] = '\0';

    cJSON* json = cJSON_Parse(data);
    free(data);

    Config config;
    cJSON* mqtt = cJSON_GetObjectItemCaseSensitive(json, "mqtt");
    config.address = strdup(cJSON_GetObjectItemCaseSensitive(mqtt, "address")->valuestring);
    config.clientId = strdup(cJSON_GetObjectItemCaseSensitive(mqtt, "clientId")->valuestring);
    config.topic = strdup(cJSON_GetObjectItemCaseSensitive(mqtt, "topic")->valuestring);
    config.stopTopic = strdup(cJSON_GetObjectItemCaseSensitive(mqtt, "stopTopic")->valuestring);
    config.qos = cJSON_GetObjectItemCaseSensitive(mqtt, "qos")->valueint;

    cJSON* defaults = cJSON_GetObjectItemCaseSensitive(json, "defaults");
    config.pulseWidth = cJSON_GetObjectItemCaseSensitive(defaults, "pulseWidth")->valueint;
    config.pauseBetweenPulses = cJSON_GetObjectItemCaseSensitive(defaults, "pauseBetweenPulses")->valueint;
    config.directionChangeDelay = cJSON_GetObjectItemCaseSensitive(defaults, "directionChangeDelay")->valueint;

    cJSON_Delete(json);
    return config;
}
