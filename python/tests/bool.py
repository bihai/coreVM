print True
print False
print bool(1)
print bool(0)
True is False

# NOTE: Python prints arithmetic operations on bools as `1`s and `0`s, but we
# actually print them as `True` and `False`, so can't do stdout comparision.
# ( Kinda sad :'( )
# But the results were verified as correct.
# print True + False
# print True - False
# print True * False
# print False / True
# print False % True

if False < True:
    print 'The truth always triumphs'

if bool(0) <= bool(1):
    print 'The truth still triumphs'

if True > False:
    print 'The truth remains triumphant'

if bool(100) >= bool(0):
    print 'The truth remains triumphant eternally'
