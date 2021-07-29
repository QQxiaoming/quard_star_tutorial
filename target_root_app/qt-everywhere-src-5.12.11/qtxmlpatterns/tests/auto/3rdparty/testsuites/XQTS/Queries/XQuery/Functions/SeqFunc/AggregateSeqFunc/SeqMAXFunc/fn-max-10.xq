(: Name: fn-max-10 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-15 :)
(: Description: Test fn:max on a sequence of numeric arguments :)

for $p in 1 to 4
let $x := (xs:integer(1), xs:decimal(2), xs:float(3), xs:double(4))[position() le $p]
return typeswitch (max($x))
       case xs:integer return "integer"
       case xs:decimal return "decimal"
       case xs:float return "float"
       case xs:double return "double"
       default return error()

