class AnotherException(Exception):

    def __init__(self):
        pass


class YetAnotherException(Exception):

    def __init__(self):
        pass

## -----------------------------------------------------------------------------

try:
    raise Exception()
except Exception:
    print 'Everything is going to be okay'

## -----------------------------------------------------------------------------

try:
    raise YetAnotherException()
except AnotherException:
    print 'This is not it either'
except YetAnotherException:
    print 'Found it'

## -----------------------------------------------------------------------------

def hello_world():
    raise Exception()

try:
    hello_world()
except Exception:
    print 'Catching exception from another function'

## -----------------------------------------------------------------------------

def greetings():
    hello_world()

try:
    greetings()
except Exception:
    print 'Catching exception from nested function'

## -----------------------------------------------------------------------------

# TODO: Run this test when inheritance is supported.
#try:
#    raise YetAnotherException()
#except AnotherException():
#    print 'This should have not be printed'

## -----------------------------------------------------------------------------
