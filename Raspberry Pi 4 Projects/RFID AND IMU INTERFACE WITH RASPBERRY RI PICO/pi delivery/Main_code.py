import time
from time import sleep
import smbus
import RPi.GPIO as GPIO
from mfrc522 import SimpleMFRC522
from threading import Thread
import serial                                           
import sys
import os.path
import csv
import socket
import pickle
import math
from usim800 import sim800
import json

GPIO.setwarnings(False)
#Select GPIO Mode
GPIO.setmode(GPIO.BOARD)
#set red,green and blue pins
redPin = 16
greenPin = 36
bluePin = 18
#set pins as outputs
GPIO.setup(redPin,GPIO.OUT) #red
GPIO.setup(greenPin,GPIO.OUT) #green
GPIO.setup(bluePin,GPIO.OUT) #blue

PWR_MGMT_1=0X6B
SMPLRT_DIV=0X19
CONFIG=0X1A
GYRO_CONFIG=0x1B
INT_ENABLE=0X38
ACCEL_XOUT_H=0x3B
ACCEL_YOUT_H=0x3D
ACCEL_ZOUT_H=0x3F
GYRO_XOUT_H=0x43
GYRO_YOUT_H=0x45
GYRO_ZOUT_H=0x47

card=False
E=1
total_A=0
led=1
imu_flag=0
n=0
init=0
ID=0
f=1
value=(321406573013)
Ax=0
Ay=0
Az=0
e_f=0
header_flag=0
header_flag2=0
lat=0
longi=0
alti=0
nmea_time=0
gps=0
gsm=0
roll=0
pitch=0
yaw=0
ab=0



def send_data_GPRS():
    gsm=sim800(baudrate=9600,path="/dev/ttyUSB0")
    gsm.requests.APN="internet"
    f=open('Accel.csv','rb')
    l=f.read(409600)
    while True:
#         print(l)
        gsm.requests.post(url="",l=json.dumps(l))
        break
    f.close()
    print("sent accel data")
    time.sleep(0.1)
    
    nf=open('gprs.csv','rb')
    nl=nf.read(409600)
    while True:
#         print(nl)
        gsm.requests.post(url="",nl=json.dumps(nl))
        break
    nf.close()
    print("sent gprs data")




def send_data_socket():
    host='192.168.10.12'
    port=5555
    s=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    s.connect((host,port))
    f=open('Accel.csv','rb')
    l=f.read(409600)
    while True:
        print(l)
        s.send(l)
        break
    f.close()
    print("sent")
    s.close()
    
    time.sleep(0.1)
    
    s=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    s.connect((host,port))
    f=open('gprs.csv','rb')
    l=f.read(409600)
    while True:
        print(l)
        s.send(l)
        break
    f.close()
    print("sent2")
    s.close()
    

def convert_to_degrees(raw_value):
    decimal_value = raw_value/100.00
    degrees = int(decimal_value)
    mm_mmmm = (decimal_value - int(decimal_value))/0.6
    position = degrees + mm_mmmm
    position = "%.4f" %(position)
    return position


def MPU_Init():
    bus.write_byte_data(Device_Address,SMPLRT_DIV,7)
    bus.write_byte_data(Device_Address,PWR_MGMT_1,1)
    bus.write_byte_data(Device_Address,CONFIG,0)
    bus.write_byte_data(Device_Address,GYRO_CONFIG,24)
    bus.write_byte_data(Device_Address,INT_ENABLE,1)
    
def read_raw_data(addr):
    high=bus.read_byte_data(Device_Address,addr)
    low=bus.read_byte_data(Device_Address,addr+1)
    value=((high<<8)|low)
    if(value>32768):
        value=value-65536
    return value

def gps_data():
    global alti,longi,lat,header_flag,nmea_time
    
    while True:  
        if(ser.inWaiting()>0):
            received_data = (str)(ser.readline()) #read NMEA string received
