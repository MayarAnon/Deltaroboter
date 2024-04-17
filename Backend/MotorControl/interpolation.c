// gcc -o interpolation interpolation.c -I/usr/local/include/cjson -L/usr/local/lib/cjson -lpigpio -lrt -pthread -lpaho-mqtt3as -lcjson
// Beispiel nachricht: // mosquitto_pub -t motors/sequence -m "[{ \"motorpulses\": [500, 100, 100], \"timing\":[70,70,5] },{ \"motorpulses\": [-500, 100, 100], \"timing\":[30,30,5] },{ \"motorpulses\": [5000, 100, 100], \"timing\":[70,70,5] }]"
// mosquitto_pub -t motors/emergencyStop -m "true"
/**
 * Programm zur präzise Echtzeit Steuerung und Interpolierung der Motoren über MQTT mit dem Raspberry Pi.
 * Es nutzt die pigpio-Bibliothek zur Ansteuerung der Motoren und cJSON für die Verarbeitung von JSON-Nachrichten.
 * MQTT wird verwendet, um Befehle zu empfangen und zu verarbeiten, die die Motorsteuerungssequenzen definieren.
 */

#include <pigpio.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTAsync.h"
#include "cJSON.h"
#include "signal.h"
#include <pthread.h>
#include <semaphore.h>
#define MOTOR_COUNT 3
#define ADDRESS "tcp://localhost:1883"
#define CLIENTID "MotorControllerClient"
#define TOPIC "motors/sequence"
#define STOP_TOPIC "motors/emergencyStop"
#define QOS 1

int motor_gpios[MOTOR_COUNT] = {17, 27, 22};
int dir_gpios[MOTOR_COUNT] = {2, 3, 4};
int enb_gpios[MOTOR_COUNT] = {14, 15, 18};
int pulseWidthDefault = 5;
int pauseBetweenPulsesDefault = 5;
int directionChangeDelayDefault = 5;
volatile sig_atomic_t emergency_stop_triggered = 0;
MQTTAsync client;
typedef struct node {
    char* data;
    struct node* next;
} Node;

typedef struct {
    Node* head;
    Node* tail;
    pthread_mutex_t lock;
} Queue;
// Prototypen

void initQueue(Queue* q);
void enqueue(Queue* q, char* data);
char* dequeue(Queue* q);
void* sequence_worker_thread(void* arg);
void initialize_motors();
void parse_and_execute_json_sequences(const char *jsonString);
void execute_interpolated_sequence(int pulses[MOTOR_COUNT], int pulseWidthUs, int pauseBetweenPulsesUs, int directionChangeDelayUs);
void initialize_mqtt();
void onConnect(void *context, MQTTAsync_successData *response);
void onConnectFailure(void *context, MQTTAsync_failureData *response);
int onMessage(void *context, char *topicName, int topicLen, MQTTAsync_message *message);
void connectionLost(void *context, char *cause);
void *message_processing_thread(void *arg);
void trigger_emergency_stop();




Queue messageQueue;
sem_t queueSemaphore;

// Initialisiert die Warteschlange
void initQueue(Queue* q) {
    q->head = NULL;
    q->tail = NULL;
    pthread_mutex_init(&q->lock, NULL);
}

// Fügt Elemente zur Warteschlange hinzu
void enqueue(Queue* q, char* data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        perror("Failed to allocate node");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    newNode->next = NULL;

    pthread_mutex_lock(&q->lock);
    if (q->tail != NULL) {
        q->tail->next = newNode;
    }
    q->tail = newNode;
    if (q->head == NULL) {
        q->head = newNode;
    }
    pthread_mutex_unlock(&q->lock);
    sem_post(&queueSemaphore);
}

// Entfernt und gibt das erste Element der Warteschlange zurück
char* dequeue(Queue* q) {
    sem_wait(&queueSemaphore);
    pthread_mutex_lock(&q->lock);
    if (q->head == NULL) {
        pthread_mutex_unlock(&q->lock);
        return NULL; // Warteschlange ist leer
    }
    Node* temp = q->head;
    char* data = temp->data;
    q->head = q->head->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    pthread_mutex_unlock(&q->lock);
    free(temp);
    return data;
}

