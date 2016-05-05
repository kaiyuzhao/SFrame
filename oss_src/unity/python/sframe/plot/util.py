'''
Copyright (C) 2015 Dato, Inc.
All rights reserved.

This software may be modified and distributed under the terms
of the BSD license. See the LICENSE file for details.
'''

def linspace(start, stop, n):
    if n == 1:
        return [ stop ]
    h = (stop - start) / (n - 1)
    ret = []
    for i in range(n):
        ret.append(start + h * i)
    return ret

def around(arr, decimals= 2):
    from decimal import Decimal
    def rounding(x, decimals):
        formats = "%."+str(decimals)+"f"
        return float(Decimal(formats % x))
    return map(lambda x: rounding(x, decimals), arr)

def positive_to_one(arr):
    def convert(x):
        if x > 0:
            return 1
        return 0
    return map(lambda x: convert(x), arr )


def padding(arr, padsize):
    '''
    padding a sparse array of shape (n, n) with value of only 1 or zeros
    all adjacent positions within radius of padsize of each non-zero
    posision (p, q) are filled with 1
    '''
    for p, row in enumerate(arr):
        for q, column in enumerate(row):
            if arr[p][q] == 1:
                for i in range(padsize * 2 + 1):
                    for j in range(padsize * 2 + 1):
                        curr_p = p-padsize+i
                        curr_q = q-padsize+j
                        dist = (curr_p - p) * (curr_p - p) + (curr_q - q) * (curr_q - q)
                        if dist < (padsize - 1) * (padsize-1):
                            arr[curr_p][curr_q] = 2
    return map(positive_to_one, arr)

def alpha_blending(arr, xmin, xmax, ymin, ymax, color):
    '''
    padding a sparse array of shape (n, n) with value of only counts
    '''
    x, y, colors = ([] for i in range(3))
    r, g, b, a = color
    bin_num = len(arr)

    xpos = linspace(xmin, xmax, bin_num + 1)
    ypos = linspace(ymin, ymax, bin_num + 1)

    for p, row in enumerate(arr):
        for q, column in enumerate(row):
            cnt = arr[p][q]
            if cnt > 0:
                xval = 0.5 * (xpos[p] + xpos[p+1])
                yval = 0.5 * (ypos[q] + ypos[q+1])
                x.append(xval)
                y.append(yval)
                colors.append((r,g,b,1-pow((1-a),cnt)))
    return (x, y, colors)
