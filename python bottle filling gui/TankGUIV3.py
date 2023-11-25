# -*- coding: utf-8 -*-
"""
Created on Fri Oct 28 19:24:32 2022

@author: Ali Shah
"""

from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtCore import *
import sys
import RPi.GPIO as GPIO
from lib_nrf24 import NRF24
import time
import spidev
from threading import Thread
import  heapq


special_value1="#"
special_value2="#"
vol_list=[]
words=[]
time_list=[0,0,0,0]
time_dec=0
time_flag=False
player_flag=False
count=0
minutes=2
seconds=60
string=""
GPIO.setmode(GPIO.BCM)
pipes = [[0xE8,0xE8,0xF0,0xF0,0xE1],[0xF0,0xF0,0xF0,0xF0,0xE1]]
radio = NRF24(GPIO,spidev.SpiDev())
radio.begin(0,17,4000000)
radio.setPayloadSize(32)
radio.setChannel(0x76)
radio.setDataRate(NRF24.BR_1MBPS)
radio.setPALevel(NRF24.PA_MIN)

radio.setAutoAck(True)
radio.enableDynamicPayloads()
radio.enableAckPayload()

radio.openReadingPipe(1,pipes[1])
radio.printDetails()
radio.startListening()




class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        MainWindow.setObjectName("MainWindow")
        MainWindow.resize(1401, 870)
        MainWindow.setWindowTitle("")
        MainWindow.setAnimated(False)
        self.centralwidget = QtWidgets.QWidget(MainWindow)
        self.centralwidget.setObjectName("centralwidget")
        
        self.progressBarPlayer1 = QtWidgets.QProgressBar(self.centralwidget)
        self.progressBarPlayer1.setGeometry(QtCore.QRect(110, 190, 21, 471))
        #self.progressBarPlayer1.setStyleSheet("color: rgb(0, 85, 255);")
        self.progressBarPlayer1.setMaximum(500)
        self.progressBarPlayer1.setProperty("value",0)
        self.progressBarPlayer1.setTextVisible(False)
        self.progressBarPlayer1.setOrientation(QtCore.Qt.Vertical)
        self.progressBarPlayer1.setObjectName("progressBarPlayer1")
        
        self.progressBarPlayer2 = QtWidgets.QProgressBar(self.centralwidget)
        self.progressBarPlayer2.setGeometry(QtCore.QRect(360, 190, 21, 481))
        self.progressBarPlayer2.setMaximum(500)
        self.progressBarPlayer2.setProperty("value", 0)
        self.progressBarPlayer2.setTextVisible(False)
        self.progressBarPlayer2.setOrientation(QtCore.Qt.Vertical)
        self.progressBarPlayer2.setObjectName("progressBarPlayer2")
        
        self.progressBarPlayer3 = QtWidgets.QProgressBar(self.centralwidget)
        self.progressBarPlayer3.setGeometry(QtCore.QRect(620, 190, 21, 481))
        self.progressBarPlayer3.setMaximum(500)
        self.progressBarPlayer3.setProperty("value", 0)
        self.progressBarPlayer3.setTextVisible(False)
        self.progressBarPlayer3.setOrientation(QtCore.Qt.Vertical)
        self.progressBarPlayer3.setObjectName("progressBarPlayer3")
        
        self.progressBarPlayer4 = QtWidgets.QProgressBar(self.centralwidget)
        self.progressBarPlayer4.setEnabled(True)
        self.progressBarPlayer4.setGeometry(QtCore.QRect(880, 190, 21, 471))
        self.progressBarPlayer4.setCursor(QtGui.QCursor(QtCore.Qt.ArrowCursor))
        self.progressBarPlayer4.setAcceptDrops(False)
        self.progressBarPlayer4.setAutoFillBackground(False)
        self.progressBarPlayer4.setMaximum(500)
        self.progressBarPlayer4.setProperty("value", 0)
        self.progressBarPlayer4.setTextVisible(False)
        self.progressBarPlayer4.setOrientation(QtCore.Qt.Vertical)
        self.progressBarPlayer4.setInvertedAppearance(False)
        self.progressBarPlayer4.setObjectName("progressBarPlayer4")

        self.labelTimer = QtWidgets.QLabel(self.centralwidget)
        self.labelTimer.setGeometry(QtCore.QRect(1110, 100, 271, 81))
        self.labelTimer.setStyleSheet("color: rgb(255, 255, 255);\n""font: 87 20pt \"Arial Black\";")
        self.labelTimer.setObjectName("labelTimer")
        self.labelTimer.setAlignment(QtCore.Qt.AlignCenter)

        
        self.labelVolumePlayer1 = QtWidgets.QLabel(self.centralwidget)
        self.labelVolumePlayer1.setGeometry(QtCore.QRect(110, 760, 191, 51))
        self.labelVolumePlayer1.setStyleSheet("color: rgb(255, 255, 255);\n""font: 87 18pt \"Arial Black\";")
        self.labelVolumePlayer1.setObjectName("labelVolumePlayer1")
        self.labelVolumePlayer1.setAlignment(QtCore.Qt.AlignCenter)

        
        self.labelVolumePlayer2 = QtWidgets.QLabel(self.centralwidget)
        self.labelVolumePlayer2.setGeometry(QtCore.QRect(350, 760, 201, 51))
        self.labelVolumePlayer2.setStyleSheet("color: rgb(255, 255, 255);\n""font: 87 18pt \"Arial Black\";")
        self.labelVolumePlayer2.setObjectName("labelVolumePlayer2")
        self.labelVolumePlayer2.setAlignment(QtCore.Qt.AlignCenter)

        
        
        self.labelVolumePlayer3 = QtWidgets.QLabel(self.centralwidget)
        self.labelVolumePlayer3.setGeometry(QtCore.QRect(610, 760, 201, 51))
        self.labelVolumePlayer3.setStyleSheet("color: rgb(255, 255, 255);\n""font: 87 18pt \"Arial Black\";")
        self.labelVolumePlayer3.setObjectName("labelVolumePlayer3")
        self.labelVolumePlayer3.setAlignment(QtCore.Qt.AlignCenter)
        
        self.labelVolumePlayer4 = QtWidgets.QLabel(self.centralwidget)
        self.labelVolumePlayer4.setGeometry(QtCore.QRect(870, 760, 201, 51))
        self.labelVolumePlayer4.setStyleSheet("color: rgb(255, 255, 255);\n""font: 87 18pt \"Arial Black\";")
        self.labelVolumePlayer4.setObjectName("labelVolumePlayer4")
        self.labelVolumePlayer4.setAlignment(QtCore.Qt.AlignCenter)
        
        self.labelWinner = QtWidgets.QLabel(self.centralwidget)
        self.labelWinner.setGeometry(QtCore.QRect(1100, 290, 291, 61))
        self.labelWinner.setStyleSheet("color: rgb(255, 255, 255);\n""font: 87 22pt \"Arial Black\";")
        self.labelWinner.setObjectName("labelWinner")
        self.labelWinner.setAlignment(QtCore.Qt.AlignCenter)

        
        self.label2ndPlace = QtWidgets.QLabel(self.centralwidget)
        self.label2ndPlace.setGeometry(QtCore.QRect(1100, 450, 291, 61))
        self.label2ndPlace.setStyleSheet("color: rgb(255, 255, 255);\n""font: 87 22pt \"Arial Black\";")
        self.label2ndPlace.setObjectName("label2ndPlace")
        self.label2ndPlace.setAlignment(QtCore.Qt.AlignCenter)
        
        
        self.label3rdPlace = QtWidgets.QLabel(self.centralwidget)
        self.label3rdPlace.setGeometry(QtCore.QRect(1100, 610, 281, 61))
        self.label3rdPlace.setStyleSheet("color: rgb(255, 255, 255);\n""font: 87 22pt \"Arial Black\";")
        self.label3rdPlace.setObjectName("label3rdPlace")
        self.label3rdPlace.setAlignment(QtCore.Qt.AlignCenter)
        
        
        self.label4thPlace = QtWidgets.QLabel(self.centralwidget)
        self.label4thPlace.setGeometry(QtCore.QRect(1100, 780, 281, 61))
        self.label4thPlace.setStyleSheet("color: rgb(255, 255, 255);\n""font: 87 22pt \"Arial Black\";")
        self.label4thPlace.setObjectName("label4thPlace")
        self.label4thPlace.setAlignment(QtCore.Qt.AlignCenter)
        
        self.labelBackgroundImage = QtWidgets.QLabel(self.centralwidget)
        self.labelBackgroundImage.setGeometry(QtCore.QRect(0, 0, 1401, 881))
        self.labelBackgroundImage.setStyleSheet("background-image: url(:/newbgprefix/NewBackground.jpg);")
        self.labelBackgroundImage.setText("")
        self.labelBackgroundImage.setObjectName("labelBackgroundImage")
        
        self.labelTotalVolume = QtWidgets.QLabel(self.centralwidget)
        self.labelTotalVolume.setGeometry(QtCore.QRect(800, 30, 271, 61))
        self.labelTotalVolume.setStyleSheet("color: rgb(0, 0, 84);\n""font: 87 22pt \"Arial Black\";")
        self.labelTotalVolume.setObjectName("labelTotalVolume")
        self.labelTotalVolume.setAlignment(QtCore.Qt.AlignCenter)
        
        self.labelBackgroundImage.raise_()
        self.progressBarPlayer1.raise_()
        self.progressBarPlayer2.raise_()
        self.progressBarPlayer3.raise_()
        self.progressBarPlayer4.raise_()
        self.labelTimer.raise_()
        self.labelVolumePlayer1.raise_()
        self.labelVolumePlayer2.raise_()
        self.labelVolumePlayer3.raise_()
        self.labelVolumePlayer4.raise_()
        self.labelWinner.raise_()
        self.label2ndPlace.raise_()
        self.label3rdPlace.raise_()
        self.label4thPlace.raise_()
        self.labelTotalVolume.raise_()
        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar(MainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 1401, 26))
        self.menubar.setObjectName("menubar")
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QtWidgets.QStatusBar(MainWindow)
        self.statusbar.setObjectName("statusbar")
        MainWindow.setStatusBar(self.statusbar)
        
        self.labelTimer.setText("Press Start \n To Begin")
        self.timer=QTimer()
        self.timer.setInterval(1000)
        self.timer.timeout.connect(self.nrf_thread)
        self.timer.start()

        self.retranslateUi(MainWindow)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)
        
        
        
    def nrf_thread(self):
        
        global string,count,minutes,seconds,player_flag,time_flag,P1,P2,P3,P4,time_dec,words,time_list,vol_list,special_value1,special_value2,result


        
        if(string=="start" and count==0):
            count=1
            
            self.labelTimer.setText("3")
            self.labelWinner.setText("")
            self.label2ndPlace.setText("")
            self.label3rdPlace.setText("")
            self.label4thPlace.setText("")
            self.labelTotalVolume.setText("")

            
        elif(count==1):
            count=2
            self.labelTimer.setText("2")
            
        elif(count==2):
            count=3
            self.labelTimer.setText("1")
        
        elif(count==3):
            count=4
            self.labelTimer.setText("Go!")
            
        elif(count==4):
            count=5
            self.labelTimer.setText("03:00")
            time_dec=180
            
        elif(count==5):
            
            time_dec=time_dec-1
            
            if(time_dec<0):
                time_dec=0
            seconds=seconds-1
            
            if(seconds<10):
                self.labelTimer.setText("0"+str(minutes)+":0"+str(seconds))
            else:
                self.labelTimer.setText("0"+str(minutes)+":"+str(seconds))
                
            if(seconds==0):
                
                minutes=minutes-1
                seconds=60
                
                if(minutes<0):
                    self.labelTimer.setText("Press Start \n To Begin")
                    minutes=2
                    seconds=60
                    count=0
                    time_flag=True
                    
            
            

        if radio.available(0):
            
            recv_msg=[]
            radio.read(recv_msg,radio.getDynamicPayloadSize())
            string =""
            for n in recv_msg:
                if(n>=32 and n<=126):
                    string +=chr(n)
            print("got:{}".format(string))
            words=string.split(',')
            
            
            if(words[0]==special_value1 and words[5]==special_value2 and count==5):

                
                del words[0]
                del words[-1]
                print(words)
           
                vol_list=[int(x)for x in words]
