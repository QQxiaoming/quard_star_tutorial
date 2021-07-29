(:*******************************************************:)
(: Test: LetExpr012.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed May 11 13:55:57 2005                        :)
(: Purpose : Use sequences with filter expressions with more than one variable :)
(:*******************************************************:)
let $x :=(1 to 100)[. mod 5 eq 0], $y := $x[. mod 10 eq 0]
return $y