// Worker-Thread-Funktion
void* sequence_worker_thread(void* arg) {
    while (1) {
        char* payloadStr = dequeue(&messageQueue);
        parse_and_execute_json_sequences(payloadStr);
        free(payloadStr);
    }
    return NULL;
}
void *message_processing_thread(void *arg)
{
    char *payloadStr = (char *)arg;
    parse_and_execute_json_sequences(payloadStr);
    free(payloadStr);
    return NULL;
}

void initialize_motors()
{
    if (gpioInitialise() < 0)
    {
        fprintf(stderr, "Pigpio initialization failed\n");
        exit(1);
    }

    for (int i = 0; i < MOTOR_COUNT; i++)
    {
        gpioSetMode(motor_gpios[i], PI_OUTPUT);
        gpioSetMode(dir_gpios[i], PI_OUTPUT);
        gpioSetMode(enb_gpios[i], PI_OUTPUT);
        gpioWrite(enb_gpios[i], 1); // Enable motors
    }
}
void parse_and_execute_json_sequences(const char *jsonString)
{
    cJSON *json = cJSON_Parse(jsonString);
    if (!json)
    {
        fprintf(stderr, "Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *sequence = NULL;
    cJSON_ArrayForEach(sequence, json)
    {
        cJSON *motors = cJSON_GetObjectItemCaseSensitive(sequence, "motorpulses");
        cJSON *timing = cJSON_GetObjectItemCaseSensitive(sequence, "timing");

        int pulses[MOTOR_COUNT];
        for (int i = 0; i < cJSON_GetArraySize(motors); i++)
        {
            pulses[i] = cJSON_GetArrayItem(motors, i)->valueint;
        }

        int sequencePulseWidth = (timing && cJSON_GetArraySize(timing) > 0) ? cJSON_GetArrayItem(timing, 0)->valueint : pulseWidthDefault;
        int sequencePauseBetweenPulses = (timing && cJSON_GetArraySize(timing) > 1) ? cJSON_GetArrayItem(timing, 1)->valueint : pauseBetweenPulsesDefault;
        int sequenceDirectionChangeDelay = (timing && cJSON_GetArraySize(timing) > 2) ? cJSON_GetArrayItem(timing, 2)->valueint : directionChangeDelayDefault;

        execute_interpolated_sequence(pulses, sequencePulseWidth, sequencePauseBetweenPulses, sequenceDirectionChangeDelay);
    }
    cJSON_Delete(json);
}

void execute_interpolated_sequence(int pulses[MOTOR_COUNT], int pulseWidthUs, int pauseBetweenPulsesUs, int directionChangeDelayUs)
{
    /**
     * Führt eine interpolierte Motorsequenz basierend auf den gegebenen Pulsen und Timing-Parametern aus.
     */
    gpioWaveClear();
    for (int i = 0; i < MOTOR_COUNT; ++i)
    {
        gpioWrite(enb_gpios[i], 1); // Motoren Aktivieren
    }
    int maxPulses = 0;
    for (int i = 0; i < MOTOR_COUNT; ++i)
    {
        maxPulses = (abs(pulses[i]) > maxPulses) ? abs(pulses[i]) : maxPulses;
    }

    gpioPulse_t combinedPulses[(maxPulses + 2) * MOTOR_COUNT * 2];
    int pulseIndex = 0;

    // Zu Beginn der Sequenz: Richtungsänderung einfügen, falls notwendig, mit einer Verzögerung vor dem ersten Puls
    for (int i = 0; i < MOTOR_COUNT; ++i)
    {
        if (pulses[i] < 0)
        { // Nur wenn die Pulse negativ sind, setze den dir_GPIO auf HIGH
            combinedPulses[pulseIndex++] = (gpioPulse_t){
                .gpioOn = (1 << dir_gpios[i]),
                .gpioOff = 0,
                .usDelay = directionChangeDelayUs // Verzögerung für den Richtungswechsel aus Parameter
            };
        }
    }

    // Generiere Pulse für jeden Motor
    for (int pulseNum = 0; pulseNum < maxPulses; ++pulseNum)
    {
        unsigned int bitsOn = 0, bitsOff = 0;
        for (int motorIndex = 0; motorIndex < MOTOR_COUNT; ++motorIndex)
        {
            if (pulseNum < abs(pulses[motorIndex]))
            {
                bitsOn |= (1 << motor_gpios[motorIndex]);
            }
            else
            {
                bitsOff |= (1 << motor_gpios[motorIndex]);
            }
        }
        combinedPulses[pulseIndex++] = (gpioPulse_t){.gpioOn = bitsOn, .gpioOff = bitsOff, .usDelay = pulseWidthUs};
        combinedPulses[pulseIndex++] = (gpioPulse_t){.gpioOn = 0, .gpioOff = bitsOn, .usDelay = pauseBetweenPulsesUs};
    }

    // Richtungsänderungen am Ende der Sequenz einfügen, falls notwendig
    for (int i = 0; i < MOTOR_COUNT; ++i)
    {
        if (pulses[i] < 0)
        { // Setze den dir_GPIO zurück auf LOW nach negativen Pulsen
            combinedPulses[pulseIndex++] = (gpioPulse_t){
                .gpioOn = 0,
                .gpioOff = (1 << dir_gpios[i]),
                .usDelay = directionChangeDelayUs};
        }
    }

    gpioWaveAddGeneric(pulseIndex, combinedPulses);
    int wave_id = gpioWaveCreate();
    if (wave_id >= 0) {
        gpioWaveTxSend(wave_id, PI_WAVE_MODE_ONE_SHOT);
        while (gpioWaveTxBusy()) usleep(10);
        gpioWaveDelete(wave_id);
    } else {
        fprintf(stderr, "Fehler beim Erzeugen der Waveform\n");
    }
}

void initialize_mqtt()
{
    MQTTAsync_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTAsync_setCallbacks(client, NULL, connectionLost, onMessage, NULL);

    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.automaticReconnect = 1;
    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.context = client;
    MQTTAsync_connect(client, &conn_opts);
}

void onConnect(void *context, MQTTAsync_successData *response)
{
    printf("Connected\n");
    int subscribed = MQTTAsync_subscribe(client, TOPIC, QOS, NULL);
    int subscribedStop = MQTTAsync_subscribe(client, STOP_TOPIC, QOS, NULL);
    if (subscribed != MQTTASYNC_SUCCESS)
    {
        fprintf(stderr, "Failed to subscribe to topic\n");
    }
}

void onConnectFailure(void *context, MQTTAsync_failureData *response)
{
    fprintf(stderr, "Connect failed, rc %d\n", response ? response->code : 0);
}

int onMessage(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
    char *payloadStr = strndup(message->payload, message->payloadlen);
    printf("Empfangene Nachricht auf Topic '%s': %s\n", topicName, payloadStr);

    if (strcmp(topicName, STOP_TOPIC) == 0 && strcmp(payloadStr, "true") == 0)
    {
        // Notstop-Nachricht empfangen
        fprintf(stderr, "Emergency stop triggered!\n");
        emergency_stop_triggered = 1;
        free(payloadStr);
        trigger_emergency_stop(); // Führe Notstop aus ohne einen neuen Thread zu starten
    }
    else
    {
        enqueue(&messageQueue, payloadStr);
    }

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

void connectionLost(void *context, char *cause)
{
    fprintf(stderr, "Connection lost, cause: %s\n", cause);
    initialize_mqtt();
}
void trigger_emergency_stop()
{
    gpioWaveTxStop();
    emergency_stop_triggered = 0;
}

int main()
{
    initQueue(&messageQueue);
    sem_init(&queueSemaphore, 0, 0);
    pthread_t workerThread;
    pthread_create(&workerThread, NULL, sequence_worker_thread, NULL);
    pthread_detach(workerThread);

    initialize_motors();
    initialize_mqtt();

    while (!emergency_stop_triggered) {
        sleep(1);
    }

    MQTTAsync_disconnect(client, NULL);
    MQTTAsync_destroy(&client);
    gpioTerminate();
    return 0;
}
