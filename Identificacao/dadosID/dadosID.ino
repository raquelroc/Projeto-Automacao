#include <Arduino.h>

// --- PWM da ventoinha ---
const int pwmPin = 9;

// --- Sensor HC-SR04 ---
const int trigPin = 5;
const int echoPin = 7;

// --- Função para converter tempo → distância ---
long microsecondsToCentimeters(long microseconds) {
  return microseconds / 29 / 2;
}

// --- Medir distância ---
float readDistance() {
  long duration, cm;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000); // timeout 30ms

  cm = microsecondsToCentimeters(duration);
  return cm;
}

void setup() {
  Serial.begin(115200);
  analogWrite(pwmPin, 0);
  delay(10000);
  pinMode(pwmPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.println("Iniciando varredura PWM...");
  Serial.println("PWM,Distancia_cm");
}

void loop() {

  // Varredura completa de PWM
  for (int pwm = 140; pwm <= 180; pwm++) {

    analogWrite(pwmPin, pwm);
    delay(5000);  // tempo para estabilizar o sistema
    for (int i = 0; i < 10; i++){
      delay(100);
      float dist = readDistance();
      // Enviar dados para o PC em formato CSV
      Serial.print(pwm);
      Serial.print(",");
      Serial.println(dist);
    }
  }

  Serial.println("Varredura concluída.");
  analogWrite(pwmPin, 0);
}
