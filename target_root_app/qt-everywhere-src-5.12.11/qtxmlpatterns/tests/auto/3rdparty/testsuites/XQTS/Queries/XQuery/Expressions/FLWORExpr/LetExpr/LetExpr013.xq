(:*******************************************************:)
(: Test: LetExpr013.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed May 11 13:55:57 2005                        :)
(: Purpose : Use a function in the let :)
(:*******************************************************:)
let $x:="hello", $y:=concat($x," there")
return $y
