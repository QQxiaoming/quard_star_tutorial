(:*******************************************************:)
(: Test: LetExpr011.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed May 11 13:55:57 2005                        :)
(: Purpose : Use sequences with filter expressions :)
(:*******************************************************:)
let $x :=(1 to 100)[. mod 5 eq 0]
return $x
