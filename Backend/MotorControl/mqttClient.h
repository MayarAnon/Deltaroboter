#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include "MQTTAsync.h"
#include "cJSON.h"
#define PULSE_WIDTH_DEFAULT 5
#define PAUSE_BETWEEN_PULSES_DEFAULT 5
#define DIRECTION_CHANGE_DELAY_DEFAULT 5
#define ADDRESS "tcp://localhost:1883"
#define CLIENTID "MotorControllerClient"
#define TOPIC "motors/sequence"
#define STOP_TOPIC "motors/emergencyStop"
#define QOS 1
void initialize_mqtt();
void onConnect(void* context, MQTTAsync_successData* response);
void onConnectFailure(void* context, MQTTAsync_failureData* response);
int onMessage(void *context, char *topicName, int topicLen, MQTTAsync_message *message);
void connectionLost(void *context, char *cause);
void parse_and_execute_json_sequences(const char *jsonString);
#endif
