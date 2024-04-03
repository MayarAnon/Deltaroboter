#include <pigpio.h>
#include <stdio.h>

#define GPIO_PIN_1 27
#define GPIO_PIN_2 24
#define PULSE_WIDTH_US 1 // Dauer des HIGH-Zustands in Mikrosekunden

int main() {
    // Initialisiere die Pigpio-Bibliothek
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Pigpio-Initialisierung fehlgeschlagen\n");
        return 1;
    }

    // Konfiguriere die GPIO-Pins als Ausgänge
    gpioSetMode(GPIO_PIN_1, PI_OUTPUT);
    gpioSetMode(GPIO_PIN_2, PI_OUTPUT);

    // Bereite die Waveform vor
    gpioWaveClear();

    gpioPulse_t pulses[2];

    // Erster Puls: Setze beide Pins auf HIGH
    pulses[0].gpioOn = (1 << GPIO_PIN_1) | (1 << GPIO_PIN_2); // Setze beide Pins
    pulses[0].gpioOff = 0; // Kein Pin wird ausgeschaltet
    pulses[0].usDelay = PULSE_WIDTH_US;

    // Zweiter Puls: Setze beide Pins auf LOW
    pulses[1].gpioOn = 0; // Kein Pin wird eingeschaltet
    pulses[1].gpioOff = (1 << GPIO_PIN_1) | (1 << GPIO_PIN_2); // Schalte beide Pins aus
    pulses[1].usDelay = PULSE_WIDTH_US;

    // Füge die Pulse zur Waveform hinzu
    gpioWaveAddGeneric(2, pulses);

    // Erzeuge die Waveform
    int wave_id = gpioWaveCreate();

    if (wave_id >= 0) {
        // Sende die Waveform einmalig (nicht wiederholend)
        gpioWaveTxSend(wave_id, PI_WAVE_MODE_ONE_SHOT);

        // Warte, bis die Waveform abgeschlossen ist
        while (gpioWaveTxBusy()) {
            time_sleep(0.1);
        }

        // Lösche die Waveform, um Ressourcen freizugeben
        gpioWaveDelete(wave_id);
    } else {
        fprintf(stderr, "Fehler beim Erzeugen der Waveform\n");
    }

    // Räume auf und beende die Nutzung der Pigpio-Bibliothek
    gpioTerminate();

    printf("Rechtecksignal gesendet.\n");

    return 0;
}
