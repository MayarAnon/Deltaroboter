#ifndef CONFIG_H
#define CONFIG_H

typedef struct Config {
    char* address;
    char* clientId;
    char* topic;
    char* stopTopic;
    int qos;
    int pulseWidth;
    int pauseBetweenPulses;
    int directionChangeDelay;
} Config;

Config load_config(const char* filename);

#endif
