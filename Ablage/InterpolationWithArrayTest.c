#include <pigpio.h>
#include <stdio.h>
#include <unistd.h> // Für sleep-Funktionen
#include <time.h>
// Definiert die GPIO-Pins, die für die Motoren verwendet werden
#define MOTOR1_GPIO 17
#define MOTOR2_GPIO 27
#define MOTOR3_GPIO 24

// Definiert die Anzahl der Sequenzen und die Anzahl der Motoren
#define SEQUENCE_COUNT 8
#define MOTOR_COUNT 3

// Prototypen
void initialize_motors();
void execute_sequence(int sequence[SEQUENCE_COUNT][MOTOR_COUNT]);
void send_pulses_to_motor(int motor_gpio, int pulse_count);

int main() {
    struct timespec start, end;
    long long duration;
    // Startzeit speichern
    clock_gettime(CLOCK_MONOTONIC, &start);
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Pigpio-Initialisierung fehlgeschlagen\n");
        return 1;
    }

    initialize_motors();

    // Definiert eine Sequenz von Pulsen für jeden Motor
    int sequence[SEQUENCE_COUNT][MOTOR_COUNT] = {
        {1, 5, 20},
        {20, 0, 10},
        {3, 3, 4},
        {10, 5, 3},
        {1, 5, 20},
        {20, 0, 10},
        {3, 3, 4},
        {10, 5, 3},
    };

    execute_sequence(sequence);

    gpioTerminate();
    printf("Alle Sequenzen abgeschlossen.\n");
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Berechne die Dauer in Nanosekunden
    duration = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);

    printf("Ausführungsdauer: %lld ns\n", duration/1000000000LL);
    return 0;
}

void initialize_motors() {
    // Konfiguriere die GPIO-Pins der Motoren als Ausgänge
    gpioSetMode(MOTOR1_GPIO, PI_OUTPUT);
    gpioSetMode(MOTOR2_GPIO, PI_OUTPUT);
    gpioSetMode(MOTOR3_GPIO, PI_OUTPUT);
}

void execute_sequence(int sequence[SEQUENCE_COUNT][MOTOR_COUNT]) {
    for (int i = 0; i < SEQUENCE_COUNT; i++) {
        gpioWaveClear(); // Bereite eine neue Waveform vor
        int motor_gpios[MOTOR_COUNT] = {MOTOR1_GPIO, MOTOR2_GPIO, MOTOR3_GPIO};
        
        for (int j = 0; j < MOTOR_COUNT; j++) {
            send_pulses_to_motor(motor_gpios[j], sequence[i][j]);
        }

        // Erstelle und sende die kombinierte Waveform für diese Sequenz
        int wave_id = gpioWaveCreate();
        if (wave_id >= 0) {
            gpioWaveTxSend(wave_id, PI_WAVE_MODE_ONE_SHOT_SYNC);
            
            // Warte, bis die Waveform abgeschlossen ist
            while (gpioWaveTxBusy()) { usleep(100); }
            
            gpioWaveDelete(wave_id); // Lösche die Waveform, um Ressourcen freizugeben
        } else {
            fprintf(stderr, "Fehler beim Erzeugen der Waveform\n");
        }
    }
}

void send_pulses_to_motor(int motor_gpio, int pulse_count) {
    gpioPulse_t pulses[2 * pulse_count]; // Jeder Puls benötigt zwei Einträge: EIN und AUS

    for (int i = 0; i < pulse_count; i++) {
        pulses[2*i].gpioOn = 1 << motor_gpio; // Puls EIN
        pulses[2*i].gpioOff = 0;
        pulses[2*i].usDelay = 500; // Pulsbreite
        
        pulses[2*i+1].gpioOn = 0; // Puls AUS
        pulses[2*i+1].gpioOff = 1 << motor_gpio;
        pulses[2*i+1].usDelay = 500; // Pause zwischen den Pulsen
    }

    gpioWaveAddGeneric(2 * pulse_count, pulses); // Füge die Pulse zur aktuellen Waveform hinzu
}
