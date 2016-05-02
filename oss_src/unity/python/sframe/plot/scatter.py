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

def scatterplot(sframe, x, y, resolution = None, numTicks = 6, **kwargs):

    if x not in sframe.column_names():
        raise ValueError, 'column x {} is not in the sframe'.format(x)
    if y not in sframe.column_names():
        raise ValueError, 'column y {} is not in the sframe'.format(y)

    if resolution <= 0:
        raise ValueError, 'Resolution must be a positive number, none or infinity, instead you used {}.'.format(resolution)

    plt.style.use('ggplot')
    if 'style' in kwargs:
        plt.style.use(kwargs['style'])

    if resolution is None or _math.isinf(resolution):
        fig = plt.figure()
        # send the data to matloptlib here
        plt.scatter(list(sframe[x]), list(sframe[y]), **kwargs)
    else:
        from heatmap import heatmap
        s = 2
        if 's' in kwargs:
            s = kwargs['s']
        return heatmap(sframe, x, y,num_bins=resolution, numTicks=numTicks, scatterMode=True, psize = s)
    return fig
