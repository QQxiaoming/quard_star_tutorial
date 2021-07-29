(:*******************************************************:)
(: Test: LetExpr017.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed May 11 13:55:57 2005                        :)
(: Purpose : Use a sequence with step :)
(:*******************************************************:)
let $x := (<a> <b> <c> 123 </c> </b> </a>)
return $x
