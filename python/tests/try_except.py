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

def catch_exception_from_same_level_of_except_block():
    try:
        raise AnotherException()
    except AnotherException:
        raise YetAnotherException()
    except YetAnotherException:
        print 'Catching exception from same level of except blocks is not supported in Python'

try:
    catch_exception_from_same_level_of_except_block()
except YetAnotherException:
    print 'Catching exception from exception raised in except block'

## -----------------------------------------------------------------------------

# TODO: Run this test when inheritance is supported.
#try:
#    raise YetAnotherException()
#except AnotherException():
#    print 'This should have not be printed'

## -----------------------------------------------------------------------------
