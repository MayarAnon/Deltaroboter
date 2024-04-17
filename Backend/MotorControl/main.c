//gcc -o MotorController config.c main.c MotorControl.c mqttClient.c utils.c -I/usr/local/include/cjson -L/usr/local/lib/cjson -lpigpio -lpaho-mqtt3as -lcjson -lpthread


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
void intHandler(int dummy) {
    keepRunning = 0;
}
void cleanup_resources() {
    pthread_join(sequenceThread, NULL);
    // MQTT Ressourcen trennen und freigeben
    MQTTAsync_disconnect(client, NULL);
    MQTTAsync_destroy(&client);

    // GPIO Bibliothek beenden
    gpioTerminate();

    // Freigeben der dynamisch zugewiesenen Speicherbereiche
    free(globalConfig.address);
    free(globalConfig.clientId);
    free(globalConfig.topic);
    free(globalConfig.stopTopic);
    free(globalConfig.motor_gpios);
    free(globalConfig.dir_gpios);
    free(globalConfig.enb_gpios);
    sem_destroy(&queueSemaphore);
    while (!messageQueue.empty()) {
        free(dequeue(&messageQueue));
    }
}


int main() {
    globalConfig = load_config("config.json");
    
    signal(SIGINT, intHandler);
   
    initQueue(&messageQueue);
    sem_init(&queueSemaphore, 0, 0);

    pthread_t workerThread;
    pthread_create(&workerThread, NULL, sequence_worker_thread, NULL);
    pthread_detach(workerThread);

    initialize_motors();
    initialize_mqtt();
    
     while (keepRunning && !emergency_stop_triggered) {
        sleep(1);  // Hauptthread führt minimale Arbeit aus
    }

    cleanup_resources();
    return 0;
}