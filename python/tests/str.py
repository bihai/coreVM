## -----------------------------------------------------------------------------

def test_str_representation():
    print str("Hello world")
    print str(str("Hi again!!!"))
    print 'Today is Earth Day!'

## -----------------------------------------------------------------------------

def test_str_hash():
    print 'Hello world'.__hash__()
    print 'Me'.__hash__() != 'You'.__hash__()
## -----------------------------------------------------------------------------

test_str_representation()
test_str_hash()

## -----------------------------------------------------------------------------