#                 print(time_dec)
                if(count==5):
                    
                    if(vol_list[0]<1500):
                        time_list[0]=time_dec
#                         print(time_list[0])

                    
                    if vol_list[1]<1500:
                        time_list[1]=time_dec
#                         print (time_list[1])
#                 
                
                    
                    if (vol_list[2]<1500):
                        time_list[2]=time_dec
#                         print(time_list[2])
                
         
                    
                    if(vol_list[3]<1500):
                        time_list[3]=time_dec
#                         print(time_list[3])
                    
                    self.progressBarPlayer1.setProperty("value", vol_list[0])
                    self.progressBarPlayer2.setProperty("value", vol_list[1])
                    self.progressBarPlayer3.setProperty("value", vol_list[2])
                    self.progressBarPlayer4.setProperty("value", vol_list[3])
                    self.labelVolumePlayer1.setText(str(vol_list[0])+" ml")
                    self.labelVolumePlayer2.setText(str(vol_list[1])+" ml")
                    self.labelVolumePlayer3.setText(str(vol_list[2])+" ml")
                    self.labelVolumePlayer4.setText(str(vol_list[3])+" ml")
                    self.labelTotalVolume.setText("Total: "+str(vol_list[0]+vol_list[1]+vol_list[2]+vol_list[3])+" ml")
                    
                    if(vol_list[0]==1500 and vol_list[1]==1500 and vol_list[2]==1500 and vol_list[3]==1500):
                        player_flag=True
                
