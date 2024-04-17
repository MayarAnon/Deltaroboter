//gcc -o MotorController config.c main.c MotorControl.c mqttClient.c utils.c -I/usr/local/include/cjson -L/usr/local/lib/cjson -lpigpio -lpaho-mqtt3as -lcjson -lpthread


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "MotorControl.h"
#include "mqttClient.h"
#include "utils.h"
#include "config.h"
#include <signal.h>

volatile sig_atomic_t keepRunning = 1;

void intHandler(int dummy) {
    keepRunning = 0;
}
void cleanup_resources() {
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
}


int main() {
    globalConfig = load_config("config.json");
    
    signal(SIGINT, intHandler);

    initialize_motors();
    initialize_mqtt();
    
     while (keepRunning && !emergency_stop_triggered) {
        sleep(1);  // Hauptthread führt minimale Arbeit aus
    }

    cleanup_resources();
    return 0;
}