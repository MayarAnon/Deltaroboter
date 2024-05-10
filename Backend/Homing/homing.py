import RPi.GPIO as GPIO
import paho.mqtt.client as mqtt
import time
import json
import logging
import threading
import signal
import logging
from datetime import datetime

# Aktuelle Zeit in gewünschtem Format erhalten
current_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
logging.basicConfig(filename='../../log/homing.log', level=logging.INFO)


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
    logging.info(f"{current_time} Connected with result code " + str(rc))
    client.subscribe(MQTT_TOPIC_CONTROL)

def send_motor_commands(pulses, timing):
    message = json.dumps([{"motorpulses": pulses, "timing": timing}])
    client.publish(MQTT_TOPIC_MOTORS_SEQUENCE, message)
   

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
        pulses = [-10, -10, -10,-20]
        timing = [2000, 2000, 5]
        while is_homing_active:
            all_homed = True
            for index, pin in enumerate(ENDSCHALTER_PINS):
                if not GPIO.input(pin):
                   
                    all_homed = False
                    pulses[index] = -10
                else:
                    pulses[index] = 0
                    

            send_motor_commands(pulses, timing)

            if all_homed:
                break
            time.sleep(0.005)  # Wartezeit zwischen den Überprüfungen
        client.publish(MQTT_TOPIC_FEEDBACK, 'Homing process completed successfully.')
    finally:
        is_homing_active = False

def check_end_switches():
    """
    Überwacht kontinuierlich die Zustände der Endschalter. Wenn der Homing-Prozess nicht aktiv ist und ein Endschalter aktiviert wird,
    wird eine Nachricht gesendet, um die Motoren zu stoppen (fehler ist aufgetreten).
    """
    while True:
        if not is_homing_active:
            for index, pin in enumerate(ENDSCHALTER_PINS):
                if GPIO.input(pin):
                    
                    client.publish(MQTT_TOPIC_MOTORS_STOP, 'true')
                    break
        time.sleep(0.05)  # Kurze Verzögerung, um das Polling zu begrenzen

def setup_end_switch_monitoring():
    threading.Thread(target=check_end_switches, daemon=True).start()

def on_message(client, userdata, msg):
    """
    Diese Funktion wird aufgerufen, wenn eine Nachricht auf einem der abonnierten Topics eintrifft.
    Es überprüft den Inhalt der Nachricht und startet oder stoppt den Homing-Prozess entsprechend.

    Parameter:
    - client: Der MQTT-Client
    - userdata: Die Benutzerdaten, normalerweise nicht verwendet
    - msg: Die empfangene Nachricht mit .topic und .payload Eigenschaften
    """
    if msg.topic == MQTT_TOPIC_CONTROL:
        command = msg.payload.decode()
        if command == 'true':
            logging.info(f"{current_time} Start homing")
            global is_homing_active
            if not is_homing_active:
                threading.Thread(target=start_homing_process).start()
        elif command == 'false':
            logging.info(f"{current_time} Stop homing")
            stop_homing_process()

def stop_homing_process():
    """
    Stoppt den Homing-Prozess, indem die globale Variable `is_homing_active` auf False gesetzt wird.
    Dies wird überprüft in der Hauptschleife des Homing-Prozesses, um den Loop vorzeitig zu beenden.
    """
    global is_homing_active
    is_homing_active = False
def signal_handler(signum, frame):
    """
    Diese Funktion wird aufgerufen, wenn das Programm ein SIGINT oder SIGTERM Signal erhält.
    Es sorgt für die saubere Beendigung des Programms.
    """
    logging.info(f"{current_time} Cleaning up resources...")
    stop_homing_process()
    client.loop_stop()
    GPIO.cleanup()
    logging.info(f"{current_time} Shutdown complete.")
    exit(0)

# Registriere den Signalhandler für SIGINT und SIGTERM
signal.signal(signal.SIGINT, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)

# MQTT-Client-Setup
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(MQTT_BROKER, MQTT_PORT, 60)

# Starte die Endschalter-Überwachung beim Start des Skripts
setup_end_switch_monitoring()

try:
    client.loop_forever()  # Dieser Aufruf blockiert, bis eine Ausnahme auftritt oder der Client manuell gestoppt wird.
except Exception as e:
    logging.error(f"{current_time} Exception occurred: {str(e)}")
finally:
    client.loop_stop()
    GPIO.cleanup()