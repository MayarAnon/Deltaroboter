# mosquitto_pub -t "homing/control" -m "true"

import paho.mqtt.client as mqtt
import RPi.GPIO as GPIO
import json
import time
# Konfiguration der GPIO-Pins für die Endschalter
GPIO.setmode(GPIO.BCM)
GPIO.setup([0, 5, 6], GPIO.IN, pull_up_down=GPIO.PUD_UP)

# MQTT-Client-Setup
client = mqtt.Client("HomingClient1", clean_session=True)
client.max_inflight_messages_set(10)  # Begrenzt die Anzahl "in-flight" Nachrichten
client.max_queued_messages_set(0)  # 0 bedeutet keine Begrenzung der Warteschlange
client.message_retry_set(5)  # Zeit in Sekunden, bevor ein unbestätigter Befehl erneut gesendet wird

def on_disconnect(client, userdata, rc):
    print(f"Disconnected with result code {rc}")


def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe("homing/control")
    client.publish("motors/sequence", json.dumps([{"motorpulses": [100, 100, 100], "timing":[40,40,5]}]))
def on_message(client, userdata, msg):
    message = msg.payload.decode()
    if msg.topic == "homing/control" and message == "true":
        print(message)
        start_homing()

def start_homing():
    homing_complete = [False, False, False]
    steps_per_revolution = 800
    motor_pulses = [steps_per_revolution // 10] * 3
    
    while not all(homing_complete):
        print("test")
        info = client.publish("motors/sequence", json.dumps([{"motorpulses": [5, 5, 5], "timing":[40,40,5]}]))
        if not info.wait_for_publish(timeout=10):  # Timeout in Sekunden
            print("Publish-Timeout erreicht, versuche erneut zu verbinden...")
            client.disconnect()
            client.reconnect()  # Zeige den Status des Publish-Aufrufs


client.on_connect = on_connect
client.on_message = on_message
client.on_disconnect = on_disconnect
client.connect("localhost", 1883, 60)
client.loop_start()  # Start the network loop in a separate thread

try:
    while True:
        time.sleep(1)  # Warte und halte das Hauptprogramm am Laufen
except KeyboardInterrupt:
    print("Programm durch Benutzereingriff beendet.")
    client.loop_stop()  # Beendet den Netzwerk-Thread sauber
    client.disconnect()  # Trenne die Verbindung sauber
