(:*******************************************************:)
(: Test: LetExpr004.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed May 11 13:55:57 2005                        :)
(: Purpose : Use xs:long(upper bound) :)
(:*******************************************************:)
let $x := xs:long("92233720368547758")
return $x
