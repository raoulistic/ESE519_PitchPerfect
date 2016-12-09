import numpy as np
import matplotlib.pyplot as plt
import datetime as dt
from os import rename

while True:

    fileNotFound=True

    while fileNotFound:
        try:
            plotdata = np.loadtxt("data/RecvdData.txt")
            print "Data found..."
            fileNotFound=False
        except Exception,e:
            #print "Not found"
            fileNotFound=True

    dateRead=open("data/RecvdDataTIME.txt",'r')
    dateUploaded = dt.datetime.strptime(dateRead.read(),'%Y-%m-%d %H:%M:%S.%f')
    print "Upload time was: \t",dateUploaded
    print "Time now is: \t\t",dt.datetime.now()

    lastMillis = plotdata[-1,0]
    print "last millis:",lastMillis
    print "size of plotData: ",plotdata.shape

    timestamp  = []
    print len(timestamp)

    #print (dt.datetime.now() - dt.timedelta(=15))

    for i in range(len(plotdata)):
        #print type(float(lastMillis))
        #print type(float(plotdata[0,i]))
        seconds= (((lastMillis)-float(plotdata[i,0]))/1000.0)
        #seconds2=dt.datetime.strptime(seconds,'%S.%f')
        stamp = dateUploaded- dt.timedelta(seconds=seconds)
        print stamp
        #print seconds2
        #print type(seconds2)
        #if i is 0:
        #    timestamp=[stamp]
        #else:
        timestamp.append(stamp)#dateUploaded-seconds2 #dt.timedelta(seconds=seconds)#dt.datetime.fromtimestamp(seconds)




    print timestamp
    fig=plt.figure("Pitching Timeline")
    print plotdata.shape
    plt.plot(timestamp,plotdata[:,-1])
    #plt.xticks(rotation=90)
    fig.autofmt_xdate()
    plt.title("Pitching Timeline")
    plt.ylabel("Pitch Count")
    #plt.tight_layout()
    #plt.show()


    plt.figure("RAW PITCHING DATA")


    labels=['Accel X','Accel Y','Accel Z','Roll','Pitch']
    #plt.title("RAW PITCHING DATA")
    for j in range(5):


        plt.subplot(5,1,j+1)
        plt.plot(plotdata[:,j+1])
        plt.ylabel(labels[j])
        plt.xlabel('Samples')

    plt.show(block=False)
    dateRead.close()
    rename("data/RecvdData.txt", "data/RecvdData_done.txt")
    rename("data/RecvdDataTIME.txt", "data/RecvdDataTIME_done.txt")
