(:*******************************************************:)
(: Test: LetExpr010.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed May 11 13:55:57 2005                        :)
(: Purpose : More than one variable cross referencing incompatible values :)
(:*******************************************************:)
let $x:=(1,2,3), $y:=$x+1
return $y
