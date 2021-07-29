(:*******************************************************:)
(: Test: LetExpr007.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed May 11 13:55:57 2005                        :)
(: Purpose : A node with an expression in curly braces :)
(:*******************************************************:)
let $x:=<a>{1+1}</a>
return $x
