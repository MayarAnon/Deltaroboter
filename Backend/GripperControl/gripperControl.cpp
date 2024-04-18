#include <iostream>
#include <json/json.h>
#include <mosquittopp.h>
#include <pigpio.h>
#include <thread>
#include <chrono>

// Definiere GPIO-Pins als globale Konstanten
const int ParallelGripperPin = 13; // PWM Output
const int MagnetRelais = 16;

const int PumpRelais = 25;
const int VacuumRelais = 8;

const int PumpValveRelais = 7;
const int VacuumValveRelais = 1;

void setup_gpio() {
    if (gpioInitialise() < 0) {
        std::cerr << "pigpio initialisation failed.\n";
        return;
    }

    // Setze Pins als Output
    gpioSetMode(ParallelGripperPin, PI_OUTPUT);
    gpioSetMode(MagnetRelais, PI_OUTPUT);
    gpioSetMode(PumpRelais, PI_OUTPUT);
    gpioSetMode(VacuumRelais, PI_OUTPUT);
    gpioSetMode(PumpValveRelais, PI_OUTPUT);
    gpioSetMode(VacuumValveRelais, PI_OUTPUT);

    // Initialisiere PWM auf Pin 13
    gpioSetPWMfrequency(ParallelGripperPin, 800); // Setze PWM-Frequenz
    gpioSetPWMrange(ParallelGripperPin, 100);     // Setze PWM Range
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
        gpioPWM(ParallelGripperPin, pwmValue * 255 / 100);  // Umrechnung auf Pigpio PWM Bereich (0-255)
        send_feedback(2);
    } else if (!obj["complientGripper"].isNull()) {
        int value = obj["complientGripper"].asInt();
        if (value == -1) {
            gpioWrite(VacuumRelais, PI_HIGH);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));  // kurze Verzögerung vor Ventilaktivierung
            gpioWrite(VakuumValveRelais, PI_HIGH);
            send_feedback(5);  // 5 Sekunden Feedback
        } else if (value == 1) {
            gpioWrite(PumpRelais, PI_HIGH);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));  // kurze Verzögerung vor Ventilaktivierung
            gpioWrite(PumpValveRelais, PI_HIGH);
            send_feedback(5);  // 5 Sekunden Feedback
        } else {
            gpioWrite(VacuumRelais, PI_LOW);
            gpioWrite(VakuumValveRelais, PI_LOW);
            gpioWrite(PumpRelais, PI_LOW);
            gpioWrite(PumpValveRelais, PI_LOW);
        }
    } else if (!obj["magnetGripper"].isNull()) {
        int value = obj["magnetGripper"].asInt();
        gpioWrite(MagnetRelais, value == 1 ? PI_HIGH : PI_LOW);
        send_feedback(1);  // 1 Sekunde Feedback
    } else if (!obj["vakuumGripper"].isNull()) {
        int value = obj["vakuumGripper"].asInt();
        if (value == -1) {
            gpioWrite(VacuumRelais, PI_HIGH);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));  // kurze Verzögerung vor Ventilaktivierung
            gpioWrite(VakuumValveRelais, PI_HIGH);
            send_feedback(5);  // 5 Sekunden Feedback
        } else if (value == 1) {
            gpioWrite(PumpRelais, PI_HIGH);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));  // kurze Verzögerung vor Ventilaktivierung
            gpioWrite(PumpValveRelais, PI_HIGH);
            send_feedback(5);  // 5 Sekunden Feedback
        } else {
            gpioWrite(VacuumRelais, PI_LOW);
            gpioWrite(VakuumValveRelais, PI_LOW);
            gpioWrite(PumpRelais, PI_LOW);
            gpioWrite(PumpValveRelais, PI_LOW);
        }
    }
}


void GripperControl::send_feedback(int delay) {
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    publish(nullptr, "gripper/feedback", strlen("true"), "true");
}

int main() {
    setup_gpio(); // Initialisiere die GPIOs

    GripperControl gripper("GripperController", "localhost", 1883);
    while (1) {
        gripper.loop();
    }

    gpioTerminate(); // Bereinige pigpio-Ressourcen
    mosqpp::lib_cleanup();
    return 0;
}