#             print(received_data)
            GPGGA_data_available = received_data.find(gpgga_info)   #check for NMEA GPGGA string                
            if (GPGGA_data_available>0):
                GPGGA_buffer = received_data.split("$GPGGA,",1)[1]  #store data coming after “$GPGGA,” string
                NMEA_buff = (GPGGA_buffer.split(','))
                nmea_time = []
                nmea_latitude = []
                nmea_longitude = []
                nmea_time = NMEA_buff[0]                    #extract time from GPGGA string
                nmea_latitude = NMEA_buff[1]                #extract latitude from GPGGA string
                nmea_longitude = NMEA_buff[3]               #extract longitude from GPGGA string
                nmea_altitude=NMEA_buff[8]
#                 print("NMEA Time: ", nmea_time,'\n')
                lat = float(nmea_latitude)
                lat = convert_to_degrees(lat)
                longi = float(nmea_longitude)
                longi = convert_to_degrees(longi)
                alti=nmea_altitude
#                 print ("NMEA Latitude:", lat,"NMEA Longitude:", longi,'\n')
#                 print("altitude is:"+alti)
#                 print('\n')
        time.sleep(0.2)   

def accel_check():
    global total_A,AX,AY,AZ,roll,pitch,yaw
    
    while(1):
        acc_x=read_raw_data(ACCEL_XOUT_H)
        acc_y=read_raw_data(ACCEL_YOUT_H)
        acc_z=read_raw_data(ACCEL_ZOUT_H)
        
        gyro_x=read_raw_data(GYRO_XOUT_H)
        gyro_y=read_raw_data(GYRO_YOUT_H)
        gyro_z=read_raw_data(GYRO_ZOUT_H)
        
        AX=acc_x/16384.0
        AY=acc_y/16384.0
        AZ=acc_z/16384.0
        
        Gx=gyro_x/131.0
        Gy=gyro_y/131.0
        Gz=gyro_z/131.0
        
        roll_angle=math.atan2(AY,AZ)*180/math.pi
        pitch_angle=math.atan2(AX,math.sqrt(AY*AY+AZ*AZ))*180/math.pi
        va=0.962
        dt=0.020
        roll=va*(roll+Gx*dt)+(1-va)*roll_angle
        pitch=va*(pitch+Gy*dt)+(1-va)*pitch_angle
        yaw=Gz
        total_A=AX+AY+AZ
#         print(total_A)
        sleep(0.02)
        
def rfid_check():
    global ID
    while(1):
        id=reader.read()
        ID=id[0]
        sleep(0.11)
        
def blue_blink():
    GPIO.output(18,GPIO.HIGH)
    sleep(1)
    GPIO.output(18,GPIO.LOW)
    sleep(1)
    GPIO.output(18,GPIO.HIGH)
    sleep(1)
    GPIO.output(18,GPIO.LOW)
    
def red_blink():
    GPIO.output(16,GPIO.HIGH)
    sleep(1)
    GPIO.output(16,GPIO.LOW)
    sleep(1)
    GPIO.output(16,GPIO.HIGH)
    sleep(1)
    GPIO.output(16,GPIO.LOW)
    
def green_blink():
    GPIO.output(36,GPIO.HIGH)
    sleep(1)
    GPIO.output(36,GPIO.LOW)
    sleep(1)
    GPIO.output(36,GPIO.HIGH)
    sleep(1)
    GPIO.output(36,GPIO.LOW)

def off():
    GPIO.output(16,GPIO.LOW)
    GPIO.output(18,GPIO.LOW)
    GPIO.output(36,GPIO.LOW)
    
def check_imu():
    global Device_Address
    global n
    global imu_flag
    global init
    for device in range(128):
        try:
            bus.read_byte(device)
#            ` print(device)
            if(device==104):
                Device_Address=0x68
                MPU_Init()
                thread2=Thread(target=accel_check)
                thread2.start()
                init=init+1
                print("imu found")
            elif(device !=104):
#                 print("noimu found")
                init=0
        except:
            pass
    sleep(1)    


def check_gps():
    global lat,longi,alti,gps,init
    la=lat
    lo=longi
    al=alti
    try:
        if(bool(la)==False or bool(lo)==False or bool(al)==False):
            gps=1
            print("gps error")
            init=0
        else:
            gps=0
            init=init+1
        
    except:
        pass
    
