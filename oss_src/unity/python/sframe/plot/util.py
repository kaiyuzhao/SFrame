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
        return float('nan')
    return map(lambda x: positive_to_one(x), arr )


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
                        arr[curr_p][curr_q] = 2
    return map(positive_to_one, arr)
