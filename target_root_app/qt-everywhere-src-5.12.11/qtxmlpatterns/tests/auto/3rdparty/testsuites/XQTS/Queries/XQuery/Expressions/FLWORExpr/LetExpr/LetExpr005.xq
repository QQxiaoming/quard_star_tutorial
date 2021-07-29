(:*******************************************************:)
(: Test: LetExpr005.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed May 11 13:55:57 2005                        :)
(: Purpose : Use xs:long(lower bound) +1  :)
(:*******************************************************:)
let $x := xs:long("-92233720368547758")+1
return $x
