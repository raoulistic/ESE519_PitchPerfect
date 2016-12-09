import numpy as np
import matplotlib.pyplot as plt
from sklearn.metrics import classification_report
import cPickle
from sklearn import svm
from sklearn.neighbors.nearest_centroid import NearestCentroid
#from pandas_ml import ConfusionMatrix


def getWindowedFFT(X,Y,windowSize,sampleRate,yThreshold=0.8,plotFFT=False,verbose=False,mute=False):
    '''
    takes X (signals) and Y (labels) and returns FFT_transform(X),Windowed(Y),freqs
    Computes FFT coefficients of all signals in various columns of X
    :param X: nxd array with n samples of d signals. n should be the samples in each window.
    :return: FFT coefficients of each of the d signals appended into rows of length
     d*(no' of FFT coefs for each signal in X)
    '''
    n_samples,n_signals=np.matrix(X).shape
    sampleSpacing = 1.00 / sampleRate
    wStart=0
    wEnd=wStart+windowSize
    if verbose:
        print "N_Samples,N_Signals: ",n_samples,n_signals

    X_tranformed=[] #FFT windowed X
    Y_transformed=[] #windowed y

    while wEnd < n_samples:
        # do nothing
        window={}
        windowFFT = {}
        absoluteFFT = {}
        absoluteFFTWhole=[]
        for signal in range(n_signals):
            window[signal] = X[wStart:wEnd, signal]
            windowFFT[signal] = np.fft.fft(window[signal])
            absoluteFFT[signal] = [abs(complex) for complex in windowFFT[signal]]
            if len(absoluteFFTWhole) is 0:  # append all together
                absoluteFFTWhole=absoluteFFT[signal]
            else:
                absoluteFFTWhole=np.append(absoluteFFTWhole,[signal])

        avgLabel = np.mean(Y[wStart:wEnd])
        if (avgLabel > yThreshold):
            label = 1
        else:
            label = 0
        if verbose:
            print "Dimensions of absoluteFFTWhole: ",absoluteFFTWhole.shape

        if len(X_tranformed) is 0:
            X_tranformed = np.array(absoluteFFTWhole)
            Y_transformed = np.array(label)
        else:
            X_tranformed = np.vstack((X_tranformed, np.array(absoluteFFTWhole)))
            Y_transformed = np.append(Y_transformed, label)
        if verbose:
            print "Xtransformed: type = ", type(X_tranformed), "\tShape = ", X_tranformed.shape
            print "Ytransformed: type = ", type(Y_transformed), "\tShape = ", Y_transformed.shape

        # print "Shape of FFT",type(absolute)
        FFT_freqs = np.fft.fftfreq(windowSize, d=sampleSpacing)
        if verbose:
            print "FFT freq: ",FFT_freqs.shape

        #if verbose:
        if not mute:
            print "Percentage complete: %0.2f" % (
            100.0 * (1.0 + (wEnd - windowSize)) / (n_samples - windowSize)), "\tLabel: %0.2f" % avgLabel

        if plotFFT:
            # plot transform of every 20 windows:
            if wStart % 20 is 0:
                # y = np.random.random()
                plt.clf()
                for signals in range(n_signals):
                    plt.subplot(n_signals, 1, (signals+1))
                    plt.ylim([0, 4500])
                    plt.xlim([-20, 20])
                    plt.xlabel("Label: %0.2f" % avgLabel)
                    #print "absoluteFFT shape:",len(absoluteFFT[signals])#shape
                    plt.plot(FFT_freqs, absoluteFFT[signals])
                plt.pause(0.05)
        # Update for next window:
        wStart += 1
        wEnd = wStart + windowSize

    return X_tranformed,Y_transformed#,FFT_freqs


def countPitches(y):
    n=len(y)
    count=0
    y_prev=0
    avgWindow=50
    threshold=0.85
    for i in range(n-avgWindow):
        ynow=np.mean(y[i:(i+avgWindow)])
        if ynow>threshold:
            ynow=1
        else:
            ynow=0
        if((ynow == 1) and (y_prev == 0)):
            count+=1
        y_prev=ynow
    return count


#...................................................Program starts here................................................
toLoadState=False
trainAgain=True


#fname = "data\ThreeRoughPitches.txt"
#allData = np.loadtxt(fname)
modelSaved=False

f2name = "data/FiveOrTenPitches.txt"#"data/PitchesLabel.txt"#
allData2= np.loadtxt(f2name)

allData3=0#np.loadtxt("data/RecvdData.txt")

datapts,_=allData2.shape
ntrain= 9300#datapts#first 9300 data points (wrt FiveOrTenPitches file #run till10726#
nlast=datapts#19797

xTrain=allData2[:ntrain,:-1] #Signal data
xTest=allData2[ntrain:nlast,:-1]#allData[:,:-1]#allData3#
yTrain=allData2[:ntrain,-1] #Labels
yTest=np.zeros([len(xTest),1])#allData2[ntrain:nlast,-1]#allData[:,-1]#


ntrain,d=xTrain.shape
ntest,d2=xTest.shape
print "len xTrain Data: ", ntrain,d
print "len xTest Data: ", ntest,d2

wSize=600#230#No of datapoints in each window
labelThreshold =0.8 # Threshold (above which) to consider label as 1

