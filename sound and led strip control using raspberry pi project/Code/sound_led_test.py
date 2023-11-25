import time
from rpi_ws281x import PixelStrip, Color
import argparse
import pygame
from threading import Thread
from gpiozero import Button
from signal import pause
from subprocess import Popen
import RPi.GPIO as GPIO

but1pin = 24
but2pin=16
but3pin=23
button_up = Button(20)
button_down = Button(21)
GPIO.setmode(GPIO.BCM)       
GPIO.setup(but1pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(but2pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(but3pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)


f1=0
f2=0
f3=0

# LED strip configuration:
LED_COUNT = 10       # Number of LED pixels.
LED_PIN = 10        # GPIO pin connected to the pixels (10 uses SPI /dev/spidev0.0).
LED_FREQ_HZ = 800000  # LED signal frequency in hertz (usually 800khz)
LED_DMA = 10          # DMA channel to use for generating signal (try 10)
LED_BRIGHTNESS = 255  # Set to 0 for darkest and 255 for brightest
LED_INVERT = False    # True to invert the signal (when using NPN transistor level shift)
LED_CHANNEL = 0       # set to '1' for GPIOs 13, 19, 41, 45 or 53



def increase_volume():
    change_volume('+')

def decrease_volume():
    change_volume('-')

def change_volume(ch_sign):
    Popen(['amixer', 'set', 'Speaker', 'unmute'])
    Popen(['amixer', '-D', 'pulse','sset','Master', '5%{}'.format(ch_sign)])




def dcolorWipe(strip, color, wait_ms):
    """Wipe color across display a pixel at a time."""
    for i in reversed( range(strip.numPixels())):
        strip.setPixelColor(i, color)
        strip.show()
        time.sleep(wait_ms)

def colorWipe(strip, color, wait_ms):
    """Wipe color across display a pixel at a time."""
    for i in range(strip.numPixels()):
        strip.setPixelColor(i, color)
        strip.show()
        time.sleep(wait_ms)

def led_on():
    global f1
    while True:
        while GPIO.input(but1pin) == 0:
            time.sleep(0.5)
            
        f1 = f1 + 1
        if f1 > 1:
            f1 = 0    
                
    while GPIO.input(but1pin) == 1:
        time.sleep(0.5)
    time.sleep(0.001)
    
    
def led_off():
    global f2
    while True:
        while GPIO.input(but2pin) == 0:
            time.sleep(0.5)
            
        f2 = f2 + 1
        if f2 > 1:
            f2 = 0    
                
    while GPIO.input(but2pin) == 1:
        time.sleep(0.5)
    time.sleep(0.00125)
    

def sound_on():
    global f3
    while True:
        while GPIO.input(but3pin) == 0:
            time.sleep(0.5)
            
        f3 = f3 + 1
        if f3 > 1:
            f3 = 0    
                
    while GPIO.input(but3pin) == 1:
        time.sleep(0.5)
    time.sleep(0.0012)


def play_sound():
    
    try:
        print("hello")
        pygame.mixer.init()
        pygame.mixer.music.load("/home/pi/Desktop/a.mp3")
        pygame.mixer.music.play()
    except:
        pass
    



# Main program logic follows:
def main():
    global f1,f2,f3
    # Process arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('-c', '--clear', action='store_true', help='clear the display on exit')
    args = parser.parse_args()

    # Create NeoPixel object with appropriate configuration.
    strip = PixelStrip(LED_COUNT, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT, LED_BRIGHTNESS, LED_CHANNEL)
    # Intialize the library (must be called once before other functions).
    strip.begin()

    print('Press Ctrl-C to quit.')
    if not args.clear:
        print('Use "-c" argument to clear LEDs on exit')

    try:
        colorWipe(strip, Color(255, 0, 0),0)

        while True:
            if(f1==1):
                colorWipe(strip, Color(255, 0, 0),0)
                f1=0
            
            if(f2==1):
                dcolorWipe(strip, Color(0, 0, 0),1)
                f2=0
            if(f3==1):
                play_sound()
                f3=0
                
      
    except KeyboardInterrupt:
        if args.clear:
            colorWipe(strip, Color(0, 0, 0), 1)




main_thread=Thread(target=main)
main_thread.start()
but1_thread=Thread(target=led_on)
but1_thread.start()
but2_thread=Thread(target=led_off)
but2_thread.start()
but3_thread=Thread(target=sound_on)
but3_thread.start()


button_up.when_pressed = increase_volume
button_down.when_pressed = decrease_volume
pause()



