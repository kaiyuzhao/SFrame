"""
GraphLab Create offers several data structures for data analysis.

Concise descriptions of the data structures and their methods are contained in
the API documentation, along with a small number of simple examples. For more
detailed descriptions and examples, please see the `User Guide
<https://dato.com/learn/userguide/>`_, `API Translator
<https://dato.com/learn/translator/>`_, `How-Tos
<https://dato.com/learn/how-to/>`_, and data science `Gallery
<https://dato.com/learn/gallery/>`_.
"""

'''
Copyright (C) 2015 Dato, Inc.
All rights reserved.

This software may be modified and distributed under the terms
of the BSD license. See the LICENSE file for details.
'''

__all__ = ['sframe', 'sarray', 'sgraph', 'sketch', 'image']

from . import image
from . import sframe
from . import sarray
from . import sgraph
from . import sketch
