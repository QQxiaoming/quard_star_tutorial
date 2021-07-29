(:*******************************************************:)
(: Test: LetExpr008.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed May 11 13:55:57 2005                        :)
(: Purpose : More than one variable cross referencing compatible values :)
(:*******************************************************:)
let $x:=1, $y:=$x+1
return $x
