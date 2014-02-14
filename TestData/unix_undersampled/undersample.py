#!/usr/bin/env python3

import sys
import os
import shutil
import glob

def downsampleEmg(srcDirectory, dstDirectory):
    lmtFilename = "lmt.txt"
    emgFilename = "emg.txt"
    downSampledEmg = [];
    timecolumn = [];
    header = ''
    with open(srcDirectory+'/'+lmtFilename) as f:
        content = f.readlines()
        for line in content[3:]:
            timecolumn.append(line.split()[0])

    sampleTime = float(timecolumn[1])-float(timecolumn[0])
  
    lastLmtTime = float(timecolumn[-1])
    
    with open(srcDirectory+'/'+emgFilename) as f:
        content = f.readlines()
        header = content[2]
        nColumns = len(content[3].split())
        a = [[float(line.split()[i]) for i in range(0,len(line.split()))] for line in content[3:]]
        data = [[row[i] for row in a] for i in range(len(a[0]))] 
        
#        idx = data[0].index(lastLmtTime)
        idx = -1
        runCondition = True        
        while (runCondition):
            idx = idx + 1
            runCondition = (abs(data[0][idx]/sampleTime-round(data[0][idx]/sampleTime)) > 0.001)
        firstValidEmgIdx = idx
        runCondition = True
        while (runCondition):
            idx = idx + 1
            runCondition = (abs(data[0][idx]/sampleTime-round(data[0][idx]/sampleTime)) > 0.001)
        secondValidEmgIdx = idx
  
        samplingInterval = secondValidEmgIdx - firstValidEmgIdx 

        with open(dstDirectory+'/emg.txt', 'w') as outF:
            nCols = len(data)
            rowRange = range(firstValidEmgIdx, data[0].index(lastLmtTime), samplingInterval)
            nRows = len(rowRange)   
            outF.write('datacolumns '+str(nCols)+'\n')
            outF.write('datarows '+str(nRows)+'\n')
            outF.write(header)        
            for iRow in rowRange:
                for iCol in range(nCols-1):
                    outF.write(str(data[iCol][iRow])+' ') 
                outF.write(str(data[-1][iRow])+'\n')             
                
def get_immediate_subdirectories(directory):
    return [name for name in os.listdir(directory)
            if os.path.isdir(os.path.join(directory, name))]


srcDirectory = sys.argv[1]
dstDirectory = sys.argv[2]

for aDir in get_immediate_subdirectories(srcDirectory):
    if not os.path.exists(dstDirectory+'/'+aDir):
        os.makedirs(dstDirectory+'/'+aDir)
    for aFile in glob.glob(srcDirectory+'/'+aDir+'/*.txt'):
        shutil.copy(aFile, dstDirectory+'/'+aDir)  
    downsampleEmg(srcDirectory+'/'+aDir, dstDirectory+'/'+aDir)
#downsampleEmg(srcDirectory, dstDirectory)







