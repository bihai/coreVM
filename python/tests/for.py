for i in [1, 2, 3, 4, 5]:
    print 'Hello world'
    print i

## -----------------------------------------------------------------------------

for i in [1, 2, 3, 4, 5]:
    print i
    break

## -----------------------------------------------------------------------------

for i in [1, 2, 3, 4, 5]:
    break
    print 'This can never be said...'

## -----------------------------------------------------------------------------

for i in [5, 4, 3, 2, 1]:
    print i
    continue

## -----------------------------------------------------------------------------

for i in [1, 2, 3, 4, 5]:
    continue
    print 'This can never be said...'

## -----------------------------------------------------------------------------

for i in [1, 2, 3, 4, 5]:
    for j in [1, 2, 3, 4, 5]:
        print i * j

## -----------------------------------------------------------------------------

for i in [5, 4, 3, 2, 1]:
    for j in [1, 2, 3, 4, 5]:
        if i == j:
            print i

## -----------------------------------------------------------------------------
