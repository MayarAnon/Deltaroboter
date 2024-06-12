import RPi.GPIO as GPIO
import paho.mqtt.client as mqtt
import time
import json
import logging
import threading
import signal
import logging
from datetime import datetime

# Get current time in the desired format
current_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
logging.basicConfig(filename='../../log/SafetyAndHoming.log', level=logging.INFO)


# Constants for the GPIO pins[26, 5, 6, 16] of the end switches and emergency stop
ENDSCHALTER_PINS = [26, 5, 6,16]
NOTAUS_PIN = 19

# MQTT configuration
MQTT_BROKER = 'localhost'
MQTT_PORT = 1883
MQTT_TOPIC_CONTROL = 'homing/control'
MQTT_TOPIC_FEEDBACK = 'homing/feedback'
MQTT_TOPIC_MOTORS_SEQUENCE = 'motors/sequence'
MQTT_TOPIC_MOTORS_STOP = 'motors/stop'
MQTT_TOPIC_ERRORS = 'Errors'
# Global variable to track the homing status
is_homing_active = False

# Initialization of GPIO pins
GPIO.setmode(GPIO.BCM)
for pin in ENDSCHALTER_PINS:
    GPIO.setup(pin, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
GPIO.setup(NOTAUS_PIN, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
def on_connect(client, userdata, flags, rc):
    """
    Callback function when the client connects to the MQTT broker.
    Subscribes to the control topic.

    Parameters:
    - client: The MQTT client
    - userdata: User data, not used
    - flags: Response flags sent by the broker
    - rc: Connection result
    """
    logging.info(f"{current_time} Connected with result code " + str(rc))
    client.subscribe(MQTT_TOPIC_CONTROL)

def send_motor_commands(pulses, timing):
    """
    Sends motor commands to the MQTT topic.

    Parameters:
    - pulses: List of pulses for each motor
    - timing: List of timing for each motor
    """
    message = json.dumps([{"motorpulses": pulses, "timing": timing}])
    client.publish(MQTT_TOPIC_MOTORS_SEQUENCE, message)
   

def start_homing_process():
    """
    Starts the homing process for the motors. This process continuously checks if each motor
    has reached its home position and adjusts the motor pulsing accordingly. The process
    runs in a loop until all motors have reached their home position.

    Uses the global variable `is_homing_active` to manage the process status and uses the
    MQTT topics to send commands and receive feedback.
    """

    global is_homing_active
    is_homing_active = True
    pulses = [-10, -10, -10,-1]
    timing = [200, 200, 10]
    try:
       
        while is_homing_active:
            all_homed = True
            for index, pin in enumerate(ENDSCHALTER_PINS):
                if not GPIO.input(pin):
                   
                    all_homed = False
                    pulses[index] = -10
                    if index == 3:
                        pulses[index]= -5
                else:
                    pulses[index] = 0
                    

            send_motor_commands(pulses, timing)

            if all_homed:
                
                client.publish(MQTT_TOPIC_FEEDBACK, 'true')
                client.publish(MQTT_TOPIC_FEEDBACK, 'false')
                client.publish(MQTT_TOPIC_ERRORS, '0')
                
                break
            time.sleep(0.04) # Wait time between checks
    finally:
        send_motor_commands([933,933,933,100], [400,400,10])
        time.sleep(1)
        is_homing_active = False
        logging.info(f"{current_time} homing done")
def check_end_switches():
    """
    Continuously monitors the states of the end switches. If the homing process is not active
    and an end switch is activated, a message is sent to stop the motors (an error has occurred).
    """
    while True:
        if not is_homing_active:
            for index, pin in enumerate(ENDSCHALTER_PINS):
                if GPIO.input(pin):   
                    client.publish(MQTT_TOPIC_MOTORS_STOP, 'true')
                    client.publish(MQTT_TOPIC_ERRORS, '2') #error code 2
                    logging.info(f"{current_time} motor stop Error 2")
                    break
        time.sleep(0.04)   # Short delay to limit polling
def check_emergency_stop():
    """
    Continuously monitors the state of the emergency stop button. If the button is pressed
    (pin is HIGH), a message is sent to stop the motors and an error code is published.
    """
    while True:
        if GPIO.input(NOTAUS_PIN):
            stop_homing_process()
            client.publish(MQTT_TOPIC_MOTORS_STOP, 'true')
            client.publish(MQTT_TOPIC_ERRORS, '1')  # Error code 1 for emergency stop
            logging.error(f"{current_time} Emergency stop activated")
            
            break
        time.sleep(0.004)  # Short delay to limit polling
def setup_monitoring():
    """
    Sets up monitoring by starting threads to check the end switches and the emergency stop button.
    """
    threading.Thread(target=check_end_switches, daemon=True).start()
    threading.Thread(target=check_emergency_stop, daemon=True).start()

def on_message(client, userdata, msg):
    """
    Callback function that is called when a message is received on a subscribed topic.
    Checks the content of the message and starts or stops the homing process accordingly.

    Parameters:
    - client: The MQTT client
    - userdata: User data, not used
    - msg: The received message with .topic and .payload attributes
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
            client.publish(MQTT_TOPIC_ERRORS, '3')

def stop_homing_process():
    """
    Stops the homing process by setting the global variable `is_homing_active` to False.
    This is checked in the main loop of the homing process to end the loop prematurely.
    """
    global is_homing_active
    is_homing_active = False
def signal_handler(signum, frame):
    """
    This function is called when the program receives a SIGINT or SIGTERM signal.
    It ensures the program is cleanly terminated.

    Parameters:
    - signum: The signal number
    - frame: The current stack frame
    """
    logging.info(f"{current_time} Cleaning up resources...")
    stop_homing_process()
    client.loop_stop()
    GPIO.cleanup()
    logging.info(f"{current_time} Shutdown complete.")
    exit(0)

# Register signal handler for SIGINT and SIGTERM
signal.signal(signal.SIGINT, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)
# MQTT client setup
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(MQTT_BROKER, MQTT_PORT, 60)

# Start monitoring of end switches and emergency stop button when the script starts
setup_monitoring()

try:
    client.loop_forever()  # This call blocks until an exception occurs or the client is manually stopped.
except Exception as e:
    logging.error(f"{current_time} Exception occurred: {str(e)}")
finally:
    client.loop_stop()
    GPIO.cleanup()