if trainAgain:
    # Moving Window Analysis: (Training data)

    if toLoadState:
        f2name = "data\ThreeRoughPitches_Train.txt"
        LoadState = np.loadtxt(f2name)
        Xtrainw=LoadState[:,:-1]
        Ytrainw=LoadState[:,-1]
    else:
        samplesPerSecond = 273
        Xtrainw,Ytrainw,_=getWindowedFFT(xTrain,yTrain,wSize,samplesPerSecond,yThreshold=0.8,plotFFT=True,verbose=True,mute=False)
        fname_storeState = "Data\ThreeRoughPitches_Train.txt"
        print "Saving training data transforms..."
        np.savetxt(fname_storeState, np.hstack((Xtrainw, np.transpose(np.matrix(Ytrainw)))), fmt='%f', delimiter=' ',
                   newline='\n', header='', footer='', comments='# ')

    try:
        print "Xtrainshape",Xtrainw.shape
        print "Ytrainshape",Ytrainw.shape
    except:
        print " Error", len(Xtrainw)
        print len(Ytrainw)



    print "Training KNN centroid..."
    #clf = svm.SVC(C=0.1, kernel='rbf', degree=3, gamma='auto')
    #clf.fit(Xtrainw, Ytrainw)
    clf = svm.SVC(C=1,kernel='poly',degree=5)#NearestCentroid()
    clf.fit(Xtrainw, Ytrainw)

    print "Attempting to save Model state..."
    try:
        # save the classifier
        with open('my_dumped_classifier.pkl', 'wb') as fid:
            cPickle.dump(clf, fid)
        modelSaved=True
        print "Model Saved successfully."
    except:
        print "Memory Error, could not save model!"
        modelSaved=False
    #store_clf = pickle.dumps(clf)
    #joblib.dump(clf, 'SVM_classifier.pkl')


    print "Training done. Generating predictions..."
    print "1) Training performance:"
    y_pred_training = clf.predict(Xtrainw)
    accuracy_training = np.mean(y_pred_training == Ytrainw)
    print "Training accuracy: ", accuracy_training
    #cnf_matrix_Train = ConfusionMatrix(Ytrainw, y_pred_training)
    #print"Train performance stats:\n",cnf_matrix_Train.print_stats()

    print "\n\nTRAIN PITCHCOUNT = ",countPitches(y_pred_training)
    print "\n\n"

#TEST DATA PROCESSING:

if toLoadState:
    print "Loading test state..."
    f2name = "data\ThreeRoughPitches_Test.txt"
    LoadState = np.loadtxt(f2name)
    Xtestw = LoadState[:, :-1]
    Ytestw = LoadState[:, -1]
else:
    print "Processing Test state..."
    samplesPerSecond = 273
    Xtestw, Ytestw = getWindowedFFT(xTest, yTest, wSize, samplesPerSecond, yThreshold=0.8, plotFFT=True,
                                         verbose=True,mute=False)
    fname_storeState = "Data\ThreeRoughPitches_Test.txt"
    '''print "Saving training data transforms..."
    np.savetxt(fname_storeState, np.hstack((Xtestw, np.transpose(np.matrix(Ytestw)))), fmt='%f', delimiter=' ',
               newline='\n', header='', footer='', comments='# ')'''

try:
    print "Xtestshape",Xtestw.shape
    print "Ytestshape",Ytestw.shape
except:
    print " Error\n", len(Xtestw)
    print len(Ytestw)

#print "Training accuracy was: ", accuracy_training
#clf = joblib.load('SVM_classifier.pkl')
#clf2 = pickle.loads

if modelSaved or not(trainAgain):
    with open('my_dumped_classifier.pkl', 'rb') as fid:
        clf2 = cPickle.load(fid)
else:
    clf2=clf
print "\n\n2) Testing performance:"
y_pred_test = clf2.predict(Xtestw)
#Save File
file = open("label.txt", 'w')
for i in range(len(y_pred_test)):
    if (i == (len(y_pred_test) - 1)):
        file.write(str(int(y_pred_test[i])))
    else:
        file.write(str(int(y_pred_test[i])) + ",")
file.close()

accuracy_test1=np.mean(y_pred_test==Ytestw)
print "Test1 accuracy: ",accuracy_test1

fname_s="Data\ThreeRoughPitches_preds.txt"
np.savetxt(fname_s, y_pred_test, fmt='%d', delimiter=' ', newline='\n', header='', footer='', comments='# ')
target_names = ['Not_Pitch', 'Pitch']
print(classification_report(Ytestw,y_pred_test, target_names=target_names))

#cnf_matrix_Test = ConfusionMatrix(Ytestw, y_pred_test)
#print"Test performance stats:\n",cnf_matrix_Test.print_stats()

pitch_count = countPitches(y_pred_test)
print "\n\nTEST PITCHCOUNT = ",pitch_count#countPitches(y_pred_test)
file = open('pitch_read.txt','w')
file.write(str(int(pitch_count)))
file.close()

print "\n\nEnded"

'''
Training SVM...
Training done. Generating predictions...
1) Training performance:
Training accuracy:  0.999351407446
2) Testing performance part 1:
Test1 accuracy:  0.961733039305
3) Testing performance part 2:
Test1 accuracy:  0.961733039305
ended
'''


