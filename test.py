import RPi.GPIO as GPIO
from time import sleep

botao = 18 
leds = [19, 26, 22]

print("heyehyehehe")


GPIO.setmode(GPIO.BCM) 
GPIO.setup(botao, GPIO.IN)
GPIO.setup(leds[0], GPIO.OUT)
GPIO.setup(leds[1], GPIO.OUT)
GPIO.setup(leds[2], GPIO.OUT)

print("heyehyehehe")

counter = 0
while True:
    GPIO.output(leds, (
            GPIO.HIGH if counter & 0b0100 else GPIO.LOW,
            GPIO.HIGH if counter & 0b0010 else GPIO.LOW,
            GPIO.HIGH if counter & 0b0001 else GPIO.LOW,
        )
    )

    # print(f"now gpios are: {GPIO.input(leds[0])} {GPIO.input(leds[1])} {GPIO.input(leds[2])}")
    
    sleep(0.1)

    if GPIO.input(botao):
        print(f"VAGA {counter+1} IS UP")

    counter += 1
    if counter >= 8:
        counter = 0
