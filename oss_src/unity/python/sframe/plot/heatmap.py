'''
Copyright (C) 2015 Dato, Inc.
All rights reserved.

This software may be modified and distributed under the terms
of the BSD license. See the LICENSE file for details.
'''
import sframe as _sframe

import matplotlib as _mpl
import matplotlib.pyplot as plt
import matplotlib.ticker as mtick
import math as _math
from util import linspace
from util import around


def heatmap(sframe, x, y, num_bins = 80, dato_style=True, scale=None, **kwargs):
    '''
    scale can be "log", "sqrt" or "linear"
    When it is not set we use linear by default
    '''

    if x not in sframe.column_names():
        raise ValueError, 'column x {} is not in the sframe'.format(x)
    if sframe[x].dtype() is not float and sframe[x].dtype() is not int:
        raise ValueError, 'column x should be numeric, {} is detected'.format(sframe[x].dtype())

    if y not in sframe.column_names():
        raise ValueError, 'column y {} is not in the sframe'.format(y)
    if sframe[y].dtype() is not float and sframe[y].dtype() is not int:
        raise ValueError, 'column x should be numeric, {} is detected'.format(sframe[y].dtype())

    from matplotlib.colors import LogNorm
    from matplotlib.colors import PowerNorm

    numTicks = 6
    if type(scale) is str:
        if scale == 'log':
            scale = LogNorm()
        elif scale == 'linear':
            scale = PowerNorm(1)
        elif scale == 'square root':
            scale = PowerNorm(.5)
        elif scale == 'quadratic':
            scale = PowerNorm(2)
        else:
            raise ValueError, 'scale must either be a string of "log", "square root", "linear", "quadratic" or a matpotlib color normalization such as "scale = LogNorm()"'

    fig = plt.figure()
    from tqdm import tqdm
    pixelization = _sframe.extensions.plot.streaming.heatmap()
    pixelization.init(sframe[x], sframe[y], num_bins, 1000000)
    data = None
    if sframe.shape[0] <= 1e6:
        data = pixelization.get()
    else:
        for i in tqdm(range(int(_math.ceil(sframe.shape[0] / 1e6)))):
            data = pixelization.get()

    arr = data.bins
    # transpose
    arr = map(list, zip(*arr))

    # flip upside down
    arr.reverse()
    colormap = plt.cm.YlGnBu
    im = plt.imshow(arr, cmap=colormap, norm= scale, interpolation='nearest', origin='upper')

    if dato_style:
        fig.patch.set_facecolor('white')
        #grid line
        plt.grid(b=True, color=(222.0/255, 222.0/255, 222.0/255), linewidth=1, linestyle='--')

        #set grid lines below axes
        plt.gca().set_axisbelow(True)

        #axes color (0, 135, 209)
        plt.gca().spines['bottom'].set_color((0, 135.0/255, 209.0/255))
        plt.gca().spines['left'].set_color((0, 135.0/255, 209.0/255))

        #axes line width
        plt.gca().spines['bottom'].set_linewidth(2)
        plt.gca().spines['left'].set_linewidth(2)

        #set top and right axes invisible
        plt.gca().spines['top'].set_visible(False)
        plt.gca().spines['right'].set_visible(False)

        # move ticks to the bottom and left side
        plt.gca().get_xaxis().tick_bottom()
        plt.gca().get_yaxis().tick_left()

        # set axes tick label color as it was set by spines.set_color
        [i.set_color("black") for i in plt.gca().get_xticklabels()]
        [i.set_color("black") for i in plt.gca().get_yticklabels()]

        #title (0, 135, 209)
        plt.title('{} vs. {}'.format(x, y))

        # set axes labels
        plt.xlabel(x)
        plt.ylabel(y)

        # format color bar
        if not isinstance(scale, _mpl.colors.LogNorm):
            plt.colorbar(im, spacing='proportional',format='%1.1e', ticks=mtick.MaxNLocator(nbins=4))
        else:
            plt.colorbar(im)

        bin_range = data.bin_extrema

        xlabels = linspace(bin_range.x.min, bin_range.x.max, numTicks)
        ylabels = linspace(bin_range.y.max, bin_range.y.min, numTicks)

        xlabels = around(xlabels, decimals=2)
        ylabels = around(ylabels, decimals=2)

        plt.xticks(linspace(0, num_bins, numTicks), xlabels)
        plt.yticks(linspace(0, num_bins, numTicks), ylabels)

    return fig
