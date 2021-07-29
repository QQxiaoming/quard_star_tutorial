(:*******************************************************:)
(: Test: LetExpr015.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed May 11 13:55:57 2005                        :)
(: Purpose : Use a sequence of different types of scalars :)
(:*******************************************************:)
let $x := (0,0.1e-1,2.0,'a',"cat",'',true())
return $x
