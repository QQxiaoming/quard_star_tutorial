(:*******************************************************:)
(: Test: K2-CondExpr-6                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: An if expression cannot directly be an axis step. :)
(:*******************************************************:)
<e/>/if(true()) then 1 else 3