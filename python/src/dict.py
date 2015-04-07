class dict(object):

    class __dict_ObjectHashPair(object):

        def __init__(self, obj):
            # TODO: clean up after we can do aug-assignment.
            _obj = obj
            _hash = __call(obj.__hash__)
            """
            ### BEGIN VECTOR ###
            [ldobj, self, 0]
            [ldobj, _obj, 0]
            [setattr, _obj, 0]
            [ldobj, _hash, 0]
            [setattr, _hash, 0]
            ### END VECTOR ###
            """

    def __init__(self, arg):
        """
        ### BEGIN VECTOR ###
        [ldobj, arg, 0]
        [gethndl, 0, 0]
        [2map, 0, 0]
        [pop, 0, 0]
        [ldobj, self, 0]
        [sethndl, 0, 0]
        ### END VECTOR ###
        """

    def __len__(self):
        """
        ### BEGIN VECTOR ###
        [ldobj, self, 0]
        [gethndl, 0, 0]
        [maplen, 0, 0]
        [new, 0, 0]
        [sethndl, 0, 0]
        [stobj, res_, 0]
        ### END VECTOR ###
        """
        return __call(int, res_)

    def __str__(self):
        size = __call(self.__len__)
        top_index = __call(size.__sub__, 1)
        index = __call(int, 0)
        res = __call(str, '')
        __call(res.__add__, __call(str, '{'))

        # TODO...

        __call(res.__add__, __call(str, '}'))
        return res
