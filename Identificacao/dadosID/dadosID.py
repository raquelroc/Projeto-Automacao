import serial
import time

ser = serial.Serial("COM5", 115200)
time.sleep(2) 

with open("dados_pwm.csv", "w") as f:
    f.write("PWM,Distancia_cm\n")
    while True:
        linha = ser.readline().decode().strip()
        print(linha)
        f.write(linha + "\n")

ser.close()