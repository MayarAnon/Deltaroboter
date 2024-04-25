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
    "robot/workspace"
]

# Initialisiere den Zustand des Roboters
robot_state = {
    "homing": False,
    "currentCoordinates": [0.0, 0.0, 0.0],
    "currentAngles": [0.0, 0.0, 0.0],
    "gripperFeedback": False,
    "gripperMode": "parallelGripper",
    "motorsSpeed": 0,
    "robotWorkspace": [400, 400]
}

# MQTT Callback für das Verbinden zum Broker
def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    # Abonniere alle notwendigen Topics
    for topic in TOPICS:
        client.subscribe(topic)

# MQTT Callback für das Empfangen von Nachrichten
def on_message(client, userdata, msg):
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
    elif msg.topic == 'gripper/mode':
        if data in ["parallelGripper", "compliantGripper", "magnetGripper", "vacuumGripper"]:
            robot_state['gripperMode'] = data
        else:
            print(f"Received invalid gripper mode: {data}")
    elif msg.topic == 'motors/speed':
        robot_state['motorsSpeed'] = data
    elif msg.topic == 'robot/workspace':
        robot_state['robotWorkspace'] = data
    
    # Veröffentliche den aktuellen Robotstate
    client.publish("robot/state", json.dumps(robot_state))

# Initialisiere und starte den MQTT-Client
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(MQTT_BROKER, MQTT_PORT, 60)
client.loop_forever()