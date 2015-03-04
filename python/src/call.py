def do_math():
  1 + 1

def hello_world(arg1, arg2, kwarg1=1+1, *args, **kwargs):
  do_math()

hello_world(1, 2, kwarg1=3, *args, **kwargs)