def check_gsm():
    global gsm,init
    port.write(b'AT\r')
    rcv = port.read(10)
    if(str(rcv).find("OK")!=-1):
        gsm=0
        init=init+1
    else:
        print("GSM error")
        gsm=1


def send_error_msg():
    try:
        
        port.write(b'AT\r')
        rcv = port.read(10)
        print(rcv)
        time.sleep(1)
        port.write(b"AT+CMGF=1\r")
        print("Text Mode Enabled…")
        time.sleep(3)
        port.write(b'AT+CMGS="+923209898372"\r')
        msg = "Error in module"
        print("sending message….")
        time.sleep(3)
        port.reset_output_buffer()
        time.sleep(1)
        port.write(str.encode(msg+chr(26)))
        time.sleep(3)
        print("message sent…")
    except:
        pass
    
def turn_off():
    print("Shutting down!")
    os.system("sudo shutdown -h now")


def sudo_reboot():
    print("rebooting!")
    os.system("sudo reboot -h now")

def main_prog():
    global led,total_A,n,init,ID,value,f,header_flag,lat,longi,alti,AX,AY,AZ,E,ab,header_flag2,pich,yaw,role,nmea_time
    off()
    check_imu()
    check_gps()
    check_gsm()
#   check_rfid()
    while True:
        f=1
        if(n==0 and led==1):
            led=0
            blue_blink()
            
        if(init==3):
#             print(total_A)
            off()
            GPIO.output(greenPin,GPIO.HIGH)
            if(ID==value):
                green_blink()
#               print("access Granted")`
                if(total_A>E):
                    print("recording data for first check")
                    with open ("gprs.csv", 'a') as csvfile:
                        headers = ['Time','Latitude', 'Longitude', 'Altitude']
                        writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                        if not file_exists and header_flag==0:
                            header_flag=1
                            writer.writeheader()  # file doesn't exist yet, write a header
                        writer.writerow({'Time':nmea_time,'Latitude': lat, 'Longitude': longi, 'Altitude': alti})
                        
                    with open ("Accel.csv", 'a') as csvfile:
                        headers = ['AX', 'AY', 'AZ','Pitch','Roll','YAW']
                        writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                        if not file_exists2 and header_flag2==0:
                            header_flag2=1
                            writer.writeheader()  # file doesn't exist yet, write a header
                        writer.writerow({'AX': AX, 'AY': AY, 'Pitch': pitch,'Roll':roll,'YAW':yaw})

                        
                    while True:
                        if(total_A>E):
                            E=1
                            print("recording data for while")
                            with open ("gprs.csv", 'a') as csvfile:
                                headers = ['Time','Latitude', 'Longitude', 'Altitude']
                                writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                                if not file_exists and header_flag==0:
                                    header_flag=1
                                    writer.writeheader()  # file doesn't exist yet, write a header
                                writer.writerow({'Time':nmea_time,'Latitude': lat, 'Longitude': longi, 'Altitude': alti})
                        
                            with open ("Accel.csv", 'a') as csvfile:
                                headers = ['AX', 'AY', 'AZ','Pitch','Roll','YAW']
                                writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                                if not file_exists2 and header_flag2==0:
                                    header_flag2=1
                                    writer.writeheader()  # file doesn't exist yet, write a header
                                writer.writerow({'AX': AX, 'AY': AY, 'Pitch': pitch,'Roll':roll,'YAW':yaw})

                            sleep(2)
                               
                                
                        else:
                           
                            print("iam in else beforesleep")
                            time.sleep(2)
                            
                            if(total_A>E):   #ab
