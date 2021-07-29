(: Test: op-numeric-mod-1 :)
(: Purpose: Check dynamic type of numeric mod on arguments of union of numeric types and untypedAtomic. :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-16 :)

for $x in (1, xs:decimal(2), xs:float(3), xs:double(4), xs:untypedAtomic(5))
for $y in (1, xs:decimal(2), xs:float(3), xs:double(4), xs:untypedAtomic(5))
return typeswitch ($x mod $y)
       case xs:integer return "integer"
       case xs:decimal return "decimal"
       case xs:float return "float"
       case xs:double return "double"
       default return error()

