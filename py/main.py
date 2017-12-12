# -*- coding:utf-8 -*-

import array
import os
from matplotlib import pyplot

fileName = './news.pcm' # 2 channel, 16 bit per sample
file = open(fileName, 'rb')
base = 1 / (1<<15)

shortArray = array.array('h') # int16
size = int(os.path.getsize(fileName) / shortArray.itemsize)
count = size
shortArray.fromfile(file, size) # faster than struct.unpack
file.close()
leftChannel = shortArray

def showPCM(leftChannel, start = 0, end = 5000):
    fig = pyplot.figure(1)

    pyplot.subplot(211)
    pyplot.title('pcm left channel [{0}-{1}] max[{2}]'.format(start, end, count))
    pyplot.plot(range(start, end), leftChannel[start:end])

    pyplot.show()
    # fig.savefig('pcm.pdf') # save figure to pdf file

showPCM(leftChannel, 0, count)
