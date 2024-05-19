import RPi.GPIO as GPIO
import paho.mqtt.client as mqtt
import json
import time
import threading
import signal
import logging
from datetime import datetime

# Aktuelle Zeit in gewünschtem Format erhalten
current_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
logging.basicConfig(filename='../../log/gripperControl.log', level=logging.INFO)

# Definiere GPIO-Pins
ParallelGripper = 12  # PWM Output
MagnetRelais = 16
PumpRelais = 25
VacuumRelais = 8
PumpValveRelais = 7
VacuumValveRelais = 1
GripperMagnetRelais = 20  # Magnet Relais für die Greiferaufnahme
# Setup der GPIO-Pins
def setup_gpio():
    """
    Initialisiert die GPIO-Pins und Konfiguriert den Parallelgripper mit PWM.
    """
    GPIO.setmode(GPIO.BCM)  # Verwende Broadcom Pin-Nummerierung
    GPIO.setup(ParallelGripper, GPIO.OUT)
    p = GPIO.PWM(ParallelGripper, 100)  # PWM mit 100Hz
    p.start(0)

    for pin in [PumpRelais, VacuumRelais, MagnetRelais, PumpValveRelais, VacuumValveRelais,GripperMagnetRelais]:
        GPIO.setup(pin, GPIO.OUT)

# Klasse für die Steuerung des Greifers
class GripperControl:
    def __init__(self, id, host, port):
        """
        Initialisiert den MQTT-Client und den PWM-Controller für den Parallelgripper.
        """
        self.client = mqtt.Client(id)
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.client.connect(host, port, 60)
        self.pwm = GPIO.PWM(ParallelGripper, 100)
        self.pwm.start(0) #anfänglicher Duty Cycle

    def on_connect(self, client, userdata, flags, rc):
        """
        Wird aufgerufen, wenn eine Verbindung zum MQTT-Broker hergestellt wird.
        Abonniert das Topic "gripper/control".
        """
        if rc == 0:
            self.client.subscribe("gripper/control")
        else:
            logging.info(f"{current_time} Connection failed.")

    def on_message(self, client, userdata, message):
        """
        Wird aufgerufen, wenn eine Nachricht auf dem abonnierten Topic empfangen wird.
        Verarbeitet die empfangene Nachricht und steuert den Greifer entsprechend.
        """
        if message.topic == "gripper/control":
            self.handle_control_message(message.payload.decode())

    def handle_control_message(self, message):
        """
        Verarbeitet die empfangene Steuerungsnachricht und steuert den Greifer entsprechend.
        """
        data = json.loads(message)
        logging.info(data)
        if "parallelGripper" in data:
            input_pwm = int(data["parallelGripper"])
            pwmValue = 10 + 0.8 * input_pwm #skallierung von 0-100 auf DutyCycle von 10% bis 90%
            self.pwm.ChangeDutyCycle(pwmValue)
            GPIO.output(VacuumRelais, GPIO.LOW)
            GPIO.output(PumpRelais, GPIO.LOW)
            self.send_feedback(2)

        elif "complientGripper" in data:
            value = int(data["complientGripper"])
            if value == 1:
                GPIO.output(PumpRelais, GPIO.HIGH)
                GPIO.output([VacuumRelais,VacuumValveRelais], GPIO.LOW)
                time.sleep(0.1)
                GPIO.output(PumpValveRelais, GPIO.HIGH)
                self.send_feedback(5)
            elif value == -1:
                GPIO.output(VacuumRelais, GPIO.HIGH)
                GPIO.output([PumpRelais,PumpValveRelais], GPIO.LOW)
                time.sleep(0.1)
                GPIO.output(VacuumValveRelais, GPIO.HIGH)
                self.send_feedback(5)
            else:
                GPIO.output([VacuumRelais, VacuumValveRelais, PumpRelais, PumpValveRelais], GPIO.LOW)
        elif "vacuumGripper" in data:
            value = int(data["vacuumGripper"])
            if value == 1:
                GPIO.output(PumpRelais, GPIO.HIGH)
                GPIO.output([VacuumRelais,VacuumValveRelais], GPIO.LOW)
                time.sleep(0.1)
                GPIO.output(PumpValveRelais, GPIO.HIGH)
                self.send_feedback(5)
            elif value == -1:
                GPIO.output(VacuumRelais, GPIO.HIGH)
                GPIO.output([PumpRelais,PumpValveRelais], GPIO.LOW)
                time.sleep(0.1)
                GPIO.output(VacuumValveRelais, GPIO.HIGH)
                self.send_feedback(5)
            else:
                GPIO.output([VacuumRelais, VacuumValveRelais, PumpRelais, PumpValveRelais], GPIO.LOW)
        elif "magnetGripper" in data:
            value = int(data["magnetGripper"])
            GPIO.output(MagnetRelais, GPIO.HIGH if value == 1 else GPIO.LOW)
            GPIO.output([VacuumRelais, PumpRelais], GPIO.LOW)
            self.send_feedback(1)
        elif "magneticGripperAttachment" in data:
            if data["magneticGripperAttachment"] == "enable":
                GPIO.output(GripperMagnetRelais, GPIO.HIGH)
                logging.info(f"{current_time} magneticGripperAttachment enabled")
            elif data["magneticGripperAttachment"] == "disable":
                GPIO.output(GripperMagnetRelais, GPIO.LOW)
                logging.info(f"{current_time} magneticGripperAttachment disabled")
    
    def send_feedback(self, delay):
        """
        Sendet ein Feedback-Signal nach einer Verzögerung.
        """
        def feedback_thread():
            self.client.publish("gripper/feedback", "true")
        threading.Thread(target=feedback_thread).start()

    def cleanup(self):
        """
        Räumt die Ressourcen auf und beendet das Programm ordnungsgemäß.
        """
        logging.info(f"{current_time} Cleaning up resources...")
        self.client.loop_stop()
        self.pwm.stop()
        GPIO.cleanup()

    def signal_handler(self, signum, frame):
        """
        Behandelt Signale wie SIGTERM oder SIGINT.
        """
        self.cleanup()
        logging.info(f"{current_time} Signal {signum} received, exiting.")
        exit(0)

    def run(self):
        """
        Startet den MQTT-Client und wartet auf Nachrichten.
        """
        self.client.loop_start()
        signal.signal(signal.SIGTERM, self.signal_handler)
        signal.signal(signal.SIGINT, self.signal_handler)
        try:
            while True:
                time.sleep(1)
        finally:
            self.cleanup()

if __name__ == '__main__':
    setup_gpio()
    gripper = GripperControl("GripperController", "localhost", 1883)
    gripper.run()