#                                 ab=1
                                print("recording data")
                                with open ("gprs.csv", 'a') as csvfile:
                                    headers = ['Time','Latitude', 'Longitude', 'Altitude']
                                    writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                                    if not file_exists and header_flag==0:
                                        header_flag=1
                                        writer.writeheader()  # file doesn't exist yet, write a header
                                    writer.writerow({'Time':nmea_time,'Latitude': lat, 'Longitude': longi, 'Altitude': alti})
                        
                                with open ("Accel.csv", 'a') as csvfile:
                                    headers = ['AX', 'AY', 'AZ','Pitch','Roll','YAW']
                                    writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                                    if not file_exists2 and header_flag2==0:
                                        header_flag2=1
                                        writer.writeheader()  # file doesn't exist yet, write a header
                                    writer.writerow({'AX': AX, 'AY': AY, 'Pitch': pitch,'Roll':roll,'YAW':yaw})

                                
                            else:
                                print("broke out of while")
                                break
                            
                    print("sleeping")        
                    timeout=time.time()+60*2
                    while True:
                        print("recording data")
                        with open ("gprs.csv", 'a') as csvfile:
                            headers = ['Time','Latitude', 'Longitude', 'Altitude']
                            writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                            if not file_exists and header_flag==0:
                                header_flag=1
                                writer.writeheader()  # file doesn't exist yet, write a header
                            writer.writerow({'Time':nmea_time,'Latitude': lat, 'Longitude': longi, 'Altitude': alti})
                        
                        with open ("Accel.csv", 'a') as csvfile:
                            headers = ['AX', 'AY', 'AZ','Pitch','Roll','YAW']
                            writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                            if not file_exists2 and header_flag2==0:
                                header_flag2=1
                                writer.writeheader()  # file doesn't exist yet, write a header
                            writer.writerow({'AX': AX, 'AY': AY, 'Pitch': pitch,'Roll':roll,'YAW':yaw})

                       
                       
                        if(time.time()>timeout):
                            break
                        sleep(2)
                        
#                     send_data_socket()
                    send_data_GPRS()
                    print("data sent")
                    ID=0
#                     E=0
#                     ab=0
                               
                else:
                    global e_f
                    print("engine off")
                    timeout=time.time()+60*3
                    while True:
                        print("inwhile")
                        if(time.time()>timeout ):
                            e_f=1
                            print("timouet flag is triggered")
                            break
                        
                        if(total_A>E):
                            break
                            off()
                            green_blink()
                            print('recording data')
                            with open ("gprs.csv", 'a') as csvfile:
                                headers = ['Time','Latitude', 'Longitude', 'Altitude']
                                writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                                if not file_exists and header_flag==0:
                                    header_flag=1
                                    writer.writeheader()  # file doesn't exist yet, write a header
                                writer.writerow({'Time':nmea_time,'Latitude': lat, 'Longitude': longi, 'Altitude': alti})
                        
                            with open ("Accel.csv", 'a') as csvfile:
                                headers = ['AX', 'AY', 'AZ','Pitch','Roll','YAW']
                                writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                                if not file_exists2 and header_flag2==0:
                                    header_flag2=1
                                    writer.writeheader()  # file doesn't exist yet, write a header
                                writer.writerow({'AX': AX, 'AY': AY, 'Pitch': pitch,'Roll':roll,'YAW':yaw})

                            
                            while True:
                                if(total_A>E):
                                    print('record data')
                                    with open ("gprs.csv", 'a') as csvfile:
                                        headers = ['Time','Latitude', 'Longitude', 'Altitude']
                                        writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                                        if not file_exists and header_flag==0:
                                            header_flag=1
                                            writer.writeheader()  # file doesn't exist yet, write a header
                                        writer.writerow({'Time':nmea_time,'Latitude': lat, 'Longitude': longi, 'Altitude': alti})
                        
                                    with open ("Accel.csv", 'a') as csvfile:
                                        headers = ['AX', 'AY', 'AZ','Pitch','Roll','YAW']
                                        writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                                        if not file_exists2 and header_flag2==0:
                                            header_flag2=1
                                            writer.writeheader()  # file doesn't exist yet, write a header
                                        writer.writerow({'AX': AX, 'AY': AY, 'Pitch': pitch,'Roll':roll,'YAW':yaw})

                                else:
                                    sleep(180)
                                    if(total_A>E):
                                        with open ("gprs.csv", 'a') as csvfile:
                                            headers = ['Time','Latitude', 'Longitude', 'Altitude']
                                            writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                                            if not file_exists and header_flag==0:
                                                header_flag=1
                                                writer.writeheader()  # file doesn't exist yet, write a header
                                            writer.writerow({'Time':nmea_time,'Latitude': lat, 'Longitude': longi, 'Altitude': alti})
                        
                                        with open ("Accel.csv", 'a') as csvfile:
                                            headers = ['AX', 'AY', 'AZ','Pitch','Roll','YAW']
                                            writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                                            if not file_exists2 and header_flag2==0:
                                                header_flag2=1
                                                writer.writeheader()  # file doesn't exist yet, write a header
                                            writer.writerow({'AX': AX, 'AY': AY, 'Pitch': pitch,'Roll':roll,'YAW':yaw})

                                    else:
                                        break
                                    
                            timeout=time.time()+60*2
                            while True:
                                if(time.time()>timeout):
                                    break
                                with open ("gprs.csv", 'a') as csvfile:
                                    headers = ['Time','Latitude', 'Longitude', 'Altitude']
                                    writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                                    if not file_exists and header_flag==0:
                                        header_flag=1
                                        writer.writeheader()  # file doesn't exist yet, write a header
                                    writer.writerow({'Time':nmea_time,'Latitude': lat, 'Longitude': longi, 'Altitude': alti})
                        
                                with open ("Accel.csv", 'a') as csvfile:
                                    headers = ['AX', 'AY', 'AZ','Pitch','Roll','YAW']
                                    writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                                    if not file_exists2 and header_flag2==0:
                                        header_flag2=1
                                        writer.writeheader()  # file doesn't exist yet, write a header
                                    writer.writerow({'AX': AX, 'AY': AY, 'Pitch': pitch,'Roll':roll,'YAW':yaw})

