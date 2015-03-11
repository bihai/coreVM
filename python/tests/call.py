def hello_world_again(*args):
    print 'Hello world again!'

def introduction():
    print 'My name is Will :-)'
    # test explicit return should work.
    return

def hello_world(*args):
    print 'Hello world!'
    introduction()

def main(*args, **kwargs):
    print 'Hi'
    hello_world()
    hello_world_again()
    hello_world()
    print 'Bye'

main()
print 'Done!'
