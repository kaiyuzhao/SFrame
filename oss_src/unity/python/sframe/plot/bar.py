from numpy import ndarray
from .. import aggregate as _aggregate
import matplotlib.pyplot as plt

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
