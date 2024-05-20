import logging
import json
import paho.mqtt.client as mqtt
from datetime import datetime
import signal
import sys



# Konfiguration des Loggings
logging.basicConfig(
    filename='../../log/statepublisher.log',
    level=logging.INFO,
    format='%(asctime)s %(levelname)s:%(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)
# MQTT-Konfiguration
MQTT_BROKER = 'localhost'
MQTT_PORT = 1883
TOPICS = [
    "homing/feedback",
    "current/coordinates",
    "current/angles",
    "gripper/feedback",
    "gripper/mode",
    "motors/speed",
    "motors/motionProfil",
    "Errors"
]

# Initialisiere den Zustand des Roboters
robot_state = {
    "homing": False,
    "currentCoordinates": [0.0, 0.0, 0.0],
    "currentAngles": [0.0, 0.0, 0.0],
    "gripperFeedback": False,
    "gripperMode": "parallelGripper",
    "motionProfil" : "TrapezProfil",
    "motorsSpeed": 50,
    "Error":0
}

# MQTT Callback für das Verbinden zum Broker
def on_connect(client, userdata, flags, rc):
    logging.info(f"Connected with result code {rc}")
    # Abonniere alle notwendigen Topics
    for topic in TOPICS:
        client.subscribe(topic)

# MQTT Callback für das Empfangen von Nachrichten
def on_message(client, userdata, msg):
    try:
        data = json.loads(msg.payload.decode('utf-8'))
        # Aktualisiere den Roboterzustand basierend auf dem empfangenen Topic
        if msg.topic == 'homing/feedback':
            robot_state['homing'] = data
        elif msg.topic == 'current/coordinates':
            robot_state['currentCoordinates'] = data
        elif msg.topic == 'current/angles':
            robot_state['currentAngles'] = data
        elif msg.topic == 'gripper/feedback':
            robot_state['gripperFeedback'] = data
        elif msg.topic == 'motors/motionProfil':
            if data in ["RectangleProfil", "TrapezProfil","SigmoidProfil"]:
                robot_state['motionProfil'] = data
            else:
                raise ValueError(f"Received invalid motionProfil: {data}")
        elif msg.topic == 'gripper/mode':
            if data in ["parallelGripper", "complientGripper", "magnetGripper", "vacuumGripper"]:
                robot_state['gripperMode'] = data
            else:
                raise ValueError(f"Received invalid gripper mode: {data}")
        elif msg.topic == 'motors/speed':
            robot_state['motorsSpeed'] = data
        elif msg.topic == 'Errors':
            robot_state['Error'] = data
        
        # Veröffentliche den aktuellen Robotstate
        client.publish("robot/state", json.dumps(robot_state))
    
    except json.JSONDecodeError:
        # Logge den fehlerhaften JSON-String zusammen mit dem Topic
        logging.error(f"Error decoding JSON from topic '{msg.topic}': {msg.payload.decode('utf-8')}")
    except ValueError as e:
        logging.error(f"Value error: {e} - Topic: '{msg.topic}'")
    except Exception as e:
        logging.error(f"Unexpected error: {str(e)} - Topic: '{msg.topic}'")
# Initialisiere und starte den MQTT-Client
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(MQTT_BROKER, MQTT_PORT, 60)

def handle_signals(signum, frame):
    logging.info(f"Program interrupted by signal {signum}")
    sys.exit(0)  # Trigger the finally block

def setup_signal_handlers():
    signal.signal(signal.SIGINT, handle_signals)  # Handle Ctrl+C
    signal.signal(signal.SIGTERM, handle_signals) # Handle kill or system shutdown
setup_signal_handlers()
try:
    client.loop_forever()
except KeyboardInterrupt:
    logging.info("Program manually interrupted by user")
finally:
    logging.info("MQTT client disconnected and loop stopped.")
    client.disconnect()
    client.loop_stop()
    
