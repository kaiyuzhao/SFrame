'''
Copyright (C) 2015 Dato, Inc.
All rights reserved.

This software may be modified and distributed under the terms
of the BSD license. See the LICENSE file for details.
'''
import sframe as _sframe

import matplotlib.pyplot as plt
import matplotlib.ticker as mtick
import math as _math
from util import linspace
from util import around


def heatmap(sframe, x, y, num_bins = 80, numTicks = 6, scatterMode=False, psize =2, **kwargs):

    fig = plt.figure()

    plt.style.use('ggplot')
    if 'style' in kwargs:
        plt.style.use(kwargs['style'])

    from tqdm import tqdm
    pixelization = _sframe.extensions.plot.streaming.heatmap()
    pixelization.init(sframe[x], sframe[y], num_bins, 1000000)
    data = []
    for i in tqdm(range(int(sframe.shape[0] / 1e6))):
        data = pixelization.get()

    arr = data.bins
    arr = map(list, zip(*arr))
    colormap = plt.cm.YlGnBu
    if scatterMode:
        from util import positive_to_one
        from util import padding
        arr = map(positive_to_one, arr)
        arr = padding(arr, psize)
        colormap = plt.cm.binary

    plt.imshow(arr, cmap=colormap, interpolation='nearest')
    plt.grid(True, color='orange', linestyle='-.')

    bin_range = data.bin_extrema
    xlabels = linspace(bin_range.x.min, bin_range.x.max, numTicks)
    ylabels = linspace(bin_range.y.max, bin_range.y.min, numTicks)

    xlabels = around(xlabels, decimals=2)
    ylabels = around(ylabels, decimals=2)

    plt.xticks(linspace(0,num_bins,numTicks), xlabels, rotation=15)
    plt.yticks(linspace(0,num_bins,numTicks), ylabels)

    plt.xlabel(x)
    plt.ylabel(y)
    return fig
