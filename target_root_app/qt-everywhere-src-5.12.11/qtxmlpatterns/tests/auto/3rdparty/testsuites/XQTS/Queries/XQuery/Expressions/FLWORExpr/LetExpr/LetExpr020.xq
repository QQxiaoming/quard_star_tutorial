(:*******************************************************:)
(: Test: LetExpr020.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed May 11 13:55:57 2005                        :)
(: Purpose : Use three variables  :)
(:*******************************************************:)
let $a := 1 
let $b := $a
let $c := $a+$b 
return ($c)
