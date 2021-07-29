(:*******************************************************:)
(: Test: LetExpr006.xq          :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Wed May 11 13:55:57 2005                        :)
(: Purpose : Use xs:double(upper bound) :)
(:*******************************************************:)
let $x := xs:double("1.7976931348623157E308")
return $x
