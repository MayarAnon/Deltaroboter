#include <pigpio.h>
#include <stdio.h>

#define MOTOR1_GPIO 17
#define MOTOR2_GPIO 27
#define MOTOR3_GPIO 23

#define PULSE_PER_SECOND 16000
#define RUN_TIME_SECONDS 10

void initialize_motors() {
    gpioSetMode(MOTOR1_GPIO, PI_OUTPUT);
    gpioSetMode(MOTOR2_GPIO, PI_OUTPUT);
    gpioSetMode(MOTOR3_GPIO, PI_OUTPUT);
}

void create_and_send_combined_waveform() {
    gpioWaveClear(); // Bestehende Waveforms löschen
    
    const int total_pulses = 3 * 2; // Anzahl der Pulse (an/aus für jeden Motor)
    gpioPulse_t pulses[total_pulses];

    // Berechne die Halbperiodendauer in Mikrosekunden
    int half_period_us = 1000000 / PULSE_PER_SECOND / 40;

    // Erstelle Pulse für jeden Motor
    for (int i = 0; i < total_pulses; i += 2) {
        int motor_gpio = (i / 2 == 0) ? MOTOR1_GPIO : (i / 2 == 1) ? MOTOR2_GPIO : MOTOR3_GPIO;

        pulses[i] = (gpioPulse_t){.gpioOn = 1 << motor_gpio, .gpioOff = 0, .usDelay = half_period_us};
        pulses[i + 1] = (gpioPulse_t){.gpioOn = 0, .gpioOff = 1 << motor_gpio, .usDelay = half_period_us};
    }

    gpioWaveAddGeneric(total_pulses, pulses); // Füge die Pulse zur Waveform hinzu

    int wave_id = gpioWaveCreate(); // Erzeuge die Waveform
    if (wave_id >= 0) {
        gpioWaveTxSend(wave_id, PI_WAVE_MODE_REPEAT);
    } else {
        fprintf(stderr, "Fehler beim Erzeugen der Waveform\n");
    }
}

int main() {
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Pigpio-Initialisierung fehlgeschlagen\n");
        return 1;
    }

    initialize_motors();
    create_and_send_combined_waveform();
    gpioSleep(PI_TIME_RELATIVE, RUN_TIME_SECONDS, 0); // Warte für die Dauer der Ausführung

    gpioWaveTxStop(); // Beendet die Waveform-Übertragung
    gpioTerminate(); // Räumt auf

    printf("Interpolation der Motoren abgeschlossen.\n");

    return 0;
}
