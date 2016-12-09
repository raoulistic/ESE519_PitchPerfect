#!/usr/bin/env python

import socket
import fcntl
import struct
import datetime as dt

def get_ip_address(ifname):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    return socket.inet_ntoa(fcntl.ioctl(
        s.fileno(),
        0x8915,  # SIOCGIFADDR
        struct.pack('256s', ifname[:15])
    )[20:24])

address = get_ip_address('wlan0')
print 'IP address',address
TCP_IP = '127.0.0.1'
TCP_PORT = 5005
BUFFER_SIZE = 50  # Normally 1024, but we want fast response
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((address, TCP_PORT))

while True:
    try:
        print "Listening for incoming connections..."
        s.listen(1)
        print "Incoming connection detected."
        conn, addr = s.accept()
        print 'Accepted connection to address: ', addr
        conn.settimeout(1)
        time_out=0;
        print "Receving data..."
        count=0
        f = open('data/RecvdData.txt', 'w')
        f2 =open('data/RecvdDataTIME.txt','w')
        timeSaved = False

        while 1:
            #print "." #looped


            try:
                data = conn.recv(BUFFER_SIZE)
                '''try:
                    file = open('DetectionCode\pitch_read.txt','r')
                    pitch = file.read()
                    conn.send(str(pitch))
                    file.close()
                except IOError:
                    conn.send(str(0))'''

                if count%25 is 0:
                    conn.send(str(count))
                count+=1
            except socket.timeout: #Timeout occurred, do things
                print "Connection timed out: ",time_out+1
                time_out+=1
                if time_out>3: #3*3 = 9 seconds to close connection
                    print "Terminating connection..."
                    break
                continue


            #if not data: break

            if not data:
                #nodata_time = dt.now()
                #print "No data at: ",nodata_time
                #continue
                break
            if(not timeSaved):
                f2.write(str(dt.datetime.now()))
                timeSaved=True
            time_out=0
            print "received data:", data
            list_to_write = data.split(',')
            list_to_write = list_to_write
            for i in range(len(list_to_write)):
                if(i == (len(list_to_write)-1)):
                    f.write(list_to_write[i] + "\n")
                else:
                    f.write(list_to_write[i] + " ")
            #f.write(str(data))
            #conn.send(data)  # echo
        print "closing connection..."
        conn.close()
        f.close()
        f2.close()
        print "Connection closed."
    except KeyboardInterrupt:

        print "Breaking out & closing connection..."
        conn.close()
        f.close()
        f2.close()
        print "Connection closed."
        break