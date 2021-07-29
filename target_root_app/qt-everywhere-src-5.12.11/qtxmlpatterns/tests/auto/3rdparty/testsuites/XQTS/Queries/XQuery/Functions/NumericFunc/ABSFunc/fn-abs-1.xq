(: Test: fn-abs-1 :)
(: Purpose: check dynamic type of fn:abs on argument of union of numeric types. :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-15 :)

for $x in (1, xs:decimal(2), xs:float(3), xs:double(4))
return typeswitch (abs($x))
       case xs:integer return "integer"
       case xs:decimal return "decimal"
       case xs:float return "float"
       case xs:double return "double"
       default return error()
