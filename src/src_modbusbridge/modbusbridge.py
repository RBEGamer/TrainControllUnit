#!/usr/bin/env python
from pymodbus.client.sync import ModbusTcpClient
import paho.mqtt.client as mqtt
import json
import time
import signal
import os

do_exit = False

def signal_handler(signum, frame):
    global do_exit
    do_exit = True
    print("signal")
    sys.exit(0)

signal.signal(signal.SIGALRM, signal_handler)
signal.signal(signal.SIGINT, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)

def on_connect(client, userdata, flags, rc):
    print("connected with res:" +str(rc))
    client.subscribe("fbcommands")
    client.subscribe("tablet_event")
    client.subscribe("remote_control_event")


def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))
    if msg.topic == "tablet_event":
        callbackui(str(msg.payload))

    if msg.topic == "remote_control_event":
        callbackfb(str(msg.payload))

mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

mqtt_client.connect("127.0.0.1")

mqtt_client.loop_start()


client = None
brklvl = 0
lastvel = 0
vellevl = 0

def callbackfb(data_string):
    tmp = json.loads(data_string)
    client.write_register(0, tmp['breaklevel'], unit=1)
    client.write_register(1, (tmp['velocity']*2)+900, unit=1)



def callbackui(data):
    global brklvl
    #rospy.loginfo(rospy.get_caller_id() + "UI %s", data.data)
    tmp = json.loads(data)
    print(tmp['event'])
    print(tmp)
    if(tmp['event'] == 'dirch'):
        rr = client.read_coils(1,1,unit=1)
        if( rr.bits[0]):
            client.write_coil(1,False, unit=1)
        else:
            client.write_coil(1,True, unit=1)
        client.write_coil(0,True, unit=1)

    if(tmp['event'] == 'ctlmode'):
        rr = client.read_coils(2,1,unit=1)
        if( rr.bits[0]):
            client.write_coil(3,False, unit=1)
        else:
            client.write_coil(3,True, unit=1)
        #client.write_coil(4,True, unit=1)

    if (tmp['event'] == 'asc'):
        print('ascch event')
        # TODO

        rr = client.read_coils(12,1,unit=1)
        if( rr.bits[0]):
            client.write_coil(12,0, unit=1)
        else:
            client.write_coil(12,1, unit=1)
    if (tmp['event'] == 'kompmode'):
        print('kompmode event TODO')
        #response.registers[22] 1/0/2 schreiben
        #client.write_register(22,2, unit = 1)

    if (tmp['event'] == 'embreak'):
        print('embreak event')
        client.write_coil(11, True, unit=1)
        time.sleep(1)
        client.write_coil(11, False, unit=1)



    if (tmp['event'] == 'lightmode'):
        print('lightmode event')
        client.write_register(23,1, unit = 1)


    if (tmp['event'] == 'brkstep0'):
        print('brkstep0 event')
        client.write_register(0,0, unit = 1)
    if (tmp['event'] == 'brkstep1'):
        print('brkstep0 event')
        client.write_register(0,1, unit = 1)
    if (tmp['event'] == 'brkstep2'):
        print('brkstep0 event')
        client.write_register(0,2, unit = 1)
    if (tmp['event'] == 'brkstep3'):
        print('brkstep0 event')
        client.write_register(0,3, unit = 1)
    if (tmp['event'] == 'brkstep4'):
        print('brkstep0 event')
        client.write_register(0,4, unit = 1)


    if (tmp['event'] == 'brkloes'):
        #if brklvl == 4:
        print('brkloes event')
        client.write_coil(6,True, unit=1)
        time.sleep(2)
        client.write_coil(6,False, unit=1)

    if(tmp['event'] == 'startup'):
        print('startup event')
        client.write_coil(6,False, unit=1)
        client.write_coil(7,False, unit=1)
        client.write_coil(10, False, unit=1)
        client.write_coil(11, False, unit=1)

    if (tmp['event'] == 'hupe'):
        #print( 'hupe event')


        if (tmp['state'] == 1):
            client.write_coil(7,True, unit=1)
            print("h1")
        else:
            client.write_coil(7,False, unit=1)
            print("h0")

    if(tmp['event'] == 'engmode'):
        print('engmode')
        rr = client.read_coils(13, 2, unit=1) # 0=STORE 1 = DRIVE
        if (not rr.bits[0] and not rr.bits[1]):
            client.write_coil(13, True, unit=1)
        elif(rr.bits[0] and not rr.bits[1]):
            client.write_coil(14, True, unit=1)
        else:
            client.write_coil(13, False, unit=1)
            client.write_coil(14, False, unit=1)






#last_state_hupe = False
#def callbackfb(data):
#    global vellevl
#    rospy.loginfo(rospy.get_caller_id() + "FB %s", data.data)
#    tmp = json.loads(data.data)
#    print(tmp['velocity'])
#    client.write_register(0,int(tmp['breaklevel']), unit = 1) #WRITE BREAKLEVEL TO REISTER 0^

#    if(tmp['hupe']):
#        last_state_hupe  = tmp['hupe']
#        client.write_coil(10,True, unit=1)
#    else:
#        client.write_coil(10, False, unit=1)#
#    a = 100-int(tmp['velocity'])
#    if a < 27 and a > 24:
#        a = 25
#    rospy.loginfo(rospy.get_caller_id() + "REMOTE %s",a)
#    client.write_register(28, (a * 2) + 900, unit=1)
#    vellevl = a*2








if __name__ == '__main__':
    try:

        client = ModbusTcpClient("192.168.1.17", port=int(5020))

        client.write_coil(6,False, unit=1)
        client.write_coil(7,False, unit=1)

        while not do_exit:
            try:
                #print(".")
                response = client.read_input_registers(0,30,unit=1)

                mqtt_client.publish("tablet_node",json.dumps({
                    "kompressordruck": response.registers[4] / 100.0,
                    "kmh": response.registers[3] / 100.0,
                    "kn":response.registers[15], # anzeige ist in N
                    "breaklevel":100-response.registers[5], # 0-4
                    "direction":response.registers[8],# 0-1
                    "state_v0":response.registers[11],
                    "state_v1":response.registers[12],
                    "state_v2":response.registers[13],
                    "state_v3":response.registers[14],
                    "ctlmode":response.registers[10], # 1= current 0=rpm
                    "fire_detcted": response.registers[20],#true fals
                    "temperature": ((response.registers[18]/100.0)-1.0)/10.0,#grad c
                    "batt_charge": response.registers[22] / 100.0,#A
                    "asc_state":response.registers[24], #0 off 1 active 2 triggered
                    "asc_rest_dist": response.registers[19] / 100.0,
                    "emergencybrake":response.registers[23], # 0 nicht 1 ausgeloesst
                    "emergencybrakereset":0,# 1 warte auf release
                    "kompressorstate":response.registers[17], # 0 = off  1= on 2= auto
                    "lightstate": response.registers[21],
                    "kompressor_power_state":response.registers[16],#0=off 1=on
                    "hupe_state":1,
                    "storedenergy":response.registers[25] / 100.0,
                    "storemode": response.registers[27], #0 = 0ff 1= store in 2 = store out
                    "vellevl":vellevl

                }))
                brklvl = response.registers[5] - 100
                #input register 0 schreibe 0-4
                #pub_fb.publish(json.dumps({
                #    "breaklevel":response.registers[5]-100
                #}))
            except:
                #rospy.loginfo("modbus_error")
                mqtt_client.publish("tablet_status","err")
            #rate.sleep()

    except Exception as e:
        print(e)
        #mqtt_client.close()
        pass

