(:*******************************************************:)
(: Test: LetExpr014.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed May 11 13:55:57 2005                        :)
(: Purpose : Use incompatible types in let :)
(:*******************************************************:)
let $x := "1", $y := $x+1
return $y
