class NoneType(object):

    def __init__(self):
        """
        ### BEGIN VECTOR ###
        [ldobj, self, 0]
        [bool, 0, 0]
        [sethndl, 0, 0]
        ### END VECTOR ###
        """

    def __str__(self):
        return __call(str, 'None')

    def __hash__(self):
        # Python 2.7.6 (default, Mar 22 2014, 22:59:56)
        # [GCC 4.8.2] on linux2
        # The version of CPython above returns integer `598041`.
        # We simply follow suit here.
        return __call(int, 598041)

    def __not__(self):
        return True


## Global `None` object.
#
# NOTE: In Python it's a syntax error to assign a value to the name `None`,
# so we cannot do `None = __call(NoneType)` here.
#
# TODO: Set the appropriate flags on the global object `None`.
__call(NoneType)
"""
### BEGIN VECTOR ###
[setflgc, 1, 0]
[setfldel, 1, 0]
[setflmute, 1, 0]
[stobj, None, 0]
### END VECTOR ###
"""
