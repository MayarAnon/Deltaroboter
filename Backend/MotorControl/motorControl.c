#include "MotorControl.h"
#include <stdio.h>
#include <stdlib.h>

int motor_gpios[MOTOR_COUNT] = {17, 27, 22};
int dir_gpios[MOTOR_COUNT] = {2, 3, 4};
int enb_gpios[MOTOR_COUNT] = {14, 15, 18};

void initialize_motors() {
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Pigpio initialization failed\n");
        exit(1);
    }
    for (int i = 0; i < MOTOR_COUNT; i++) {
        gpioSetMode(motor_gpios[i], PI_OUTPUT);
        gpioSetMode(dir_gpios[i], PI_OUTPUT);
        gpioSetMode(enb_gpios[i], PI_OUTPUT);
        gpioWrite(enb_gpios[i], 1);  // Enable motors
    }
}

void execute_interpolated_sequence(int pulses[MOTOR_COUNT], int pulseWidthUs, int pauseBetweenPulsesUs, int directionChangeDelayUs)
{
    /**
     * Führt eine interpolierte Motorsequenz basierend auf den gegebenen Pulsen und Timing-Parametern aus.
     */
    gpioWaveClear();
    for (int i = 0; i < MOTOR_COUNT; ++i) {
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
        pthread_mutex_lock(&waveMutex);
        waveTransmissionActive = 1;
        gpioWaveTxSend(wave_id, PI_WAVE_MODE_ONE_SHOT);

        // Warte auf die Beendigung der Wellenform-Übertragung
        while (waveTransmissionActive && gpioWaveTxBusy()) {
            pthread_cond_wait(&waveCond, &waveMutex);
        }
        waveTransmissionActive = 0;
        pthread_mutex_unlock(&waveMutex);

        gpioWaveDelete(wave_id);
    } else {
        fprintf(stderr, "Fehler beim Erzeugen der Waveform\n");
    }
}