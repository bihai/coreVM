class type:
    pass


class object:

    def __new__(cls, arg, *args, **kwargs):
        """
        ### BEGIN VECTOR ###
        [new, 0, 0]
        [ldobj, cls, 0]
        [setattr, __class__, 0]
        ### END VECTOR ###
        """


def __call(caller, *args, **kwargs):
    # TODO: to be completed.
    # Note: here we can check on the type of the caller and act
    # accordingly. However, this is blocked until we support boolean
    # conditioning in Python.
    caller()
