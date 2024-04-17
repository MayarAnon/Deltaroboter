#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include "config.h"
#include "MQTTAsync.h"
#include "cJSON.h"
#include "queue.h"
extern MQTTAsync client;
extern Config globalConfig;
extern Queue messageQueue;
void initialize_mqtt(); 
void onConnect(void* context, MQTTAsync_successData* response);
void onConnectFailure(void* context, MQTTAsync_failureData* response);
int onMessage(void *context, char *topicName, int topicLen, MQTTAsync_message *message);
void connectionLost(void *context, char *cause);
void parse_and_execute_json_sequences(const char *jsonString);
#endif