#                             send_data_socket()
                            send_data_GPRS()
                            ID=0        
                            
                        else:
                            
                            if(e_f==1):
                                off()
                                GPIO.output(greenPin,GPIO.HIGH)
                                ID=0
                                print("out of while")
                                e_f=0
                                break
                            
                    
            elif (ID!=0 and ID!=value and f==1):
                print("incorrect id")
                if(total_A>E):
                    print("reciord data")
                    with open ("gprs.csv", 'a') as csvfile:
                        headers = ['Time','Latitude', 'Longitude', 'Altitude']
                        writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                        if not file_exists and header_flag==0:
                            header_flag=1
                            writer.writeheader()  # file doesn't exist yet, write a header
                        writer.writerow({'Time':nmea_time,'Latitude': lat, 'Longitude': longi, 'Altitude': alti})
                        
                    with open ("Accel.csv", 'a') as csvfile:
                        headers = ['AX', 'AY', 'AZ','Pitch','Roll','YAW']
                        writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                        if not file_exists2 and header_flag2==0:
                            header_flag2=1
                            writer.writeheader()  # file doesn't exist yet, write a header
                        writer.writerow({'AX': AX, 'AY': AY, 'Pitch': pitch,'Roll':roll,'YAW':yaw})

                        
                    while True:
                        if(total_A>E):
#                             E=1
                            print("reciord data")
                            with open ("gprs.csv", 'a') as csvfile:
                                headers = ['Time','Latitude', 'Longitude', 'Altitude']
                                writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                                if not file_exists and header_flag==0:
                                    header_flag=1
                                    writer.writeheader()  # file doesn't exist yet, write a header
                                writer.writerow({'Time':nmea_time,'Latitude': lat, 'Longitude': longi, 'Altitude': alti})
                        
                            with open ("Accel.csv", 'a') as csvfile:
                                headers = ['AX', 'AY', 'AZ','Pitch','Roll','YAW']
                                writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                                if not file_exists2 and header_flag2==0:
                                    header_flag2=1
                                    writer.writeheader()  # file doesn't exist yet, write a header
                                writer.writerow({'AX': AX, 'AY': AY, 'Pitch': pitch,'Roll':roll,'YAW':yaw})

                        else:                            
                            time.sleep(180) #sleep 3mins here
                            if(total_A>E):
