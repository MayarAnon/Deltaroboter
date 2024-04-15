//gcc -o MotorController main.c MotorControl.c mqttClient.c utils.c -I/usr/local/include/cjson -L/usr/local/lib/cjson -lpigpio -lpaho-mqtt3as -lcjson -lpthread


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "MotorControl.h"
#include "mqttClient.h"
#include "utils.h"

int main() {
    pthread_t watchdog;
    if (pthread_create(&watchdog, NULL, waveWatchdog, NULL) != 0) {
        fprintf(stderr, "Failed to create watchdog thread\n");
        exit(1);
    }
    pthread_detach(watchdog);

    initialize_motors();
    initialize_mqtt();

    while (!emergency_stop_triggered) {
        sleep(1);  // Hauptthread führt minimale Arbeit aus
    }

    MQTTAsync_disconnect(client, NULL);
    MQTTAsync_destroy(&client);
    gpioTerminate();
    return 0;
}