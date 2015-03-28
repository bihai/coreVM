class list(object):

    def __init__(self, arg):
        """
        ### BEGIN VECTOR ###
        [ldobj, arg, 0]
        [gethndl, 0, 0]
        [2ary, 0, 0]
        [pop, 0, 0]
        [ldobj, self, 0]
        [sethndl, 0, 0]
        ### END VECTOR ###
        """

    def append(self, arg):
        """
        ### BEGIN VECTOR ###
        [ldobj, self, 0]
        [putobj, 0, 0]
        [ldobj, arg, 0]
        [gethndl, 0, 0]
        [pop, 0, 0]
        [aryapnd, 0, 0]
        [sethndl, 0, 0]
        ### END VECTOR ###
        """

    def pop(self, arg):
        pass
