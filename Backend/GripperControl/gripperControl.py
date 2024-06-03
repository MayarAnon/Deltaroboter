import RPi.GPIO as GPIO
import paho.mqtt.client as mqtt
import json
import time
import threading
import signal
import logging
from datetime import datetime

# Get current time in the desired format
current_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
logging.basicConfig(filename='../../log/gripperControl.log', level=logging.INFO)


# Define GPIO pins

PumpRelais = 25
PumpValveRelais = 25
VacuumRelais = 8
VacuumValveRelais = 8
MagnetRelais = 7
GripperMagnetRelais = 1  # Magnet relay for gripper attachment
ParallelGripper = 12  # PWM Output

# GPIO pins setup
def setup_gpio():
    """
    Initializes the GPIO pins and configures the parallel gripper with PWM.
    """
    GPIO.setmode(GPIO.BCM)  # Use Broadcom pin numbering
    GPIO.setup(ParallelGripper, GPIO.OUT)
    p = GPIO.PWM(ParallelGripper, 100)  # PWM with 100Hz
    p.start(0)

    for pin in [PumpRelais, VacuumRelais, MagnetRelais, PumpValveRelais, VacuumValveRelais,GripperMagnetRelais]:
        GPIO.setup(pin, GPIO.OUT)

# Class for controlling the gripper
class GripperControl:
    def __init__(self, id, host, port):
        """
        Initializes the MQTT client and the PWM controller for the parallel gripper.
        """
        self.client = mqtt.Client(id)
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.client.connect(host, port, 60)
        self.pwm = GPIO.PWM(ParallelGripper, 100) 
        self.pwm.start(0) # initial duty cycle

    def on_connect(self, client, userdata, flags, rc):
        """
        Called when a connection to the MQTT broker is established.
        Subscribes to the "gripper/control" topic.
        """
        if rc == 0:
            self.client.subscribe("gripper/control")
        else:
            logging.info(f"{current_time} Connection failed.")

    def on_message(self, client, userdata, message):
        """
        Called when a message is received on a subscribed topic.
        Processes the received message and controls the gripper accordingly.
        """

        if message.topic == "gripper/control":
            self.handle_control_message(message.payload.decode())

    def handle_control_message(self, message):
        """
        Processes the received control message and controls the gripper accordingly.
        """
        data = json.loads(message)
        logging.info(data)
        if "parallelGripper" in data:
            input_pwm = int(data["parallelGripper"])
            pwmValue = 40 + 0.128 * input_pwm # scaling from 0-100 to duty cycle of 40% to 52,8%
            self.pwm.ChangeDutyCycle(pwmValue)
            GPIO.output(VacuumRelais, GPIO.LOW)
            GPIO.output(PumpRelais, GPIO.LOW)
            self.send_feedback(0)

        elif "complientGripper" in data:
            value = int(data["complientGripper"])
            if value == 1:
                GPIO.output(PumpRelais, GPIO.HIGH)
                GPIO.output(PumpValveRelais, GPIO.HIGH)
                GPIO.output([VacuumRelais,VacuumValveRelais], GPIO.LOW)
                
                self.send_feedback(0)
            elif value == -1:
                GPIO.output(VacuumRelais, GPIO.HIGH)
                GPIO.output(VacuumValveRelais, GPIO.HIGH)
                GPIO.output([PumpRelais,PumpValveRelais], GPIO.LOW)              
                self.send_feedback(0)
            else:
                GPIO.output([VacuumRelais, VacuumValveRelais, PumpRelais, PumpValveRelais], GPIO.LOW)
        elif "vacuumGripper" in data:
            value = int(data["vacuumGripper"])
            if value == 1:
                GPIO.output(PumpRelais, GPIO.HIGH)
                GPIO.output(PumpValveRelais, GPIO.HIGH)
                GPIO.output([VacuumRelais,VacuumValveRelais], GPIO.LOW)
                
                self.send_feedback(0)
            elif value == -1:
                GPIO.output(VacuumRelais, GPIO.HIGH)
                GPIO.output(VacuumValveRelais, GPIO.HIGH)
                GPIO.output([PumpRelais,PumpValveRelais], GPIO.LOW)

                self.send_feedback(0)
            else:
                GPIO.output([VacuumRelais, VacuumValveRelais, PumpRelais, PumpValveRelais], GPIO.LOW)
        elif "magnetGripper" in data:
            value = int(data["magnetGripper"])
            GPIO.output(MagnetRelais, GPIO.HIGH if value == 1 else GPIO.LOW)
            GPIO.output([VacuumRelais, PumpRelais], GPIO.LOW)
            self.send_feedback(0)
        elif "magneticGripperAttachment" in data:
            if data["magneticGripperAttachment"] == "enable":
                GPIO.output(GripperMagnetRelais, GPIO.HIGH)
                logging.info(f"{current_time} magneticGripperAttachment enabled")
            elif data["magneticGripperAttachment"] == "disable":
                GPIO.output(GripperMagnetRelais, GPIO.LOW)
                logging.info(f"{current_time} magneticGripperAttachment disabled")
    
    def send_feedback(self, delay):
        """
        Sends a feedback signal after a delay.

        Note: This function does not currently serve a meaningful purpose as there is no
        feedback mechanism implemented in the current gripper hardware. It is implemented
        in anticipation of a future feedback mechanism.
        """
        def feedback_thread():
            self.client.publish("gripper/feedback", "true")
        threading.Thread(target=feedback_thread).start()

    def cleanup(self):
        """
        Cleans up resources and terminates the program properly.
        """
        logging.info(f"{current_time} Cleaning up resources...")
        self.client.loop_stop()
        self.pwm.stop()
        GPIO.cleanup()

    def signal_handler(self, signum, frame):
        """
        Handles signals like SIGTERM or SIGINT.
        """
        self.cleanup()
        logging.info(f"{current_time} Signal {signum} received, exiting.")
        exit(0)

    def run(self):
        """
        Starts the MQTT client and waits for messages.
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