#                                 ab=1
                                print("reciord data")
                                with open ("gprs.csv", 'a') as csvfile:
                                    headers = ['Time','Latitude', 'Longitude', 'Altitude']
                                    writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                                    if not file_exists and header_flag==0:
                                        header_flag=1
                                        writer.writeheader()  # file doesn't exist yet, write a header
                                    writer.writerow({'Time':nmea_time,'Latitude': lat, 'Longitude': longi, 'Altitude': alti})
                        
                                with open ("Accel.csv", 'a') as csvfile:
                                    headers = ['AX', 'AY', 'AZ','Pitch','Roll','YAW']
                                    writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                                    if not file_exists2 and header_flag2==0:
                                        header_flag2=1
                                        writer.writeheader()  # file doesn't exist yet, write a header
                                    writer.writerow({'AX': AX, 'AY': AY, 'Pitch': pitch,'Roll':roll,'YAW':yaw})

                            else:
                                break
                    
                            
                    print("sleeping")
                    timeout=time.time()+60*2
                    while True:
                        with open ("gprs.csv", 'a') as csvfile:
                            headers = ['Time','Latitude', 'Longitude', 'Altitude']
                            writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                            if not file_exists and header_flag==0:
                                header_flag=1
                                writer.writeheader()  # file doesn't exist yet, write a header
                            writer.writerow({'Time':nmea_time,'Latitude': lat, 'Longitude': longi, 'Altitude': alti})
                        
                        with open ("Accel.csv", 'a') as csvfile:
                            headers = ['AX', 'AY', 'AZ','Pitch','Roll','YAW']
                            writer = csv.DictWriter(csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
                            if not file_exists2 and header_flag2==0:
                                header_flag2=1
                                writer.writeheader()  # file doesn't exist yet, write a header
                            writer.writerow({'AX': AX, 'AY': AY, 'Pitch': pitch,'Roll':roll,'YAW':yaw})

                                    
                        if(time.time()>timeout):
                            break
#                     send_data_socket()
                    send_data_GPRS()
                    ID=0    
#                     E=0
#                     ab=0
                    
                else:
                    ID=0
                    f=0
                    print("in else")
                    
                    
        else:
            print("in the main program else")
            
#             print(init)
            fi= open ("n.csv", 'rb')
            li=fi.read(100)
            if(int(li)==0):
                n=int(li)
                n=n+1
                fi=open("n.csv","w")
                fi.write(str(n))
                fi.close()
            if(int(li)==1):
                n=int(li)
                n=n+1
                fi=open("n.csv","w")
                fi.write(str(n))
                fi.close()
                
            if(int(li)==2):
                n=int(li)
                n=n+1
                fi=open("n.csv","w")
                fi.write(str(n))
                fi.close()
                
            if(int(li)==3):
                n=int(li)
                li=0
                fi=open("n.csv","w")
                fi.write(str(li))
                fi.close()
                
            off()
            red_blink()
            check_gps()
            check_gsm()
            if(gsm==0 and gps==0):
                send_error_msg()
                turn_off()
            else:
                if(n==1):
                    print("reboot 1")
                    sudo_reboot()
                elif(n==2):
                    print("reboot 2")
                    sudo_reboot()
                elif(n==3):
                    print("turn off")
                    turn_off()

        sleep(0.11)

    
    
    
    
file_exists = os.path.isfile("gprs.csv")
file_exists2 = os.path.isfile("Accel.csv")
file2=os.path.isfile("n.csv")     
ser = serial.Serial ("/dev/ttyAMA0",baudrate=9600, timeout=1)
port = serial.Serial("/dev/ttyUSB0", baudrate=9600, timeout=1)
gpgga_info = "$GPGGA,"
GPGGA_buffer = 0
NMEA_buff = 0        
reader= SimpleMFRC522()
bus=smbus.SMBus(1)

main_thread=Thread(target=main_prog)
main_thread.start()

gps_thread=Thread(target=gps_data)
gps_thread.start()

rfid_thread=Thread(target=rfid_check)
rfid_thread.start()

