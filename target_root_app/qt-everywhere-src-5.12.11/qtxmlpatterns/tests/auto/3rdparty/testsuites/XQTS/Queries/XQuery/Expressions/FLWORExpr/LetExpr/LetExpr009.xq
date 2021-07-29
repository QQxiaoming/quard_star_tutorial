(:*******************************************************:)
(: Test: LetExpr009.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed May 11 13:55:57 2005                        :)
(: Purpose : More than one variable cross referencing compatible values :)
(:*******************************************************:)
let $x:=1, $y:=<a>{$x+1}</a>
return $y
