//gcc -o MotorController config.c main.c queue.c MotorControl.c mqttClient.c utils.c -I/usr/local/include/cjson -L/usr/local/lib/cjson -lpigpio -lpaho-mqtt3as -lcjson -lpthread


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "MotorControl.h"
#include "mqttClient.h"
#include "utils.h"
#include "config.h"
#include <signal.h>
#include "queue.h"
#include <semaphore.h>
volatile sig_atomic_t keepRunning = 1;
pthread_t sequenceThread;
sem_t queueSemaphore;
void handle_signal(int sig) {
    printf("MotorControl: Caught signal %d, stopping...\n", sig);
    keepRunning = 0;
}
void setup_signal_handling() {
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;  // Keine Flags gesetzt

    if (sigaction(SIGINT, &sa, NULL) < 0) {
    perror("MotorControl: Unable to set SIGINT handler");
    exit(1);
    }
    if (sigaction(SIGTERM, &sa, NULL) < 0) {
        perror("MotorControl: Unable to set SIGTERM handler");
        exit(1);
    }
}
void cleanup_resources() {
    printf("MotorControl: Cleaning up resources...\n");
    // GPIO Bibliothek beenden
    gpioTerminate();
    
    pthread_join(sequenceThread, NULL);
    // MQTT Ressourcen trennen und freigeben
    MQTTAsync_disconnect(client, NULL);
    MQTTAsync_destroy(&client);
    // Freigeben der dynamisch zugewiesenen Speicherbereiche
    free(globalConfig.address);
    free(globalConfig.clientId);
    free(globalConfig.topic);
    free(globalConfig.stopTopic);
    free(globalConfig.motor_gpios);
    free(globalConfig.dir_gpios);
    free(globalConfig.enb_gpios);
    sem_destroy(&queueSemaphore);
}


int main() {
    globalConfig = load_config("config.json");
    
    
    
    initQueue(&messageQueue);
    sem_init(&queueSemaphore, 0, 0);

    pthread_t workerThread;
    pthread_create(&workerThread, NULL, sequence_worker_thread, NULL);
    pthread_detach(workerThread);

    initialize_motors();
    initialize_mqtt();
    setup_signal_handling();
     while (keepRunning && !emergency_stop_triggered) {
        sleep(1);  // Hauptthread führt minimale Arbeit aus
    }

    cleanup_resources();
    return 0;
}