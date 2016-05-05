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
from util import alpha_blending

def scatterplot(sframe, x, y, resolution = None, color = (0.0, 0.78, 0.79, .5), dato_style=True, **kwargs):

    if x not in sframe.column_names():
        raise ValueError, 'column x {} is not in the sframe'.format(x)
    if sframe[x].dtype() is not float and sframe[x].dtype() is not int:
        raise ValueError, 'column x should be numeric, {} is detected'.format(sframe[x].dtype())

    if y not in sframe.column_names():
        raise ValueError, 'column y {} is not in the sframe'.format(y)
    if sframe[y].dtype() is not float and sframe[y].dtype() is not int:
        raise ValueError, 'column x should be numeric, {} is detected'.format(sframe[y].dtype())

    if resolution <= 0:
        raise ValueError, 'Resolution must be a positive number, none or infinity, instead you used {}.'.format(resolution)

    fig = plt.figure()

    # styling
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
    else:
        # if the dato_style is set fo False:
        # 1. we will use the user specified matplotlib style
        # 2. if no style is specified we use the style encoded in the matplotlibrc file
        if 'style' in kwargs:
            # example style: ggplot, bmh, fivethirtyeight, dark_background
            plt.style.use(kwargs['style'])
        # by default matplotlibrc file is used if no style is specified


    if resolution is None or _math.isinf(resolution):
        # send the data to matloptlib here
        plt.scatter(list(sframe[x]), list(sframe[y]), color=color, **kwargs)
    else:
        from tqdm import tqdm

        pixelization = _sframe.extensions.plot.streaming.heatmap()
        pixelization.init(sframe[x], sframe[y], resolution, 1000000)
        data = None
        if sframe.shape[0] <= 1e6:
            data = pixelization.get()
        else:
            for i in tqdm(range(int(_math.ceil(sframe.shape[0] / 1e6)))):
                data = pixelization.get()

        arr = data.bins
        extrema = data.extrema
        xval, yval, colors = alpha_blending(arr, extrema.x.min, extrema.x.max, extrema.y.min, extrema.y.max, color)
        plt.scatter(xval, yval, color=colors, **kwargs)
    return fig
