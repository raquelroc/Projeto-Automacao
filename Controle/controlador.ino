#include <Arduino.h>
#define KP_50_40 2.1751
#define KP_40_30 2.6028
#define KP_30_20 2.2017
#define KP_20_10 1.6938
#define KP_10_0  3.5341

// ---------------- Ventoinha -----------------
const int pwmPin = 9;
const int tachoPin = 2;

volatile unsigned long pulses = 0;
unsigned long lastMeasureRPM = 0;

float pwmValueFloat = 0; // PWM em float
int pwmValueInt = 0;     // PWM convertido para analogWrite


// ---------------- HC-SR04 -------------------
const int trigPin = 11;
const int echoPin = 13;

// ---------------- CONTROLADOR P CONDICIONAL -----------------------
float setpoint = 20.0; // altura desejada (cm)
float Kp = 0;

const float dt = 1;  // tempo fixo do PID em segundos
unsigned long lastControl = 0;

// -----------------------------------------------------

void setup() {

    // Freq serial
    Serial.begin(115200);

    // Definição as portas
    pinMode(pwmPin, OUTPUT);
    pinMode(tachoPin, INPUT_PULLUP);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    analogWrite(pwmPin, 0);

    Serial.println("Digite a altura desejada (cm) e pressione ENTER.");


}

long microsecondsToCentimeters(long microseconds) {
    // The speed of sound is 340 m/s or 29 microseconds per centimeter.
    // The ping travels out and back, so to find the distance of the object we
    // take half of the distance travelled.
    return microseconds / 29 / 2;
}

// ------------- Leitura HC-SR04 ----------------
float readDistance() {

    // Esse trecho foi retirado do exemplo padrão da IDE do arduino

    long duration, inches, cm;

    // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
    // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);

    cm = microsecondsToCentimeters(duration);

    delay(100);

    return cm;
}


// ---------- Seleção Condicional do Kp ----------
float getKp(float setpoint) {

    if (setpoint > 40 && setpoint <= 50) return Kp_50_40;
    if (setpoint > 30 && setpoint <= 40) return Kp_40_30;
    if (setpoint > 20 && setpoint <= 30) return Kp_30_20;
    if (setpoint > 10 && setpoint <= 20) return Kp_20_10;
    if (setpoint >= 0 && setpoint <= 10) return Kp_10_0;

}

void loop() {
    unsigned long now = millis();

    // ---------- Receber novo setpoint ----------
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();

        if (input.length() > 0) {
            float val = input.toFloat();
            if (val > 3 && val < 300) {
                setpoint = val;
                Serial.print("Novo setpoint definido: ");
                Serial.print(setpoint);
                Serial.println(" cm");
            } else {
                Serial.println("Digite um valor entre 3 e 300 cm.");
            }
        }
    }

    // ---------- Medir distância ----------
    float rawDist = readDistance();

    // ---------- CONTROLADOR P CONDICIONAL ----------
    if (now - lastControl >= dt * 1000) {
        float error = -(setpoint - rawDist);

        // Seleciona o Kp baseado no setpoint
        Kp = getKp(setpoint);

        // Controlador P simples
        float output = Kp * error;

        pwmValueFloat += output;
        pwmValueFloat = constrain(pwmValueFloat, 0, 255);

        pwmValueInt = (int)pwmValueFloat;
        analogWrite(pwmPin, pwmValueInt);

        lastControl = now;

        // ---------- Dados relevantes ----------
        Serial.print("ERROR=");
        Serial.print(error);
        Serial.print(" | SP=");
        Serial.print(setpoint);
        Serial.print("\n cm | Dist=");
        Serial.print(rawDist);
        Serial.print(" cm | PWM=");
        Serial.print(pwmValueFloat);
    }
}
