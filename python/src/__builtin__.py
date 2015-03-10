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

def __call(caller, arg, *args, **kwargs):
    if caller.__class__ is type:
        return caller.__init__(
            object.__new__(caller, arg, *args, **kwargs),
            arg, *args, **kwargs)
    else:
        return caller(arg, *args, **kwargs)
