(:*******************************************************:)
(: Test: LetExpr021.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed May 11 13:55:57 2005                        :)
(: Purpose : Refer to an undefined variable  :)
(:*******************************************************:)
let $a := $b 
return ($a)
