(: Test: op-numeric-divide-1 :)
(: Purpose: Check dynamic type of numeric divide on arguments of union of numeric types and untypedAtomic. :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-16 :)

for $x in (1, xs:decimal(2), xs:float(3), xs:double(4), xs:untypedAtomic(5))
for $y in (6, xs:decimal(6), xs:float(6), xs:double(6), xs:untypedAtomic(6))
return typeswitch ($x div $y)
       case xs:integer return "integer"
       case xs:decimal return "decimal"
       case xs:float return "float"
       case xs:double return "double"
       default return error()

