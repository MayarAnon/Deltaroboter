#include <pigpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h> // Für sleep-Funktionen
#define SEQUENCE_COUNT 8
#define MOTOR_COUNT 3

// Definiert die GPIO-Pins, die für die Motoren verwendet werden
const int motor_gpios[MOTOR_COUNT] = {17, 27, 24};

// Sequenzdefinitionen
const int sequences[][MOTOR_COUNT] = {
    {1, 5, 20},
    {20, 0, 10},
    {3, 3, 4},
    {10, 5, 3},
    {1, 5, 20},
    {20, 0, 10},
    {3, 3, 4},
    {10, 5, 3}
};
//#define SEQUENCE_COUNT (sizeof(sequences) / sizeof(sequences[0]))

void initialize_motors() {
    for (int i = 0; i < MOTOR_COUNT; ++i) {
        gpioSetMode(motor_gpios[i], PI_OUTPUT);
    }
}

void send_pulses_to_motor(int motor_gpio, int pulse_count, gpioPulse_t *pulses, int *index) {
    const int pulseWidthUs = 500; // Pulsbreite in Mikrosekunden
    for (int i = 0; i < pulse_count; ++i) {
        pulses[(*index)++] = (gpioPulse_t){.gpioOn = 1 << motor_gpio, .gpioOff = 0, .usDelay = pulseWidthUs};
        pulses[(*index)++] = (gpioPulse_t){.gpioOn = 0, .gpioOff = 1 << motor_gpio, .usDelay = pulseWidthUs};
    }
}

void execute_sequences() {
    for (int i = 0; i < SEQUENCE_COUNT; ++i) {
        gpioWaveClear();

        // Angenommen, jeder Motor könnte maximal 20 Pulse in einer Sequenz haben
        gpioPulse_t pulses[MOTOR_COUNT * 40 * 2];
        int index = 0;

        for (int j = 0; j < MOTOR_COUNT; ++j) {
            send_pulses_to_motor(motor_gpios[j], sequences[i][j], pulses, &index);
        }

        gpioWaveAddGeneric(index, pulses);
        int wave_id = gpioWaveCreate();
        if (wave_id >= 0) {
            gpioWaveTxSend(wave_id, PI_WAVE_MODE_ONE_SHOT_SYNC);
            while (gpioWaveTxBusy()) usleep(100);
            gpioWaveDelete(wave_id);
        } else {
            fprintf(stderr, "Fehler beim Erzeugen der Waveform\n");
        }
    }
}

int main() {
    struct timespec start, end;
    long long duration;
    clock_gettime(CLOCK_MONOTONIC, &start);
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Pigpio-Initialisierung fehlgeschlagen\n");
        return 1;
    }

    

    initialize_motors();
    execute_sequences();

    gpioTerminate();

    clock_gettime(CLOCK_MONOTONIC, &end);
    duration = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);

    printf("Alle Sequenzen abgeschlossen.\n");
    printf("Ausführungsdauer: %lld ns (%.2f Sekunden)\n", duration, duration / 1000000000.0);

    return 0;
}
