'''
Copyright (C) 2015 Dato, Inc.
All rights reserved.

This software may be modified and distributed under the terms
of the BSD license. See the LICENSE file for details.
'''
import numpy as _numpy
import sframe as _sframe

import matplotlib.pyplot as plt
import matplotlib.ticker as mtick
import math as _math

def scatterplot(sframe, x, y, resolution = None, **kwargs):
    numTicks = 6
    if x not in sframe.column_names():
        raise ValueError, 'column x {} is not in the sframe'.format(x)
    if y not in sframe.column_names():
        raise ValueError, 'column y {} is not in the sframe'.format(y)

    if resolution <= 0:
        raise ValueError, 'Resolution must be a positive number, none or infinity, instead you used {}.'.format(resolution)
    fig = plt.figure()
    ax = fig.add_subplot(1, 1, 1)
    plt.style.use('ggplot')
    if 'style' in kwargs:
        plt.style.use(kwargs['style'])

    if resolution is None or _math.isinf(resolution):
        # send the data to matloptlib here
        ax.scatter(list(sframe[x]), list(sframe[y]), **kwargs)
    else:
        # apply streaming aggregator here
        from tqdm import tqdm
        pixelization = _sframe.extensions.plot.streaming.heatmap()
        pixelization.init(sframe[x], sframe[y], 1000, 1000000)
        data = []
        for i in tqdm(range(int(sframe.shape[0] / 1e6))):
            data = pixelization.get()

        arr = _numpy.array(data.bins)
        paddedRD = _numpy.pad(arr, ((1,0),(1,0)),mode='constant')
        paddedR = _numpy.pad(arr, ((0,0),(1,0)),mode='constant')
        paddedD = _numpy.pad(arr, ((1,0),(0,0)),mode='constant')
        arr = arr + paddedRD[:-1,:-1] + paddedR[:,:-1] + paddedD[:-1,:]
        arr[arr>0] = 1
        arr = arr.transpose()
        ax.imshow(arr, cmap=plt.cm.binary)
        ax.grid(True, color='gray', linestyle='-.')

        bin_range = data.bin_extrema
        xlabels = _numpy.linspace(bin_range.x.min, bin_range.x.max, numTicks)
        ylabels = _numpy.linspace(bin_range.y.max, bin_range.y.min, numTicks)

        xlabels = _numpy.around(xlabels, decimals=2)
        ylabels = _numpy.around(ylabels, decimals=2)

        ax.set_xticks(_numpy.linspace(0,resolution,numTicks), minor=False)
        ax.set_yticks(_numpy.linspace(0,resolution,numTicks), minor=False)

        ax.set_xticklabels(xlabels, rotation=15)
        ax.set_yticklabels(ylabels) #flip the y axi

        # ax.xaxis.set_major_formatter(mtick.FormatStrFormatter('%.2e'))
        # ax.yaxis.set_major_formatter(mtick.FormatStrFormatter('%.2e'))

        ax.set_xlabel(x)
        ax.set_ylabel(y)

    return fig