#                 print(time_list)
                
  

        if(time_flag==True):
            
            result=[0,0,0,0]
           
            time_flag=False
            print(vol_list)
            
            result[0]=vol_list[0]+time_list[0]
            result[1]=vol_list[1]+time_list[1]
            result[2]=vol_list[2]+time_list[2]
            result[3]=vol_list[3]+time_list[3]
            print(result)
            
            res=heapq.nlargest(4,range(len(result)),key=result.__getitem__)
            print(res)
            
            self.labelTimer.setText("Press Start \n To Begin")
            self.labelWinner.setText( "Player "+str(res[0]+1))
            self.label2ndPlace.setText( "Player "+str(res[1]+1))
            self.label3rdPlace.setText( "Player "+str(res[2]+1))
            self.label4thPlace.setText( "Player "+str(res[3]+1))
            self.labelTotalVolume.setText("Total: "+str(vol_list[0]+vol_list[1]+vol_list[2]+vol_list[3])+" mm")
            
                
        if(player_flag==True):
            minutes=2
            seconds=60
            count=0
            result=[0,0,0,0]
           
            player_flag=False
            print(vol_list)
            
            result[0]=vol_list[0]+time_list[0]
            result[1]=vol_list[1]+time_list[1]
            result[2]=vol_list[2]+time_list[2]
            result[3]=vol_list[3]+time_list[3]
            print(result)
            
            res=heapq.nlargest(4,range(len(result)),key=result.__getitem__)
            print(res)
            
            self.labelTimer.setText("Press Start \n To Begin")
            self.labelWinner.setText( "Player "+str(res[0]+1))
            self.label2ndPlace.setText( "Player "+str(res[1]+1))
            self.label3rdPlace.setText( "Player "+str(res[2]+1))
            self.label4thPlace.setText( "Player "+str(res[3]+1))
            self.labelTotalVolume.setText("Total: "+str(vol_list[0]+vol_list[1]+vol_list[2]+vol_list[3])+" mm")
            
    

    def retranslateUi(self, MainWindow):
        _translate = QtCore.QCoreApplication.translate
        MainWindow.setWindowTitle(_translate("MainWindow", "Main"))
        self.labelTimer.setText(_translate("MainWindow", "Press Start \n To Begin"))
        self.labelVolumePlayer1.setText(_translate("MainWindow", "-- ml"))
        self.labelVolumePlayer2.setText(_translate("MainWindow", "-- ml"))
        self.labelVolumePlayer3.setText(_translate("MainWindow", "-- ml"))
        self.labelVolumePlayer4.setText(_translate("MainWindow", "-- ml"))
        self.labelWinner.setText(_translate("MainWindow", "--"))
        self.label2ndPlace.setText(_translate("MainWindow", "--"))
        self.label3rdPlace.setText(_translate("MainWindow", "--"))
        self.label4thPlace.setText(_translate("MainWindow", "--"))
        self.labelTotalVolume.setText(_translate("MainWindow", "-- ml"))

import NewBGQRC

if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    MainWindow = QtWidgets.QMainWindow()
    ui = Ui_MainWindow()
    ui.setupUi(MainWindow)
    MainWindow.show()
    sys.exit(app.exec_())

