#include <mosquittopp.h>
#include <iostream>
#include <json/json.h>
#include <wiringPi.h>
#include <softPwm.h>
#include <thread>
#include <chrono>

// Definiere GPIO-Pins als globale Konstanten
const int ParallelGripperPin = 13; // PWM Output
const int MagnetRelais = 16;

const int PumpRelais = 25;
const int VacuumRelais = 8;

const int PumpValveRelais = 7;
const int VakuumValveRelais = 1;

void setup_gpio() {
    wiringPiSetupGpio(); // Verwende Broadcom Pin-Nummerierung

    pinMode(ParallelGripperPin, OUTPUT);
    softPwmCreate(ParallelGripperPin, 0, 100); // Initialisiere PWM auf Pin 18

    pinMode(PumpRelais, OUTPUT);
    pinMode(VacuumRelais, OUTPUT);
    pinMode(MagnetRelais, OUTPUT);
    pinMode(PumpValveRelais, OUTPUT);
    pinMode(VakuumValveRelais, OUTPUT);
}
class GripperControl : public mosqpp::mosquittopp {
public:
    GripperControl(const char *id, const char *host, int port);
    void on_connect(int rc);
    void on_message(const struct mosquitto_message *message);
    void handle_control_message(const std::string &message);
    void send_feedback(int delay);
};

GripperControl::GripperControl(const char *id, const char *host, int port) : mosquittopp(id) {
    connect(host, port, 60);
}

void GripperControl::on_connect(int rc) {
    if (rc == 0) {
        subscribe(nullptr, "gripper/control");
    } else {
        std::cerr << "Connection failed." << std::endl;
    }
}

void GripperControl::on_message(const struct mosquitto_message *message) {
    if (strcmp(message->topic, "gripper/control") == 0) {
        handle_control_message(std::string((char *)message->payload));
    }
}

void GripperControl::handle_control_message(const std::string &message) {
    Json::Reader reader;
    Json::Value obj;
    reader.parse(message, obj);

    if (!obj["parallelGripper"].isNull()) {
        int pwmValue = obj["parallelGripper"].asInt();
        softPwmWrite(ParallelGripperPin, pwmValue);
        send_feedback(2); 
    } else if (!obj["complientGripper"].isNull()) {
        int value = obj["complientGripper"].asInt();
        if(value==-1){
            digitalWrite(VacuumRelais, HIGH);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));  // kurze Verzögerung vor Ventilaktivierung
            digitalWrite(VakuumValveRelais, HIGH);
            send_feedback(5); // 5 Sekunden Feedback
        }else if(value==1){
            digitalWrite(PumpRelais, HIGH);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));  // kurze Verzögerung vor Ventilaktivierung
            digitalWrite(PumpValveRelais, HIGH);
            send_feedback(5); // 5 Sekunden Feedback
        }else{
            digitalWrite(VacuumRelais, LOW);
            digitalWrite(VakuumValveRelais, LOW);
            digitalWrite(PumpRelais, LOW);
            digitalWrite(PumpValveRelais, LOW);
        }
        
    } else if (!obj["magnetGripper"].isNull()) {
        int value = obj["magnetGripper"].asInt();
        digitalWrite(MagnetRelais, value == 1 ? HIGH : LOW);
        send_feedback(1); // 1 Sekunde Feedback
    } else if (!obj["vakuumGripper"].isNull()) {
        int value = obj["vakuumGripper"].asInt();
        if(value==-1){
            digitalWrite(VacuumRelais, HIGH);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));  // kurze Verzögerung vor Ventilaktivierung
            digitalWrite(VakuumValveRelais, HIGH);
            send_feedback(5); // 5 Sekunden Feedback
        }else if(value==1){
            digitalWrite(PumpRelais, HIGH);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));  // kurze Verzögerung vor Ventilaktivierung
            digitalWrite(PumpValveRelais, HIGH);
            send_feedback(5); // 5 Sekunden Feedback
        }else{
            digitalWrite(VacuumRelais, LOW);
            digitalWrite(VakuumValveRelais, LOW);
            digitalWrite(PumpRelais, LOW);
            digitalWrite(PumpValveRelais, LOW);
        }
    }
}

void GripperControl::send_feedback(int delay) {
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    publish(nullptr, "gripper/feedback", strlen("true"), "true");
}

int main() {
    mosqpp::lib_init();

    setup_gpio(); // Initialisiere die GPIOs

    GripperControl gripper("GripperController", "localhost", 1883);
    while (1) {
        gripper.loop();
    }

    mosqpp::lib_cleanup();
    return 0;
}