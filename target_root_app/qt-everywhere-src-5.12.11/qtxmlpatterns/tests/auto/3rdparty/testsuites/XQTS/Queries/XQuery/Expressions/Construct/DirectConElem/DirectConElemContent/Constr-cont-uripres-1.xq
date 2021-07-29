(: Name: Constr-cont-uripres1 :)
(: Written by: Andreas Behm :)
(: Description: Preserve base uri for copied element nodes :)

for $x in <a xml:base="http://www.example.com/base1"><b/></a>,
    $y in <a xml:base="http://www.example.com/base2">{$x/b}</a>
return fn:base-uri(exactly-one($y/b))
