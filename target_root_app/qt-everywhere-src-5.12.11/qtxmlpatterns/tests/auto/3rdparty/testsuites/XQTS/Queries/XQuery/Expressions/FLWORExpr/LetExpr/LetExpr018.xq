(:*******************************************************:)
(: Test: LetExpr018.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed May 11 13:55:57 2005                        :)
(: Purpose : Use a sequence of different values:)
(:*******************************************************:)
let $x := (0,0.1e-1,2.0,'a',"cat",'',true(), ('<a> <b> <c> 123 </c> </b> </a>')/a/b)
return $x
