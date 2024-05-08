import paho.mqtt.client as mqtt
import json

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
    "motors/motionProfil"
]

# Initialisiere den Zustand des Roboters
robot_state = {
    "homing": False,
    "currentCoordinates": [0.0, 0.0, 0.0],
    "currentAngles": [0.0, 0.0, 0.0],
    "gripperFeedback": False,
    "gripperMode": "parallelGripper",
    "motionProfil" : "TrapezProfil",
    "motorsSpeed": 50
}

# MQTT Callback für das Verbinden zum Broker
def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
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
            if data in ["RectangleProfil", "TrapezProfil"]:
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
        
        # Veröffentliche den aktuellen Robotstate
        client.publish("robot/state", json.dumps(robot_state))
    except json.JSONDecodeError:
        print("Error decoding JSON")
    except ValueError as e:
        print(e)
    except Exception as e:
        print(f"Unexpected error: {str(e)}")
# Initialisiere und starte den MQTT-Client
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(MQTT_BROKER, MQTT_PORT, 60)
client.loop_forever()
