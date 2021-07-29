(:*******************************************************:)
(: Test: LetExpr019.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed May 11 13:55:57 2005                        :)
(: Purpose : combine two 'Let' exprs in one 'Return' :)
(:*******************************************************:)
let $a := <elem/>
let $b := <elem2/>
return ($a,$b)
