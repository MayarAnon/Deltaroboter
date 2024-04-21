# mosquitto_pub -t "homing/control" -m "true"

import paho.mqtt.client as mqtt
import RPi.GPIO as GPIO
import json

# Konfiguration der GPIO-Pins für die Endschalter
GPIO.setmode(GPIO.BCM)
GPIO.setup([0, 5, 6], GPIO.IN, pull_up_down=GPIO.PUD_UP)

# MQTT-Client-Setup
client = mqtt.Client()

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe("homing/control")

def on_message(client, userdata, msg):
    message = msg.payload.decode()
    if msg.topic == "homing/control" and message == "true":
        start_homing()

def start_homing():
    homing_complete = [False, False, False]
    steps_per_revolution = 800
    motor_pulses = [steps_per_revolution // 10] * 3  # Kleine Schritte nehmen, um Überfahren zu vermeiden
    
    while not all(homing_complete):
        # Status der Endschalter überprüfen
        for i, pin in enumerate([0, 5, 6]):
            if GPIO.input(pin) == GPIO.LOW:
                homing_complete[i] = True
                motor_pulses[i] = 0  # Stoppe den Motor, wenn der Endschalter betätigt ist
        
        # Sende Steuerbefehl für die Motoren
        mqtt_message = json.dumps({"motorpulses": motor_pulses})
        client.publish("motorcontrol/topic", mqtt_message)
    
    # Sende Homing-Feedback
    client.publish("homing/feedback", "true")
    print("Homing completed.")

client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost", 1883, 60)
client.loop_forever()
