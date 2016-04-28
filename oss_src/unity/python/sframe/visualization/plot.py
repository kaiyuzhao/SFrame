'''
Copyright (C) 2015 Dato, Inc.
All rights reserved.

This software may be modified and distributed under the terms
of the BSD license. See the LICENSE file for details.
'''
import numpy as _numpy
from numpy import ndarray
from .. import aggregate as _aggregate
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
        pixelization = _sframe.extensions._canvas.streaming.heatmap()
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

def barplot(x, y, aggregate, **kwargs):
    import matplotlib
    import matplotlib.pyplot as plt
    import inspect
    assert all(type(var) in [list, SArray, ndarray] for var in [x, y]), "x of type {} and y of type {} must be either list or SArray or numpy ndarray".format(type(x), type(y))
    assert len(x)==len(y), "Length of x is {} and length of y is {}. They do not match!".format(len(x), len(y))
    assert len(x) > 0, "Length of x is {} and it must be greater than 0".format(len(x))

    def aggregateCheck(agg):
        supportedAGG = [_aggregate.SUM, _aggregate.COUNT, _aggregate.AVG, _aggregate.MEAN, _aggregate.MIN, _aggregate.MAX, _aggregate.STD, _aggregate.STDV, _aggregate.COUNT_DISTINCT]
        assert agg in supportedAGG, "we currently only support SUM, COUNT, AVG, MEAN, MIN, MAX, STD, STDV, COUNT_DISTINCT. {} is not supported".format(agg.__name__)

    def stackBars(data, aggregate):
        operator = {}
        for agg in aggregate:
            aggregateCheck(agg)
            if agg is _aggregate.COUNT:
                operator['y_'+agg.__name__] = agg()
            else:
                operator['y_'+agg.__name__] = agg('y')
        aggData = data.groupby('x', operator)
        offset = 0.0
        width = 0.8
        if len(aggregate) >1:
            offset = 1.0 / (len(aggregate) + 1)
            width = offset
        ind = np.arange(len(aggData))
        colors = matplotlib.colors.cnames.keys()
        for i, agg in enumerate(aggregate):
            plt.bar(ind + i*offset - width/2.0, list(aggData['y_'+agg.__name__]-baseline), width=width, color=colors[i], bottom=baseline)

        if 'xformat' in kwargs:
            plt.xticks(ind, list(aggData['x'].apply(lambda x: kwargs['xformat'](x))), rotation=20)
        else:
            plt.xticks(ind, list(aggData['x']),rotation=20)

    data = SFrame({'x':list(x),'y':list(y)})
    plt.style.use('ggplot')
    if 'style' in kwargs:
        plt.style.use(kwargs['style'])

    fig, ax = plt.subplots()

    baseline = 0
    if 'bottom' in kwargs:
        baseline = kwargs['bottom']

    if isinstance(aggregate, list):
        stackBars(data, aggregate)
    else:
        stackBars(data, [aggregate])
    return plt
