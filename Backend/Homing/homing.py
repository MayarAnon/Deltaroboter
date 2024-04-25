import RPi.GPIO as GPIO
import paho.mqtt.client as mqtt
import time
import json
import logging
import threading

logging.basicConfig(level=logging.DEBUG)

# Konstanten für die GPIO-Pins der Endschalter
ENDSCHALTER_PINS = [0, 5, 6, 13]

# MQTT-Konfiguration
MQTT_BROKER = 'localhost'
MQTT_PORT = 1883
MQTT_TOPIC_CONTROL = 'homing/control'
MQTT_TOPIC_FEEDBACK = 'homing/feedback'
MQTT_TOPIC_MOTORS_SEQUENCE = 'motors/sequence'
MQTT_TOPIC_MOTORS_STOP = 'motors/stop'

# Globale Variable, um den Homing-Status zu tracken
is_homing_active = False

# Initialisierung der GPIO-Pins
GPIO.setmode(GPIO.BCM)
for pin in ENDSCHALTER_PINS:
    GPIO.setup(pin, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe(MQTT_TOPIC_CONTROL)

def send_motor_commands(pulses, timing):
    message = json.dumps([{"motorpulses": pulses, "timing": timing}])
    client.publish(MQTT_TOPIC_MOTORS_SEQUENCE, message)
    print(f"Motor commands sent: {message}")

def start_homing_process():
    """
    Startet den Homing-Prozess für die Motoren. Dieser Prozess prüft kontinuierlich, ob jeder Motor seine Home-Position
    erreicht hat, und passt die Motorpulsung entsprechend an. Der Prozess läuft in einer Schleife, bis alle Motoren
    ihre Home-Position erreicht haben.

    Verwendet die globalen Variablen `is_homing_active`, um den Prozessstatus zu verwalten und verwendet die 
    MQTT-Topics, um Befehle zu senden und Feedback zu erhalten.

    Globale Variablen:
    - is_homing_active: Eine Boolesche Variable, die angibt, ob der Homing-Prozess aktiv ist.
    """
    global is_homing_active
    is_homing_active = True
    try:
        pulses = [80, 80, 80,80]  # Standardpulswerte für die Motoren
        timing = [100, 100, 5]      # Standard-Timing für die Motoren
        while True:
            all_homed = True
            for index, pin in enumerate(ENDSCHALTER_PINS):
                if not GPIO.input(pin):
                    print(f"Motor {index + 1} erreicht noch nicht die Home-Position.")
                    all_homed = False
                    pulses[index] = 80 #pulse-anpassung wenn nötig
                else:
                    pulses[index] = 0
                    print(f"Motor {index + 1} ist homed.")

            send_motor_commands(pulses, timing)

            if all_homed:
                break
            time.sleep(1)  # Wartezeit zwischen den Überprüfungen
        client.publish(MQTT_TOPIC_FEEDBACK, 'Homing process completed successfully.')
        print("Homing-Prozess erfolgreich abgeschlossen.")
    finally:
        is_homing_active = False

def check_end_switches():
    while True:
        if not is_homing_active:
            for index, pin in enumerate(ENDSCHALTER_PINS):
                if GPIO.input(pin):
                    print(f"Endschalter {index + 1} betätigt! Sende Stop-Signal.")
                    client.publish(MQTT_TOPIC_MOTORS_STOP, 'true')
                    break
        time.sleep(0.1)  # Kurze Verzögerung, um das Polling zu begrenzen

def setup_end_switch_monitoring():
    threading.Thread(target=check_end_switches, daemon=True).start()

def on_message(client, userdata, msg):
    if msg.topic == MQTT_TOPIC_CONTROL and msg.payload.decode() == 'true':
        print("Start homing")
        threading.Thread(target=start_homing_process).start()

# MQTT-Client-Setup
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(MQTT_BROKER, MQTT_PORT, 60)

# Starte die Endschalter-Überwachung beim Start des Skripts
setup_end_switch_monitoring()

try:
    client.loop_forever()  # Dieser Aufruf blockiert, bis eine Ausnahme auftritt oder der Client manuell gestoppt wird.
finally:
    client.loop_stop()  # Stoppt den Loop, wenn der Prozess beendet oder unterbrochen wird
    GPIO.cleanup()      # Räumt die GPIO-Einstellungen auf
