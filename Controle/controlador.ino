#include <Arduino.h>
#define Kp_50_45 0.2166
#define Kp_45_40 0.2158
#define Kp_40_35 0.1938
#define Kp_35_30 0.6444
#define Kp_30_25 1.1372
#define Kp_25_20 1.4089
#define Kp_20_15 1.4278
#define Kp_15_10 1.4183
#define Kp_10_5 1.1659
#define Kp_5_0  0.9099

// ---------------- Ventoinha -----------------
const int pwmPin = 9;
const int tachoPin = 2;

volatile unsigned long pulses = 0;
unsigned long lastMeasureRPM = 0;

float pwmValueFloat = 140;  // PWM em float
int pwmValueInt = 0;      // PWM convertido para analogWrite


// ---------------- HC-SR04 -------------------
const int trigPin = 5;
const int echoPin = 7;

// ---------------- CONTROLADOR P CONDICIONAL -----------------------
float setpoint = 20.0;  // altura desejada (cm)
float Kp = 0;

const float dt = 3;  // tempo fixo do PID em segundos
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

  analogWrite(pwmPin, pwmValueFloat);

  Serial.println("Digite a altura desejada (cm) e pressione ENTER.");

  delay(10000);
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

  if (setpoint > 45 && setpoint <= 50) return Kp_50_45;
  if (setpoint > 40 && setpoint <= 45) return Kp_45_40;
  if (setpoint > 35 && setpoint <= 40) return Kp_40_35;
  if (setpoint > 30 && setpoint <= 35) return Kp_35_30;
  if (setpoint > 25 && setpoint <= 30) return Kp_30_25;
  if (setpoint > 20 && setpoint <= 25) return Kp_25_20;
  if (setpoint > 15 && setpoint <= 20) return Kp_20_15;
  if (setpoint > 10 && setpoint <= 15) return Kp_15_10;
  if (setpoint > 5 && setpoint <= 10) return Kp_10_5;
  if (setpoint >= 0 && setpoint <= 5) return Kp_5_0;
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
        setpoint = 50 - val;
        Serial.print("Novo setpoint definido: ");
        Serial.print(val);
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
    pwmValueFloat = constrain(pwmValueFloat, 0, 175);

    pwmValueInt = (int)pwmValueFloat;
    analogWrite(pwmPin, pwmValueInt);

    lastControl = now;

    // ---------- Dados relevantes ----------
    Serial.print("\n ERROR=");
    Serial.print(error);
    Serial.print(" | SP Distancia=");
    Serial.print(setpoint);
    Serial.print("cm | \n Distancia=");
    Serial.print(rawDist);
    Serial.print(" cm | PWM=");
    Serial.print(pwmValueFloat);
    Serial.print(" | \n Kp=");
    Serial.print(Kp);
  }
}

